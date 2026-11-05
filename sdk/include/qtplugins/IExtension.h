#pragma once

// The contract between a host application and its extensions (plugins).
//
// An extension is a Qt plugin (shared library) whose root object implements IExtension:
//
//     class MyExtension : public QObject, public IExtension
//     {
//         Q_OBJECT
//         Q_PLUGIN_METADATA(IID IExtension_iid FILE "myextension.json")
//         Q_INTERFACES(IExtension)
//     public:
//         QWidget *createWidget(QWidget *parent) override;
//     };
//
// The metadata JSON describes the extension so the host can list it without loading it:
//
//     { "id": "org.example.myextension", "name": "My Extension", "version": "1.0.0",
//       "description": "...", "apiVersion": 1 }
//
// Host and extensions must be built against the same Qt version, compiler and build type.
// Bump IExtension_iid and QTPLUGINS_API_VERSION together whenever this interface changes.

#include <QtPlugin>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

#define QTPLUGINS_API_VERSION 1

class IExtension
{
public:
    virtual ~IExtension() = default;

    // Creates the extension's UI. The host takes ownership (normally via `parent`) and may call
    // this more than once, e.g. to open several instances.
    virtual QWidget *createWidget(QWidget *parent) = 0;
};

#define IExtension_iid "org.qtplugins.IExtension/1"
Q_DECLARE_INTERFACE(IExtension, IExtension_iid)
