#include "platform/taskbar/TaskbarMenuDetailWidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QProgressBar>
#include <QVBoxLayout>

namespace Vitals {

namespace {

QString normalizedAccent(const QString& color)
{
    return color.isEmpty() ? QStringLiteral("#0a84ff") : color;
}

QLabel* label(const QString& text, const QString& objectName, QWidget* parent)
{
    auto* widget = new QLabel(text, parent);
    widget->setObjectName(objectName);
    widget->setTextInteractionFlags(Qt::NoTextInteraction);
    return widget;
}

} // namespace

TaskbarMenuDetailWidget::TaskbarMenuDetailWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumWidth(320);
    setMaximumWidth(380);
    setObjectName(QStringLiteral("taskbarDetailMenu"));

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    setStyleSheet(QStringLiteral(R"(
        QWidget#taskbarDetailMenu {
            background: #f5f5f7;
            color: #1d1d1f;
        }
        QFrame#detailPanel {
            background: white;
            border: 1px solid rgba(0, 0, 0, 18);
            border-radius: 8px;
        }
        QLabel#detailTitle {
            color: #1d1d1f;
            font-size: 13px;
            font-weight: 700;
        }
        QLabel#detailSubtitle {
            color: #6e6e73;
            font-size: 11px;
        }
        QLabel#primaryValue {
            color: #1d1d1f;
            font-size: 30px;
            font-weight: 800;
        }
        QLabel#primaryLabel,
        QLabel#badgeLabel,
        QLabel#rowLabel,
        QLabel#sectionTitle {
            color: #6e6e73;
            font-size: 10px;
            font-weight: 700;
            text-transform: uppercase;
        }
        QLabel#badgeValue,
        QLabel#rowValue {
            color: #1d1d1f;
            font-size: 12px;
            font-weight: 700;
        }
        QLabel#rowDetail {
            color: #8e8e93;
            font-size: 10px;
        }
        QFrame#badge {
            background: #f2f2f7;
            border: 1px solid rgba(0, 0, 0, 12);
            border-radius: 7px;
        }
        QProgressBar {
            background: #e5e5ea;
            border: none;
            border-radius: 3px;
            max-height: 6px;
            min-height: 6px;
            text-align: center;
        }
        QProgressBar::chunk {
            background: #0a84ff;
            border-radius: 3px;
        }
    )"));
}

QSize TaskbarMenuDetailWidget::sizeHint() const
{
    if (m_contentSize.isValid()) {
        return m_contentSize;
    }

    return QWidget::sizeHint();
}

QSize TaskbarMenuDetailWidget::minimumSizeHint() const
{
    return sizeHint();
}

void TaskbarMenuDetailWidget::setContents(const QList<TaskbarDetailContent>& contents)
{
    clearLayout(m_layout);

    if (contents.isEmpty()) {
        auto* emptyLabel = label(QStringLiteral("Waiting for metrics"), QStringLiteral("detailSubtitle"), this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        m_layout->addWidget(emptyLabel);
        updateContentGeometry();
        return;
    }

    for (const TaskbarDetailContent& content : contents) {
        if (!content.isEmpty()) {
            m_layout->addWidget(createContentPanel(content));
        }
    }

    updateContentGeometry();
}

QWidget* TaskbarMenuDetailWidget::createContentPanel(const TaskbarDetailContent& content)
{
    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("detailPanel"));
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(10);

    auto* header = new QHBoxLayout();
    header->setSpacing(10);

    auto* titleStack = new QVBoxLayout();
    titleStack->setContentsMargins(0, 0, 0, 0);
    titleStack->setSpacing(2);
    titleStack->addWidget(label(content.title, QStringLiteral("detailTitle"), panel));
    if (!content.subtitle.isEmpty()) {
        auto* subtitle = label(content.subtitle, QStringLiteral("detailSubtitle"), panel);
        subtitle->setWordWrap(true);
        titleStack->addWidget(subtitle);
    }
    header->addLayout(titleStack, 1);

    if (!content.primaryValue.isEmpty()) {
        auto* primaryStack = new QVBoxLayout();
        primaryStack->setContentsMargins(0, 0, 0, 0);
        primaryStack->setSpacing(0);
        auto* value = label(content.primaryValue, QStringLiteral("primaryValue"), panel);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        primaryStack->addWidget(value);
        if (!content.primaryLabel.isEmpty()) {
            auto* primaryLabel = label(content.primaryLabel, QStringLiteral("primaryLabel"), panel);
            primaryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            primaryStack->addWidget(primaryLabel);
        }
        header->addLayout(primaryStack);
    }

    layout->addLayout(header);

    if (!content.badges.isEmpty()) {
        auto* badgeLayout = new QHBoxLayout();
        badgeLayout->setSpacing(6);
        for (const TaskbarDetailBadge& badge : content.badges) {
            badgeLayout->addWidget(createBadge(badge));
        }
        layout->addLayout(badgeLayout);
    }

    for (const TaskbarDetailSection& section : content.sections) {
        if (!section.title.isEmpty()) {
            layout->addWidget(label(section.title, QStringLiteral("sectionTitle"), panel));
        }

        for (const TaskbarDetailRow& row : section.rows) {
            layout->addWidget(createRow(row));
        }
    }

    return panel;
}

QWidget* TaskbarMenuDetailWidget::createBadge(const TaskbarDetailBadge& badge)
{
    auto* frame = new QFrame(this);
    frame->setObjectName(QStringLiteral("badge"));
    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(9, 7, 9, 7);
    layout->setSpacing(2);
    layout->addWidget(label(badge.label, QStringLiteral("badgeLabel"), frame));
    layout->addWidget(label(badge.value, QStringLiteral("badgeValue"), frame));
    return frame;
}

QWidget* TaskbarMenuDetailWidget::createRow(const TaskbarDetailRow& row)
{
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto* top = new QHBoxLayout();
    top->setContentsMargins(0, 0, 0, 0);
    top->setSpacing(8);
    top->addWidget(label(row.label, QStringLiteral("rowLabel"), container), 1);
    auto* value = label(row.value, QStringLiteral("rowValue"), container);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    top->addWidget(value);
    layout->addLayout(top);

    if (!row.detail.isEmpty()) {
        auto* detail = label(row.detail, QStringLiteral("rowDetail"), container);
        detail->setWordWrap(true);
        layout->addWidget(detail);
    }

    if (row.progress >= 0.0) {
        auto* progress = new QProgressBar(container);
        progress->setRange(0, 100);
        progress->setValue(qBound(0, static_cast<int>(row.progress + 0.5), 100));
        progress->setTextVisible(false);
        if (!row.accentColor.isEmpty()) {
            progress->setStyleSheet(QStringLiteral(
                "QProgressBar::chunk { background: %1; border-radius: 3px; }")
                .arg(normalizedAccent(row.accentColor)));
        }
        layout->addWidget(progress);
    }

    return container;
}

void TaskbarMenuDetailWidget::clearLayout(QLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
        }
        delete item;
    }
}

void TaskbarMenuDetailWidget::updateContentGeometry()
{
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);

    if (m_layout) {
        m_layout->invalidate();
        m_layout->activate();
    }

    const QSize preferredSize = m_layout ? m_layout->sizeHint() : QWidget::sizeHint();
    const int preferredWidth = qBound(minimumWidth(), preferredSize.width(), maximumWidth());
    m_contentSize = QSize(preferredWidth, preferredSize.height());
    setFixedSize(m_contentSize);
    resize(m_contentSize);
    updateGeometry();
}

} // namespace Vitals
