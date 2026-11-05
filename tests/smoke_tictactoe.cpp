// End-to-end check that the tic-tac-toe extension loads, renders its 3D board and responds to
// input. Needs a real (possibly virtual) display with OpenGL: run under xvfb-run on headless Linux.
// Screenshots are written to SCREENSHOT_DIR for inspection.

#include "ExtensionManager.h"
#include "SmokeUtils.h"

#include <qtplugins/IExtension.h>

#include <QQmlContext>
#include <QQuickItem>
#include <QVBoxLayout>

using namespace smoke;

namespace {

QList<int> cells(QObject *game)
{
    return game->property("cells").value<QList<int>>();
}

} // namespace

class SmokeTicTacToe : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { captureQmlWarnings(); }

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
        screenshot(view, "smoke-empty");

        // The camera orbits the board centre, so the middle of the view is always cell 4.
        const QPoint centre(view->width() / 2, view->height() / 2);
        QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, centre);
        QTRY_COMPARE(cells(game).value(4), 1);
        QTest::qWait(500); // drop-in animation
        const QImage afterMove = view->grabWindow();
        QVERIFY2(afterMove != empty, "placing a piece did not change the rendering");
        screenshot(view, "smoke-move");

        // Finish with X winning on the 2-4-6 diagonal.
        for (int index : {0, 2, 1, 6})
            QVERIFY(QMetaObject::invokeMethod(game, "play", Q_ARG(int, index)));
        QVERIFY(game->property("gameOver").toBool());
        QCOMPARE(game->property("winningLine").value<QList<int>>(), (QList<int>{2, 4, 6}));
        QTest::qWait(700);
        screenshot(view, "smoke-win");

        QVERIFY2(qmlWarnings().isEmpty(), qPrintable(qmlWarnings().join('\n')));
    }
};

QTEST_MAIN(SmokeTicTacToe)
#include "smoke_tictactoe.moc"
