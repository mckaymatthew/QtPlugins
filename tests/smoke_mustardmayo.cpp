// End-to-end check of the Mustard and Mayonnaise extension: plays through every phase, using real
// key events where the player would, and saves a screenshot of each scene to SCREENSHOT_DIR.
// Needs a display with OpenGL: run under xvfb-run on headless Linux.

#include "ExtensionManager.h"
#include "SmokeUtils.h"

#include <qtplugins/IExtension.h>

#include <QQmlContext>
#include <QQuickItem>
#include <QVBoxLayout>

using namespace smoke;

class SmokeMustardMayo : public QObject
{
    Q_OBJECT

    ExtensionManager m_manager;
    QWidget m_window;
    QQuickView *m_view = nullptr;
    QObject *m_game = nullptr;
    QObject *m_typing = nullptr;
    QObject *m_driving = nullptr;

    QString phase() const { return m_game->property("phase").toString(); }
    qreal lanePosition() const { return m_driving->property("lanePosition").toReal(); }

    void tire()
    {
        while (!m_typing->property("unreadable").toBool())
            QMetaObject::invokeMethod(m_typing, "typeText", Q_ARG(QString, "x"));
    }

    void invoke(const char *method)
    {
        bool ok = false;
        QVERIFY(QMetaObject::invokeMethod(m_game, method, Q_RETURN_ARG(bool, ok)));
        QVERIFY2(ok, method);
    }

    // Shortens the drive so it completes quickly, then waits for the destination scene.
    void finishDrive()
    {
        m_driving->setProperty("duration", 0.3);
        QTRY_COMPARE_WITH_TIMEOUT(phase(), QStringLiteral("destination"), 5000);
    }

    void typeKeys(const char *text)
    {
        for (const char *c = text; *c; ++c)
            QTest::keyClick(m_view, *c);
    }

    void settle(int ms = 1000) { QTest::qWait(ms); } // scene fade-in + animations

    void expectRendered(const QString &name)
    {
        const QImage image = screenshot(m_view, name);
        QVERIFY(!image.isNull());
        QVERIFY2(distinctColors(image) > 40, qPrintable(name + " appears blank"));
    }

private slots:
    void initTestCase()
    {
        captureQmlWarnings();
        m_manager.loadFromDirectory(QStringLiteral(EXTENSIONS_DIR));
        const ExtensionManager::Extension *extension = m_manager.findById("org.qtplugins.mustardmayo");
        QVERIFY2(extension, "mustardmayo extension not loaded");

        m_window.resize(960, 600);
        auto *layout = new QVBoxLayout(&m_window);
        layout->setContentsMargins(0, 0, 0, 0);
        QWidget *widget = extension->instance->createWidget(&m_window);
        QCOMPARE(widget->objectName(), QStringLiteral("mustardMayoGame"));
        layout->addWidget(widget);
        m_window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&m_window));

        m_view = findQuickView();
        QVERIFY(m_view);
        QCOMPARE(m_view->status(), QQuickView::Ready);
        QTRY_VERIFY(m_view->isExposed());
        m_view->requestActivate();
        QTRY_VERIFY(m_view->isActive());

        m_game = m_view->rootContext()->contextProperty("game").value<QObject *>();
        QVERIFY(m_game);
        m_typing = m_game->property("typing").value<QObject *>();
        m_driving = m_game->property("driving").value<QObject *>();
        QVERIFY(m_typing && m_driving);
    }

    void work()
    {
        QCOMPARE(phase(), QStringLiteral("work"));
        settle();
        typeKeys("// flight");
        QCOMPARE(m_typing->property("typedLength").toInt(), 9);
        typeKeys("Q"); // typo
        QCOMPARE(m_typing->property("typos").toInt(), 1);
        settle(300);
        expectRendered("mm-work");

        tire();
        settle(300);
        expectRendered("mm-work-unreadable");
        invoke("goHome");
        QCOMPARE(phase(), QStringLiteral("drive"));
    }

    void drive()
    {
        settle();
        expectRendered("mm-drive");

        // Hold right until the car crosses the white line.
        QTest::keyPress(m_view, Qt::Key_Right);
        QTRY_VERIFY_WITH_TIMEOUT(lanePosition() > 1.05, 6000);
        QTest::qWait(400);
        QCOMPARE(m_driving->property("warningText").toString(), QStringLiteral("Stay off the mayo!"));
        expectRendered("mm-drive-mayo");
        QTest::keyRelease(m_view, Qt::Key_Right);

        // And left across the yellow line.
        QTest::keyPress(m_view, Qt::Key_Left);
        QTRY_VERIFY_WITH_TIMEOUT(lanePosition() < -1.05, 8000);
        QTest::qWait(300);
        QCOMPARE(m_driving->property("warningText").toString(), QStringLiteral("Get out the mustard!"));
        expectRendered("mm-drive-mustard");
        QTest::keyRelease(m_view, Qt::Key_Left);

        // Back into the lane and finish.
        QTest::keyPress(m_view, Qt::Key_Right);
        QTRY_VERIFY_WITH_TIMEOUT(lanePosition() > -0.3, 6000);
        QTest::keyRelease(m_view, Qt::Key_Right);
        finishDrive();
    }

    void burger()
    {
        settle();
        expectRendered("mm-destination");
        QTest::keyClick(m_view, Qt::Key_Right); // the What-A-Burger exit
        QCOMPARE(phase(), QStringLiteral("burger"));
        settle();
        expectRendered("mm-burger");

        m_game->setProperty("eatDuration", 2500);
        invoke("eat");
        QTest::qWait(1200); // close-up fades in and the burger grows into view
        expectRendered("mm-burger-eating");
        QTRY_COMPARE(m_game->property("eatingProgress").toReal(), 1.0);
        invoke("backToWork");
        QCOMPARE(phase(), QStringLiteral("work"));
        QCOMPARE(m_game->property("night").toInt(), 2);
    }

    void home()
    {
        settle(300);
        tire();
        invoke("goHome");
        finishDrive();
        settle(300);
        QTest::keyClick(m_view, Qt::Key_Left); // home
        QCOMPARE(phase(), QStringLiteral("home"));
        settle();
        expectRendered("mm-home");

        invoke("goToBed");
        QCOMPARE(phase(), QStringLiteral("asleep"));
        settle();
        expectRendered("mm-asleep");

        QTest::keyClick(m_view, Qt::Key_Return); // play again
        QCOMPARE(phase(), QStringLiteral("work"));
        QCOMPARE(m_game->property("night").toInt(), 1);
    }

    void noQmlWarnings()
    {
        QVERIFY2(qmlWarnings().isEmpty(), qPrintable(qmlWarnings().join('\n')));
    }
};

QTEST_MAIN(SmokeMustardMayo)
#include "smoke_mustardmayo.moc"
