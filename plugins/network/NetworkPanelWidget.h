#pragma once

#include "INetworkCollector.h"

#include <QWidget>

namespace Vitals {

class InfoPanelWidget;

class NetworkPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkPanelWidget(QWidget* parent = nullptr);

    void applySnapshot(const NetworkSnapshot& snapshot);

private:
    static QString formatBytes(quint64 bytes);
    static QString formatRate(double bytesPerSecond);
    static QString formatInterfaceSummary(const QStringList& interfaces);

    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
