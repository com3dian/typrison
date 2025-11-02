#include "progressborderwidget.h"

ProgressBorderWidget::ProgressBorderWidget(QWidget *parent, PrisonerManager *prisonerManager)
    : QWidget(parent),
    timerProgress(0.0),
    isTimerRunning(false),
    isFullScreen(false),
    isPrisoner(false),
    typingProgressLengthRatio(0.0),
    prisonerManager(prisonerManager)
{
    prisonerTimer = new QTimer(this);
    connect(prisonerTimer,
            &QTimer::timeout,
            this,
            &ProgressBorderWidget::updateTimerProgress);

    if (prisonerManager) {
        // update the typ b ing progress in the progress border widget
        connect(prisonerManager,
        &PrisonerManager::updateTypingProgressInProgressBorder,
        this,
        &ProgressBorderWidget::updateTypingProgress);
        
        // if prisoner mode failed, deactivate the prisoner mode
        connect(prisonerManager,
        &PrisonerManager::prisonerModeFailed,
        this,
        &ProgressBorderWidget::deactivatePrisonerMode);
    }
}

/*
activate the prisoner mode

get full screen; start timer progress
*/

void ProgressBorderWidget::activatePrisonerMode(int timeLimit, int wordGoal) {
    isPrisoner = true;

    this->startTimerProgress(timeLimit, wordGoal);
    update(); // Update to show current typing progress
    repaint();
}

void ProgressBorderWidget::deactivatePrisonerMode() {
    isPrisoner = false;

    this->clearTimerProgress();
    this->clearTypingProgress();
    repaint();
}

void ProgressBorderWidget::startTimerProgress(int timeLimit, int wordGoal) {
    timerProgress = 0.0;
    // convert minutes time to seconds
    totalTime = timeLimit * 60 * 10;
    targetWordCount = wordGoal;
    prisonerTimer->start(100); // update interval
    isTimerRunning = true;
}

void ProgressBorderWidget::clearTimerProgress() {
    prisonerTimer->stop();
    timerProgress = 0;
    update(); // repaint
    emit needsRepaint();
    isTimerRunning = false;
    isPrisoner = false;
}

void ProgressBorderWidget::updateTimerProgress() {
    timerProgress = timerProgress + 1;
    update(); // repaint
    emit needsRepaint();

    if (prisonerManager && totalTime > 0.0) {
        qreal normalizedProgress = timerProgress / totalTime;
        prisonerManager->updateTimerProgress(normalizedProgress);
    }
}

void ProgressBorderWidget::updateTypingProgress(int wordCount) {
    if (targetWordCount > 0) {
        typingProgressLengthRatio = static_cast<qreal>(wordCount) / targetWordCount;
    } else {
        typingProgressLengthRatio = 0.0;
    }
    
    // Only update visual progress if prisoner mode is actually active
    if (isPrisoner) {
        update();
        emit needsRepaint();
    }
}

void ProgressBorderWidget::clearTypingProgress() {
    typingProgressLengthRatio = 0.0;
    update();
    emit needsRepaint();
}

/*
paints border with a color

+--------------+
               |
               |
               |
   <-----------+
*/
void ProgressBorderWidget::paintBorder(QPainter &painter,
                                       const QRectF &rect,
                                       qreal progressLength,
                                       int borderMargin,
                                       int innerRadius,
                                       qreal width,
                                       qreal height,
                                       QColor color) {
    QPointF topLeftArcFrom = rect.topLeft() + QPointF(borderMargin, 2 * borderMargin + innerRadius);
    QPointF topLeftArcTo = rect.topLeft() + QPointF(2 * borderMargin + innerRadius, borderMargin);

    QPointF topRightArcFrom = rect.topRight() + QPointF(- 2 * borderMargin - innerRadius, borderMargin);
    QPointF topRightArcTo = rect.topRight() + QPointF(- borderMargin, 2 * borderMargin + innerRadius);

    QPointF bottomRightArcFrom = rect.bottomRight() + QPointF(-borderMargin, - 2 * borderMargin - innerRadius);
    QPointF bottomRightArcTo = rect.bottomRight() + QPointF(- 2 * borderMargin - innerRadius, -borderMargin);

    QPointF bottomLeftArcFrom = rect.bottomLeft() + QPointF(2 * borderMargin + innerRadius, -borderMargin);
    QPointF bottomLeftArcTo = rect.bottomLeft() + QPointF(borderMargin, - 2 * borderMargin - innerRadius);

    startDistance = 120;
    endDistance = 60;
    QPointF startPoint = topLeftArcTo + QPointF(startDistance, 0);
    QPointF endPoint = topLeftArcFrom + QPointF(0, endDistance);
    QPainterPath path;
    // start from a distance from the top left corner
    path.moveTo(startPoint);

    qreal remaining = progressLength;

    qreal topEdge = width - 2 * borderMargin - startDistance;
    if (remaining <= topEdge) {
        path.lineTo(startPoint + QPointF(remaining, 0));
        drawPath(painter, path, color);
        return;
    } else {
        path.lineTo(topRightArcFrom);
        remaining -= topEdge;
    }

    qreal cornerArcRadius = borderMargin + innerRadius;
    qreal cornerArcLength = 3.1416 * cornerArcRadius * 0.5;
    QPointF topRightCenter = topRightArcFrom + QPointF(0, cornerArcRadius);
    
    QRectF topRightRect(topRightCenter.x() - cornerArcRadius,
                        topRightCenter.y() - cornerArcRadius,
                        2 * cornerArcRadius,
                        2 * cornerArcRadius);

    if (remaining <= cornerArcLength) {
        path.arcTo(topRightRect, 90.0, - remaining/cornerArcLength * 90.0);
        drawPath(painter, path, color);
        return;
    } else {
        path.arcTo(topRightRect, 90.0, - 90.0);
        remaining -=  cornerArcLength;
    }

    qreal rightEdge = height - 2 * borderMargin;
    if (remaining <= rightEdge) {
        path.lineTo(topRightArcTo + QPointF(0, remaining));
        drawPath(painter, path, color);
        return;
    } else {
        path.lineTo(bottomRightArcFrom);
        remaining -= rightEdge;
    }

    // bottom right corner arc
    QPointF bottomRightCenter = bottomRightArcFrom + QPointF(-cornerArcRadius, 0);
    QRectF bottomRightRect(bottomRightCenter.x() - cornerArcRadius,
                           bottomRightCenter.y() - cornerArcRadius,
                           2 * cornerArcRadius,
                           2 * cornerArcRadius);

    if (remaining <= cornerArcLength) {
        path.arcTo(bottomRightRect, 0.0, - remaining/cornerArcLength * 90.0);
        drawPath(painter, path, color);
        return;
    } else {
        path.arcTo(bottomRightRect, 0.0, - 90.0);
        remaining -=  cornerArcLength;
    }

    qreal bottomEdge = width - 2 * borderMargin;
    if (remaining <= bottomEdge) {
        path.lineTo(bottomRightArcTo + QPointF(-remaining, 0));
        drawPath(painter, path, color);
        return;
    } else {
        path.lineTo(bottomLeftArcFrom);
        remaining -= bottomEdge;
    }

    // bottom left corner arc
    QPointF bottomLeftCenter = bottomLeftArcFrom + QPointF(0, -cornerArcRadius);
    QRectF bottomLeftRect(bottomLeftCenter.x() - cornerArcRadius,
                          bottomLeftCenter.y() - cornerArcRadius,
                          2 * cornerArcRadius,
                          2 * cornerArcRadius);

    if (remaining <= cornerArcLength) {
        path.arcTo(bottomLeftRect, -90.0, -remaining/cornerArcLength * 90.0);
        drawPath(painter, path, color);
        return;
    } else {
        path.arcTo(bottomLeftRect, -90.0, -90.0);
        remaining -=  cornerArcLength;
    }

    qreal leftEdge = height - 2 * borderMargin - endDistance;
    if (remaining <= leftEdge) {
        path.lineTo(bottomLeftArcTo - QPointF(0, remaining));
        drawPath(painter, path, color);
        return;
    } else {
        path.lineTo(endPoint);
    }
    
    drawPath(painter, path, color);
    // // top left corner arc
    // QPointF topLeftCenter = topLeftArcFrom + QPointF(cornerArcRadius, 0);
    // QRectF topLeftRect(topLeftCenter.x() - cornerArcRadius,
    //                    topLeftCenter.y() - cornerArcRadius,
    //                    2 * cornerArcRadius,
    //                    2 * cornerArcRadius);
    
    // if (remaining <= cornerArcLength) {
    //     path.arcTo(topLeftRect, 180.0, - remaining/cornerArcLength * 90.0);
    // } else {
    //     path.arcTo(topLeftRect, 180.0, - 90.0);
    //     remaining -=  cornerArcLength;
    // }

    // drawPath(painter, path, color);
}


/*
paint the widget

+---------------+
|               |
|      TXT      |
|  lorem ipsum  |
|               |
+---------------+
*/
void ProgressBorderWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect();
    qreal cornerRadius = 9.0;
    QPen borderPen; // Declare borderPen once
    if (isFullScreen) {
        borderPen.setColor(QColor("#2C2C2C")); // Set color for fullscreen
        painter.setBrush(QColor("#2C2C2C"));
        painter.setPen(borderPen); // Set the pen for the border
        painter.drawRoundedRect(rect, 0, 0);
    } else {
        borderPen.setColor(QColor("#393B3B")); // Set color for non-fullscreen
        painter.setBrush(QColor("#1F2020"));
        borderPen.setWidth(1);
        painter.setPen(borderPen); // Set the pen for the border
        painter.drawRoundedRect(rect, cornerRadius, cornerRadius);
    }

    int borderMargin = 4;
    int innerRadius = 4;

    qreal width = rect.width() - 2 * borderMargin -  2 * innerRadius;
    qreal height = rect.height() - 2 * borderMargin - 2 * innerRadius;

    qreal cornerArcRadius = borderMargin + innerRadius;
    qreal cornerArcLength = 3.1416 * cornerArcRadius * 0.5;

    qreal perimeter = 2 * (width + height) - startDistance - endDistance + 3 * cornerArcLength;
    qreal timerProgressLength;
    if (totalTime == 0.0){
        timerProgressLength = 0;
    } else {
        timerProgressLength = (timerProgress / totalTime) * perimeter;
    }

    QColor chaserColor = QColor("#393B3B");
    QColor progressColor = QColor("#ACD3A8");
    QColor backgroundColor = QColor("#1F2020");
    if (isFullScreen) {
        if (isPrisoner) {
            paintBorder(painter,
                    rect,
                    perimeter,
                    borderMargin,
                    innerRadius,
                    width,
                    height,
                    backgroundColor);
        } else {
            borderPen.setColor(QColor("#1F2020")); // Set color for fullscreen
            painter.setBrush(QColor("#1F2020"));
            painter.setPen(borderPen); // Set the pen for the border
            painter.drawRoundedRect(rect, 0, 0);
        }
        
    }
    

    qreal typingProgressLength = typingProgressLengthRatio * perimeter;
    paintBorder(painter,
                rect,
                typingProgressLength,
                borderMargin,
                innerRadius,
                width,
                height,
                progressColor);

    // Call the paintBorder method with all necessary parameters
    paintBorder(painter,
                rect,
                timerProgressLength,
                borderMargin,
                innerRadius,
                width,
                height,
                chaserColor);
}

void ProgressBorderWidget::drawPath(QPainter &painter, const QPainterPath &path, QColor color) {
    QPen pen(color, 8);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPath(path);
}

void ProgressBorderWidget::drawPathTest(QPainter &painter, const QPainterPath &path) {
    QPen pen(QColor("#5652d1"), 8);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPath(path);
}

void ProgressBorderWidget::setFullScreen(bool fullScreen) {
    isFullScreen = fullScreen;
    update();
    emit needsRepaint();
}

void ProgressBorderWidget::setPrisonerModeForProgress(bool isPrisoner) {
    this->isPrisoner = isPrisoner;
    update();
    emit needsRepaint();
}

void ProgressBorderWidget::setTargetWordCount(int wordGoal) {
    this->targetWordCount = wordGoal;
}