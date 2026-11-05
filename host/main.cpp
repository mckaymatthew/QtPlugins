#include "ExtensionManager.h"
#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qtplugins-host"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Example host that loads extensions."));
    parser.addHelpOption();
#ifdef Q_OS_MACOS
    // Inside an app bundle (Contents/MacOS/<exe>) plugins conventionally live in Contents/PlugIns.
    const QString defaultDir = QStringLiteral("../PlugIns/extensions");
#else
    const QString defaultDir = QStringLiteral("extensions");
#endif
    QCommandLineOption dirOption(QStringLiteral("extensions-dir"),
                                 QStringLiteral("Directory to load extensions from."),
                                 QStringLiteral("dir"),
                                 QDir::cleanPath(QDir(QApplication::applicationDirPath())
                                                     .filePath(defaultDir)));
    QCommandLineOption openOption(QStringLiteral("open"),
                                  QStringLiteral("Open the extension with this id on startup."),
                                  QStringLiteral("id"));
    parser.addOptions({dirOption, openOption});
    parser.process(app);

    ExtensionManager extensions(&app);
    extensions.loadFromDirectory(parser.value(dirOption));

    MainWindow window(&extensions);
    for (const QString &id : parser.values(openOption)) {
        if (!window.openExtension(id))
            qWarning("No loaded extension with id \"%s\"", qPrintable(id));
    }
    window.show();

    return app.exec();
}
