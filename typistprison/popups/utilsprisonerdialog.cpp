#include "utilsprisonerdialog.h"

CustomSplitterHandle::CustomSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
    : QSplitterHandle(orientation, parent) {}

void CustomSplitterHandle::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set the handle color to match the text edit
    painter.setBrush(QBrush(QColor("#1F2020"))); // Dark gray
    painter.setPen(Qt::NoPen);

    // Draw a rounded rectangle with rounded corners only on the right side
    QRect r = rect();
    QPainterPath path;
    path.moveTo(r.left(), r.top());
    path.lineTo(r.right() - 4, r.top());
    path.arcTo(r.right() - 8, r.top(), 8, 8, 90, -90);
    path.lineTo(r.right(), r.bottom() - 2);
    path.arcTo(r.right() - 8, r.bottom() - 7, 8, 8, 0, -90);
    path.lineTo(r.left(), r.bottom()+1);
    path.closeSubpath();

    painter.drawPath(path);

    QPainterPath secondPath;
    int inset = 4;  // Adjust inset for positioning
    QRect innerRect(r.left() + inset, r.top()+r.height()/3, 2, r.height()/3);
    secondPath.addRoundedRect(innerRect, 1, 1);
    painter.setBrush(QBrush(QColor("#BDBDBD")));
    painter.drawPath(secondPath);
}

CustomSplitter::CustomSplitter(Qt::Orientation orientation, QWidget *parent)
    : QSplitter(orientation, parent) {}

QSplitterHandle* CustomSplitter::createHandle() {
    return new CustomSplitterHandle(orientation(), this);
}

RoundedWidget::RoundedWidget(QWidget *parent)
    : QWidget(parent) {
    setStyleSheet("background-color: #f0f0f0; border-radius: 6px;");
}

void RoundedWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush brush(Qt::lightGray);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
}