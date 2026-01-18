// TrafficButton.h
#pragma once

#include <QPushButton>

class TrafficButton : public QPushButton {
    Q_OBJECT
public:
    explicit TrafficButton(QWidget *parent = nullptr) : QPushButton(parent) {}

signals:
    void hoverEntered();
    void hoverLeft();

protected:
    void enterEvent(QEnterEvent *event) override {
        emit hoverEntered();
        QPushButton::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override {
        emit hoverLeft();
        QPushButton::leaveEvent(event);
    }
};
