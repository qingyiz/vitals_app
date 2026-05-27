#pragma once

#include "plugin/PluginManager.h"

#include <QWidget>

class QLabel;
class QVBoxLayout;

namespace Vitals {

class ConfigManager;
class LanguageManager;

/**
 * \if ENGLISH
 * @brief Host-owned plugin management overview page
 *
 * Presents discovered plugin binaries together with their runtime status,
 * metadata, and skip/failure reasons so users can understand what the host
 * loaded and why.
 * \endif
 *
 * \if CHINESE
 * @brief 宿主拥有的插件管理总览页面
 *
 * 该页面用于展示已发现的插件二进制、运行时状态、插件元信息，以及跳过/
 * 失败原因，帮助用户理解宿主到底加载了什么插件、为什么。
 * \endif
 */
class PluginCenterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginCenterWidget(ConfigManager* configManager, LanguageManager* languageManager, QWidget* parent = nullptr);

    void setPluginInfos(const QList<PluginRuntimeInfo>& pluginInfos);

Q_SIGNALS:
    void pluginEnabledChanged(const QString& pluginId, const QString& filePath, bool enabled);
    void pluginTaskbarVisibilityChanged(const QString& pluginId, const QString& filePath, bool enabled);

private:
    QWidget* createPluginCard(const PluginRuntimeInfo& pluginInfo);
    QString statusText(PluginRuntimeInfo::Status status) const;
    static QString statusColor(PluginRuntimeInfo::Status status);
    static QString supportedPlatformsText(const PluginRuntimeInfo& pluginInfo);
    static void clearLayout(QLayout* layout);
    QString text(const QString& key, const QString& fallback) const;

    ConfigManager* m_configManager = nullptr;
    LanguageManager* m_languageManager = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_summaryLabel = nullptr;
    QVBoxLayout* m_cardsLayout = nullptr;
};

} // namespace Vitals
