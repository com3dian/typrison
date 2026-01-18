/*
MacOS-style-ish minimize/maximize/close buttons.

------+
o o o |
------|

when silent, all 3 buttons are grey;
when mouse entering widget, 3 butons get coloured;
when mouse entering one of the buttons, all 3 buttons start showing icon.
*/


#include "trafficlightwidget.h"

TrafficLightWidget::TrafficLightWidget(QWidget *parent) : QWidget(parent) {
    minimalButton = new TrafficButton(this);
    minimalButton->setIconSize(QSize(8, 8));;
    minimalButton->setFixedSize(14, 14);
    minimalButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: #333333;"
                                "    border-radius: 7px;"
                                "    padding: 0;"
                                "    margin: 0;"
                                "}");
    connect(minimalButton, &QPushButton::clicked, this, &TrafficLightWidget::minimalButtonClicked);
    connect(minimalButton, &QPushButton::clicked, this, &TrafficLightWidget::greyButtons);

    maximalButton = new TrafficButton(this);
    maximalButton->setIconSize(QSize(8, 8));;
    maximalButton->setFixedSize(14, 14);
    maximalButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: #333333;"
                                "    border-radius: 7px;"
                                "    padding: 0;"
                                "    margin: 0;"
                                "}");
    connect(maximalButton, &QPushButton::clicked, this, &TrafficLightWidget::maximalButtonClicked);
    connect(maximalButton, &QPushButton::clicked, this, &TrafficLightWidget::greyButtons);

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
    connect(closeButton, &QPushButton::clicked, this, &TrafficLightWidget::closeButtonClicked);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(10);
    
    layout->addWidget(minimalButton);
    layout->addWidget(maximalButton);
    layout->addWidget(closeButton);

    // Connect the signals:
    connect(minimalButton, &TrafficButton::hoverEntered, this, &TrafficLightWidget::onButtonHovered);
    connect(minimalButton, &TrafficButton::hoverLeft, this, &TrafficLightWidget::onButtonUnhovered);

    connect(maximalButton, &TrafficButton::hoverEntered, this, &TrafficLightWidget::onButtonHovered);
    connect(maximalButton, &TrafficButton::hoverLeft, this, &TrafficLightWidget::onButtonUnhovered);

    connect(closeButton, &TrafficButton::hoverEntered, this, &TrafficLightWidget::onButtonHovered);
    connect(closeButton, &TrafficButton::hoverLeft, this, &TrafficLightWidget::onButtonUnhovered);

}

// Add these methods after the constructor
void TrafficLightWidget::enterEvent(QEnterEvent *event) {
    minimalButton->setIconSize(QSize(8, 8));;
    minimalButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: #F5BD4E;"
                                "    border-radius: 7px;"
                                "    padding: 0;"
                                "    margin: 0;"
                                "}"
                                "QPushButton:pressed {"
                                "    background-color: #f5cd7d;"
                                "}"
                            );

    maximalButton->setIconSize(QSize(8, 8));;
    maximalButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: #62C454;"
                                "    border-radius: 7px;"
                                "    padding: 0;"
                                "    margin: 0;"
                                "}"
                                "QPushButton:pressed {"
                                "    background-color: #83c779;"
                                "}"
                            );
    
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

void TrafficLightWidget::leaveEvent(QEvent *event) {
    minimalButton->setIconSize(QSize(8, 8));;
    minimalButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: #333333;"
                                "    border-radius: 7px;"
                                "    padding: 0;"
                                "    margin: 0;"
                                "}");
    minimalButton->setIcon(QIcon(":/icons/emptyicon.png"));

    maximalButton->setIconSize(QSize(8, 8));;
    maximalButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: #333333;"
                                "    border-radius: 7px;"
                                "    padding: 0;"
                                "    margin: 0;"
                                "}");
    maximalButton->setIcon(QIcon(":/icons/emptyicon.png"));
    
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

void TrafficLightWidget::greyButtons() {
    leaveEvent(nullptr); // Call the leaveEvent method to reset the style
}

void TrafficLightWidget::onButtonHovered() {
    minimalButton->setIcon(QIcon(":/icons/windowminimize.png"));
    maximalButton->setIcon(QIcon(":/icons/windowmaximize.png"));
    closeButton->setIcon(QIcon(":/icons/windowclose.png"));
    this->enterEvent(nullptr);
}

void TrafficLightWidget::onButtonUnhovered() {
    minimalButton->setIcon(QIcon(":/icons/emptyicon.png"));
    maximalButton->setIcon(QIcon(":/icons/emptyicon.png"));
    closeButton->setIcon(QIcon(":/icons/emptyicon.png"));
}