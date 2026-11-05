// End-to-end check that the tic-tac-toe extension loads, renders its 3D board and responds to
// input. Needs a real (possibly virtual) display with OpenGL: run under xvfb-run on headless Linux.
// Screenshots are written to SCREENSHOT_DIR for inspection.

#include "ExtensionManager.h"

#include <qtplugins/IExtension.h>

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSet>
#include <QTest>
#include <QVBoxLayout>

namespace {

QStringList qmlWarnings;
QtMessageHandler previousHandler = nullptr;

void captureWarnings(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (type >= QtWarningMsg && (message.contains("qml", Qt::CaseInsensitive)
                                 || QByteArray(context.category).startsWith("qt.qml")))
        qmlWarnings.append(message);
    previousHandler(type, context, message);
}

QQuickView *findQuickView()
{
    const QWindowList windows = QGuiApplication::allWindows();
    for (QWindow *window : windows) {
        if (auto *view = qobject_cast<QQuickView *>(window))
            return view;
    }
    return nullptr;
}

int distinctColors(const QImage &image)
{
    QSet<QRgb> colors;
    for (int y = 0; y < image.height(); y += 4) {
        for (int x = 0; x < image.width(); x += 4)
            colors.insert(image.pixel(x, y));
    }
    return colors.size();
}

QList<int> cells(QObject *game)
{
    return game->property("cells").value<QList<int>>();
}

} // namespace

class SmokeTicTacToe : public QObject
{
    Q_OBJECT

private:
    void saveScreenshot(QQuickView *view, const QString &name)
    {
        const QImage image = view->grabWindow();
        QVERIFY(!image.isNull());
        const QString path = QStringLiteral(SCREENSHOT_DIR "/%1.png").arg(name);
        QVERIFY2(image.save(path), qPrintable(path));
        qInfo("saved %s", qPrintable(path));
    }

private slots:
    void initTestCase() { previousHandler = qInstallMessageHandler(captureWarnings); }

    void rendersAndPlays()
    {
        ExtensionManager manager;
        manager.loadFromDirectory(QStringLiteral(EXTENSIONS_DIR));
        const ExtensionManager::Extension *extension = manager.findById("org.qtplugins.tictactoe");
        QVERIFY2(extension, "tictactoe extension not loaded");

        QWidget window;
        window.resize(800, 600);
        auto *layout = new QVBoxLayout(&window);
        layout->setContentsMargins(0, 0, 0, 0);
        QWidget *board = extension->instance->createWidget(&window);
        QVERIFY(board);
        QCOMPARE(board->objectName(), QStringLiteral("ticTacToeBoard"));
        layout->addWidget(board);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QQuickView *view = findQuickView();
        QVERIFY(view);
        QCOMPARE(view->status(), QQuickView::Ready);
        QVERIFY(view->rootObject());
        QTRY_VERIFY(view->isExposed());
        QTest::qWait(800); // let the scene settle

        QObject *game = view->rootContext()->contextProperty("game").value<QObject *>();
        QVERIFY(game);
        game->setProperty("vsComputer", false);

        // The empty board must actually render: more than a flat clear color.
        const QImage empty = view->grabWindow();
        QVERIFY2(distinctColors(empty) > 50, "scene appears blank");
        saveScreenshot(view, "smoke-empty");

        // The camera orbits the board centre, so the middle of the view is always cell 4.
        const QPoint centre(view->width() / 2, view->height() / 2);
        QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, centre);
        QTRY_COMPARE(cells(game).value(4), 1);
        QTest::qWait(500); // drop-in animation
        const QImage afterMove = view->grabWindow();
        QVERIFY2(afterMove != empty, "placing a piece did not change the rendering");
        saveScreenshot(view, "smoke-move");

        // Finish with X winning on the 2-4-6 diagonal.
        for (int index : {0, 2, 1, 6})
            QVERIFY(QMetaObject::invokeMethod(game, "play", Q_ARG(int, index)));
        QVERIFY(game->property("gameOver").toBool());
        QCOMPARE(game->property("winningLine").value<QList<int>>(), (QList<int>{2, 4, 6}));
        QTest::qWait(700);
        saveScreenshot(view, "smoke-win");

        QVERIFY2(qmlWarnings.isEmpty(), qPrintable(qmlWarnings.join('\n')));
    }
};

QTEST_MAIN(SmokeTicTacToe)
#include "smoke_tictactoe.moc"
