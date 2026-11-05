#include "TypingSession.h"

#include <utility>

namespace {

const char Code[] = "bool preflight(Algorithms &a) {\n"
                    "    if (!a.sensorsOk())\n"
                    "        return false;\n"
                    "    return a.errantFlight() > 0.9;\n"
                    "}\n";

// Keystrokes, as a multiple of the snippet's length, after which the screen is unreadable even if
// the code isn't finished. Leaves room for typos; finishing the code also ends the night.
constexpr qreal StaminaFraction = 1.25;

} // namespace

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
}

qreal TypingSession::fatigue() const
{
    if (m_typedLength >= m_code.size())
        return 1.0; // done, and done in
    return qMin(1.0, qreal(m_keystrokes) / m_requiredKeystrokes);
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
    return correct;
}

void TypingSession::reset()
{
    m_typedLength = 0;
    m_keystrokes = 0;
    m_typos = 0;
    emit progressChanged();
}

void TypingSession::skipIndentation()
{
    while (m_typedLength < m_code.size() && m_code.at(m_typedLength) == u' ')
        ++m_typedLength;
}
