#include "customtreestyle.h"
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QDir>

// Constructor that accepts paths to the closed and open folder icons
CustomTreeStyle::CustomTreeStyle(const QString& closedIconPath, const QString& openIconPath)
    : QProxyStyle(), closedIconPath(closedIconPath), openIconPath(openIconPath) {}

// Overriding the drawPrimitive function to replace the branch indicator
void CustomTreeStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_IndicatorBranch) {
        const QStyleOptionViewItem *viewItemOption = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        if (viewItemOption) {
            // Define margins for the icons (left, top, right, bottom)
            int margin = 5;  // Adjust margin size as needed

            QRect iconRect = option->rect.adjusted(margin, margin, -margin, -margin);  // Apply margin to the rect

            // Draw the custom icon based on expanded/collapsed state
            QPixmap pixmap;
            if (viewItemOption->state & QStyle::State_Children) {
                if (viewItemOption->state & QStyle::State_Open) {
                    pixmap.load(openIconPath);  // Load the expanded icon
                } else {
                    pixmap.load(closedIconPath); // Load the collapsed icon
                }
                
                // Maintain the original width/height ratio
                QSize pixmapSize = pixmap.size();
                QSize scaledSize;

                // Scale the icon to fit within the iconRect while maintaining aspect ratio
                if (iconRect.width() > iconRect.height()) {
                    scaledSize.setWidth(iconRect.height() * pixmapSize.width() / pixmapSize.height());
                    scaledSize.setHeight(iconRect.height());
                } else {
                    scaledSize.setWidth(iconRect.width());
                    scaledSize.setHeight(iconRect.width() * pixmapSize.height() / pixmapSize.width());
                }

                // Center the pixmap in the iconRect
                QRect scaledRect = QRect(
                    iconRect.center() - QPoint(scaledSize.width() / 2, scaledSize.height() / 2),
                    scaledSize
                );

                painter->drawPixmap(scaledRect, pixmap);  // Draw the icon with the maintained aspect ratio
            }
            return;
        }
    }
    // Fallback to default style if not the branch indicator
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
