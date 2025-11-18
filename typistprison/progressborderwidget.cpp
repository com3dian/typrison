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

    QColor chaserColorEnd = QColor("#2C2C2C");
    QColor chaserColorBackground = QColor("#2C2C2C"); // Color the line fades to
    QColor chaserColorStart = QColor("#393B3B");

    QColor progressColorEnd = QColor("#84E0A5");
    QColor progressColorBackground = QColor("#2C2C2C"); // Color the line fades to
    QColor progressColorStart = QColor("#4F8B64");
    
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

void ProgressBorderWidget::drawPath(QPainter &painter,
                                    const QPainterPath &path,
                                    QColor startColor,
                                    QColor backgroundColor,
                                    QColor endColor) {
    // Check if path has zero length - if so, don't draw anything
    if (path.length() <= 0.0 || path.elementCount() == 0) {
        return;
    }
    
    qreal penWidth = 8;
    qreal radius = penWidth / 2.0; // Half of the borderline width
    qreal pathLength = path.length();
    
    // To make the gradient follow the path geometry (not just a straight line),
    // we draw the path in many small segments, each with a color based on its
    // position along the path. This ensures the gradient follows curves and corners.
    
    // Calculate number of segments based on path length for smooth gradient
    // More segments = smoother gradient, especially around curves
    const int numSegments = qMax(100, static_cast<int>(pathLength / 1.5));
    const qreal segmentStep = 1.0 / numSegments;
    
    // Gradient configuration: limit gradient length in actual path units (not ratio)
    // maxGradientLength: maximum actual length of gradient transition in path units (e.g., pixels)
    // The gradient will always reach the end (100%), but its length is limited
    const qreal maxGradientLength = 150.0;  // Maximum gradient length in path units (e.g., 150 pixels)
    
    // Calculate gradient start position and length
    // Gradient always ends at 100% (path end), but length is limited to maxGradientLength in actual units
    qreal actualGradientLength = qMin(maxGradientLength, pathLength);
    qreal gradientLengthRatio = (pathLength > 0) ? (actualGradientLength / pathLength) : 1.0;  // Convert to ratio for calculations
    qreal gradientStart = qMax(0.0, 1.0 - gradientLengthRatio);  // Start position so gradient ends at 100%, clamped to [0, 1]
    
    // Get the end point for drawing the end circle
    QPointF endPoint = path.pointAtPercent(1.0);
    
    // Configure pen properties that remain constant
    QPen pen;
    pen.setWidth(penWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    
    // Draw the path segment by segment, with color based on position along the path
    for (int i = 0; i < numSegments; ++i) {
        qreal tStart = i * segmentStep;
        qreal tEnd = (i + 1) * segmentStep;
        
        // Clamp to [0, 1]
        if (tEnd > 1.0) {
            tEnd = 1.0;
        }
        
        // Calculate color based on position along the path (path-following gradient)
        // Gradient always reaches the end (100%), with length limited to maxGradientLength
        qreal colorPosition = (tStart + tEnd) / 2.0; // Use midpoint for smoother color transitions
        QColor segmentColor;
        
        if (colorPosition <= gradientStart) {
            // Keep startColor for the first part of the path (before gradient starts)
            segmentColor = startColor;
        } else {
            // Transition from startColor to backgroundColor within the gradient range
            // Gradient always ends at 100% (path end), so we interpolate from gradientStart to 1.0
            qreal transitionProgress = (colorPosition - gradientStart) / gradientLengthRatio;
            transitionProgress = qBound(0.0, transitionProgress, 1.0);
            
            // Interpolate between startColor and backgroundColor
            segmentColor = QColor(
                static_cast<int>(startColor.red() + (backgroundColor.red() - startColor.red()) * transitionProgress),
                static_cast<int>(startColor.green() + (backgroundColor.green() - startColor.green()) * transitionProgress),
                static_cast<int>(startColor.blue() + (backgroundColor.blue() - startColor.blue()) * transitionProgress),
                static_cast<int>(startColor.alpha() + (backgroundColor.alpha() - startColor.alpha()) * transitionProgress)
            );
        }
        
        // Create a small subpath for this segment that follows the curve
        // We sample multiple points within the segment to accurately follow curved paths
        QPainterPath segmentPath;
        QPointF segmentStart = path.pointAtPercent(tStart);
        segmentPath.moveTo(segmentStart);
        
        // Sample points along this segment to accurately follow curves
        // More subdivisions = better curve following, but slightly more expensive
        const int segmentSubdivisions = 3;
        for (int j = 1; j <= segmentSubdivisions; ++j) {
            qreal t = tStart + (tEnd - tStart) * (j / static_cast<qreal>(segmentSubdivisions));
            if (t > 1.0) t = 1.0;
            segmentPath.lineTo(path.pointAtPercent(t));
        }
        
        // Draw this segment with the calculated color
        pen.setColor(segmentColor);
        painter.setPen(pen);
        painter.drawPath(segmentPath);
    }
    
    // Draw a round point at the end of the path with endColor
    painter.setBrush(endColor);
    painter.setPen(Qt::NoPen); // No outline for the circle
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