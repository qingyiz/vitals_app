#pragma once

#include <QListWidget>

namespace Vitals {

class NavigationWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget* parent = nullptr);

    void addNavigationItem(const QString& id, const QString& title);
    QString currentItemId() const;
};

} // namespace Vitals

