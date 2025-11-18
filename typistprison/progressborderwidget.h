#ifndef PROGRESSBORDERWIDGET_H
#define PROGRESSBORDERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QBrush>
#include "prisonermanager.h"

class ProgressBorderWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProgressBorderWidget(QWidget *parent = nullptr, PrisonerManager *prisonerManager = nullptr);
    void activatePrisonerMode(int timeLimit, int wordGoal);
    void deactivatePrisonerMode();
    void startTimerProgress(int timeLimit, int wordGoal);
    void clearTimerProgress();
    void setFullScreen(bool fullScreen);
    void setPrisonerModeForProgress(bool isPrisoner);
    void setTargetWordCount(int wordGoal);

signals:
    void needsRepaint();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateTimerProgress();
    void updateTypingProgress(int wordCount);
    void clearTypingProgress();
private:
    QTimer *prisonerTimer;
    qreal timerProgress;
    qreal totalTime; // total time in seconds
    int targetWordCount;
    qreal typingProgressLengthRatio;
    bool isTimerRunning;
    bool isFullScreen;
    bool isPrisoner;
    qreal startDistance;
    qreal endDistance;
    PrisonerManager *prisonerManager;

    void drawPath(QPainter &painter, const QPainterPath &path, QColor startColor, QColor backgroundColor, QColor endColor);
    void drawPathTest(QPainter &painter, const QPainterPath &path);
    void paintBorder(QPainter &painter,
                     const QRectF &rect,
                     qreal progressLength,
                     int borderMargin,
                     int innerRadius,
                     qreal width,
                     qreal height,
                     QColor startColor,
                     QColor backgroundColor,
                     QColor endColor);
};

#endif // PROGRESSBORDERWIDGET_H