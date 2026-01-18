#include "wikiframe.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QShowEvent>
#include <QPoint>
#include <QSize>

WikiFrame::WikiFrame(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet(
        "WikiFrame {"
        "    background-color: #1F2020;"
        "    border: 1px solid #1F2020;"
        "    border-radius: 4px;"
        "}"
    );

    // Create layout
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 8, 8, 8);
    layout->setSpacing(0);

    // Create editor
    editor = new QMarkdownTextEdit(this);
    editor->setReadOnly(true);  // Make it read-only since it's for display
    editor->changeFontSize(-2); // Reduce font size by 2px from default 14px
    editor->setStyleSheet(
        "QMarkdownTextEdit {"
        "    background-color: #1F2020;"
        "    color: #FFFFFF;"
        "    border: none;"
        "}"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 8px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #262626;"
        "    min-height: 16px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #2C2C2C;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
        "    height: 0px;"
        "    width: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );

    layout->addWidget(editor);

    // Set up shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setColor(QColor("#1F2020"));
    shadow->setOffset(0, 0);
    setGraphicsEffect(shadow);

    // Set fixed width and maximum height for the frame
    setFixedWidth(360);  // Fixed width
    setMinimumHeight(420); // Set a minimum height for the frame
}

void WikiFrame::setContent(const QString& content) {
    // Split content into lines and remove the first line
    QStringList lines = content.split('\n');
    if (!lines.isEmpty()) {
        lines.removeFirst();  // Remove the first line
    }
    
    // Join the remaining lines back together
    editor->setPlainText(lines.join('\n'));
}

void WikiFrame::showEvent(QShowEvent* event) {
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