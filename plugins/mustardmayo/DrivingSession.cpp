#include "DrivingSession.h"

#include <QtMath>

namespace {

// Lane units: +-1 is a wheel on a line. With a 2.8 m lane and a 1.8 m car, one unit is 0.5 m.
constexpr qreal SteerRate = 1.5;      // lane units per second at full lock
constexpr qreal CurveDrift = 1.45;    // lane units per second pushed outward at full curvature
constexpr qreal WobbleDrift = 1.2;    // extra drift while vision is blurred
constexpr qreal BlurredSteering = 0.4;
constexpr qreal SteerResponse = 3.0;  // how quickly the wheel follows the keys (1/s)
constexpr qreal OffRoadPenalty = 0.5; // progress lost over a line, relative to progress gained
constexpr qreal MaxOffset = 1.8;      // car is fully across the line
constexpr qreal EpisodeLength = 3.2;  // seconds
constexpr qreal WarningHold = 1.2;    // seconds a warning lingers after returning to the lane
constexpr qreal NominalSpeed = 24.6;  // m/s, 55 mph

// The road winds as a sum of sines over distance (amplitude, wavelength in metres, phase).
struct Bend
{
    qreal amplitude, wavelength, phase;
};
constexpr Bend Bends[] = {{0.45, 400, 0.0}, {0.40, 180, 1.3}, {0.25, 90, 2.1}};
constexpr qreal RoadCurvature = 0.012; // 1/m at curvature 1 (an 83 m radius)

qreal wavenumber(const Bend &b)
{
    return 2 * M_PI / b.wavelength;
}

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

void DrivingSession::setSeed(quint32 seed)
{
    m_random.seed(seed);
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
        return tr("Get out of the mustard!");
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
    m_speedPhase[0] = m_random.bounded(2.0 * M_PI);
    m_speedPhase[1] = m_random.bounded(2.0 * M_PI);
    m_speed = speedAt(0);
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
    const qreal moved = m_speed * dt;
    m_distance += moved;
    m_speed = speedAt(m_time);
    m_curvature = curvatureAt(m_distance);
    const qreal speedFactor = m_speed / NominalSpeed;

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
    m_steeringAngle += (target - m_steeringAngle) * qMin(1.0, dt * SteerResponse);
    const qreal steering = m_steeringAngle * (1 - (1 - BlurredSteering) * m_blur);
    const qreal wobble = m_blur * WobbleDrift * qSin(2.3 * m_time + m_wobblePhase);
    const qreal velocity = steering * SteerRate - m_curvature * CurveDrift * speedFactor + wobble;
    m_lanePosition = qBound(-MaxOffset, m_lanePosition + velocity * dt, MaxOffset);

    const qreal step = moved / (m_duration * NominalSpeed);
    if (qAbs(m_lanePosition) > 1) {
        setWarning(m_lanePosition < 0 ? Mustard : Mayo);
        m_warningHold = WarningHold;
        m_progress = qMax(0.0, m_progress - step * OffRoadPenalty);
    } else {
        m_progress = qMin(1.0, m_progress + step);
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

qreal DrivingSession::curvatureAt(qreal distance)
{
    qreal c = 0;
    for (const Bend &b : Bends)
        c += b.amplitude * qSin(wavenumber(b) * distance + b.phase);
    return c;
}

// The tired driver's foot: speed drifts between roughly 34 and 75 mph.
qreal DrivingSession::speedAt(qreal time) const
{
    return NominalSpeed * (1 + 0.25 * qSin(0.16 * time + m_speedPhase[0])
                             + 0.12 * qSin(0.41 * time + m_speedPhase[1]));
}

// Heading and offset are the first and second integrals of curvature over distance; for sines
// they have closed forms. With a = k * (distance driven) + phase:
//   heading(d) = K * sum A / k * (cos a - cos(a + k d))
//   offset(d)  = K * sum A / k * (d cos a - (sin(a + k d) - sin a) / k)
qreal DrivingSession::roadHeadingAt(qreal metres) const
{
    qreal heading = 0;
    for (const Bend &b : Bends) {
        const qreal k = wavenumber(b);
        const qreal a = k * m_distance + b.phase;
        heading += b.amplitude / k * (qCos(a) - qCos(a + k * metres));
    }
    return RoadCurvature * heading;
}

qreal DrivingSession::roadOffsetAt(qreal metres) const
{
    qreal offset = 0;
    for (const Bend &b : Bends) {
        const qreal k = wavenumber(b);
        const qreal a = k * m_distance + b.phase;
        offset += b.amplitude / k * (metres * qCos(a) - (qSin(a + k * metres) - qSin(a)) / k);
    }
    return RoadCurvature * offset;
}

void DrivingSession::tick()
{
    const qreal dt = qMin(0.1, m_clock.restart() / 1000.0);
    advance(dt);
}

void DrivingSession::scheduleNextEpisode()
{
    m_nextEpisodeIn = 3.0 + m_random.bounded(4.0);
}

void DrivingSession::setWarning(Warning warning)
{
    if (m_warning == warning)
        return;
    m_warning = warning;
    emit warningChanged();
}
