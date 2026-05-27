#pragma once

#include "IMemoryCollector.h"

#include <QWidget>

namespace Vitals {

class IAppContext;
class InfoPanelWidget;

class MemoryPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryPanelWidget(IAppContext* context, QWidget* parent = nullptr);

    void applySnapshot(const MemorySnapshot& snapshot);

private:
    static QString formatBytes(quint64 bytes);
    static QString formatPercent(double value);
    QString text(const QString& key, const QString& fallback) const;

    IAppContext* m_context = nullptr;
    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
