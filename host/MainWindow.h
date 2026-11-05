#pragma once

#include <QMainWindow>

class ExtensionManager;
class QTabWidget;
class QTreeWidget;

// Example host window: an "Extensions" overview tab listing everything found (including load
// errors), and one tab per opened extension.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ExtensionManager *extensions, QWidget *parent = nullptr);

    bool openExtension(const QString &id);

private:
    void populateOverview();

    ExtensionManager *m_extensions;
    QTabWidget *m_tabs;
    QTreeWidget *m_overview;
};
