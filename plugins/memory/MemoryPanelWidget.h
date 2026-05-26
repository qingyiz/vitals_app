#pragma once

#include "IMemoryCollector.h"

#include <QWidget>

namespace Vitals {

class InfoPanelWidget;

class MemoryPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryPanelWidget(QWidget* parent = nullptr);

    void applySnapshot(const MemorySnapshot& snapshot);

private:
    static QString formatBytes(quint64 bytes);
    static QString formatPercent(double value);

    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
