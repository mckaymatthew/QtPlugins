#include "DrivingSession.h"
#include "MustardMayoGame.h"
#include "TypingSession.h"

#include <QSignalSpy>
#include <QTest>

namespace {

// Types the next expected character, the way a key press would deliver it, first dismissing any
// email with Enter.
bool typeNext(TypingSession &typing)
{
    if (typing.isEmailOpen())
        typing.typeText(QStringLiteral("\r"));
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
        for (const QString &c : {"b", "o", "o", "l"})
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
        // The four spaces of indentation were filled in.
        QCOMPARE(code.mid(typing.typedLength(), 6), QStringLiteral("return"));
    }

    void typingEmailsEndTheNightEarly()
    {
        TypingSession typing;
        QSignalSpy emails(&typing, &TypingSession::emailChanged);
        qreal last = 0;
        while (!typing.isUnreadable()) {
            QVERIFY(typeNext(typing));
            QVERIFY(typing.fatigue() >= last);
            last = typing.fatigue();
        }
        // A clean typist gets three emails, and the last one finishes them off mid-code.
        QCOMPARE(typing.emailCount(), 3);
        QVERIFY(typing.isEmailOpen());
        QVERIFY(typing.typedLength() < typing.code().size());
        QCOMPARE(typing.fatigue(), 1.0);
    }

    void typingEmailArrivesAndCostsStamina()
    {
        TypingSession typing;
        QSignalSpy emails(&typing, &TypingSession::emailChanged);
        while (typing.keystrokes() < TypingSession::FirstEmailAt - 1)
            QVERIFY(typeNext(typing));
        QVERIFY(!typing.isEmailOpen());
        const qreal before = typing.fatigue();

        QVERIFY(typeNext(typing));
        QVERIFY(typing.isEmailOpen());
        QCOMPARE(emails.count(), 1);
        QVERIFY(TypingSession::emailSubjects().contains(typing.emailSubject()));
        QVERIFY(!typing.emailBody().isEmpty());
        // One keystroke of fatigue plus a fifth of the night's stamina.
        QVERIFY2(typing.fatigue() - before > TypingSession::EmailCost,
                 qPrintable(QStringLiteral("%1 -> %2").arg(before).arg(typing.fatigue())));
    }

    void typingEmailBlocksUntilEnter()
    {
        TypingSession typing;
        while (!typing.isEmailOpen())
            QVERIFY(typeNext(typing));
        const int typed = typing.typedLength();
        const int keystrokes = typing.keystrokes();
        const QChar next = typing.code().at(typed);

        // Even the right key does nothing, and doesn't tire you, while the email is up.
        QVERIFY(!typing.typeText(QString(next)));
        QVERIFY(!typing.typeText("x"));
        QCOMPARE(typing.typedLength(), typed);
        QCOMPARE(typing.keystrokes(), keystrokes);
        QCOMPARE(typing.typos(), 0);
        QVERIFY(typing.isEmailOpen());

        // Enter dismisses it without typing a newline.
        QVERIFY(!typing.typeText("\r"));
        QVERIFY(!typing.isEmailOpen());
        QCOMPARE(typing.typedLength(), typed);
        QCOMPARE(typing.keystrokes(), keystrokes);
        QVERIFY(typing.typeText(QString(next)));
    }

    void typingEmailSubjects()
    {
        const QStringList subjects = TypingSession::emailSubjects();
        for (const char *subject : {"IMS Replan", "IMS Replan Wall Walk", "BCN Notification",
                                    "Urgent need 2028 BOE completed ASAP"})
            QVERIFY2(subjects.contains(QLatin1String(subject)), subject);

        // The emails in a night are all different.
        TypingSession typing;
        QStringList seen;
        while (!typing.isUnreadable()) {
            if (typing.isEmailOpen() && !seen.contains(typing.emailSubject()))
                seen.append(typing.emailSubject());
            QVERIFY(typeNext(typing));
        }
        if (typing.isEmailOpen() && !seen.contains(typing.emailSubject()))
            seen.append(typing.emailSubject());
        QCOMPARE(seen.size(), typing.emailCount());
    }

    void typingStaysFreshUntilTheFirstEmail()
    {
        TypingSession typing;
        while (!typing.isEmailOpen())
            QVERIFY(typeNext(typing));
        QVERIFY2(typing.fatigue() < 0.4, qPrintable(QString::number(typing.fatigue())));
    }

    void typingMashingKeysAlsoTires()
    {
        TypingSession typing;
        for (int i = 0; i < 500 && !typing.isUnreadable(); ++i)
            typing.typeText(typing.isEmailOpen() ? "\r" : "q");
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
        QCOMPARE(typing.fatigue(), 0.0);
        QCOMPARE(typing.emailCount(), 0);
        QVERIFY(!typing.isEmailOpen());
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

    void drivingLosesProgressOutsideTheLane()
    {
        DrivingSession driving;
        driving.setSeed(1);
        driving.setRightHeld(true);
        drive(driving, 3);
        QVERIFY(driving.lanePosition() > 1);
        const qreal progress = driving.progress();
        const qreal distance = driving.distance();
        drive(driving, 1);
        QVERIFY(driving.lanePosition() > 1);
        QVERIFY(driving.progress() < progress);
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
        driving.setSeed(1);
        QSignalSpy finished(&driving, &DrivingSession::finished);
        driving.setDuration(0.5); // under 0.8 s even at the slowest speed, too short to drift out
        drive(driving, 1.2);
        QCOMPARE(driving.progress(), 1.0);
        QCOMPARE(finished.count(), 1);
        const qreal distance = driving.distance();
        driving.advance(0.5);
        QCOMPARE(driving.distance(), distance); // stopped
    }

    void drivingRoadAheadMatchesTheDrive()
    {
        // The road drawn ahead must be the road driven: d(offset)/dd = heading, and
        // d(heading)/dd = the curvature the car meets after driving that far.
        constexpr qreal RoadCurvature = 0.012; // DrivingSession.cpp
        DrivingSession now;
        now.setSeed(1);
        drive(now, 3);
        QCOMPARE(now.roadOffsetAt(0), 0.0);
        QCOMPARE(now.roadHeadingAt(0), 0.0);

        for (qreal ahead : {20.0, 75.0, 150.0}) {
            const qreal h = 0.5;
            const qreal slope = (now.roadOffsetAt(ahead + h) - now.roadOffsetAt(ahead - h)) / (2 * h);
            QVERIFY(qAbs(slope - now.roadHeadingAt(ahead)) < 1e-4);

            DrivingSession later; // same drive as `now`, then exactly `ahead` metres further
            later.setSeed(1);
            drive(later, 3);
            later.advance(ahead / later.speed());
            const qreal turn = (now.roadHeadingAt(ahead + h) - now.roadHeadingAt(ahead - h)) / (2 * h);
            QVERIFY2(qAbs(turn - RoadCurvature * later.curvature()) < 1e-6,
                     qPrintable(QStringLiteral("%1 m: %2 vs %3").arg(ahead).arg(turn)
                                    .arg(RoadCurvature * later.curvature())));
        }
    }

    void drivingSpeedVaries()
    {
        DrivingSession driving;
        driving.setSeed(3);
        qreal slowest = 1e9, fastest = 0;
        for (int i = 0; i < 60 * 60; ++i) {
            driving.advance(1.0 / 60);
            slowest = qMin(slowest, driving.speed());
            fastest = qMax(fastest, driving.speed());
        }
        QVERIFY2(slowest > 15 && fastest < 34, qPrintable(QStringLiteral("%1..%2").arg(slowest).arg(fastest)));
        QVERIFY2(fastest - slowest > 8, "speed hardly changes");
    }

    void drivingIsBeatable()
    {
        // A simple driver that corrects whenever it drifts off centre must get home without
        // spending long over the lines, whatever the blur episodes do.
        for (quint32 seed : {1u, 2u, 3u, 4u, 5u}) {
            DrivingSession driving;
            driving.setSeed(seed);
            const qreal dt = 1.0 / 60;
            qreal time = 0, outside = 0;
            while (driving.progress() < 1 && time < 120) {
                const qreal x = driving.lanePosition();
                driving.setLeftHeld(x > 0.25);
                driving.setRightHeld(x < -0.25);
                driving.advance(dt);
                time += dt;
                if (qAbs(driving.lanePosition()) > 1)
                    outside += dt;
            }
            qInfo("seed %u: home in %.1f s, %.1f s over a line", seed, time, outside);
            QCOMPARE(driving.progress(), 1.0);
            QVERIFY(outside < 0.15 * time);
        }
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
        game.setAftermathChance(0);
        QSignalSpy bites(&game, &MustardMayoGame::bitesChanged);
        typeUntilUnreadable(*game.typing());
        QVERIFY(game.goHome());
        game.driving()->stop();
        game.driving()->setDuration(0.5);
        drive(*game.driving(), 1);
        QVERIFY(game.chooseBurger());
        QCOMPARE(game.phaseName(), QStringLiteral("burger"));

        // One click per bite; work has to wait until the last one.
        for (int bite = 1; bite <= MustardMayoGame::biteCount(); ++bite) {
            QVERIFY(!game.backToWork());
            QVERIFY(game.eat());
            QCOMPARE(game.bites(), bite);
        }
        QVERIFY(game.finishedEating());
        QVERIFY(!game.eat()); // nothing left
        QCOMPARE(bites.count(), MustardMayoGame::biteCount());
        QVERIFY(game.backToWork());
        QCOMPARE(game.phaseName(), QStringLiteral("work"));
        QCOMPARE(game.night(), 2);
        QVERIFY(!game.typing()->isUnreadable());

        game.restart();
        QCOMPARE(game.night(), 1);
        QCOMPARE(game.bites(), 0);
    }

    void gameAftermathDefaultChance()
    {
        QCOMPARE(MustardMayoGame().aftermathChance(), 0.25);
    }

    void gameAftermathShowsBeforeLeaving()
    {
        MustardMayoGame game;
        game.setAftermathChance(1);
        for (int visit = 0; visit < 2; ++visit) {
            typeUntilUnreadable(*game.typing());
            QVERIFY(game.goHome());
            game.driving()->stop();
            game.driving()->setDuration(0.5);
            drive(*game.driving(), 1);
            QVERIFY(game.chooseBurger());
            QVERIFY(!game.aftermath()); // fresh each visit
            while (game.eat()) {}

            QVERIFY(!game.backToWork()); // first click reveals the aftermath instead
            QVERIFY(game.aftermath());
            QCOMPARE(game.phaseName(), QStringLiteral("burger"));
            QVERIFY(game.backToWork());
            QCOMPARE(game.phaseName(), QStringLiteral("work"));
        }
    }
};

QTEST_GUILESS_MAIN(TestMustardMayo)
#include "tst_mustardmayo.moc"
