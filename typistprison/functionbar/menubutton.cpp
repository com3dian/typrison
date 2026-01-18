/*
Buttons in 
    1. drop down (menu) list
    2. right click menu

+-------------------+
|  New...   Ctrl+N  |
|  Open     Ctrl+Z  |
|  Save     Ctrl+S  |
|  Search   Ctrl+F  |
+-------------------+

when hovering on this button, `leftText` will be highlighted and `rightText` will show.
*/


#include "menubutton.h"

MenuButton::MenuButton(const QString &leftText, const QString &rightText, QWidget *parent)
    : QPushButton(parent) 
{
    setStyleSheet("QPushButton { border: none; padding: 0px; background-color: transparent; }");

    leftLabel = new QLabel(leftText, this);
    rightLabel = new QLabel(rightText, this);

    leftLabel->setStyleSheet("color: #BDBDBD; background-color: transparent;");
    rightLabel->setStyleSheet("color: transparent; background-color: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(leftLabel);
    layout->addStretch();
    layout->addWidget(rightLabel);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
    setAttribute(Qt::WA_Hover);

    // Connect QPushButton's clicked signal to emit our custom mouseClick signal
    connect(this, &QPushButton::clicked, this, &MenuButton::mouseClick);
}

void MenuButton::setTexts(const QString &leftText, const QString &rightText) {
    leftLabel->setText(leftText);
    rightLabel->setText(rightText);
}

void MenuButton::enterEvent(QEnterEvent *event) {
    if (isEnabled()) {
        leftLabel->setStyleSheet("color: #FFFFFF; background-color: transparent;");
        rightLabel->setStyleSheet("color: #656565; background-color: transparent;");
        emit hovered();
    }
    QPushButton::enterEvent(event);
}

void MenuButton::leaveEvent(QEvent *event) {
    if (isEnabled()) {
        leftLabel->setStyleSheet("color: #BDBDBD; background-color: transparent;");
        rightLabel->setStyleSheet("color: transparent; background-color: transparent;");
        emit notHovered();
    }
    QPushButton::leaveEvent(event);
}

void MenuButton::showEvent(QShowEvent *event) {
    QPushButton::showEvent(event);
    
    // Mac-specific fix: Force initial label styling to ensure proper text rendering
    #ifdef Q_OS_MAC
    if (isEnabled()) {
        if (leftLabel) {
            leftLabel->setStyleSheet("color: #BDBDBD; background-color: transparent;");
            leftLabel->update();
        }
    } else {
        if (leftLabel) {
            leftLabel->setStyleSheet("color: #656565; background-color: transparent;");
            leftLabel->update();
        }
    }
    if (rightLabel) {
        rightLabel->setStyleSheet("color: transparent; background-color: transparent;");
        rightLabel->update();
    }
    #endif
}

void MenuButton::changeEvent(QEvent* event) {
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        if (!isEnabled()) {
            leftLabel->setStyleSheet("color: #656565; background-color: transparent;");
            rightLabel->setStyleSheet("color: transparent; background-color: transparent;");
        } else {
            leftLabel->setStyleSheet("color: #BDBDBD; background-color: transparent;");
            rightLabel->setStyleSheet("color: transparent; background-color: transparent;");
        }
    }
}
