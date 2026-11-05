#pragma once

// Helpers shared by the rendering smoke tests.

#include <QGuiApplication>
#include <QImage>
#include <QQuickView>
#include <QSet>
#include <QStringList>
#include <QTest>

namespace smoke {

inline QStringList &qmlWarnings()
{
    static QStringList warnings;
    return warnings;
}

// Records QML warnings (while still printing them) so a test can fail on any.
inline void captureQmlWarnings()
{
    static QtMessageHandler previous = nullptr;
    previous = qInstallMessageHandler(
        [](QtMsgType type, const QMessageLogContext &context, const QString &message) {
            if (type >= QtWarningMsg
                && (message.contains("qml", Qt::CaseInsensitive)
                    || QByteArray(context.category).startsWith("qt.qml")))
                qmlWarnings().append(message);
            previous(type, context, message);
        });
}

inline QQuickView *findQuickView()
{
    const QWindowList windows = QGuiApplication::allWindows();
    for (QWindow *window : windows) {
        if (auto *view = qobject_cast<QQuickView *>(window))
            return view;
    }
    return nullptr;
}

// Number of distinct colours on a coarse grid: a blank or flat frame scores very low.
inline int distinctColors(const QImage &image)
{
    QSet<QRgb> colors;
    for (int y = 0; y < image.height(); y += 4) {
        for (int x = 0; x < image.width(); x += 4)
            colors.insert(image.pixel(x, y));
    }
    return colors.size();
}

// Saves a frame of `view` to SCREENSHOT_DIR/<name>.png and returns it.
inline QImage screenshot(QQuickView *view, const QString &name)
{
    const QImage image = view->grabWindow();
    const QString path = QStringLiteral(SCREENSHOT_DIR "/%1.png").arg(name);
    if (!image.isNull() && image.save(path))
        qInfo("saved %s", qPrintable(path));
    else
        qWarning("could not save %s", qPrintable(path));
    return image;
}

} // namespace smoke
