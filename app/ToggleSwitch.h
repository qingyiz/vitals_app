#pragma once

#include <QAbstractButton>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Lightweight host switch control used for plugin enable toggles
 * \endif
 *
 * \if CHINESE
 * @brief 用于插件启用开关的轻量宿主切换控件
 * \endif
 */
class ToggleSwitch : public QAbstractButton
{
    Q_OBJECT

public:
    explicit ToggleSwitch(QWidget* parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace Vitals
