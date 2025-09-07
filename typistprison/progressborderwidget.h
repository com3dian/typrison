#ifndef PROGRESSBORDERWIDGET_H
#define PROGRESSBORDERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include "prisonermanager.h"

class ProgressBorderWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProgressBorderWidget(QWidget *parent = nullptr, PrisonerManager *prisonerManager = nullptr);
    void startTimerProgress(int timeLimit, int wordGoal);
    void clearTimerProgress();
    void setFullScreen(bool fullScreen);

signals:
    void needsRepaint();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateTimerProgress();
    void updateTypingProgress(int wordCount);

private:
    QTimer *prisonerTimer;
    qreal timerProgress;
    qreal totalTime; // total time in seconds
    int targetWordCount;
    qreal typingProgressLengthRatio;
    bool isTimerRunning;
    bool isFullScreen;
    PrisonerManager *prisonerManager;

    void drawPath(QPainter &painter, const QPainterPath &path, QColor color);
    void drawPathTest(QPainter &painter, const QPainterPath &path);
    void paintBorder(QPainter &painter,
                     const QRectF &rect,
                     qreal progressLength,
                     int borderMargin,
                     int innerRadius,
                     qreal width,
                     qreal height,
                     QColor color);
};

#endif // PROGRESSBORDERWIDGET_H