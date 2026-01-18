#include "instructionframe.h"
#include "../utils/colorpalette.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QShowEvent>
#include <QPoint>
#include <QSize>
#include <QLabel>
#include <QColor>

InstructionFrame::InstructionFrame(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet(
        QString("InstructionFrame {"
        "    background-color: %1;"
        "    border: 1px solid %1;"
        "    border-radius: 4px;"
        "}").arg(ColorPalette::Background::Dark)
    );

    // Create layout
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(0);

    // Create label for instruction text
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    label->setStyleSheet(
        QString("QLabel {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: none;"
        "    font-size: 13px;"
        "    line-height: 1.4;"
        "}").arg(ColorPalette::Background::Dark, ColorPalette::Text::Grey)
    );
    label->setTextInteractionFlags(Qt::NoTextInteraction);

    layout->addWidget(label);

    // Set up shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setColor(ColorPalette::Shadow::defaultColor());
    shadow->setOffset(0, 0);
    setGraphicsEffect(shadow);

    // Set fixed width and adjust height based on content
    setFixedWidth(280);  // Fixed width
    setMinimumHeight(60); // Set a minimum height for the frame
}

void InstructionFrame::setText(const QString& text) {
    label->setText(text);
    // Set label width to match frame width minus margins for proper word wrapping
    // Frame width is fixed at 280px, margins are 16px on each side
    int labelWidth = 280 - 32; // 16px margin on each side
    label->setFixedWidth(labelWidth);
    // Adjust height based on content
    label->adjustSize();
    int contentHeight = label->height();
    int minHeight = qMax(60, contentHeight + 24); // Add padding (12px top + 12px bottom)
    setMinimumHeight(minHeight);
    setMaximumHeight(minHeight); // Set maximum to same as minimum for fixed height
    adjustSize();
}

void InstructionFrame::showEvent(QShowEvent* event) {
    QFrame::showEvent(event);
    
    // Ensure the frame is fully visible within the parent window
    if (QWidget* parentWidget = this->parentWidget()) {
        QPoint pos = this->pos();
        QSize parentSize = parentWidget->size();
        QSize frameSize = this->size();

        // Adjust x-coordinate if needed
        if (pos.x() + frameSize.width() > parentSize.width()) {
            pos.setX(parentSize.width() - frameSize.width());
        }
        if (pos.x() < 0) {
            pos.setX(0);
        }

        // Adjust y-coordinate if needed
        if (pos.y() + frameSize.height() > parentSize.height()) {
            pos.setY(parentSize.height() - frameSize.height());
        }
        if (pos.y() < 0) {
            pos.setY(0);
        }

        // Update position if it was adjusted
        if (pos != this->pos()) {
            move(pos);
        }
    }
}
