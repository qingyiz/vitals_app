#pragma once

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Normalized value type used by metric descriptors
 * \endif
 *
 * \if CHINESE
 * @brief Metric 描述中使用的统一数值类型枚举
 * \endif
 */
enum class MetricValueType
{
    Integer,
    Double,
    String,
    Boolean,
    Percentage,
    Bytes,
    BytesPerSecond,
    Temperature,
    Frequency
};

/**
 * \if ENGLISH
 * @brief Static description of a metric published by a plugin
 * \endif
 *
 * \if CHINESE
 * @brief 插件发布指标时使用的静态描述信息
 * \endif
 */
struct MetricDescriptor
{
    QString key;
    QString name;
    QString description;
    QString unit;
    MetricValueType type = MetricValueType::String;
};

/**
 * \if ENGLISH
 * @brief One concrete metric sample captured at a point in time
 * \endif
 *
 * \if CHINESE
 * @brief 某一时刻采集到的一条具体指标样本
 * \endif
 */
struct MetricValue
{
    QString key;
    QVariant value;
    QDateTime timestamp;
    QMap<QString, QString> labels;
};

/**
 * \if ENGLISH
 * @brief Batch of metric values published together by one plugin
 * \endif
 *
 * \if CHINESE
 * @brief 某个插件一次性发布的一组指标数据
 * \endif
 */
struct MetricFrame
{
    QString pluginId;
    QDateTime timestamp;
    QList<MetricValue> values;
};

} // namespace Vitals
