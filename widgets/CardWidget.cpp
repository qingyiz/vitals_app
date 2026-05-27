#include "CardWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace Vitals {

CardWidget::CardWidget(const QString& title, QWidget* parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setObjectName(QStringLiteral("metricCard"));
    setMinimumHeight(82);
    setMinimumWidth(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_accentStrip = new QFrame(this);
    m_accentStrip->setObjectName(QStringLiteral("cardAccent"));
    m_accentStrip->setFixedWidth(4);

    auto* layout = new QVBoxLayout();
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(5);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("cardTitle"));
    m_titleLabel->setMinimumWidth(0);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_valueLabel = new QLabel(QStringLiteral("--"), this);
    m_valueLabel->setObjectName(QStringLiteral("cardValue"));
    m_valueLabel->setMinimumWidth(0);
    m_valueLabel->setWordWrap(true);
    m_valueLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setObjectName(QStringLiteral("cardHint"));
    m_hintLabel->setMinimumWidth(0);
    m_hintLabel->setWordWrap(true);
    m_hintLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName(QStringLiteral("cardProgress"));
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(4);
    m_progressBar->hide();

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_valueLabel);
    layout->addWidget(m_progressBar);
    layout->addWidget(m_hintLabel);

    outerLayout->addWidget(m_accentStrip);
    outerLayout->addLayout(layout, 1);
}

void CardWidget::setValueText(const QString& value)
{
    m_valueLabel->setText(value);
    m_valueLabel->setToolTip(value);
}

void CardWidget::setTitleText(const QString& title)
{
    m_titleLabel->setText(title);
}

void CardWidget::setHintText(const QString& hint)
{
    m_hintLabel->setText(hint);
    m_hintLabel->setToolTip(hint);
}

void CardWidget::setAccentColor(const QColor& color)
{
    const QString colorName = color.name();
    m_accentStrip->setStyleSheet(QStringLiteral("background: %1; border-top-left-radius: 8px; border-bottom-left-radius: 8px;").arg(colorName));
    m_progressBar->setStyleSheet(QStringLiteral(R"(
        QProgressBar#cardProgress {
            background: #e8ebef;
            border: none;
            border-radius: 2px;
        }
        QProgressBar#cardProgress::chunk {
            background: %1;
            border-radius: 2px;
        }
    )").arg(colorName));
}

void CardWidget::setProgressValue(int value)
{
    m_progressBar->setValue(qBound(0, value, 100));
    m_progressBar->show();
}

void CardWidget::clearProgress()
{
    m_progressBar->hide();
}

} // namespace Vitals
