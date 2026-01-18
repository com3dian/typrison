#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QEnterEvent>
#include <QShowEvent>

class MenuButton : public QPushButton {
    Q_OBJECT

public:
    explicit MenuButton(const QString &leftText = QString(), const QString &rightText = QString(), QWidget *parent = nullptr);

    void setTexts(const QString &leftText, const QString &rightText);

signals:
    void mouseClick();  // Custom signal
    void hovered();
    void notHovered();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent* event) override;

private:
    QLabel *leftLabel;
    QLabel *rightLabel;
};

#endif // MENUBUTTON_H
