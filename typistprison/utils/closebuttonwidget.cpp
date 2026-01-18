/*
MacOS-style-ish minimize/maximize/close buttons.

---+
 o |
   |

when silent, the button is grey;
when mouse entering widget, the button gets coloured;
when mouse entering the button, the button starts showing icon.
*/


#include "closebuttonwidget.h"
#include "../functionbar/trafficbutton.h"

CloseButtonWidget::CloseButtonWidget(QWidget *parent) : QWidget(parent) {
    closeButton = new TrafficButton(this);
    closeButton->setIconSize(QSize(8, 8));;
    closeButton->setFixedSize(14, 14);
    closeButton->setStyleSheet("QPushButton {"
                              "    border: none;"
                              "    background-color: #333333;"
                              "    border-radius: 7px;"
                              "    padding: 0;"
                              "    margin: 0;"
                              "}");
    connect(closeButton, &QPushButton::clicked, this, &CloseButtonWidget::closeButtonClicked);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 0, 8);
    layout->setSpacing(10);
    layout->addWidget(closeButton);

    // Connect the signals:
    connect(closeButton, &TrafficButton::hoverEntered, this, &CloseButtonWidget::onButtonHovered);
    connect(closeButton, &TrafficButton::hoverLeft, this, &CloseButtonWidget::onButtonUnhovered);

}

// Add these methods after the constructor
void CloseButtonWidget::enterEvent(QEnterEvent *event) {
    closeButton->setIconSize(QSize(8, 8));;
    closeButton->setStyleSheet("QPushButton {"
                              "    border: none;"
                              "    background-color: #EE6A5E;"
                              "    border-radius: 7px;"
                              "    padding: 0;"
                              "    margin: 0;"
                              "}"
                              "QPushButton:pressed {"
                              "    background-color: #f08075;"
                              "}"
                            );

    QWidget::enterEvent(event);
}

void CloseButtonWidget::leaveEvent(QEvent *event) {
    closeButton->setIconSize(QSize(8, 8));;
    closeButton->setStyleSheet("QPushButton {"
                              "    border: none;"
                              "    background-color: #333333;"
                              "    border-radius: 7px;"
                              "    padding: 0;"
                              "    margin: 0;"
                              "}");
    closeButton->setIcon(QIcon(":/icons/emptyicon.png"));
    QWidget::leaveEvent(event);
}

void CloseButtonWidget::greyButtons() {
    leaveEvent(nullptr); // Call the leaveEvent method to reset the style
}

void CloseButtonWidget::onButtonHovered() {
    closeButton->setIcon(QIcon(":/icons/windowclose.png"));
    this->enterEvent(nullptr);
}

void CloseButtonWidget::onButtonUnhovered() {
    closeButton->setIcon(QIcon(":/icons/emptyicon.png"));
}