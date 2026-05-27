#pragma once

#include "ICpuCollector.h"

#include <QWidget>

namespace Vitals {

class IAppContext;
class InfoPanelWidget;

class CpuPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CpuPanelWidget(IAppContext* context, QWidget* parent = nullptr);

    void applySnapshot(const CpuSnapshot& snapshot);

private:
    static QString formatPercent(double value);
    QString text(const QString& key, const QString& fallback) const;

    IAppContext* m_context = nullptr;
    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
