#ifndef INSTRUCTIONFRAME_H
#define INSTRUCTIONFRAME_H

#include <QFrame>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QDebug>
#include <QLabel>

class InstructionFrame : public QFrame {
    Q_OBJECT

private:
    QLabel* label;
    QVBoxLayout* layout;

public:
    explicit InstructionFrame(QWidget *parent = nullptr);
    void setText(const QString& text);

protected:
    void showEvent(QShowEvent* event) override;
};

#endif // INSTRUCTIONFRAME_H
