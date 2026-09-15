#include "nomenuiconstyle.h"

#include <QApplication>
#include <QPalette>

void NoMenuIconStyle::drawControl(ControlElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const {
    if (_hideMenuIcons && element == CE_MenuItem && widget && qobject_cast<const QMenu *>(widget)) {
        if (const auto *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
#ifdef __clang__
#if __has_warning("-Wdeprecated-copy-with-user-provided-copy")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#endif
#endif
            QStyleOptionMenuItem opt(*menuItem);    // Copy original option
#ifdef __clang__
#if __has_warning("-Wdeprecated-copy-with-user-provided-copy")
#pragma clang diagnostic pop
#endif
#endif
            opt.icon = QIcon();      // Remove the icon
            opt.maxIconWidth = 0;    // Prevent space allocation

            QProxyStyle::drawControl(element, &opt, painter, widget);
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

QPixmap NoMenuIconStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                             const QStyleOption *opt) const {
    if (!_fixDarkModeDisabledIcons || iconMode != QIcon::Disabled || pixmap.isNull()) {
        return QProxyStyle::generatedIconPixmap(iconMode, pixmap, opt);
    }

    const QColor disabledColor =
        opt ? opt->palette.color(QPalette::Disabled, QPalette::ButtonText)
            : QApplication::palette().color(QPalette::Disabled, QPalette::ButtonText);

    QPixmap disabledPixmap(pixmap.size());
    disabledPixmap.setDevicePixelRatio(pixmap.devicePixelRatio());
    disabledPixmap.fill(Qt::transparent);

    QPainter painter(&disabledPixmap);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawPixmap(0, 0, pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    QColor tintedDisabledColor = disabledColor;
    tintedDisabledColor.setAlphaF(0.7);
    painter.fillRect(disabledPixmap.rect(), tintedDisabledColor);

    return disabledPixmap;
}
