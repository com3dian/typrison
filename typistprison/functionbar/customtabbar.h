#ifndef CUSTOMTABBAR_H
#define CUSTOMTABBAR_H

#include <QTabBar>
#include <QPainter>
#include <QStyleOptionTab>
#include <QPainterPath>
#include <QSize>
#include <QMouseEvent>
#include <QStylePainter>
#include <QScreen>
#include <QToolButton>
#include <QTimer>
#include <QPropertyAnimation> // Added for QPropertyAnimation
#include <QPixmap>

class CustomTabBar : public QTabBar {
    Q_OBJECT
    // Property for animating tab insertion width
    Q_PROPERTY(qreal insertAnimationProgress READ insertAnimationProgress WRITE setInsertAnimationProgress)

public:
    explicit CustomTabBar(QWidget *parent = nullptr);
    ~CustomTabBar(); // Added destructor declaration
    bool isTabInsertionAnimating() const;
    bool isRightmostVisibleTabSelected() const;

public slots:
    // Slot to trigger and manage the tab insertion animation
    void animateTabInsertion(int index, int durationMs = 200);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;     // Added for hover detection
    void mousePressEvent(QMouseEvent *event) override;    // Added for click detection
    void leaveEvent(QEvent *event) override;              // Added to reset hover state
    void resizeEvent(QResizeEvent *event) override;
    QSize tabSizeHint(int index) const override;          // Added to ensure space for close icon
    QSize minimumTabSizeHint(int index) const override;

private slots:
    void customizeScrollButtons();
    // Setter for the insertAnimationProgress property
    void setInsertAnimationProgress(qreal progress);
    // Slot called when the insertion animation finishes
    void onAnimationFinished();

signals:
    void lastTabFocus();
    void lastTabNoFocus();
    void firstTabFocus();
    void firstTabNoFocus();
    void scrollbuttonActivate();
    void scrollbuttonInactivate();
    void animateTabInsertionFinished();

private:
    // Getter for the insertAnimationProgress property
    qreal insertAnimationProgress() const;

    // Member variables for managing tab insertion animation
    int insertingTabIndex;          // Index of the tab currently being animated for insertion
    qreal insertAnimationProgressValue;  // Current progress of the insertion animation (0.0 to 1.0)
    QPropertyAnimation *tabInsertAnimation; // Animation object for tab insertion

    // Members for custom close button drawing
    QPixmap closeIcon;
    QPixmap closeIconHover;
    int hoveredCloseButtonIndex; // Index of tab whose close button is hovered, -1 if none
    QWidget *topLeftCurveWidget;
    QWidget *topRightCurveWidget;
    
    QRect getCloseButtonRect(int tabIndex) const; // Helper to calculate close button rect
    QRect tabsViewportRect() const;
    void positionCurveWidgets();

    // Constants for close button appearance
    static const int CLOSE_ICON_SIZE = 16;
    static const int CLOSE_ICON_MARGIN_RIGHT = 16; // Margin from the right of the tab
    static const int CLOSE_ICON_TEXT_PADDING = 32; // Padding between text and close icon
};

#endif // CUSTOMTABBAR_H
