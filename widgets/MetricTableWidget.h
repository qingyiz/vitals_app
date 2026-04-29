#pragma once

#include <QTableWidget>

namespace Vitals {

class MetricTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    using QTableWidget::QTableWidget;
};

} // namespace Vitals

