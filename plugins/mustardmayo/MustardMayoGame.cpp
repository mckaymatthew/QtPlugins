#include "MustardMayoGame.h"

namespace {
constexpr int EatTick = 50; // ms
}

MustardMayoGame::MustardMayoGame(QObject *parent)
    : QObject(parent)
{
    connect(&m_driving, &DrivingSession::finished, this, [this] {
        if (m_phase == Phase::Drive)
            setPhase(Phase::Destination);
    });

    m_eatTimer.setInterval(EatTick);
    connect(&m_eatTimer, &QTimer::timeout, this, [this] {
        m_eatElapsed += EatTick;
        setEatingProgress(qMin(1.0, qreal(m_eatElapsed) / m_eatDuration));
        if (m_eatingProgress >= 1.0)
            m_eatTimer.stop();
    });
}

QString MustardMayoGame::phaseName() const
{
    switch (m_phase) {
    case Phase::Work:
        return QStringLiteral("work");
    case Phase::Drive:
        return QStringLiteral("drive");
    case Phase::Destination:
        return QStringLiteral("destination");
    case Phase::Home:
        return QStringLiteral("home");
    case Phase::Asleep:
        return QStringLiteral("asleep");
    case Phase::Burger:
        return QStringLiteral("burger");
    }
    return QString();
}

void MustardMayoGame::setEatDuration(int ms)
{
    ms = qMax(EatTick, ms);
    if (m_eatDuration == ms)
        return;
    m_eatDuration = ms;
    emit eatDurationChanged();
}

bool MustardMayoGame::goHome()
{
    if (m_phase != Phase::Work || !m_typing.isUnreadable())
        return false;
    m_driving.reset();
    setPhase(Phase::Drive);
    m_driving.start();
    return true;
}

bool MustardMayoGame::chooseHome()
{
    if (m_phase != Phase::Destination)
        return false;
    setPhase(Phase::Home);
    return true;
}

bool MustardMayoGame::chooseBurger()
{
    if (m_phase != Phase::Destination)
        return false;
    m_eatElapsed = 0;
    setEatingProgress(0);
    setPhase(Phase::Burger);
    return true;
}

bool MustardMayoGame::goToBed()
{
    if (m_phase != Phase::Home)
        return false;
    setPhase(Phase::Asleep);
    return true;
}

bool MustardMayoGame::eat()
{
    if (m_phase != Phase::Burger || m_eatTimer.isActive() || m_eatingProgress > 0)
        return false;
    m_eatElapsed = 0;
    m_eatTimer.start();
    return true;
}

bool MustardMayoGame::backToWork()
{
    if (m_phase != Phase::Burger || m_eatingProgress < 1.0)
        return false;
    ++m_night;
    emit nightChanged();
    m_typing.reset();
    setPhase(Phase::Work);
    return true;
}

void MustardMayoGame::restart()
{
    m_eatTimer.stop();
    m_driving.reset();
    m_typing.reset();
    setEatingProgress(0);
    if (m_night != 1) {
        m_night = 1;
        emit nightChanged();
    }
    setPhase(Phase::Work);
}

void MustardMayoGame::setPhase(Phase phase)
{
    if (m_phase == phase)
        return;
    if (m_phase == Phase::Drive)
        m_driving.stop();
    m_phase = phase;
    emit phaseChanged();
}

void MustardMayoGame::setEatingProgress(qreal progress)
{
    if (qFuzzyCompare(m_eatingProgress + 1, progress + 1))
        return;
    m_eatingProgress = progress;
    emit eatingProgressChanged();
}
