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

public slots:
    void requestRepaint();
    void updateTypingProgress(int typingWordCount);
    void setFullScreen(bool fullScreen);
    void setPrisonerModeForProgress(bool isPrisoner);
    void setTargetWordCount(int wordGoal);


signals:
    void needsRepaint();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void startTimerProgress(int timeLimit, int wordGoal);
    void clearTimerProgress();
    void updateTimerProgress();
    void clearTypingProgress();
    void paintBorder(QPainter &painter, const QRectF &rect, qreal progressLength, int borderMargin, int innerRadius, qreal width, qreal height, QColor startColor, QColor backgroundColor, QColor endColor);
    void drawPath(QPainter &painter, const QPainterPath &path, QColor startColor, QColor backgroundColor, QColor endColor);
    void drawPathTest(QPainter &painter, const QPainterPath &path);

    QTimer *prisonerTimer;
    qreal timerProgress;
    bool isTimerRunning;
    bool isFullScreen;
    bool isPrisoner;
    PrisonerManager *prisonerManager;
    int baseWordCount;

    qreal typingProgressLengthRatio;
    int targetWordCount;
    qreal totalTime;
    QTimer *repaintTimer;

    qreal startDistance;
    qreal endDistance;
};

#endif // PROGRESSBORDERWIDGET_H