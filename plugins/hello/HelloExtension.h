#pragma once

#include <qtplugins/IExtension.h>

#include <QObject>

// Smallest possible extension: proves the plugin pipeline without any 3D dependencies.
class HelloExtension : public QObject, public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IExtension_iid FILE "hello.json")
    Q_INTERFACES(IExtension)

public:
    QWidget *createWidget(QWidget *parent) override;
};
