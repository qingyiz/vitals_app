#pragma once

#include "INetworkCollector.h"

#include <QWidget>

namespace Vitals {

class IAppContext;
class InfoPanelWidget;

class NetworkPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkPanelWidget(IAppContext* context, QWidget* parent = nullptr);

    void applySnapshot(const NetworkSnapshot& snapshot);

private:
    static QString formatBytes(quint64 bytes);
    static QString formatRate(double bytesPerSecond);
    QString formatInterfaceSummary(const QStringList& interfaces) const;
    QString text(const QString& key, const QString& fallback) const;

    IAppContext* m_context = nullptr;
    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
