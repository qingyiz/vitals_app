#pragma once

#include "ITaskbarDetailPlugin.h"

#include <QWidget>

class QVBoxLayout;
class QLayout;

namespace Vitals {

class TaskbarMenuDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskbarMenuDetailWidget(QWidget* parent = nullptr);

    void setContents(const QList<TaskbarDetailContent>& contents);

private:
    QWidget* createContentPanel(const TaskbarDetailContent& content);
    QWidget* createBadge(const TaskbarDetailBadge& badge);
    QWidget* createRow(const TaskbarDetailRow& row);
    static void clearLayout(QLayout* layout);

    QVBoxLayout* m_layout = nullptr;
};

} // namespace Vitals
