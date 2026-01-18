#ifndef UTILSPRISONERDIALOG_H
#define UTILSPRISONERDIALOG_H

#include <QSplitter>
#include <QSplitterHandle>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

class CustomSplitterHandle : public QSplitterHandle {
public:
    CustomSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
protected:
    void paintEvent(QPaintEvent *event) override;
};

class CustomSplitter : public QSplitter {
public:
    CustomSplitter(Qt::Orientation orientation, QWidget *parent = nullptr);
protected:
    QSplitterHandle *createHandle() override;
};

class RoundedWidget : public QWidget {
public:
    explicit RoundedWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // UTILSPRISONERDIALOG_H