#include "HelloExtension.h"

#include <QLabel>

QWidget *HelloExtension::createWidget(QWidget *parent)
{
    auto *label = new QLabel(tr("Hello from an extension!"), parent);
    label->setObjectName(QStringLiteral("helloLabel"));
    label->setAlignment(Qt::AlignCenter);
    return label;
}
