#include "TypingSession.h"

#include <QRandomGenerator>

#include <iterator>
#include <utility>

namespace {

const char Code[] = "bool preflight(Algorithms &a) {\n"
                    "    return a.errantFlight() > 0.9;\n"
                    "}\n";

// Keystrokes, as a multiple of the snippet's length, after which the screen is unreadable even if
// the code isn't finished. The manager's emails eat into it too; a clean typist gets three before
// the night ends.
constexpr qreal StaminaFraction = 1.25;

struct Email
{
    const char *subject;
    const char *body;
};
const Email Inbox[] = {
    {QT_TRANSLATE_NOOP("TypingSession", "IMS Replan"),
     QT_TRANSLATE_NOOP("TypingSession", "The Integrated Master Schedule moved again. Please re-baseline all of your tasks before the flight test.")},
    {QT_TRANSLATE_NOOP("TypingSession", "IMS Replan Wall Walk"),
     QT_TRANSLATE_NOOP("TypingSession", "Wall walk of the new IMS in Conference Room B in 10 minutes. Bring sticky notes. Bring two colors of sticky notes.")},
    {QT_TRANSLATE_NOOP("TypingSession", "BCN Notification"),
     QT_TRANSLATE_NOOP("TypingSession", "A Baseline Change Notice has been issued. Your work package no longer exists. Please keep working on it.")},
    {QT_TRANSLATE_NOOP("TypingSession", "Urgent need 2028 BOE completed ASAP"),
     QT_TRANSLATE_NOOP("TypingSession", "Need your Basis of Estimate for 2028 hours by end of day. Yes, today. Yes, I know what time it is.")},
    {QT_TRANSLATE_NOOP("TypingSession", "RE: RE: RE: FW: Variance at Completion"),
     QT_TRANSLATE_NOOP("TypingSession", "Please explain the variance in your variance explanation.")},
    {QT_TRANSLATE_NOOP("TypingSession", "Timecard Charging Training (OVERDUE)"),
     QT_TRANSLATE_NOOP("TypingSession", "Complete the 90-minute training on how to charge the 90 minutes of training.")},
    {QT_TRANSLATE_NOOP("TypingSession", "CDR deck v14_FINAL_final2"),
     QT_TRANSLATE_NOOP("TypingSession", "Can you add one slide? Just one. Due at 06:00. Also update the other 47.")},
    {QT_TRANSLATE_NOOP("TypingSession", "Action Item Review: Action Items"),
     QT_TRANSLATE_NOOP("TypingSession", "Reviewing the action items from the last action item review. You have 3 new action items.")},
};

} // namespace

QStringList TypingSession::emailSubjects()
{
    QStringList subjects;
    for (const Email &email : Inbox)
        subjects.append(tr(email.subject));
    return subjects;
}

QString TypingSession::emailSubject() const
{
    if (m_emailCount == 0)
        return {};
    return tr(Inbox[(m_emailOffset + m_emailCount - 1) % std::size(Inbox)].subject);
}

QString TypingSession::emailBody() const
{
    if (m_emailCount == 0)
        return {};
    return tr(Inbox[(m_emailOffset + m_emailCount - 1) % std::size(Inbox)].body);
}

TypingSession::TypingSession(QObject *parent)
    : QObject(parent)
    , m_code(QString::fromLatin1(Code))
{
    int typeable = 0;
    bool lineStart = true;
    for (QChar c : std::as_const(m_code)) {
        if (lineStart && c == u' ')
            continue; // auto-indented
        lineStart = c == u'\n';
        ++typeable;
    }
    m_requiredKeystrokes = qMax(1, qRound(typeable * StaminaFraction));
    m_emailOffset = int(QRandomGenerator::global()->bounded(quint32(std::size(Inbox))));
}

qreal TypingSession::fatigue() const
{
    if (m_typedLength >= m_code.size())
        return 1.0; // done, and done in
    return qMin(1.0, (m_keystrokes + m_penalty) / m_requiredKeystrokes);
}

bool TypingSession::typeText(const QString &text)
{
    if (text.isEmpty())
        return false;
    QChar c = text.at(0);
    if (c == u'\r')
        c = u'\n';
    if (c != u'\n' && !c.isPrint())
        return false; // backspace, escape, tab, ...
    if (m_emailOpen) {
        if (c == u'\n')
            dismissEmail();
        return false;
    }

    ++m_keystrokes;
    const bool correct = m_typedLength < m_code.size() && m_code.at(m_typedLength) == c;
    if (correct) {
        ++m_typedLength;
        if (c == u'\n')
            skipIndentation();
    } else if (m_typedLength < m_code.size()) {
        ++m_typos;
    }
    emit progressChanged();
    if (!correct && m_typedLength < m_code.size())
        emit typo();
    maybeSendEmail();
    return correct;
}

void TypingSession::dismissEmail()
{
    if (!m_emailOpen)
        return;
    m_emailOpen = false;
    emit emailChanged();
}

void TypingSession::maybeSendEmail()
{
    if (m_emailOpen || isUnreadable())
        return;
    if (m_keystrokes < FirstEmailAt + m_emailCount * EmailInterval)
        return;
    ++m_emailCount;
    m_emailOpen = true;
    m_penalty += EmailCost * m_requiredKeystrokes;
    emit emailChanged();
    emit progressChanged();
}

void TypingSession::reset()
{
    m_typedLength = 0;
    m_keystrokes = 0;
    m_typos = 0;
    m_penalty = 0;
    m_emailOpen = false;
    m_emailCount = 0;
    m_emailOffset = int(QRandomGenerator::global()->bounded(quint32(std::size(Inbox))));
    emit emailChanged();
    emit progressChanged();
}

void TypingSession::skipIndentation()
{
    while (m_typedLength < m_code.size() && m_code.at(m_typedLength) == u' ')
        ++m_typedLength;
}
