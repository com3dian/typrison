#ifndef TRAFFICLIGHTWIDGET_H
#define TRAFFICLIGHTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>
#include <QSize>

#include "trafficbutton.h"

class TrafficLightWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrafficLightWidget(QWidget *parent = nullptr);

private:
    TrafficButton *minimalButton;
    TrafficButton *maximalButton;
    TrafficButton *closeButton;

private slots:
    void greyButtons();
    void onButtonHovered();
    void onButtonUnhovered();

// Add these methods to the class declaration in the header
protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void minimalButtonClicked();
    void maximalButtonClicked();
    void closeButtonClicked();
};

#endif // TRAFFICLIGHTWIDGET_H