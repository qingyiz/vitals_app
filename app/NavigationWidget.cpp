#include "NavigationWidget.h"

namespace Vitals {

NavigationWidget::NavigationWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName(QStringLiteral("navigation"));
    setFixedWidth(216);
    setSpacing(3);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void NavigationWidget::addNavigationItem(const QString& id, const QString& title)
{
    auto* item = new QListWidgetItem(title, this);
    item->setData(Qt::UserRole, id);
    addItem(item);
}

QString NavigationWidget::currentItemId() const
{
    const QListWidgetItem* item = currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

} // namespace Vitals
