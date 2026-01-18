/*


__
  \
   \
    \
     \___
*/

#ifndef PAINTCORNERWIDGET_H
#define PAINTCORNERWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QRect>
#include <QMouseEvent>

class PaintCornerWidget : public QWidget {
    Q_OBJECT
    
private:
    QColor m_backgroundColor;
    bool isFullBlackMode;
    bool isDragging;
    QPoint dragStartPosition;
    
public:
    explicit PaintCornerWidget(QWidget *parent = nullptr) : QWidget(parent), isDragging(false) {
        // Set a fixed width (change 150 to your desired width)
        setFixedWidth(24);
    }

protected:
    // Override paintEvent to customize drawing
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Normal complex path drawing
        int slant = 2;  // Adjust this to control the slant amount
        int radius = 4;  // Corner radius
        int moveLeft = 3;

        QRect rect = this->rect();
        int xRightTopCorner = rect.left() - slant - moveLeft;
        int xRightBottomCorner = rect.left() + slant - moveLeft;

        int yLeftTopCorner = rect.bottom() - 48 + 9;
        int yLeftBottomCorner = rect.bottom() + 2;

        QPainterPath path;
        path.moveTo(xRightBottomCorner, yLeftBottomCorner);  // Start with rounded bottom-left corner

        path.lineTo(xRightTopCorner - 2.5 * radius, yLeftTopCorner);
        path.quadTo(xRightTopCorner - 1.75 * radius, yLeftTopCorner, xRightTopCorner - 1.25 * radius, yLeftTopCorner + radius);  // Rounded top-right corner

        path.lineTo(xRightBottomCorner + 1.25 * radius, yLeftBottomCorner - radius);
        path.quadTo(xRightBottomCorner + 2 * radius, yLeftBottomCorner, xRightBottomCorner + 3 * radius, yLeftBottomCorner);  // Rounded bottom-right corner

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
    
    // Mouse event handlers for window dragging
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
        #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                if (window()->windowHandle()) {
                    window()->windowHandle()->startSystemMove();
                }
        #else
                // fallback for old Qt versions
                isDragging = true;
                dragStartPosition = event->globalPos() - window()->frameGeometry().topLeft();
        #endif
                event->accept();
        }
    }
    
    void mouseMoveEvent(QMouseEvent *event) override {
        #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            if (isDragging && (event->buttons() & Qt::LeftButton)) {
                window()->move(event->globalPos() - dragStartPosition);
                event->accept();
            }
        #endif
    }
    
    void mouseReleaseEvent(QMouseEvent *event) override {
        #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            if (event->button() == Qt::LeftButton) {
                isDragging = false;
                event->accept();
            }
        #endif
    }
};

#endif // PAINTCORNERWIDGET_H
