#pragma once

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

class IExtension;
class QPluginLoader;

// Discovers and loads extensions (Qt plugins implementing IExtension) from a directory.
//
// Copy ExtensionManager.{h,cpp} and sdk/include/qtplugins/IExtension.h into a host application to
// give it extension support. Loaded plugins stay loaded for the lifetime of the manager's parent;
// widgets created by an extension must be destroyed before its library is unloaded.
class ExtensionManager : public QObject
{
    Q_OBJECT

public:
    struct Extension
    {
        QString filePath;
        QString id;
        QString name;
        QString version;
        QString description;
        IExtension *instance = nullptr; // null if loading failed
        QString error;                  // why loading failed, empty on success
    };

    explicit ExtensionManager(QObject *parent = nullptr);

    // Scans `directory` (non-recursively) and loads every compatible extension found.
    // Files that aren't extensions for this host are recorded with an error, not loaded.
    void loadFromDirectory(const QString &directory);

    const QList<Extension> &extensions() const { return m_extensions; }
    QList<Extension> loadedExtensions() const;
    const Extension *findById(const QString &id) const;

private:
    Extension load(const QString &filePath);

    QList<Extension> m_extensions;
};
