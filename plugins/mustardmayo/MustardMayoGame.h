#pragma once

#include "DrivingSession.h"
#include "TypingSession.h"

#include <QObject>
#include <QTimer>

// Ties the phases together:
//
//   work --goHome--> drive --(arrive)--> destination --chooseHome--> home --goToBed--> asleep
//                                                    \--chooseBurger--> burger --eat--> (eating)
//                                                                        --backToWork--> work
//
// QML reads `phase` as a string ("work", "drive", ...) to pick the scene to show.
class MustardMayoGame : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString phase READ phaseName NOTIFY phaseChanged)
    Q_PROPERTY(TypingSession *typing READ typing CONSTANT)
    Q_PROPERTY(DrivingSession *driving READ driving CONSTANT)
    Q_PROPERTY(int night READ night NOTIFY nightChanged)
    Q_PROPERTY(qreal eatingProgress READ eatingProgress NOTIFY eatingProgressChanged)
    Q_PROPERTY(int eatDuration READ eatDuration WRITE setEatDuration NOTIFY eatDurationChanged)

public:
    enum class Phase { Work, Drive, Destination, Home, Asleep, Burger };
    Q_ENUM(Phase)

    explicit MustardMayoGame(QObject *parent = nullptr);

    Phase phase() const { return m_phase; }
    QString phaseName() const;
    TypingSession *typing() { return &m_typing; }
    DrivingSession *driving() { return &m_driving; }
    int night() const { return m_night; }               // 1 + times sent back to work
    qreal eatingProgress() const { return m_eatingProgress; }
    int eatDuration() const { return m_eatDuration; }   // milliseconds
    void setEatDuration(int ms);

    // Each action only applies in the phase it belongs to and returns false otherwise.
    Q_INVOKABLE bool goHome();       // work, once the screen is unreadable
    Q_INVOKABLE bool chooseHome();   // destination
    Q_INVOKABLE bool chooseBurger(); // destination
    Q_INVOKABLE bool goToBed();      // home
    Q_INVOKABLE bool eat();          // burger, not yet eaten
    Q_INVOKABLE bool backToWork();   // burger, finished eating
    Q_INVOKABLE void restart();

signals:
    void phaseChanged();
    void nightChanged();
    void eatingProgressChanged();
    void eatDurationChanged();

private:
    void setPhase(Phase phase);
    void setEatingProgress(qreal progress);

    TypingSession m_typing;
    DrivingSession m_driving;
    QTimer m_eatTimer;
    Phase m_phase = Phase::Work;
    int m_night = 1;
    qreal m_eatingProgress = 0;
    int m_eatDuration = 5000;
    int m_eatElapsed = 0;
};
