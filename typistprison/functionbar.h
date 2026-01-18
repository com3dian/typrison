#ifndef FunctionBar_H
#define FunctionBar_H

#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabBar>
#include <QMenuBar>
#include <QPropertyAnimation>
#include <QEvent>
#include <QMouseEvent>
#include <QEasingCurve>
#include <QSpacerItem>
#include <QApplication>
#include <QLabel>
#include <QScrollArea>
#include <QWindow>
#include <QGraphicsOpacityEffect>

#include "customtabwidget.h"
#include "functionbar/customtabbar.h"
#include "functionbar/paintcornerwidget.h"
#include "functionbar/paintleftedgewidget.h"
#include "functionbar/trafficlightwidget.h"


class FunctionBar : public QWidget {
    Q_OBJECT

public:
    explicit FunctionBar(QWidget *parent = nullptr, CustomTabWidget *syncedTabWidget = nullptr);
    ~FunctionBar();

    QList<QPushButton*> getAllButtons() const;

protected:
    // Add these protected methods for window dragging
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool isDragging;
    QPoint dragStartPosition;
    QFrame *menuBar;
    QPushButton *toggleButton;

    QPushButton *button1;
    QPushButton *button2;
    CustomTabBar *tabBar;
    PaintCornerWidget *paintCornerWidget;
    QGraphicsOpacityEffect *paintCornerWidgetOpacityEffect;
    PaintLeftEdgeWidget *paintLeftEdgeWidget;
    QWidget *transparentLeftWidget;

    QSpacerItem *fixedSpacer1;
    QSpacerItem *fixedSpacer2;

    bool isScrollbuttonActive;

    QHBoxLayout *mainLayout;

    QMenu *menu;
    bool isExpanded;
    CustomTabWidget *syncedTabWidget;

    void setupMenuBar();
    void setupTabBar();
    void setupToggleButton();

    void onTabGainedAttention(int index);
    void onTabInserted(int index, const QString &label);
    void onTabRemoved(int index);
    void onTabClosedFromSyncedWidget(int index);
    void onTabActivatedFromSyncWidget(int index);
    void onTabMoved(int from, int to);
    void onTabTitleUpdated(int index, QString newTitle);

signals:
    void minimalButtonClicked();
    void maximalButtonClicked();
    void closeButtonClicked();

public slots:
    void toggleMenuBar();
    void closeMenuBar();
    void expandMenuBar();
    void showPaintCornerWidget();
    void hidePaintCornerWidget();
    void showPaintLeftEdgeWidget();
    void hidePaintLeftEdgeWidget();
    void animatePaintRightEdgeWidget();
    void hideBothPaintCornerWidget();
    void notHideBothPaintCornerWidget();
};

#endif // FunctionBar_H
