#pragma once

#include "DrivingSession.h"
#include "TypingSession.h"

#include <QObject>

// Ties the phases together:
//
//   work --goHome--> drive --(arrive)--> destination --chooseHome--> home --goToBed--> asleep
//                                                    \--chooseBurger--> burger --eat x N-->
//                                                     (sometimes the aftermath) --backToWork--> work
//
// QML reads `phase` as a string ("work", "drive", ...) to pick the scene to show.
class MustardMayoGame : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString phase READ phaseName NOTIFY phaseChanged)
    Q_PROPERTY(TypingSession *typing READ typing CONSTANT)
    Q_PROPERTY(DrivingSession *driving READ driving CONSTANT)
    Q_PROPERTY(int night READ night NOTIFY nightChanged)
    Q_PROPERTY(int bites READ bites NOTIFY bitesChanged)
    Q_PROPERTY(int biteCount READ biteCount CONSTANT)
    Q_PROPERTY(bool finishedEating READ finishedEating NOTIFY bitesChanged)
    Q_PROPERTY(bool aftermath READ aftermath NOTIFY bitesChanged)
    Q_PROPERTY(qreal aftermathChance READ aftermathChance WRITE setAftermathChance NOTIFY aftermathChanceChanged)

public:
    enum class Phase { Work, Drive, Destination, Home, Asleep, Burger };
    Q_ENUM(Phase)

    explicit MustardMayoGame(QObject *parent = nullptr);

    Phase phase() const { return m_phase; }
    QString phaseName() const;
    TypingSession *typing() { return &m_typing; }
    DrivingSession *driving() { return &m_driving; }
    int night() const { return m_night; }               // 1 + times sent back to work
    // Each "Eat" click is one bite; the burger scene shows one photo per bite.
    int bites() const { return m_bites; }
    static constexpr int biteCount() { return 7; }
    bool finishedEating() const { return m_bites >= biteCount(); }
    // After the last bite, the first "Go back to work" sometimes reveals one more photo instead.
    bool aftermath() const { return m_aftermath; }
    qreal aftermathChance() const { return m_aftermathChance; }
    void setAftermathChance(qreal chance);

    // Each action only applies in the phase it belongs to and returns false otherwise.
    Q_INVOKABLE bool goHome();       // work, once the screen is unreadable
    Q_INVOKABLE bool chooseHome();   // destination
    Q_INVOKABLE bool chooseBurger(); // destination
    Q_INVOKABLE bool goToBed();      // home
    Q_INVOKABLE bool eat();          // burger, until every bite is taken
    Q_INVOKABLE bool backToWork();   // burger, after the last bite; false if it showed the aftermath
    Q_INVOKABLE void restart();

signals:
    void phaseChanged();
    void nightChanged();
    void bitesChanged();
    void aftermathChanceChanged();

private:
    void setPhase(Phase phase);
    void setBites(int bites);

    TypingSession m_typing;
    DrivingSession m_driving;
    Phase m_phase = Phase::Work;
    int m_night = 1;
    int m_bites = 0;
    bool m_aftermath = false;
    bool m_aftermathRolled = false;
    qreal m_aftermathChance = 0.15;
};
