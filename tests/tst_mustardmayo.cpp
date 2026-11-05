#include "DrivingSession.h"
#include "MustardMayoGame.h"
#include "TypingSession.h"

#include <QSignalSpy>
#include <QTest>

namespace {

// Types the next expected character, the way a key press would deliver it.
bool typeNext(TypingSession &typing)
{
    const QChar c = typing.code().at(typing.typedLength());
    return typing.typeText(c == u'\n' ? QStringLiteral("\r") : QString(c));
}

void typeUntilUnreadable(TypingSession &typing)
{
    while (!typing.isUnreadable())
        QVERIFY(typeNext(typing));
}

void drive(DrivingSession &driving, qreal seconds, qreal dt = 1.0 / 60)
{
    for (qreal t = 0; t < seconds; t += dt)
        driving.advance(dt);
}

} // namespace

class TestMustardMayo : public QObject
{
    Q_OBJECT

private slots:
    // --- TypingSession -------------------------------------------------------------------

    void typingStartsFresh()
    {
        TypingSession typing;
        QVERIFY(typing.code().contains("preflight"));
        QCOMPARE(typing.typedLength(), 0);
        QCOMPARE(typing.fatigue(), 0.0);
        QVERIFY(!typing.isUnreadable());
    }

    void typingCorrectCharactersAdvance()
    {
        TypingSession typing;
        for (const QString &c : {"/", "/", " ", "f"})
            QVERIFY(typing.typeText(c));
        QCOMPARE(typing.typedLength(), 4);
        QCOMPARE(typing.typos(), 0);
        QCOMPARE(typing.keystrokes(), 4);
        QVERIFY(typing.fatigue() > 0);
    }

    void typingWrongCharacterIsATypo()
    {
        TypingSession typing;
        QSignalSpy typoSpy(&typing, &TypingSession::typo);
        QVERIFY(!typing.typeText("x"));
        QCOMPARE(typing.typedLength(), 0);
        QCOMPARE(typing.typos(), 1);
        QCOMPARE(typing.keystrokes(), 1); // still tiring
        QCOMPARE(typoSpy.count(), 1);
    }

    void typingIgnoresControlKeys()
    {
        TypingSession typing;
        QVERIFY(!typing.typeText(QString()));
        QVERIFY(!typing.typeText("\b"));
        QVERIFY(!typing.typeText("\x1b"));
        QCOMPARE(typing.keystrokes(), 0);
    }

    void typingEnterAndAutoIndent()
    {
        TypingSession typing;
        const QString code = typing.code();
        const int secondLine = code.indexOf('\n') + 1;
        while (typing.typedLength() < secondLine)
            QVERIFY(typeNext(typing));
        QCOMPARE(code.mid(typing.typedLength(), 4), QStringLiteral("bool"));

        const int thirdLine = code.indexOf('\n', secondLine) + 1;
        while (typing.typedLength() < thirdLine)
            QVERIFY(typeNext(typing));
        // The four spaces of indentation were filled in.
        QCOMPARE(code.mid(typing.typedLength(), 2), QStringLiteral("if"));
    }

    void typingBecomesUnreadableBeforeTheEnd()
    {
        TypingSession typing;
        qreal last = 0;
        while (!typing.isUnreadable()) {
            QVERIFY(typeNext(typing));
            QVERIFY(typing.fatigue() >= last);
            last = typing.fatigue();
        }
        QCOMPARE(typing.fatigue(), 1.0);
        QVERIFY(typing.typedLength() < typing.code().size());
    }

    void typingMashingKeysAlsoTires()
    {
        TypingSession typing;
        for (int i = 0; i < 500 && !typing.isUnreadable(); ++i)
            typing.typeText("q");
        QVERIFY(typing.isUnreadable());
        QCOMPARE(typing.typedLength(), 0);
    }

    void typingReset()
    {
        TypingSession typing;
        typeUntilUnreadable(typing);
        typing.reset();
        QCOMPARE(typing.typedLength(), 0);
        QCOMPARE(typing.keystrokes(), 0);
        QVERIFY(!typing.isUnreadable());
    }

    // --- DrivingSession ------------------------------------------------------------------

    void drivingStartsCentred()
    {
        DrivingSession driving;
        QCOMPARE(driving.lanePosition(), 0.0);
        QCOMPARE(driving.progress(), 0.0);
        QCOMPARE(driving.warning(), DrivingSession::NoWarning);
        QVERIFY(!driving.isRunning());
    }

    void drivingSteeringRightHitsTheMayo()
    {
        DrivingSession driving;
        driving.setSeed(1);
        driving.setRightHeld(true);
        drive(driving, 4);
        QVERIFY(driving.lanePosition() > 1);
        QVERIFY(driving.lanePosition() <= 1.8);
        QCOMPARE(driving.warning(), DrivingSession::Mayo);
        QVERIFY(driving.warningText().contains("mayo"));
        QVERIFY(driving.steeringAngle() > 0.9);
    }

    void drivingSteeringLeftHitsTheMustard()
    {
        DrivingSession driving;
        driving.setSeed(1);
        driving.setLeftHeld(true);
        drive(driving, 4);
        QVERIFY(driving.lanePosition() < -1);
        QCOMPARE(driving.warning(), DrivingSession::Mustard);
        QVERIFY(driving.warningText().contains("mustard"));
    }

    void drivingNoProgressOutsideTheLane()
    {
        DrivingSession driving;
        driving.setSeed(1);
        driving.setRightHeld(true);
        drive(driving, 3);
        QVERIFY(driving.lanePosition() > 1);
        const qreal progress = driving.progress();
        const qreal distance = driving.distance();
        drive(driving, 1);
        QCOMPARE(driving.progress(), progress);
        QVERIFY(driving.distance() > distance); // the car still moves
    }

    void drivingWarningLingersThenClears()
    {
        DrivingSession driving;
        driving.setSeed(1);
        driving.setRightHeld(true);
        drive(driving, 3);
        QCOMPARE(driving.warning(), DrivingSession::Mayo);

        driving.setRightHeld(false);
        driving.setLeftHeld(true);
        while (driving.lanePosition() > 0.5)
            driving.advance(1.0 / 60);
        driving.setLeftHeld(false);
        QCOMPARE(driving.warning(), DrivingSession::Mayo); // lingers briefly
        drive(driving, 1.5);
        QCOMPARE(driving.warning(), DrivingSession::NoWarning);
    }

    void drivingVisionBlurs()
    {
        DrivingSession driving;
        driving.setSeed(7);
        qreal maxBlur = 0;
        for (int i = 0; i < 60 * 12; ++i) {
            driving.advance(1.0 / 60);
            QVERIFY(driving.blur() >= 0 && driving.blur() <= 1);
            maxBlur = qMax(maxBlur, driving.blur());
        }
        QVERIFY2(maxBlur > 0.9, "no blur episode within 12 s");
    }

    void drivingFinishes()
    {
        DrivingSession driving;
        QSignalSpy finished(&driving, &DrivingSession::finished);
        driving.setDuration(1);
        drive(driving, 1.2);
        QCOMPARE(driving.progress(), 1.0);
        QCOMPARE(finished.count(), 1);
        const qreal distance = driving.distance();
        driving.advance(0.5);
        QCOMPARE(driving.distance(), distance); // stopped
    }

    void drivingRunsOnItsTimer()
    {
        DrivingSession driving;
        driving.start();
        QVERIFY(driving.isRunning());
        QTRY_VERIFY(driving.distance() > 2);
        driving.stop();
        QVERIFY(!driving.isRunning());
        driving.reset();
        QCOMPARE(driving.distance(), 0.0);
    }

    // --- MustardMayoGame -----------------------------------------------------------------

    void gameGoHomeNeedsUnreadableScreen()
    {
        MustardMayoGame game;
        QCOMPARE(game.phaseName(), QStringLiteral("work"));
        QVERIFY(!game.goHome());
        typeUntilUnreadable(*game.typing());
        QVERIFY(game.goHome());
        QCOMPARE(game.phaseName(), QStringLiteral("drive"));
        QVERIFY(game.driving()->isRunning());
    }

    void gameActionsOnlyInTheirPhase()
    {
        MustardMayoGame game;
        QVERIFY(!game.chooseHome());
        QVERIFY(!game.chooseBurger());
        QVERIFY(!game.goToBed());
        QVERIFY(!game.eat());
        QVERIFY(!game.backToWork());
        QCOMPARE(game.phaseName(), QStringLiteral("work"));
    }

    void gameHomePath()
    {
        MustardMayoGame game;
        QSignalSpy phases(&game, &MustardMayoGame::phaseChanged);
        typeUntilUnreadable(*game.typing());
        QVERIFY(game.goHome());
        game.driving()->stop();
        game.driving()->setDuration(0.5);
        drive(*game.driving(), 1);
        QCOMPARE(game.phaseName(), QStringLiteral("destination"));
        QVERIFY(!game.driving()->isRunning());

        QVERIFY(game.chooseHome());
        QCOMPARE(game.phaseName(), QStringLiteral("home"));
        QVERIFY(game.goToBed());
        QCOMPARE(game.phaseName(), QStringLiteral("asleep"));
        QCOMPARE(phases.count(), 4);

        game.restart();
        QCOMPARE(game.phaseName(), QStringLiteral("work"));
        QCOMPARE(game.typing()->keystrokes(), 0);
    }

    void gameBurgerPathLoopsBackToWork()
    {
        MustardMayoGame game;
        game.setEatDuration(100);
        typeUntilUnreadable(*game.typing());
        QVERIFY(game.goHome());
        game.driving()->stop();
        game.driving()->setDuration(0.5);
        drive(*game.driving(), 1);
        QVERIFY(game.chooseBurger());
        QCOMPARE(game.phaseName(), QStringLiteral("burger"));

        QVERIFY(!game.backToWork()); // haven't eaten
        QVERIFY(game.eat());
        QVERIFY(!game.eat());
        QTRY_COMPARE(game.eatingProgress(), 1.0);
        QVERIFY(game.backToWork());
        QCOMPARE(game.phaseName(), QStringLiteral("work"));
        QCOMPARE(game.night(), 2);
        QVERIFY(!game.typing()->isUnreadable());

        game.restart();
        QCOMPARE(game.night(), 1);
    }
};

QTEST_GUILESS_MAIN(TestMustardMayo)
#include "tst_mustardmayo.moc"
