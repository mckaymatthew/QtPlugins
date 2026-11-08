#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// Phase 1, at work: the player types a fixed snippet of C++. Correct characters advance through
// the code; every keystroke (right or wrong) adds fatigue, which the scene shows as blur. At full
// fatigue (or once the code is finished) the screen is unreadable and the player may go home.
//
// Every so often an email from the manager pops up. It blocks typing until the player presses
// Enter, and costs a large slice of the night's stamina the moment it arrives.
class TypingSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString code READ code CONSTANT)
    Q_PROPERTY(int typedLength READ typedLength NOTIFY progressChanged)
    Q_PROPERTY(int keystrokes READ keystrokes NOTIFY progressChanged)
    Q_PROPERTY(int typos READ typos NOTIFY progressChanged)
    Q_PROPERTY(qreal fatigue READ fatigue NOTIFY progressChanged)
    Q_PROPERTY(bool unreadable READ isUnreadable NOTIFY progressChanged)
    Q_PROPERTY(bool emailOpen READ isEmailOpen NOTIFY emailChanged)
    Q_PROPERTY(QString emailSubject READ emailSubject NOTIFY emailChanged)
    Q_PROPERTY(QString emailBody READ emailBody NOTIFY emailChanged)
    Q_PROPERTY(int emailCount READ emailCount NOTIFY emailChanged)

public:
    explicit TypingSession(QObject *parent = nullptr);

    QString code() const { return m_code; }
    int typedLength() const { return m_typedLength; }
    int keystrokes() const { return m_keystrokes; }
    int typos() const { return m_typos; }
    qreal fatigue() const;                     // 0 (fresh) .. 1 (unreadable)
    bool isUnreadable() const { return fatigue() >= 1.0; }

    bool isEmailOpen() const { return m_emailOpen; }
    QString emailSubject() const;
    QString emailBody() const;
    int emailCount() const { return m_emailCount; } // emails received tonight

    static QStringList emailSubjects(); // every subject an email can have
    static constexpr int FirstEmailAt = 10;  // keystrokes
    static constexpr int EmailInterval = 14; // keystrokes between emails
    static constexpr qreal EmailCost = 0.2;  // share of the night's stamina each email takes

    // Feeds text from a key press. Return/Enter count as a newline; indentation after a newline is
    // filled in automatically. Returns true if the character was the expected one. While an email
    // is open, Return/Enter dismisses it and every other key is ignored.
    Q_INVOKABLE bool typeText(const QString &text);
    Q_INVOKABLE void dismissEmail();
    Q_INVOKABLE void reset();

signals:
    void progressChanged();
    void typo();
    void emailChanged();

private:
    void skipIndentation();
    void maybeSendEmail();

    QString m_code;
    int m_requiredKeystrokes = 1; // keystrokes until unreadable
    int m_typedLength = 0;
    int m_keystrokes = 0;
    int m_typos = 0;
    qreal m_penalty = 0;          // keystrokes' worth of stamina lost to emails
    bool m_emailOpen = false;
    int m_emailCount = 0;
    int m_emailOffset = 0;        // where tonight's run through the inbox starts
};
