#ifndef CLOSEBUTTONWIDGET_H
#define CLOSEBUTTONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>
#include <QSize>

#include "../functionbar/trafficbutton.h"

class CloseButtonWidget : public QWidget {
    Q_OBJECT

public:
    explicit CloseButtonWidget(QWidget *parent = nullptr);

private:
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
    void closeButtonClicked();
};

#endif // CLOSEBUTTONWIDGET_H