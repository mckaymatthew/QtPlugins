#include "MainWindow.h"

#include "ExtensionManager.h"

#include <qtplugins/IExtension.h>

#include <QFileInfo>
#include <QHeaderView>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTreeWidget>

MainWindow::MainWindow(ExtensionManager *extensions, QWidget *parent)
    : QMainWindow(parent)
    , m_extensions(extensions)
    , m_tabs(new QTabWidget(this))
    , m_overview(new QTreeWidget(this))
{
    setWindowTitle(tr("Extension Host"));
    resize(1024, 768);

    m_overview->setHeaderLabels({tr("Name"), tr("Version"), tr("Status"), tr("File")});
    m_overview->setRootIsDecorated(false);
    m_overview->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(m_overview, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *item) {
        openExtension(item->data(0, Qt::UserRole).toString());
    });

    m_tabs->setTabsClosable(true);
    m_tabs->setDocumentMode(true);
    m_tabs->addTab(m_overview, tr("Extensions"));
    m_tabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr); // overview can't be closed
    m_tabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget *page = m_tabs->widget(index);
        m_tabs->removeTab(index);
        delete page;
    });
    setCentralWidget(m_tabs);

    QMenu *menu = menuBar()->addMenu(tr("&Extensions"));
    for (const ExtensionManager::Extension &extension : m_extensions->loadedExtensions()) {
        const QString id = extension.id;
        menu->addAction(extension.name, this, [this, id] { openExtension(id); });
    }
    if (menu->isEmpty())
        menu->addAction(tr("(none loaded)"))->setEnabled(false);
    menu->addSeparator();
    menu->addAction(tr("&Quit"), QKeySequence::Quit, this, &QWidget::close);

    populateOverview();
}

bool MainWindow::openExtension(const QString &id)
{
    const ExtensionManager::Extension *extension = m_extensions->findById(id);
    if (!extension)
        return false;

    QWidget *widget = extension->instance->createWidget(m_tabs);
    if (!widget) {
        statusBar()->showMessage(tr("%1 failed to create a widget").arg(extension->name), 5000);
        return false;
    }
    m_tabs->setCurrentIndex(m_tabs->addTab(widget, extension->name));
    return true;
}

void MainWindow::populateOverview()
{
    int loaded = 0;
    const QList<ExtensionManager::Extension> extensions = m_extensions->extensions();
    for (const ExtensionManager::Extension &extension : extensions) {
        auto *item = new QTreeWidgetItem(m_overview);
        item->setText(0, extension.name.isEmpty() ? QFileInfo(extension.filePath).fileName()
                                                  : extension.name);
        item->setText(1, extension.version);
        item->setText(2, extension.instance ? tr("Loaded") : extension.error);
        item->setText(3, extension.filePath);
        item->setToolTip(0, extension.description);
        if (extension.instance) {
            item->setData(0, Qt::UserRole, extension.id);
            ++loaded;
        } else {
            item->setForeground(2, Qt::red);
        }
    }
    statusBar()->showMessage(tr("%1 of %2 extensions loaded — double-click one to open it")
                                 .arg(loaded)
                                 .arg(extensions.size()));
}
