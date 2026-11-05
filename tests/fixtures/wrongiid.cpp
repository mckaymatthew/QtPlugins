// A valid Qt plugin, but for some other interface: the host must not load it.
#include <QObject>
#include <QtPlugin>

class WrongIid : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.example.SomethingElse/1" FILE "wrongiid.json")
};

#include "wrongiid.moc"
