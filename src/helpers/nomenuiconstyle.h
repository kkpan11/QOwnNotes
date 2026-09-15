#ifndef NOMENUICONSTYLE_H
#define NOMENUICONSTYLE_H

#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QProxyStyle>
#include <QStyleOptionMenuItem>

class NoMenuIconStyle : public QProxyStyle {
   public:
    explicit NoMenuIconStyle(QStyle *style = nullptr, bool hideMenuIcons = false,
                             bool fixDarkModeDisabledIcons = false)
        : QProxyStyle(style),
          _hideMenuIcons(hideMenuIcons),
          _fixDarkModeDisabledIcons(fixDarkModeDisabledIcons) {}

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget) const override;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                const QStyleOption *opt) const override;

   private:
    bool _hideMenuIcons;
    bool _fixDarkModeDisabledIcons;
};

#endif    // NOMENUICONSTYLE_H
