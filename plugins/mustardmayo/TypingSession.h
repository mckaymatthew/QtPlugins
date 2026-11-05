#pragma once

#include <QObject>
#include <QString>

// Phase 1, at work: the player types a fixed snippet of C++. Correct characters advance through
// the code; every keystroke (right or wrong) adds fatigue, which the scene shows as blur. At full
// fatigue (or once the code is finished) the screen is unreadable and the player may go home.
class TypingSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString code READ code CONSTANT)
    Q_PROPERTY(int typedLength READ typedLength NOTIFY progressChanged)
    Q_PROPERTY(int keystrokes READ keystrokes NOTIFY progressChanged)
    Q_PROPERTY(int typos READ typos NOTIFY progressChanged)
    Q_PROPERTY(qreal fatigue READ fatigue NOTIFY progressChanged)
    Q_PROPERTY(bool unreadable READ isUnreadable NOTIFY progressChanged)

public:
    explicit TypingSession(QObject *parent = nullptr);

    QString code() const { return m_code; }
    int typedLength() const { return m_typedLength; }
    int keystrokes() const { return m_keystrokes; }
    int typos() const { return m_typos; }
    qreal fatigue() const;                     // 0 (fresh) .. 1 (unreadable)
    bool isUnreadable() const { return fatigue() >= 1.0; }

    // Feeds text from a key press. Return/Enter count as a newline; indentation after a newline is
    // filled in automatically. Returns true if the character was the expected one.
    Q_INVOKABLE bool typeText(const QString &text);
    Q_INVOKABLE void reset();

signals:
    void progressChanged();
    void typo();

private:
    void skipIndentation();

    QString m_code;
    int m_requiredKeystrokes = 1; // keystrokes until unreadable
    int m_typedLength = 0;
    int m_keystrokes = 0;
    int m_typos = 0;
};
