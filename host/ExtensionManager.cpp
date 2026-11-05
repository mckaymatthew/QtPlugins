#include "ExtensionManager.h"

#include <qtplugins/IExtension.h>

#include <QDir>
#include <QJsonValue>
#include <QLibrary>
#include <QPluginLoader>

ExtensionManager::ExtensionManager(QObject *parent)
    : QObject(parent)
{
}

void ExtensionManager::loadFromDirectory(const QString &directory)
{
    const QDir dir(directory);
    const QStringList files = dir.entryList(QDir::Files, QDir::Name);
    for (const QString &fileName : files) {
        const QString filePath = dir.absoluteFilePath(fileName);
        if (QLibrary::isLibrary(filePath))
            m_extensions.append(load(filePath));
    }
}

QList<ExtensionManager::Extension> ExtensionManager::loadedExtensions() const
{
    QList<Extension> result;
    for (const Extension &extension : m_extensions) {
        if (extension.instance)
            result.append(extension);
    }
    return result;
}

const ExtensionManager::Extension *ExtensionManager::findById(const QString &id) const
{
    for (const Extension &extension : m_extensions) {
        if (extension.instance && extension.id == id)
            return &extension;
    }
    return nullptr;
}

ExtensionManager::Extension ExtensionManager::load(const QString &filePath)
{
    Extension extension;
    extension.filePath = filePath;

    // Parented to the manager so the library stays loaded as long as the manager lives.
    auto *loader = new QPluginLoader(filePath, this);

    // Inspect the metadata before loading so foreign libraries are never initialised.
    const QJsonObject pluginMetaData = loader->metaData();
    if (pluginMetaData.isEmpty()) {
        extension.error = tr("Not a Qt plugin: %1").arg(loader->errorString());
        delete loader;
        return extension;
    }

    const QJsonObject metaData = pluginMetaData.value(QLatin1String("MetaData")).toObject();
    extension.id = metaData.value(QLatin1String("id")).toString();
    extension.name = metaData.value(QLatin1String("name")).toString(extension.id);
    extension.version = metaData.value(QLatin1String("version")).toString();
    extension.description = metaData.value(QLatin1String("description")).toString();

    const QString iid = pluginMetaData.value(QLatin1String("IID")).toString();
    if (iid != QLatin1String(IExtension_iid)) {
        extension.error = tr("Incompatible interface \"%1\" (host expects \"%2\")")
                              .arg(iid, QLatin1String(IExtension_iid));
        delete loader;
        return extension;
    }

    const int apiVersion = metaData.value(QLatin1String("apiVersion")).toInt();
    if (apiVersion != QTPLUGINS_API_VERSION) {
        extension.error = tr("Incompatible API version %1 (host expects %2)")
                              .arg(apiVersion)
                              .arg(QTPLUGINS_API_VERSION);
        delete loader;
        return extension;
    }

    if (extension.id.isEmpty()) {
        extension.error = tr("Metadata has no \"id\"");
        delete loader;
        return extension;
    }

    if (findById(extension.id)) {
        extension.error = tr("Duplicate extension id \"%1\"").arg(extension.id);
        delete loader;
        return extension;
    }

    QObject *root = loader->instance();
    extension.instance = qobject_cast<IExtension *>(root);
    if (!extension.instance) {
        extension.error = root ? tr("Root object does not implement IExtension")
                               : loader->errorString();
        loader->unload();
        delete loader;
    }
    return extension;
}
