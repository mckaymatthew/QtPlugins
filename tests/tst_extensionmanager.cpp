#include "ExtensionManager.h"

#include <qtplugins/IExtension.h>

#include <QFileInfo>
#include <QLabel>
#include <QScopedPointer>
#include <QTest>

class TestExtensionManager : public QObject
{
    Q_OBJECT

private:
    static const ExtensionManager::Extension *byFileName(const ExtensionManager &manager,
                                                         const QString &needle)
    {
        const QList<ExtensionManager::Extension> &all = manager.extensions();
        for (const ExtensionManager::Extension &extension : all) {
            if (QFileInfo(extension.filePath).fileName().contains(needle))
                return &extension;
        }
        return nullptr;
    }

private slots:
    void loadsValidExtension()
    {
        ExtensionManager manager;
        manager.loadFromDirectory(QStringLiteral(EXTENSIONS_DIR));

        const ExtensionManager::Extension *hello = manager.findById("org.qtplugins.hello");
        QVERIFY(hello);
        QVERIFY(hello->instance);
        QVERIFY(hello->error.isEmpty());
        QCOMPARE(hello->name, QStringLiteral("Hello"));
        QCOMPARE(hello->version, QStringLiteral("1.0.0"));
        QVERIFY(!hello->description.isEmpty());

        QWidget parent;
        QWidget *widget = hello->instance->createWidget(&parent);
        QVERIFY(qobject_cast<QLabel *>(widget));
        QCOMPARE(widget->parentWidget(), &parent);
    }

    void rejectsIncompatibleLibraries()
    {
        ExtensionManager manager;
        manager.loadFromDirectory(QStringLiteral(FIXTURE_DIR));

        QVERIFY(manager.loadedExtensions().isEmpty());
        QCOMPARE(manager.extensions().size(), 3); // README.txt is not considered at all

        const auto *wrongIid = byFileName(manager, "wrongiid");
        QVERIFY(wrongIid);
        QVERIFY2(wrongIid->error.contains("Incompatible interface"), qPrintable(wrongIid->error));

        const auto *badApi = byFileName(manager, "badapi");
        QVERIFY(badApi);
        QVERIFY2(badApi->error.contains("API version 999"), qPrintable(badApi->error));
        QCOMPARE(badApi->id, QStringLiteral("org.qtplugins.test.badapi"));
        QVERIFY(!manager.findById(badApi->id));

        const auto *notPlugin = byFileName(manager, "notaplugin");
        QVERIFY(notPlugin);
        QVERIFY2(notPlugin->error.contains("Not a Qt plugin"), qPrintable(notPlugin->error));
    }

    void rejectsDuplicateIds()
    {
        ExtensionManager manager;
        manager.loadFromDirectory(QStringLiteral(EXTENSIONS_DIR));
        const int loaded = manager.loadedExtensions().size();
        manager.loadFromDirectory(QStringLiteral(EXTENSIONS_DIR));

        QCOMPARE(manager.loadedExtensions().size(), loaded);
        const ExtensionManager::Extension &last = manager.extensions().constLast();
        QVERIFY(!last.instance);
        QVERIFY2(last.error.contains("Duplicate"), qPrintable(last.error));
    }

    void missingDirectoryIsEmpty()
    {
        ExtensionManager manager;
        manager.loadFromDirectory(QStringLiteral("/nonexistent/qtplugins"));
        QVERIFY(manager.extensions().isEmpty());
    }
};

QTEST_MAIN(TestExtensionManager)
#include "tst_extensionmanager.moc"
