/*
Widget holds all visible items.

Border of this widget used to show progress in prisoner mode.
*/

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

    repaintTimer = new QTimer(this);
    repaintTimer->setInterval(16); // ~60 FPS
    repaintTimer->setSingleShot(true);
    connect(repaintTimer, &QTimer::timeout, this, QOverload<>::of(&ProgressBorderWidget::update));
}

void ProgressBorderWidget::requestRepaint()
{
    if (!repaintTimer->isActive()) {
        repaintTimer->start();
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
    requestRepaint();
    isTimerRunning = false;
    isPrisoner = false;                                                      
}

void ProgressBorderWidget::updateTimerProgress() {
    timerProgress = timerProgress + 1;
    requestRepaint();

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
        requestRepaint();
    }
}

void ProgressBorderWidget::clearTypingProgress() {
    typingProgressLengthRatio = 0.0;
    requestRepaint();
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
                                       QColor startColor,
                                       QColor backgroundColor,
                                       QColor endColor) {
    if (progressLength < 0) {
        // useful in case of unlimited time
        // don't paint anything
        return;
    }

    QPointF topLeftArcFrom = rect.topLeft() + QPointF(borderMargin, 2 * borderMargin + innerRadius);
    QPointF topLeftArcTo = rect.topLeft() + QPointF(2 * borderMargin + innerRadius, borderMargin);

    QPointF topRightArcFrom = rect.topRight() + QPointF(- 2 * borderMargin - innerRadius, borderMargin);
    QPointF topRightArcTo = rect.topRight() + QPointF(- borderMargin, 2 * borderMargin + innerRadius);

    QPointF bottomRightArcFrom = rect.bottomRight() + QPointF(-borderMargin, - 2 * borderMargin - innerRadius);
    QPointF bottomRightArcTo = rect.bottomRight() + QPointF(- 2 * borderMargin - innerRadius, -borderMargin);

    QPointF bottomLeftArcFrom = rect.bottomLeft() + QPointF(2 * borderMargin + innerRadius, -borderMargin);
    QPointF bottomLeftArcTo = rect.bottomLeft() + QPointF(borderMargin, - 2 * borderMargin - innerRadius);

    startDistance = 180;
    endDistance = 90;
    QPointF startPoint = topLeftArcTo + QPointF(startDistance, 0);
    QPointF endPoint = topLeftArcFrom + QPointF(0, endDistance);
    QPainterPath path;
    // start from a distance from the top left corner
    path.moveTo(startPoint);

    qreal remaining = progressLength;

    qreal topEdge = width - 2 * borderMargin - startDistance;
    if (remaining <= topEdge) {
        path.lineTo(startPoint + QPointF(remaining, 0));
        drawPath(painter, path, startColor, backgroundColor, endColor);
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
        drawPath(painter, path, startColor, backgroundColor, endColor);
        return;
    } else {
        path.arcTo(topRightRect, 90.0, - 90.0);
        remaining -=  cornerArcLength;
    }

    qreal rightEdge = height - 2 * borderMargin;
    if (remaining <= rightEdge) {
        path.lineTo(topRightArcTo + QPointF(0, remaining));
        drawPath(painter, path, startColor, backgroundColor, endColor);
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
        drawPath(painter, path, startColor, backgroundColor, endColor);
        return;
    } else {
        path.arcTo(bottomRightRect, 0.0, - 90.0);
        remaining -=  cornerArcLength;
    }

    qreal bottomEdge = width - 2 * borderMargin;
    if (remaining <= bottomEdge) {
        path.lineTo(bottomRightArcTo + QPointF(-remaining, 0));
        drawPath(painter, path, startColor, backgroundColor, endColor);
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
        drawPath(painter, path, startColor, backgroundColor, endColor);
        return;
    } else {
        path.arcTo(bottomLeftRect, -90.0, -90.0);
        remaining -=  cornerArcLength;
    }

    qreal leftEdge = height - 2 * borderMargin - endDistance;
    if (remaining <= leftEdge) {
        path.lineTo(bottomLeftArcTo - QPointF(0, remaining));
        drawPath(painter, path, startColor, backgroundColor, endColor);
        return;
    } else {
        path.lineTo(endPoint);
    }
    
    drawPath(painter, path, startColor, backgroundColor, endColor);
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
    qDebug() << "ProgressBorderWidget::paintEvent";
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

    QColor chaserColorEnd = QColor("#804337");
    QColor chaserColorBackground = QColor("#2C2C2C"); // Color the line fades to
    QColor chaserColorStart = QColor("#E0715C");

    QColor progressColorEnd = QColor("#3F7050"); 
    QColor progressColorBackground = QColor("#2C2C2C"); // Color the line fades to
    QColor progressColorStart = QColor("#84E0A5");
    
    QColor backgroundColorEnd = QColor("#1F2020");
    QColor backgroundColorBackground = QColor("#1F2020"); // Color the line fades to
    QColor backgroundColorStart = QColor("#1F2020");
    if (isFullScreen) {
        if (isPrisoner) {
            paintBorder(painter,
                    rect,
                    perimeter,
                    borderMargin,
                    innerRadius,
                    width,
                    height,
                    backgroundColorStart,
                    backgroundColorBackground,
                    backgroundColorEnd);
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
                progressColorStart,
                progressColorBackground,
                progressColorEnd);

    // Call the paintBorder method with all necessary parameters
    paintBorder(painter,    
                rect,
                timerProgressLength,
                borderMargin,
                innerRadius,
                width,
                height,
                chaserColorStart,
                chaserColorBackground,
                chaserColorEnd);
}

/*
  [tail]--------------->[head]
[end color]          [start color]
*/
void ProgressBorderWidget::drawPath(QPainter &painter,
                                    const QPainterPath &path,
                                    QColor startColor,
                                    QColor backgroundColor,
                                    QColor endColor) {
    if (path.isEmpty()) {
        return;
    }

    qreal penWidth = 8;
    QPen pen;
    pen.setWidth(penWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    qreal pathLength = path.length();
    qreal gradientLength = 80.0; // Only last 40 pixels have gradient
    
    if (pathLength <= gradientLength) {
        // If path is shorter than gradient length, draw with gradient along the curve
        int segments = qMax(10, (int)(pathLength / 2)); // More segments for smoother gradient
        for (int i = 0; i < segments; i++) {
            qreal t1 = (qreal)i / segments;
            qreal t2 = (qreal)(i + 1) / segments;
            
            // Interpolate color based on position along path
            qreal colorT = t1;
            QColor segmentColor;
            segmentColor.setRedF(endColor.redF() + (backgroundColor.redF() - endColor.redF()) * colorT);
            segmentColor.setGreenF(endColor.greenF() + (backgroundColor.greenF() - endColor.greenF()) * colorT);
            segmentColor.setBlueF(endColor.blueF() + (backgroundColor.blueF() - endColor.blueF()) * colorT);
            segmentColor.setAlphaF(endColor.alphaF() + (backgroundColor.alphaF() - endColor.alphaF()) * colorT);
            
            QPainterPath segment;
            segment.moveTo(path.pointAtPercent(t1));
            segment.lineTo(path.pointAtPercent(t2));
            
            pen.setColor(segmentColor);
            painter.setPen(pen);
            painter.drawPath(segment);
        }
    } else {
        // Draw the solid color part (everything except the last 40 pixels)
        qreal solidPercent = (pathLength - gradientLength) / pathLength;
        
        // Draw solid portion in fewer segments for performance
        int solidSegments = qMax(10, (int)((pathLength - gradientLength) / 5));
        pen.setColor(endColor);
        painter.setPen(pen);
        
        for (int i = 0; i < solidSegments; i++) {
            qreal t1 = (qreal)i / solidSegments * solidPercent;
            qreal t2 = (qreal)(i + 1) / solidSegments * solidPercent;
            
            QPainterPath segment;
            segment.moveTo(path.pointAtPercent(t1));
            segment.lineTo(path.pointAtPercent(t2));
            painter.drawPath(segment);
        }
        
        // Draw the gradient part (last 40 pixels) with color interpolation
        int gradientSegments = qMax(10, (int)(gradientLength / 2)); // More segments for smooth gradient
        
        for (int i = 0; i < gradientSegments; i++) {
            qreal t1 = solidPercent + (qreal)i / gradientSegments * (1.0 - solidPercent);
            qreal t2 = solidPercent + (qreal)(i + 1) / gradientSegments * (1.0 - solidPercent);
            
            // Interpolate color from endColor to backgroundColor
            qreal colorT = (qreal)i / gradientSegments;
            QColor segmentColor;
            segmentColor.setRedF(endColor.redF() + (backgroundColor.redF() - endColor.redF()) * colorT);
            segmentColor.setGreenF(endColor.greenF() + (backgroundColor.greenF() - endColor.greenF()) * colorT);
            segmentColor.setBlueF(endColor.blueF() + (backgroundColor.blueF() - endColor.blueF()) * colorT);
            segmentColor.setAlphaF(endColor.alphaF() + (backgroundColor.alphaF() - endColor.alphaF()) * colorT);
            
            QPainterPath segment;
            segment.moveTo(path.pointAtPercent(t1));
            segment.lineTo(path.pointAtPercent(t2));
            
            pen.setColor(segmentColor);
            painter.setPen(pen);
            painter.drawPath(segment);
        }
    }

    // Draw a round point at the head of the path with startColor
    painter.setBrush(startColor);
    painter.setPen(Qt::NoPen); // No outline for the circle
    qreal radius = penWidth / 2.0;
    QPointF endPoint = path.pointAtPercent(1.0);
    QRectF endCircleRect(endPoint.x() - radius, endPoint.y() - radius, 
                         2 * radius, 2 * radius);
    painter.drawEllipse(endCircleRect);
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