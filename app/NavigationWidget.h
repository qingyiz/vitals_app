#pragma once

#include <QIcon>
#include <QListWidget>

namespace Vitals {

class NavigationWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget* parent = nullptr);

    void addNavigationItem(const QString& id, const QString& title, const QIcon& icon = QIcon());
    QString currentItemId() const;
    bool setCurrentItemById(const QString& id);
};

} // namespace Vitals
