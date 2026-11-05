#include "DrivingSession.h"

#include <QtMath>

namespace {

constexpr qreal SteerRate = 0.9;      // lane units per second at full lock
constexpr qreal CurveDrift = 0.35;    // lane units per second pushed outward at full curvature
constexpr qreal WobbleDrift = 0.45;   // extra drift while vision is blurred
constexpr qreal BlurredSteering = 0.6;
constexpr qreal MaxOffset = 1.8;      // car is fully across the line
constexpr qreal EpisodeLength = 2.6;  // seconds
constexpr qreal WarningHold = 1.2;    // seconds a warning lingers after returning to the lane

} // namespace

DrivingSession::DrivingSession(QObject *parent)
    : QObject(parent)
    , m_random(QRandomGenerator::global()->generate())
{
    m_timer.setInterval(16);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &DrivingSession::tick);
    reset();
}

void DrivingSession::setDuration(qreal seconds)
{
    seconds = qMax(0.01, seconds);
    if (qFuzzyCompare(m_duration, seconds))
        return;
    m_duration = seconds;
    emit durationChanged();
}

QString DrivingSession::warningText() const
{
    switch (m_warning) {
    case Mustard:
        return tr("Get out the mustard!");
    case Mayo:
        return tr("Stay off the mayo!");
    case NoWarning:
        break;
    }
    return QString();
}

void DrivingSession::setLeftHeld(bool held)
{
    m_leftHeld = held;
}

void DrivingSession::setRightHeld(bool held)
{
    m_rightHeld = held;
}

void DrivingSession::start()
{
    if (m_timer.isActive() || m_progress >= 1.0)
        return;
    m_clock.start();
    m_timer.start();
    emit runningChanged();
}

void DrivingSession::stop()
{
    if (!m_timer.isActive())
        return;
    m_timer.stop();
    emit runningChanged();
}

void DrivingSession::reset()
{
    stop();
    m_leftHeld = m_rightHeld = false;
    m_time = 0;
    m_lanePosition = 0;
    m_steeringAngle = 0;
    m_curvature = 0;
    m_distance = 0;
    m_progress = 0;
    m_blur = 0;
    m_episodeTime = -1;
    m_warningHold = 0;
    scheduleNextEpisode();
    setWarning(NoWarning);
    emit updated();
}

void DrivingSession::advance(qreal dt)
{
    if (m_progress >= 1.0 || dt <= 0)
        return;
    m_time += dt;

    // Gently winding road from two incommensurate sines.
    m_curvature = 0.6 * qSin(0.21 * m_time) + 0.4 * qSin(0.53 * m_time + 1.3);

    // Vision blur episodes.
    if (m_episodeTime < 0) {
        m_nextEpisodeIn -= dt;
        if (m_nextEpisodeIn <= 0) {
            m_episodeTime = 0;
            m_wobblePhase = m_random.bounded(2.0 * M_PI);
        }
    } else {
        m_episodeTime += dt;
        if (m_episodeTime >= EpisodeLength) {
            m_episodeTime = -1;
            scheduleNextEpisode();
        }
    }
    m_blur = m_episodeTime < 0 ? 0 : qSin(M_PI * m_episodeTime / EpisodeLength);

    // Steering: the wheel eases towards the held direction; blurred vision weakens it.
    const qreal target = (m_rightHeld ? 1 : 0) - (m_leftHeld ? 1 : 0);
    m_steeringAngle += (target - m_steeringAngle) * qMin(1.0, dt * 6);
    const qreal steering = m_steeringAngle * (1 - (1 - BlurredSteering) * m_blur);
    const qreal wobble = m_blur * WobbleDrift * qSin(2.3 * m_time + m_wobblePhase);
    const qreal velocity = steering * SteerRate - m_curvature * CurveDrift + wobble;
    m_lanePosition = qBound(-MaxOffset, m_lanePosition + velocity * dt, MaxOffset);

    m_distance += speed() * dt;

    if (m_lanePosition < -1) {
        setWarning(Mustard);
        m_warningHold = WarningHold;
    } else if (m_lanePosition > 1) {
        setWarning(Mayo);
        m_warningHold = WarningHold;
    } else {
        m_progress = qMin(1.0, m_progress + dt / m_duration);
        m_warningHold -= dt;
        if (m_warningHold <= 0)
            setWarning(NoWarning);
    }

    emit updated();
    if (m_progress >= 1.0) {
        stop();
        emit finished();
    }
}

void DrivingSession::tick()
{
    const qreal dt = qMin(0.1, m_clock.restart() / 1000.0);
    advance(dt);
}

void DrivingSession::scheduleNextEpisode()
{
    m_nextEpisodeIn = 5.0 + m_random.bounded(5.0);
}

void DrivingSession::setWarning(Warning warning)
{
    if (m_warning == warning)
        return;
    m_warning = warning;
    emit warningChanged();
}
