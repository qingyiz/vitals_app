#pragma once

#include "ITaskbarDetailPlugin.h"

#include <QSize>
#include <QWidget>

class QVBoxLayout;
class QLayout;

namespace Vitals {

class TaskbarMenuDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskbarMenuDetailWidget(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setContents(const QList<TaskbarDetailContent>& contents);

private:
    QWidget* createContentPanel(const TaskbarDetailContent& content);
    QWidget* createBadge(const TaskbarDetailBadge& badge);
    QWidget* createRow(const TaskbarDetailRow& row);
    void updateContentGeometry();
    static void clearLayout(QLayout* layout);

    QVBoxLayout* m_layout = nullptr;
    QSize m_contentSize;
};

} // namespace Vitals
