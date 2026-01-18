/*

Widget for painting bottom left corner of tabbar in functionbar

  ___
 /
 |
 |
_/



*/

#ifndef PAINTLEFTEDGEWIDGET_H
#define PAINTLEFTEDGEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QRect>

class PaintLeftEdgeWidget : public QWidget {
    Q_OBJECT
public:
    explicit PaintLeftEdgeWidget(QWidget *parent = nullptr) : QWidget(parent) {
        // Set a fixed width
        setFixedWidth(8);
    }

protected:
    // Override paintEvent to customize drawing
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // custom painting
        int slant = 2;  // Adjust this to control the slant amount
        int radius = 4;  // Corner radius
        int moveRight = 1;

        QRect rect = this->rect();
        int xLeftBottomCorner = rect.right() - 2 * radius + moveRight;
        int yLeftBottomCorner = rect.bottom() + 2;

        int xLeftTopCorner = rect.right() + moveRight;
        int yLeftTopCorner = rect.bottom() - 48 + 9 ;

        QPainterPath path;
        path.moveTo(xLeftBottomCorner, yLeftBottomCorner);  // Start with rounded bottom-left corner
        path.quadTo(xLeftTopCorner, yLeftBottomCorner, xLeftTopCorner, yLeftBottomCorner - 2*radius);  // Rounded corner

        path.lineTo(xLeftTopCorner, yLeftTopCorner + radius);
        path.quadTo(xLeftTopCorner, yLeftTopCorner, xLeftTopCorner + radius, yLeftTopCorner);  // Rounded corner

        path.lineTo(xLeftBottomCorner + 48, yLeftBottomCorner);

        path.closeSubpath();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#2c2c2c"));  // Background color for selected tab
        painter.drawPath(path);

        // Draw border by setting a pen
        QPen borderPen(QColor("#2c2c2c"), 2); 
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
};

#endif // PAINTLEFTEDGEWIDGET_H
