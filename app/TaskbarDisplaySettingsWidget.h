#pragma once

#include <QFrame>

namespace Vitals {

class ConfigManager;
class ITaskbarCapability;
class LanguageManager;

/**
 * \if ENGLISH
 * @brief Host-owned generic taskbar display settings for plugin panels
 *
 * Renders common taskbar/menu-bar preferences that are declared by a plugin's
 * taskbar capability. The widget persists values in the plugin config file and
 * does not know about concrete CPU, memory, or disk collectors.
 * \endif
 *
 * \if CHINESE
 * @brief 宿主拥有的插件面板通用任务栏显示设置控件
 *
 * 根据插件 taskbar capability 声明的通用能力渲染任务栏/菜单栏偏好设置，
 * 并将配置持久化到对应插件配置文件中；该控件不依赖具体 CPU、内存或磁盘采集逻辑。
 * \endif
 */
class TaskbarDisplaySettingsWidget : public QFrame
{
    Q_OBJECT

public:
    TaskbarDisplaySettingsWidget(
        const QString& pluginId,
        const ITaskbarCapability* taskbarCapability,
        ConfigManager* configManager,
        LanguageManager* languageManager,
        QWidget* parent = nullptr);

Q_SIGNALS:
    void settingsChanged();

private:
    QString text(const QString& key, const QString& fallback) const;
    QString currentLabel(const QString& configKey, const QString& defaultLabel) const;
    void saveLabel(const QString& configKey, const QString& label) const;

    QString m_pluginId;
    ConfigManager* m_configManager = nullptr;
    LanguageManager* m_languageManager = nullptr;
};

} // namespace Vitals
