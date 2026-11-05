#include "MustardMayoGame.h"

#include <QRandomGenerator>

MustardMayoGame::MustardMayoGame(QObject *parent)
    : QObject(parent)
{
    connect(&m_driving, &DrivingSession::finished, this, [this] {
        if (m_phase == Phase::Drive)
            setPhase(Phase::Destination);
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
    setBites(0);
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
    if (m_phase != Phase::Burger || finishedEating())
        return false;
    setBites(m_bites + 1);
    return true;
}

bool MustardMayoGame::backToWork()
{
    if (m_phase != Phase::Burger || !finishedEating())
        return false;
    if (!m_aftermathRolled) {
        m_aftermathRolled = true;
        if (QRandomGenerator::global()->generateDouble() < m_aftermathChance) {
            m_aftermath = true;
            emit bitesChanged();
            return false;
        }
    }
    ++m_night;
    emit nightChanged();
    m_typing.reset();
    setPhase(Phase::Work);
    return true;
}

void MustardMayoGame::restart()
{
    m_driving.reset();
    m_typing.reset();
    setBites(0);
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

void MustardMayoGame::setAftermathChance(qreal chance)
{
    chance = qBound(0.0, chance, 1.0);
    if (qFuzzyCompare(m_aftermathChance + 1, chance + 1))
        return;
    m_aftermathChance = chance;
    emit aftermathChanceChanged();
}

void MustardMayoGame::setBites(int bites)
{
    if (bites == 0) {
        m_aftermathRolled = false;
        m_aftermath = false;
    }
    if (m_bites == bites)
        return;
    m_bites = bites;
    emit bitesChanged();
}
