#include "MustardMayoExtension.h"

#include "MustardMayoGame.h"

#include <QFontDatabase>
#include <QLabel>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickView>
#include <QWidget>

QWidget *MustardMayoExtension::createWidget(QWidget *parent)
{
    // Same embedding as the tic-tac-toe extension: a QQuickView in a window container renders
    // through the platform's native graphics API.
    auto *view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor(Qt::black);

    auto *game = new MustardMayoGame(view);
    view->rootContext()->setContextProperty(QStringLiteral("game"), game);
    view->rootContext()->setContextProperty(
        QStringLiteral("fixedFont"), QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    view->setSource(QUrl(QStringLiteral("qrc:/qtplugins/mustardmayo/Main.qml")));

    if (view->status() == QQuickView::Error) {
        QStringList messages;
        const QList<QQmlError> errors = view->errors();
        for (const QQmlError &error : errors)
            messages.append(error.toString());
        delete view;
        auto *label = new QLabel(tr("Failed to load the game:\n%1").arg(messages.join('\n')),
                                 parent);
        label->setWordWrap(true);
        return label;
    }

    QWidget *container = QWidget::createWindowContainer(view, parent); // takes ownership of view
    container->setObjectName(QStringLiteral("mustardMayoGame"));
    container->setMinimumSize(480, 320);
    container->setFocusPolicy(Qt::StrongFocus);
    return container;
}
