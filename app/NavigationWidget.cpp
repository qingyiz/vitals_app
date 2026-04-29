#include "NavigationWidget.h"

namespace Vitals {

NavigationWidget::NavigationWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName(QStringLiteral("navigation"));
    setFixedWidth(216);
    setSpacing(3);
    setIconSize(QSize(18, 18));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void NavigationWidget::addNavigationItem(const QString& id, const QString& title, const QIcon& icon)
{
    auto* item = new QListWidgetItem(icon, title, this);
    item->setData(Qt::UserRole, id);
    addItem(item);
}

QString NavigationWidget::currentItemId() const
{
    const QListWidgetItem* item = currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

} // namespace Vitals
