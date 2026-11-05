#include "TicTacToeExtension.h"

#include "GameModel.h"

#include <QLabel>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickView>
#include <QWidget>

QWidget *TicTacToeExtension::createWidget(QWidget *parent)
{
    // A QQuickView in a window container (rather than QQuickWidget) renders through the platform's
    // native graphics API — Metal, Direct3D, Vulkan or OpenGL — with no extra offscreen copy.
    auto *view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor(QColor(0x1d, 0x23, 0x30));

    // Each view has its own engine, so a context property is private to this game instance.
    auto *game = new GameModel(view);
    game->setVsComputer(true);
    view->rootContext()->setContextProperty(QStringLiteral("game"), game);
    view->setSource(QUrl(QStringLiteral("qrc:/qtplugins/tictactoe/Board.qml")));

    if (view->status() == QQuickView::Error) {
        QStringList messages;
        const QList<QQmlError> errors = view->errors();
        for (const QQmlError &error : errors)
            messages.append(error.toString());
        delete view;
        auto *label = new QLabel(tr("Failed to load the game board:\n%1").arg(messages.join('\n')),
                                 parent);
        label->setWordWrap(true);
        return label;
    }

    QWidget *container = QWidget::createWindowContainer(view, parent); // takes ownership of view
    container->setObjectName(QStringLiteral("ticTacToeBoard"));
    container->setMinimumSize(320, 240);
    container->setFocusPolicy(Qt::StrongFocus);
    return container;
}
