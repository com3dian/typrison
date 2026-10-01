/*
The tabbar that is the tabbar actually showing at the top of the app.
used in functionBar.

+---------------------+
| file.cell.txt      x \ file.txt   x | another_file.txt   x
+                       +------------------------------------

this tabbar has a twin tab widget `customtabwidget`, all actions from customtabwidget
are synced to this tabbar, and vice versa.
*/

#include "customtabbar.h"
#include <QMouseEvent> // For mouse events
#include <QProxyStyle>
#include <QPainterPathStroker>

namespace {
// Let Qt use the same left-hand geometry for layout, scrolling and hit testing.
class LeftScrollButtonsStyle : public QProxyStyle {
public:
    int pixelMetric(PixelMetric metric, const QStyleOption *option,
                    const QWidget *widget) const override {
        if (metric == PM_TabBarScrollButtonWidth)
            return 22;
        return QProxyStyle::pixelMetric(metric, option, widget);
    }

    QRect subElementRect(SubElement element, const QStyleOption *option,
                         const QWidget *widget) const override {
        if (element == SE_TabBarScrollLeftButton || element == SE_TabBarScrollRightButton) {
            const int buttonWidth = pixelMetric(PM_TabBarScrollButtonWidth, option, widget);
            const int offset = element == SE_TabBarScrollRightButton ? buttonWidth : 0;
            // return QRect(option->rect.left() + offset, option->rect.top(),
            //              buttonWidth, option->rect.height());
            const int verticalMargin = 8; // Top and bottom margin, in pixels
            return QRect(option->rect.left() + offset,
                         option->rect.top() + verticalMargin,
                         buttonWidth,
                         qMax(0, option->rect.height() - 2 * verticalMargin));
        }
        return QProxyStyle::subElementRect(element, option, widget);
    }
};

class TopLeftTabCurveWidget : public QWidget {
public:
    explicit TopLeftTabCurveWidget(QWidget *parent = nullptr)
        : QWidget(parent) {
        setObjectName("TopLeftTabCurveWidget");
        setFixedSize(32, 48);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const int radius = 4;
        const QRect widgetRect = rect();
        const int xLeftTopCorner = widgetRect.left() + 2 * radius;
        const int xLeftBottomCorner = widgetRect.left();
        const int yTopCorner = widgetRect.top() + 9;
        const int yBottomCorner = widgetRect.bottom() + 2;

        QPainterPath areaUnderCurve;
        areaUnderCurve.moveTo(xLeftBottomCorner, yBottomCorner);
        areaUnderCurve.quadTo(xLeftTopCorner, yBottomCorner,
                              xLeftTopCorner, yBottomCorner - 2 * radius);
        areaUnderCurve.lineTo(xLeftTopCorner, yTopCorner + radius);
        areaUnderCurve.quadTo(xLeftTopCorner, yTopCorner,
                              xLeftTopCorner + radius, yTopCorner);
        areaUnderCurve.lineTo(widgetRect.right() + 1, yTopCorner);
        areaUnderCurve.lineTo(widgetRect.right() + 1, yBottomCorner);
        areaUnderCurve.closeSubpath();

        QPainterPath areaOutsideCurve;
        areaOutsideCurve.addRect(QRectF(widgetRect));
        areaOutsideCurve = areaOutsideCurve.subtracted(areaUnderCurve);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#1F2020"));
        painter.drawPath(areaOutsideCurve);
    }
};

class TopRightTabCurveWidget : public QWidget {
public:
    explicit TopRightTabCurveWidget(QWidget *parent = nullptr)
        : QWidget(parent) {
        setObjectName("TopRightTabCurveWidget");
        setFixedSize(24, 48);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        const int slant = 2;
        const int radius = 4;
        const int moveLeft = 2;
        const QRect widgetRect = rect();
        const int xRightTopCorner = widgetRect.right() - slant - moveLeft;
        const int xRightBottomCorner = widgetRect.right() + slant - moveLeft;
        const int yTopCorner = widgetRect.top() + 9;
        const int yBottomCorner = widgetRect.bottom() + 2;

        QPainterPath areaUnderCurve;
        areaUnderCurve.moveTo(widgetRect.left(), yBottomCorner);
        areaUnderCurve.lineTo(widgetRect.left(), yTopCorner);
        areaUnderCurve.lineTo(xRightTopCorner - 2.5 * radius, yTopCorner);
        areaUnderCurve.quadTo(xRightTopCorner - 1.75 * radius, yTopCorner,
                              xRightTopCorner - 1.25 * radius, yTopCorner + radius);
        areaUnderCurve.lineTo(xRightBottomCorner + 1.25 * radius,
                              yBottomCorner - radius);
        areaUnderCurve.quadTo(xRightBottomCorner + 2 * radius, yBottomCorner,
                              xRightBottomCorner + 3 * radius, yBottomCorner);
        areaUnderCurve.closeSubpath();

        // The selected tab also has a 2 px stroke centered on its path.
        // Keep that stroke and its antialiased fringe clear of the mask.
        QPainterPathStroker border;
        border.setWidth(2.0 + 1.0 / devicePixelRatioF());
        areaUnderCurve = areaUnderCurve.united(border.createStroke(areaUnderCurve));

        QPainterPath areaOutsideCurve;
        areaOutsideCurve.addRect(QRectF(widgetRect));
        areaOutsideCurve = areaOutsideCurve.subtracted(areaUnderCurve);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#1F2020"));
        painter.drawPath(areaOutsideCurve);
    }
};
}

CustomTabBar::CustomTabBar(QWidget *parent) 
    : QTabBar(parent),
      insertingTabIndex(-1),                      
      insertAnimationProgressValue(1.0),
      tabInsertAnimation(nullptr),                
      hoveredCloseButtonIndex(-1),
      topLeftCurveWidget(new TopLeftTabCurveWidget(this)),
      topRightCurveWidget(new TopRightTabCurveWidget(this))
{
    auto *scrollStyle = new LeftScrollButtonsStyle;
    scrollStyle->setParent(this);
    setStyle(scrollStyle);
    this->setContentsMargins(0, 0, 0, 0);
    this->setStyleSheet("QTabBar::tab { margin: 0px; padding: 0px; }"); // Base style

    QTimer::singleShot(0, this, SLOT(customizeScrollButtons()));

    // Load close icons
    closeIcon.load(":/icons/tab_close.png");         
    closeIconHover.load(":/icons/tab_hover.png");  
    
    setMouseTracking(true); // Enable mouse move events without button press

    positionCurveWidgets();
    topLeftCurveWidget->show();
    topLeftCurveWidget->raise();
    topRightCurveWidget->show();
    topRightCurveWidget->raise();
}

// Add destructor to manage tabInsertAnimation if necessary
CustomTabBar::~CustomTabBar() {
    if (tabInsertAnimation) {  
        tabInsertAnimation->stop();  
        // tabInsertAnimation will be deleted by QAbstractAnimation::DeleteWhenStopped policy
        // or QObject parenting if not using DeleteWhenStopped and it's parented.
        // Explicit deletion is not needed if DeleteWhenStopped is used.
    }
}

// Add setter for the Q_PROPERTY
void CustomTabBar::setInsertAnimationProgress(qreal progress) {
    if (insertAnimationProgressValue != progress) {
        insertAnimationProgressValue = progress;
        update(); // Trigger a repaint to show the animation frame
    }
}

// Add getter for the Q_PROPERTY
qreal CustomTabBar::insertAnimationProgress() const {
    return insertAnimationProgressValue;
}

bool CustomTabBar::isTabInsertionAnimating() const {
    return insertingTabIndex >= 0;
}

// Add public slot to start the animation
void CustomTabBar::animateTabInsertion(int index, int durationMs) {
    if (tabInsertAnimation) {
        tabInsertAnimation->stop();  
        tabInsertAnimation = nullptr;  
    }

    insertingTabIndex = index;  
    // insertAnimationProgressValue will be driven by the animation from 0.0 to 1.0

    // Corrected: The property name here MUST match the Q_PROPERTY name in customtabbar.h
    tabInsertAnimation = new QPropertyAnimation(this, "insertAnimationProgress");
    tabInsertAnimation->setDuration(durationMs);
    tabInsertAnimation->setStartValue(0.0);
    tabInsertAnimation->setEndValue(1.0);
    tabInsertAnimation->setEasingCurve(QEasingCurve::Linear);

    connect(tabInsertAnimation, &QPropertyAnimation::finished, this, &CustomTabBar::onAnimationFinished); // Corrected

    tabInsertAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

// Add slot to handle animation finished
void CustomTabBar::onAnimationFinished() {
    insertingTabIndex = -1;
    // insertAnimationProgressValue is now 1.0
    tabInsertAnimation = nullptr;
    update(); // Ensure a final repaint with the tab fully visible and no animation logic active
    emit animateTabInsertionFinished();
}

/*
Style the overflow controls without hiding them.
*/
void CustomTabBar::customizeScrollButtons() {
    // Retrieve the scroll buttons by their object names
    QToolButton *leftButton = findChild<QToolButton*>("ScrollLeftButton");
    QToolButton *rightButton = findChild<QToolButton*>("ScrollRightButton");

    QString noStyle = "QToolButton { border: none; background-color: transparent; padding: 0px; } "
                      "QToolButton:hover { background-color: #2c2c2c; border-radius: 4px; }";

    if (leftButton) {
        leftButton->setStyleSheet(noStyle);

        // Install event filter to detect visibility changes
        leftButton->installEventFilter(this);
    }
    if (rightButton) {
        rightButton->setStyleSheet(noStyle);

        rightButton->installEventFilter(this);
    }

    positionCurveWidgets();
}

QRect CustomTabBar::tabsViewportRect() const {
    QRect viewport = rect();
    for (const auto *button : findChildren<QToolButton *>()) {
        if (button->isVisible() && (button->objectName() == "ScrollLeftButton"
                                   || button->objectName() == "ScrollRightButton")) {
            viewport.setLeft(qMax(viewport.left(), button->geometry().right() + 1));
        }
    }
    return viewport;
}

void CustomTabBar::positionCurveWidgets() {
    const int tabsLeft = tabsViewportRect().left();
    const int leftCurveX = tabsLeft == rect().left() ? tabsLeft - 8 : tabsLeft;
    topLeftCurveWidget->move(leftCurveX, 0);
    topRightCurveWidget->move(qMax(0, width() - topRightCurveWidget->width()), 0);
    topLeftCurveWidget->raise();
    topRightCurveWidget->raise();
}

bool CustomTabBar::isRightmostVisibleTabSelected() const {
    const QRect viewport = tabsViewportRect();
    int rightmostVisibleIndex = -1;
    int rightmostEdge = viewport.left() - 1;

    for (int i = 0; i < count(); ++i) {
        const QRect visiblePart = tabRect(i).intersected(viewport);
        if (!visiblePart.isEmpty() && visiblePart.right() > rightmostEdge) {
            rightmostVisibleIndex = i;
            rightmostEdge = visiblePart.right();
        }
    }

    return rightmostVisibleIndex >= 0 && currentIndex() == rightmostVisibleIndex;
}

// Helper function to calculate the close button rectangle for a given tab
QRect CustomTabBar::getCloseButtonRect(int tabIndex) const {
    QRect r = tabRect(tabIndex);
    if (r.isNull() || r.isEmpty()) {
        return QRect();
    }
    // Position on the right, vertically centered, then offset downwards
    int verticalOffset = 2; // Pixels to lower the icon
    int y = r.top() + (r.height() - CLOSE_ICON_SIZE) / 2 + verticalOffset;
    int x = r.right() - CLOSE_ICON_MARGIN_RIGHT - CLOSE_ICON_SIZE;
    const QRect closeRect(x, y, CLOSE_ICON_SIZE, CLOSE_ICON_SIZE);
    return tabsViewportRect().contains(closeRect) ? closeRect : QRect();
}

// Override tabSizeHint to make enough space for text and the close icon
QSize CustomTabBar::tabSizeHint(int index) const {
    // Get the text for the tab
    QString currentTabText = tabText(index);
    QFontMetrics fm = fontMetrics();

    // Calculate the width needed for the text
    int textWidth = fm.horizontalAdvance(currentTabText);

    // Define horizontal paddings and icon sizes (consistent with paintEvent and constants)
    int leftTextPadding = 16; // This matches the textDrawingRect.adjust(16, ...) in paintEvent

    // Calculate total width required for the tab content
    int totalWidth = leftTextPadding +         // Padding on the left of the text
                     textWidth +                // Actual width of the text
                     CLOSE_ICON_TEXT_PADDING +  // Padding between text and close icon
                     CLOSE_ICON_SIZE +          // Width of the close icon
                     CLOSE_ICON_MARGIN_RIGHT;   // Margin on the right of the close icon

    // Get the base size hint from QTabBar (primarily for height and any style-driven minimums)
    QSize hintedSize = QTabBar::tabSizeHint(index);
    
    hintedSize.setWidth(qBound(110, totalWidth, 220));
    
    return hintedSize;
}

QSize CustomTabBar::minimumTabSizeHint(int index) const {
    QSize hintedSize = QTabBar::minimumTabSizeHint(index);
    hintedSize.setWidth(110);
    return hintedSize;
}

void CustomTabBar::paintEvent(QPaintEvent *event) {
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // Smoother edges
    painter.setRenderHint(QPainter::SmoothPixmapTransform); // Higher quality pixmap scaling
    painter.setClipRect(tabsViewportRect());

    for (int i = 0; i < count(); ++i) {
        QStyleOptionTab opt;
        initStyleOption(&opt, i);
        QRect currentTabRect = tabRect(i);
        
        painter.save(); // Save painter state

        // Animation clipping logic (if any)
        if (i == insertingTabIndex && insertingTabIndex != -1 && insertAnimationProgressValue < 1.0) {
            QRect animatedClipRect = currentTabRect;
            animatedClipRect.setWidth(qRound(currentTabRect.width() * insertAnimationProgressValue));
            if (animatedClipRect.width() < 1) animatedClipRect.setWidth(1);
            painter.setClipRect(animatedClipRect, Qt::IntersectClip);
        }

        // Draw tab background and border (your existing logic)
        if (i == currentIndex()) {

            int slant = 2;  // Adjust this to control the slant amount
            int radius = 4;  // Corner radius
            int moveLeft = 2;

            // Use currentTabRect instead of rect
            int xLeftBottomCorner = currentTabRect.left() - 2*radius;
            int yLeftBottomCorner = currentTabRect.bottom() + 2;

            int xLeftTopCorner = currentTabRect.left();
            int yLeftTopCorner = currentTabRect.top() + 9;

            int xRightTopCorner = currentTabRect.right() - slant - moveLeft;
            int xRightBottomCorner = currentTabRect.right() + slant - moveLeft;
            
            // Re-evaluate on every repaint so overflow scrolling and tab moves
            // immediately update the right corner.
            if (isRightmostVisibleTabSelected()) {
                emit lastTabFocus();
            } else {
                emit lastTabNoFocus();
            }

            if (i == 0) {
                emit firstTabFocus();
            } else {
                emit firstTabNoFocus();
            }

            QPainterPath path; // Path for the selected tab
            path.moveTo(xLeftBottomCorner, yLeftBottomCorner);  // Start with rounded bottom-left corner
            path.quadTo(xLeftTopCorner, yLeftBottomCorner, xLeftTopCorner, yLeftBottomCorner - 2*radius);  // Rounded corner

            path.lineTo(xLeftTopCorner, yLeftTopCorner + radius);
            path.quadTo(xLeftTopCorner, yLeftTopCorner, xLeftTopCorner + radius, yLeftTopCorner);  // Rounded corner

            path.lineTo(xRightTopCorner - 2.5 * radius, yLeftTopCorner);
            path.quadTo(xRightTopCorner - 1.75 * radius, yLeftTopCorner, xRightTopCorner - 1.25 * radius, yLeftTopCorner + radius);  // Rounded top-right corner

            path.lineTo(xRightBottomCorner + 1.25 * radius, yLeftBottomCorner - radius);
            path.quadTo(xRightBottomCorner + 2 * radius, yLeftBottomCorner, xRightBottomCorner + 3 * radius, yLeftBottomCorner);  // Rounded bottom-right corner

            path.closeSubpath();

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#2c2c2c"));  // Background color for selected tab
            painter.drawPath(path);

            // Draw border by setting a pen
            QPen borderPen(QColor("#2c2c2c"), 2); 
            painter.setPen(borderPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        } else {
            // Draw normal tab as a rectangle
            painter.fillRect(currentTabRect, QColor("transparent")); // Use currentTabRect here

            if (not ( i == (count() - 1) or i == (currentIndex() - 1) )) {
                // Draw a white vertical line at the right border
                // Use currentTabRect here
                painter.fillRect(QRect(currentTabRect.right() - 2, currentTabRect.top() + 16.5, 2, currentTabRect.height() - 27), QColor("#3A3B3B"));
            }
        }

        // Calculate rect for text, leaving space for the close icon
        QRect textDrawingRect = currentTabRect;
        // Adjust right side for close icon, its margin, and padding
        textDrawingRect.setRight(textDrawingRect.right() - CLOSE_ICON_MARGIN_RIGHT - CLOSE_ICON_SIZE - CLOSE_ICON_TEXT_PADDING);
        // Apply existing left/vertical padding for text (e.g., 16px from left)
        textDrawingRect.adjust(16, 2, 0, 0); 

        painter.setPen(QColor("#BDBDBD")); // Text color
        // Use elidedText to prevent text from overflowing
        QString elided = fontMetrics().elidedText(tabText(i), Qt::ElideRight, textDrawingRect.width());
        painter.drawText(textDrawingRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
    
        // Draw the close icon
        QRect closeIconRect = getCloseButtonRect(i);
        // Ensure the icon is drawn only if it's valid and intersects the current tab's visible area
        if (!closeIconRect.isNull() && currentTabRect.intersects(closeIconRect)) { 
            const QPixmap& iconToDraw = (i == hoveredCloseButtonIndex && !closeIconHover.isNull()) 
                                        ? closeIconHover 
                                        : closeIcon;
            if (!iconToDraw.isNull()) {
                // QPainter will scale iconToDraw (which is full-res) into closeIconRect smoothly
                painter.drawPixmap(closeIconRect, iconToDraw);
            }
        }
        
        painter.restore(); // Restore painter state (removes clipping, etc.)

        // Emit focus signals (these are state signals, not purely visual, so outside save/restore)
        // Original placement is fine as they relate to the logical state of the tab.
        // This part was outside the painter.save/restore block in your original code, which is fine.
        // However, the focus signals were inside the `if (i == currentIndex())` block in your original code.
        // I've kept them there as it seems logical to only emit focus for the current tab.
        // If they were meant to be outside, they should be moved.
        // The provided snippet in the prompt had them inside the `if (i == currentIndex())` block.
    }
}

void CustomTabBar::mouseMoveEvent(QMouseEvent *event) {
    int oldHoverIndex = hoveredCloseButtonIndex;
    hoveredCloseButtonIndex = -1; // Reset

    for (int i = 0; i < count(); ++i) {
        if (getCloseButtonRect(i).contains(event->pos())) {
            hoveredCloseButtonIndex = i;
            break;
        }
    }

    if (oldHoverIndex != hoveredCloseButtonIndex) {
        update(); // Trigger repaint if hover state changed
    }
    QTabBar::mouseMoveEvent(event); // Call base implementation
}

void CustomTabBar::mousePressEvent(QMouseEvent *event) {
    if (!tabsViewportRect().contains(event->pos())) {
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < count(); ++i) {
            if (getCloseButtonRect(i).contains(event->pos())) {
                emit tabCloseRequested(i); // Emit standard QTabBar signal
                event->accept();
                return; // Event handled
            }
        }
    }
    QTabBar::mousePressEvent(event); // Call base implementation for other tab interactions
}

void CustomTabBar::leaveEvent(QEvent *event) {
    if (hoveredCloseButtonIndex != -1) {
        hoveredCloseButtonIndex = -1;
        update(); // Trigger repaint to remove hover state
    }
    QTabBar::leaveEvent(event); // Call base implementation
}

void CustomTabBar::resizeEvent(QResizeEvent *event) {
    QTabBar::resizeEvent(event);
    positionCurveWidgets();
}

bool CustomTabBar::eventFilter(QObject *obj, QEvent *event) {
    // Check if the object is one of the scroll buttons
    if (obj->objectName() == "ScrollLeftButton" || obj->objectName() == "ScrollRightButton") {
        if (event->type() == QEvent::Show) {
            emit scrollbuttonActivate();
        } else if (event->type() == QEvent::Hide) {
            emit scrollbuttonInactivate();
        }
        QTimer::singleShot(0, this, [this]() { positionCurveWidgets(); });
    }
    // Ensure the base class processes the event too
    return QTabBar::eventFilter(obj, event);
}
