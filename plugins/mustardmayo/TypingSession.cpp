#include "TypingSession.h"

#include <iterator>
#include <utility>

namespace {

const char *const Code[] = {
    "bool preflight(Algorithms &a) {",
    "    if (!a.sensorsOk())",
    "        return false;",
    "    return a.errantFlight() > 0.9;",
    "}",
};

// Requirement changes, in the order they arrive: the email, and the line it rewrites.
struct Change
{
    const char *email;
    int line;
    const char *text;
};
const Change Changes[] = {
    {QT_TRANSLATE_NOOP("TypingSession", "Actually, errantFlight() should be less than 0.9."),
     3, "    return a.errantFlight() < 0.9;"},
    {QT_TRANSLATE_NOOP("TypingSession", "Actually, sensorsOk() is deprecated. Use sensorsNominal()."),
     1, "    if (!a.sensorsNominal())"},
    {QT_TRANSLATE_NOOP("TypingSession", "Actually, make that threshold 0.95."),
     3, "    return a.errantFlight() < 0.95;"},
    {QT_TRANSLATE_NOOP("TypingSession", "Actually, errantFlight() should be greater than 0.95 again."),
     3, "    return a.errantFlight() > 0.95;"},
};

// Keystrokes, as a multiple of the original snippet's length, after which the screen is
// unreadable. The requirement changes mean retyping lines; a clean typist sees three of the four
// emails before the night ends. Finishing the code ends it too.
constexpr qreal StaminaFraction = 1.4;

} // namespace

TypingSession::TypingSession(QObject *parent)
    : QObject(parent)
{
    for (const char *line : Code)
        m_lines.append(QString::fromLatin1(line));

    int typeable = 0;
    for (const QString &line : std::as_const(m_lines))
        typeable += line.trimmed().size() + 1; // indentation is filled in; +1 for the newline
    m_requiredKeystrokes = qMax(1, qRound(typeable * StaminaFraction));
}

qreal TypingSession::fatigue() const
{
    if (m_typedLength >= code().size())
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
    const QString current = code();
    const bool inCode = m_typedLength < current.size();
    const bool correct = inCode && current.at(m_typedLength) == c;
    if (correct) {
        ++m_typedLength;
        if (c == u'\n') {
            skipIndentation();
            lineFinished(int(current.left(m_typedLength).count(u'\n')) - 1);
        }
    } else if (inCode) {
        ++m_typos;
    }
    emit progressChanged();
    if (!correct && inCode)
        emit typo();
    return correct;
}

void TypingSession::reset()
{
    m_lines.clear();
    for (const char *line : Code)
        m_lines.append(QString::fromLatin1(line));
    m_email.clear();
    m_changesMade = 0;
    m_emailedLines = 0;
    m_typedLength = 0;
    m_keystrokes = 0;
    m_typos = 0;
    emit codeChanged();
    emit requirementChanged(m_email);
    emit progressChanged();
}

void TypingSession::lineFinished(int line)
{
    const bool lastLine = line >= m_lines.size() - 1;
    if (lastLine || (m_emailedLines & (1u << line)) || m_changesMade >= int(std::size(Changes)))
        return;
    m_emailedLines |= 1u << line;
    const Change &change = Changes[m_changesMade++];

    m_lines[change.line] = QString::fromLatin1(change.text);
    // Keep about half of the finished lines, but never a line that has just changed.
    const int keep = qMin((line + 1) / 2, change.line);
    m_typedLength = lineStart(keep);
    skipIndentation();
    m_email = tr(change.email);

    emit codeChanged();
    emit requirementChanged(m_email);
}

int TypingSession::lineStart(int line) const
{
    int offset = 0;
    for (int i = 0; i < line; ++i)
        offset += m_lines.at(i).size() + 1;
    return offset;
}

void TypingSession::skipIndentation()
{
    const QString current = code();
    while (m_typedLength < current.size() && current.at(m_typedLength) == u' ')
        ++m_typedLength;
}
