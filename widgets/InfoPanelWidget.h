#pragma once

#include <QWidget>

class QLabel;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Compact label-value pair used by hero badges inside InfoPanelWidget
 * \endif
 *
 * \if CHINESE
 * @brief InfoPanelWidget 中 hero 徽标区使用的紧凑标签-数值数据
 * \endif
 */
struct InfoBadgeData
{
    QString label;
    QString value;
};

/**
 * \if ENGLISH
 * @brief One key-value row shown in the details snapshot section
 * \endif
 *
 * \if CHINESE
 * @brief 详情快照区域中展示的一行 key-value 数据
 * \endif
 */
struct InfoRowData
{
    QString key;
    QString value;
};

/**
 * \if ENGLISH
 * @brief One dense information tile rendered in the lower grid area
 * \endif
 *
 * \if CHINESE
 * @brief 下方网格区域中渲染的单个紧凑信息 tile 数据
 * \endif
 */
struct InfoTileData
{
    QString eyebrow;
    QString value;
};

/**
 * \if ENGLISH
 * @brief Reusable host-owned information panel for system-style plugin pages
 *
 * Encapsulates the common layout pattern used by inspector-style plugins:
 * page title, summary text, hero block, key-value snapshot, and dense tiles.
 * Plugins provide content data only, while the host-owned widget controls
 * spacing, alignment, and visual consistency.
 * \endif
 *
 * \if CHINESE
 * @brief 面向系统信息类插件页面的可复用宿主信息面板
 *
 * 该组件封装了检查器风格插件常见的页面结构：标题、副标题、hero 概览区、
 * key-value 快照区以及紧凑 tile 区。插件只需要提供内容数据，布局、对齐
 * 和视觉一致性由宿主侧统一控制。
 * \endif
 */
class InfoPanelWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * \if ENGLISH
     * @brief Creates the reusable information page scaffold
     *
     * The widget owns layout, spacing, and presentation rules for a whole
     * inspector-style page. Callers are expected to feed structured content
     * through the setter APIs instead of manually adding child widgets.
     * \endif
     *
     * \if CHINESE
     * @brief 创建可复用的信息页面骨架组件
     *
     * 该组件负责整页检查器风格页面的布局、间距与展示规则。调用方应通过
     * setter 接口传入结构化内容，而不是手动往内部继续塞子控件。
     * \endif
     */
    explicit InfoPanelWidget(QWidget* parent = nullptr);

    /**
     * \if ENGLISH
     * @brief Sets the page title displayed at the top of the panel
     * \endif
     *
     * \if CHINESE
     * @brief 设置页面顶部显示的主标题
     * \endif
     */
    void setPageTitle(const QString& title);

    /**
     * \if ENGLISH
     * @brief Sets the descriptive subtitle shown below the page title
     * \endif
     *
     * \if CHINESE
     * @brief 设置显示在主标题下方的描述性副标题
     * \endif
     */
    void setPageSubtitle(const QString& subtitle);

    /**
     * \if ENGLISH
     * @brief Sets the section title used by the details snapshot panel
     * \endif
     *
     * \if CHINESE
     * @brief 设置右侧详情快照区域的标题
     * \endif
     */
    void setDetailsTitle(const QString& title);

    /// Sets the small eyebrow text used above the hero title.
    void setHeroEyebrow(const QString& text);

    /// Sets the dominant hero title, usually the primary object name.
    void setHeroTitle(const QString& text);

    /// Sets the secondary summary line shown below the hero title.
    void setHeroSubtitle(const QString& text);

    /// Sets the tertiary metadata line shown below the hero subtitle.
    void setHeroMeta(const QString& text);

    /**
     * \if ENGLISH
     * @brief Replaces the hero badge list with the provided structured content
     * \endif
     *
     * \if CHINESE
     * @brief 使用新的结构化数据替换 hero 徽标列表
     * \endif
     */
    void setBadges(const QList<InfoBadgeData>& badges);

    /**
     * \if ENGLISH
     * @brief Replaces the detail snapshot rows with structured key-value data
     * \endif
     *
     * \if CHINESE
     * @brief 使用结构化 key-value 数据替换详情快照行内容
     * \endif
     */
    void setDetailsRows(const QList<InfoRowData>& rows);

    /**
     * \if ENGLISH
     * @brief Replaces the lower dense tile grid with structured tile content
     * \endif
     *
     * \if CHINESE
     * @brief 使用结构化 tile 数据替换下方紧凑网格内容
     * \endif
     */
    void setTiles(const QList<InfoTileData>& tiles);

private:
    // Builds one badge widget from structured badge data.
    QWidget* createBadge(const InfoBadgeData& badge);

    // Builds one key-value snapshot row from structured row data.
    QWidget* createDetailRow(const InfoRowData& row);

    // Builds one dense tile widget from structured tile data.
    QWidget* createTile(const InfoTileData& tile);

    // Re-renders the hero badge area from cached badge data.
    void rebuildBadges();

    // Re-renders the details snapshot area from cached row data.
    void rebuildDetails();

    // Re-renders the lower tile grid from cached tile data.
    void rebuildTiles();

    // Deletes all child items under a layout before rebuilding it.
    static void clearLayout(QLayout* layout);

    QLabel* m_pageTitleLabel = nullptr;
    QLabel* m_pageSubtitleLabel = nullptr;
    QLabel* m_heroEyebrowLabel = nullptr;
    QLabel* m_heroTitleLabel = nullptr;
    QLabel* m_heroSubtitleLabel = nullptr;
    QLabel* m_heroMetaLabel = nullptr;
    QLabel* m_detailsTitleLabel = nullptr;

    QWidget* m_badgesContainer = nullptr;
    QHBoxLayout* m_badgesLayout = nullptr;
    QWidget* m_detailsRowsContainer = nullptr;
    QVBoxLayout* m_detailsRowsLayout = nullptr;
    QGridLayout* m_tilesLayout = nullptr;

    QList<InfoBadgeData> m_badges;
    QList<InfoRowData> m_detailRows;
    QList<InfoTileData> m_tiles;
};

} // namespace Vitals
