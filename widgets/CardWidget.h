#pragma once

#include <QFrame>
#include <QColor>

class QLabel;
class QProgressBar;
class QVBoxLayout;

namespace Vitals {

class CardWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CardWidget(const QString& title, QWidget* parent = nullptr);

    void setTitleText(const QString& title);
    void setValueText(const QString& value);
    void setHintText(const QString& hint);
    void setAccentColor(const QColor& color);
    void setProgressValue(int value);
    void clearProgress();

private:
    QFrame* m_accentStrip = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_valueLabel = nullptr;
    QLabel* m_hintLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
};

} // namespace Vitals
