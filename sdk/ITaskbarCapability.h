#pragma once

#include "ITaskbarDetailPlugin.h"

#include <QHash>
#include <QList>
#include <QString>

namespace Vitals {

struct TaskbarLabelDescriptor
{
    QString configKey;
    QString titleKey;
    QString titleFallback;
    QString defaultLabel;
};

class ITaskbarCapability
{
public:
    virtual ~ITaskbarCapability() = default;

    virtual QString displayText(const QHash<QString, MetricValue>& latestValues) const = 0;
    virtual QString tooltip(const QHash<QString, MetricValue>& latestValues) const = 0;
    virtual bool isEnabledByDefault() const
    {
        return true;
    }

    /**
     * \if ENGLISH
     * @brief Returns whether the host should expose a generic single label editor
     *
     * This is intended for compact taskbar/menu-bar text such as "CPU", "MEM",
     * or "Disk". More complex taskbar displays, such as separate upload and
     * download labels, should expose a dedicated settings model later.
     * \endif
     *
     * \if CHINESE
     * @brief 返回宿主是否应展示通用的单字段任务栏标签编辑器
     *
     * 该能力用于 CPU、MEM、Disk 这类紧凑任务栏/菜单栏文本。上传/下载等多字段
     * 展示后续应使用专门的设置模型。
     * \endif
     */
    virtual bool supportsCustomTaskbarLabel() const
    {
        return false;
    }

    /**
     * \if ENGLISH
     * @brief Returns the default label used when no user override is stored
     * \endif
     *
     * \if CHINESE
     * @brief 返回未存储用户覆盖值时使用的默认标签
     * \endif
     */
    virtual QString defaultTaskbarLabel() const
    {
        return {};
    }

    /**
     * \if ENGLISH
     * @brief Returns the plugin config key used for the custom label
     * \endif
     *
     * \if CHINESE
     * @brief 返回自定义标签在插件配置中的字段名
     * \endif
     */
    virtual QString taskbarLabelConfigKey() const
    {
        return QStringLiteral("taskbarLabel");
    }

    /**
     * \if ENGLISH
     * @brief Returns all generic taskbar labels the host can edit
     *
     * Plugins with one compact label can rely on the single-label compatibility
     * methods above. Plugins with multiple taskbar labels, such as network
     * upload/download, should override this method.
     * \endif
     *
     * \if CHINESE
     * @brief 返回宿主可编辑的所有通用任务栏标签
     *
     * 只有一个紧凑标签的插件可以继续使用上面的单标签兼容方法；网络上传/下载
     * 这类多标签插件应重写该方法。
     * \endif
     */
    virtual QList<TaskbarLabelDescriptor> taskbarLabelDescriptors() const
    {
        if (!supportsCustomTaskbarLabel()) {
            return {};
        }

        return {
            {
                taskbarLabelConfigKey(),
                QStringLiteral("common.taskbarLabel"),
                QStringLiteral("Label"),
                defaultTaskbarLabel()
            }
        };
    }

    virtual TaskbarDetailContent detailContent(
        const QHash<QString, MetricValue>& latestValues) const = 0;
};

} // namespace Vitals
