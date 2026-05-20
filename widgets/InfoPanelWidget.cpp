#include "InfoPanelWidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace Vitals {

namespace {

QLabel* createSelectableLabel(QWidget* parent, const QString& objectName)
{
    auto* label = new QLabel(parent);
    label->setObjectName(objectName);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    return label;
}

} // namespace

InfoPanelWidget::InfoPanelWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(680, 620);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // The panel is composed of three coordinated zones:
    // top page header, hero/details row, and the lower dense tile grid.
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* content = new QWidget(scrollArea);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(22, 18, 22, 20);
    contentLayout->setSpacing(10);

    m_pageTitleLabel = new QLabel(content);
    m_pageTitleLabel->setObjectName(QStringLiteral("pageTitle"));

    m_pageSubtitleLabel = new QLabel(content);
    m_pageSubtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    m_pageSubtitleLabel->setWordWrap(true);

    auto* contentGrid = new QGridLayout();
    contentGrid->setHorizontalSpacing(12);
    contentGrid->setVerticalSpacing(10);
    contentGrid->setColumnStretch(0, 1);
    contentGrid->setColumnStretch(1, 1);

    // The left column is the hero summary block for the primary object.
    auto* heroPanel = new QFrame(content);
    heroPanel->setObjectName(QStringLiteral("systemHeroPanel"));
    heroPanel->setMinimumSize(310, 168);
    heroPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* heroLayout = new QVBoxLayout(heroPanel);
    heroLayout->setContentsMargins(16, 14, 16, 14);
    heroLayout->setSpacing(6);

    m_heroEyebrowLabel = new QLabel(heroPanel);
    m_heroEyebrowLabel->setObjectName(QStringLiteral("systemHeroEyebrow"));

    m_heroTitleLabel = new QLabel(heroPanel);
    m_heroTitleLabel->setObjectName(QStringLiteral("systemHeroTitle"));
    m_heroTitleLabel->setWordWrap(true);
    m_heroTitleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_heroSubtitleLabel = new QLabel(heroPanel);
    m_heroSubtitleLabel->setObjectName(QStringLiteral("systemHeroSubtitle"));
    m_heroSubtitleLabel->setWordWrap(true);

    m_heroMetaLabel = new QLabel(heroPanel);
    m_heroMetaLabel->setObjectName(QStringLiteral("systemHeroMeta"));
    m_heroMetaLabel->setWordWrap(true);

    m_badgesContainer = new QWidget(heroPanel);
    m_badgesLayout = new QHBoxLayout(m_badgesContainer);
    m_badgesLayout->setContentsMargins(0, 0, 0, 0);
    m_badgesLayout->setSpacing(8);

    heroLayout->addWidget(m_heroEyebrowLabel);
    heroLayout->addWidget(m_heroTitleLabel);
    heroLayout->addWidget(m_heroSubtitleLabel);
    heroLayout->addWidget(m_heroMetaLabel);
    heroLayout->addSpacing(2);
    heroLayout->addWidget(m_badgesContainer);
    heroLayout->addStretch();

    // The right column is a dense key-value snapshot for quick scanning.
    auto* detailsPanel = new QFrame(content);
    detailsPanel->setObjectName(QStringLiteral("systemDetailsPanel"));
    detailsPanel->setMinimumSize(310, 168);
    detailsPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* detailsLayout = new QVBoxLayout(detailsPanel);
    detailsLayout->setContentsMargins(16, 14, 16, 14);
    detailsLayout->setSpacing(8);

    m_detailsTitleLabel = new QLabel(detailsPanel);
    m_detailsTitleLabel->setObjectName(QStringLiteral("systemSectionTitle"));

    m_detailsRowsContainer = new QWidget(detailsPanel);
    m_detailsRowsLayout = new QVBoxLayout(m_detailsRowsContainer);
    m_detailsRowsLayout->setContentsMargins(0, 0, 0, 0);
    m_detailsRowsLayout->setSpacing(7);

    detailsLayout->addWidget(m_detailsTitleLabel);
    detailsLayout->addWidget(m_detailsRowsContainer);
    detailsLayout->addStretch(1);

    // The lower grid hosts repeatable compact tiles that plugins can swap freely.
    auto* tilesContainer = new QWidget(content);
    tilesContainer->setMinimumWidth(640);
    tilesContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_tilesLayout = new QGridLayout(tilesContainer);
    m_tilesLayout->setContentsMargins(0, 0, 0, 0);
    m_tilesLayout->setHorizontalSpacing(12);
    m_tilesLayout->setVerticalSpacing(10);
    m_tilesLayout->setColumnStretch(0, 1);
    m_tilesLayout->setColumnStretch(1, 1);

    contentGrid->addWidget(heroPanel, 0, 0);
    contentGrid->addWidget(detailsPanel, 0, 1);
    contentGrid->addWidget(tilesContainer, 1, 0, 1, 2);

    contentLayout->addWidget(m_pageTitleLabel);
    contentLayout->addWidget(m_pageSubtitleLabel);
    contentLayout->addLayout(contentGrid);
    contentLayout->addStretch(1);

    scrollArea->setWidget(content);
    rootLayout->addWidget(scrollArea);
}

void InfoPanelWidget::setPageTitle(const QString& title)
{
    m_pageTitleLabel->setText(title);
}

void InfoPanelWidget::setPageSubtitle(const QString& subtitle)
{
    m_pageSubtitleLabel->setText(subtitle);
}

void InfoPanelWidget::setDetailsTitle(const QString& title)
{
    m_detailsTitleLabel->setText(title);
}

void InfoPanelWidget::setHeroEyebrow(const QString& text)
{
    m_heroEyebrowLabel->setText(text);
}

void InfoPanelWidget::setHeroTitle(const QString& text)
{
    m_heroTitleLabel->setText(text);
}

void InfoPanelWidget::setHeroSubtitle(const QString& text)
{
    m_heroSubtitleLabel->setText(text);
}

void InfoPanelWidget::setHeroMeta(const QString& text)
{
    m_heroMetaLabel->setText(text);
}

void InfoPanelWidget::setBadges(const QList<InfoBadgeData>& badges)
{
    m_badges = badges;
    rebuildBadges();
}

void InfoPanelWidget::setDetailsRows(const QList<InfoRowData>& rows)
{
    m_detailRows = rows;
    rebuildDetails();
}

void InfoPanelWidget::setTiles(const QList<InfoTileData>& tiles)
{
    m_tiles = tiles;
    rebuildTiles();
}

QWidget* InfoPanelWidget::createBadge(const InfoBadgeData& badge)
{
    // Badges are intentionally lightweight so plugins can surface 1-3 headline facts.
    auto* frame = new QFrame(m_badgesContainer);
    frame->setObjectName(QStringLiteral("systemBadge"));

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(10, 7, 10, 7);
    layout->setSpacing(2);

    auto* label = new QLabel(badge.label, frame);
    label->setObjectName(QStringLiteral("systemBadgeLabel"));
    label->setMinimumWidth(0);

    auto* value = new QLabel(badge.value, frame);
    value->setObjectName(QStringLiteral("systemBadgeValue"));
    value->setMinimumWidth(0);
    value->setWordWrap(true);

    layout->addWidget(label);
    layout->addWidget(value);
    return frame;
}

QWidget* InfoPanelWidget::createDetailRow(const InfoRowData& row)
{
    // Detail rows are rebuilt from plain data objects so plugin pages do not own layout code.
    auto* container = new QWidget(m_detailsRowsContainer);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* keyLabel = new QLabel(row.key, container);
    keyLabel->setObjectName(QStringLiteral("systemInfoKeyLabel"));

    auto* valueLabel = createSelectableLabel(container, QStringLiteral("systemInfoValueLabel"));
    valueLabel->setText(row.value);

    layout->addWidget(keyLabel, 0, Qt::AlignTop);
    layout->addWidget(valueLabel, 1);
    return container;
}

QWidget* InfoPanelWidget::createTile(const InfoTileData& tile)
{
    // Tiles are fixed-height by default to keep the lower grid visually stable.
    auto* frame = new QFrame(this);
    frame->setObjectName(QStringLiteral("systemInfoTile"));
    frame->setMinimumHeight(76);
    frame->setMinimumWidth(300);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(12, 9, 12, 9);
    layout->setSpacing(3);

    auto* eyebrow = new QLabel(tile.eyebrow, frame);
    eyebrow->setObjectName(QStringLiteral("systemInfoTileEyebrow"));

    auto* value = createSelectableLabel(frame, QStringLiteral("systemInfoTileValue"));
    value->setText(tile.value);

    layout->addWidget(eyebrow);
    layout->addWidget(value);
    return frame;
}

void InfoPanelWidget::rebuildBadges()
{
    // Setter APIs replace the entire content model, then trigger a focused rebuild.
    clearLayout(m_badgesLayout);
    for (const InfoBadgeData& badge : m_badges) {
        m_badgesLayout->addWidget(createBadge(badge));
    }
    m_badgesLayout->addStretch(1);
}

void InfoPanelWidget::rebuildDetails()
{
    clearLayout(m_detailsRowsLayout);
    for (const InfoRowData& row : m_detailRows) {
        m_detailsRowsLayout->addWidget(createDetailRow(row));
    }
}

void InfoPanelWidget::rebuildTiles()
{
    clearLayout(m_tilesLayout);
    for (int index = 0; index < m_tiles.size(); ++index) {
        m_tilesLayout->addWidget(createTile(m_tiles.at(index)), index / 2, index % 2);
    }
}

void InfoPanelWidget::clearLayout(QLayout* layout)
{
    // This utility lets the panel fully regenerate dynamic sections from data models.
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        delete item;
    }
}

} // namespace Vitals
