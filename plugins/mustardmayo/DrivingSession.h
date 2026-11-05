#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QRandomGenerator>
#include <QString>
#include <QTimer>

// Phase 2, the drive home. The car's lateral position in its lane is `lanePosition`: 0 is the
// lane centre, -1 puts the left wheels on the yellow centre line and +1 the right wheels on the
// white edge line. Road curvature pushes the car outward (harder the faster it goes), the player
// counters with left/right, the speed wanders, and every few seconds the driver's vision blurs and
// steering gets unreliable. Progress home only accrues while the car is between the lines.
class DrivingSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal lanePosition READ lanePosition NOTIFY updated)
    Q_PROPERTY(qreal steeringAngle READ steeringAngle NOTIFY updated)
    Q_PROPERTY(qreal curvature READ curvature NOTIFY updated)
    Q_PROPERTY(qreal distance READ distance NOTIFY updated)
    Q_PROPERTY(qreal progress READ progress NOTIFY updated)
    Q_PROPERTY(qreal blur READ blur NOTIFY updated)
    Q_PROPERTY(qreal speed READ speed NOTIFY updated)
    Q_PROPERTY(qreal duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(Warning warning READ warning NOTIFY warningChanged)
    Q_PROPERTY(QString warningText READ warningText NOTIFY warningChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

public:
    enum Warning { NoWarning, Mustard, Mayo };
    Q_ENUM(Warning)

    explicit DrivingSession(QObject *parent = nullptr);

    qreal lanePosition() const { return m_lanePosition; }
    qreal steeringAngle() const { return m_steeringAngle; } // smoothed -1 (left) .. 1 (right)
    qreal curvature() const { return m_curvature; }         // -1 .. 1, positive bends right
    qreal distance() const { return m_distance; }           // metres driven
    qreal progress() const { return m_progress; }           // 0 .. 1
    qreal blur() const { return m_blur; }                   // 0 .. 1
    qreal speed() const { return m_speed; }                 // m/s, wanders around 55 mph
    qreal duration() const { return m_duration; }           // seconds in lane at 55 mph to get home
    void setDuration(qreal seconds);
    Warning warning() const { return m_warning; }
    QString warningText() const;
    bool isRunning() const { return m_timer.isActive(); }

    Q_INVOKABLE void setLeftHeld(bool held);
    Q_INVOKABLE void setRightHeld(bool held);

    // Shape of the road `metres` ahead of the car, relative to the road at the car: lateral
    // offset of the lane centre in metres (positive = right) and heading in radians. The bends
    // drawn ahead are exactly the curvature the car will drift through when it gets there.
    Q_INVOKABLE qreal roadOffsetAt(qreal metres) const;
    Q_INVOKABLE qreal roadHeadingAt(qreal metres) const;

    void start();
    void stop();
    void reset();
    void setSeed(quint32 seed);

    // Advances the simulation by `dt` seconds. Called by the internal timer while running;
    // public so tests can step deterministically.
    void advance(qreal dt);

signals:
    void updated();
    void durationChanged();
    void warningChanged();
    void runningChanged();
    void finished();

private:
    static qreal curvatureAt(qreal distance);
    qreal speedAt(qreal time) const;
    void tick();
    void scheduleNextEpisode();
    void setWarning(Warning warning);

    QTimer m_timer;
    QElapsedTimer m_clock;
    QRandomGenerator m_random;

    bool m_leftHeld = false;
    bool m_rightHeld = false;
    qreal m_time = 0;
    qreal m_speed = 0;
    qreal m_speedPhase[2] = {0, 0};
    qreal m_lanePosition = 0;
    qreal m_steeringAngle = 0;
    qreal m_curvature = 0;
    qreal m_distance = 0;
    qreal m_progress = 0;
    qreal m_duration = 40;
    qreal m_blur = 0;
    qreal m_nextEpisodeIn = 0;
    qreal m_episodeTime = -1; // < 0 when not in a blur episode
    qreal m_wobblePhase = 0;
    qreal m_warningHold = 0;
    Warning m_warning = NoWarning;
};
