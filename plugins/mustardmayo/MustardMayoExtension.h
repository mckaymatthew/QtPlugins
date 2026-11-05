#pragma once

#include <qtplugins/IExtension.h>

#include <QObject>

class MustardMayoExtension : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IExtension_iid FILE "mustardmayo.json")
    Q_INTERFACES(IExtension)

public:
    QWidget *createWidget(QWidget *parent) override;
};
