// Implements IExtension but declares an API version the host doesn't support.
#include <qtplugins/IExtension.h>

#include <QObject>

class BadApi : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IExtension_iid FILE "badapi.json")
    Q_INTERFACES(IExtension)

public:
    QWidget *createWidget(QWidget *) override { return nullptr; }
};

#include "badapi.moc"
