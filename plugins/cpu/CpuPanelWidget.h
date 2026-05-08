#pragma once

#include "ICpuCollector.h"

#include <QWidget>

namespace Vitals {

class InfoPanelWidget;

class CpuPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CpuPanelWidget(QWidget* parent = nullptr);

    void applySnapshot(const CpuSnapshot& snapshot);

private:
    static QString formatPercent(double value);

    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
