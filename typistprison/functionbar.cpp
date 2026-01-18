#include "functionbar.h"
#include <QGraphicsOpacityEffect> // Add this
#include <QPropertyAnimation>   // Ensure this is included (likely already via functionbar.h or other uses)
#include <QTimer> // Make sure QTimer is included

FunctionBar::FunctionBar(QWidget *parent, CustomTabWidget *syncedTabWidget)
    : QWidget(parent)
    , syncedTabWidget(syncedTabWidget)
    , isExpanded(false)
    , isDragging(false)
{
    // Main container layout
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    this->setFixedHeight(48);

    // Setup the toggle button
    setupToggleButton();

    // Setup the menu bar
    setupMenuBar();

    // Setup the tab bar
    setupTabBar();

    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    fixedSpacer1 = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    fixedSpacer2 = new QSpacerItem(1, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    paintLeftEdgeWidget = new PaintLeftEdgeWidget(this);
    paintLeftEdgeWidget->setVisible(false);

    transparentLeftWidget = new QWidget(this);
    transparentLeftWidget->setAttribute(Qt::WA_TranslucentBackground), transparentLeftWidget->setFixedWidth(8);
    transparentLeftWidget->setVisible(false);

    paintCornerWidget = new PaintCornerWidget(this);
    paintCornerWidget->setVisible(false); // Start hidden

    TrafficLightWidget *trafficLightWidget = new TrafficLightWidget(this);
    connect(trafficLightWidget, &TrafficLightWidget::minimalButtonClicked, this, &FunctionBar::minimalButtonClicked);
    connect(trafficLightWidget, &TrafficLightWidget::maximalButtonClicked, this, &FunctionBar::maximalButtonClicked);
    connect(trafficLightWidget, &TrafficLightWidget::closeButtonClicked, this, &FunctionBar::closeButtonClicked);

    // Add toggle button, menu bar, and tab bar to the main layout
    mainLayout->addWidget(menuBar);            // Menu bar in the left
    mainLayout->addItem(fixedSpacer1);         // Spacer between menu bar & toggle button
    mainLayout->addWidget(toggleButton);       // Toggle button on the middle
    mainLayout->addItem(fixedSpacer2);         // Spacer between menu bar & toggle button
    mainLayout->addWidget(transparentLeftWidget);
    mainLayout->addWidget(paintLeftEdgeWidget);
    mainLayout->addWidget(tabBar);             // Tab bar on the right
    mainLayout->addWidget(paintCornerWidget);  // corner painting for last tab
    mainLayout->addItem(spacer);
    mainLayout->addWidget(trafficLightWidget);

    isScrollbuttonActive = false;
}

FunctionBar::~FunctionBar() {
    // Cleanup if necessary
}

void FunctionBar::setupToggleButton() {
    // Create the toggle button
    toggleButton = new QPushButton(this);
    toggleButton->setFixedSize(32, 32);
    toggleButton->setStyleSheet("QPushButton {"
        "border: none;"
        "border-image: url(:/icons/toggle_menu.png) 0 0 0 0 stretch stretch;"
        "}"
    );

    // Connect the toggle button click to toggleMenuBar()
    connect(toggleButton, &QPushButton::clicked, this, &FunctionBar::expandMenuBar, Qt::UniqueConnection);
}

void FunctionBar::onTabGainedAttention(int index) {
    if (syncedTabWidget) {
        syncedTabWidget->blockSignals(true);

        syncedTabWidget->setCurrentIndex(index);

        syncedTabWidget->blockSignals(false);
    }
}

void FunctionBar::onTabInserted(int index, const QString &label) {
    if (syncedTabWidget) {
        syncedTabWidget->blockSignals(true);

        // Insert the tab
        tabBar->insertTab(index, label);
        // tabBar->setTabButton(index, QTabBar::RightSide, closeButton); // REMOVED
        
        // Trigger the insertion animation for the new tab
        if (auto* customTabBar = qobject_cast<CustomTabBar*>(tabBar)) {
            customTabBar->animateTabInsertion(index);
        }

        syncedTabWidget->blockSignals(false);
        tabBar->setCurrentIndex(index);
    }
}

void FunctionBar::onTabClosedFromSyncedWidget(int index) {
    if (syncedTabWidget) {
        syncedTabWidget->blockSignals(true);

        // tabBar->setTabButton(index, QTabBar::RightSide, nullptr);  // Remove the close button // REMOVED
        QApplication::processEvents();
        tabBar->removeTab(index);                                  // Remove the tab
        QApplication::processEvents();

        syncedTabWidget->blockSignals(false);
    }
}

void FunctionBar::onTabActivatedFromSyncWidget(int index) {
    if (syncedTabWidget) {
        tabBar->setCurrentIndex(index);
    }
}

void FunctionBar::onTabRemoved(int index) {
    if (syncedTabWidget) {
        syncedTabWidget->closeWindowIfNoTabs(index);
    }
}

void FunctionBar::onTabMoved(int from, int to) {
    if (syncedTabWidget) {
        syncedTabWidget->blockSignals(true);

        // Move the corresponding tab in QTabWidget
        QWidget *widget = syncedTabWidget->widget(from);
        QString label = syncedTabWidget->tabText(from);
        syncedTabWidget->removeTab(from);
        syncedTabWidget->insertTab(to, widget, label);

        syncedTabWidget->blockSignals(false);
    }
}

void FunctionBar::onTabTitleUpdated(int index, QString newTitle) {
    tabBar->setTabText(index, newTitle);
}

void FunctionBar::setupTabBar() {
    // Create the tab bar
    tabBar = new CustomTabBar(this);
    tabBar->setFixedHeight(48);
    tabBar->setStyleSheet(
        "QTabBar::tab {"
        "    height: 48px;"  // Ensures the tabs match the tab bar height
        "    min-height: 48px;"
        "    padding: 0px;"
        "}"
        "QTabBar::tab:selected {"
        "    padding: 0px;"
        "    color: transparent;"
        "    background-color: transparent;"
        "}"
    );

    // Optional: Customize tab behavior or appearance
    tabBar->setExpanding(true);  // Tabs expand to fill available space
    tabBar->setMovable(true);

    connect(syncedTabWidget, &CustomTabWidget::tabInsertedSignal, this, &FunctionBar::onTabInserted);
    connect(syncedTabWidget, &CustomTabWidget::tabClosedFromSyncedTabWidgetSignal, this, &FunctionBar::onTabClosedFromSyncedWidget);
    connect(syncedTabWidget, &CustomTabWidget::tabActivatedSignal, this, &FunctionBar::onTabActivatedFromSyncWidget);
    connect(syncedTabWidget, &CustomTabWidget::updatedTabTitleSignal, this, &FunctionBar::onTabTitleUpdated);

    connect(tabBar, &QTabBar::tabBarClicked, this, &FunctionBar::onTabGainedAttention);
    connect(tabBar, &QTabBar::tabCloseRequested, this, &FunctionBar::onTabRemoved);
    connect(tabBar, &QTabBar::tabMoved, this, &FunctionBar::onTabMoved);

    // right out-border painting
    connect(tabBar, &CustomTabBar::lastTabFocus,
        this, &FunctionBar::showPaintCornerWidget,
        Qt::UniqueConnection);
    connect(tabBar, &CustomTabBar::lastTabNoFocus,
        this, &FunctionBar::hidePaintCornerWidget,
        Qt::UniqueConnection);
    connect(tabBar, &CustomTabBar::animateTabInsertionFinished,
        this, &FunctionBar::animatePaintRightEdgeWidget,
        Qt::UniqueConnection);

    // left out-border painting 
    connect(tabBar, &CustomTabBar::firstTabFocus,
        this, &FunctionBar::showPaintLeftEdgeWidget,
        Qt::UniqueConnection);
    connect(tabBar, &CustomTabBar::firstTabNoFocus,
        this, &FunctionBar::hidePaintLeftEdgeWidget,
        Qt::UniqueConnection);

    connect(tabBar, &CustomTabBar::firstTabFocus,
        this, &FunctionBar::showPaintLeftEdgeWidget,
        Qt::UniqueConnection);
    connect(tabBar, &CustomTabBar::firstTabNoFocus,
        this, &FunctionBar::hidePaintLeftEdgeWidget,
        Qt::UniqueConnection);

    connect(tabBar, &CustomTabBar::scrollbuttonActivate,
        this, &FunctionBar::hideBothPaintCornerWidget,
        Qt::UniqueConnection);
    connect(tabBar, &CustomTabBar::scrollbuttonInactivate,
        this, &FunctionBar::notHideBothPaintCornerWidget,
        Qt::UniqueConnection);

    return;
}

void FunctionBar::setupMenuBar() {
    menuBar = new QFrame(this);
    menuBar->setStyleSheet("background-color: transparent; border: 0px solid #cccccc;");
    menuBar->setFixedHeight(48);  // set height to 56
    menuBar->setMaximumWidth(0);  // initial width
    menuBar->setMinimumWidth(0);  // minimal width folded

    QHBoxLayout *menuLayout = new QHBoxLayout(menuBar);
    menuLayout->setContentsMargins(6, 0, 8, 0);
    menuLayout->setSpacing(0);

    // add some PushButton
    button1 = new QPushButton("File ", this);
    QIcon customIcon;
    customIcon.addPixmap(QPixmap(":/icons/angle_down.png"), QIcon::Normal);

    button1->setIcon(customIcon);
    button1->setIconSize(QSize(16, 16));

    button1->setStyleSheet(
        "QPushButton {"
        "    padding: 0px 14px 0px 10px; "
        "    height: 32px; "
        "    color: #BDBDBD; "
        "    background-color: transparent;"
        "    border-radius: 4px;"        // Rounded corners
        "    text-align: left;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2c2c2c;" 
        "}"
    );
    button1->setLayoutDirection(Qt::RightToLeft);

    button1->setVisible(false);

    button2 = new QPushButton("Project ", this);
    button2->setIcon(customIcon);
    button2->setIconSize(QSize(16, 16));
    button2->setStyleSheet(
        "QPushButton {"
        "    padding: 0px 14px 0px 10px; "
        "    height: 32px; "
        "    color: #BDBDBD; "
        "    background-color: transparent;"
        "    border-radius: 4px;"        // Rounded corners
        "}"
        "QPushButton:hover {"
        "    background-color: #2c2c2c;" 
        "}"
    );
    button2->setLayoutDirection(Qt::RightToLeft);

    button2->setVisible(false); // default hide

    menuLayout->addWidget(button1);
    menuLayout->addWidget(button2);

    menuBar->setLayout(menuLayout);
}

void FunctionBar::toggleMenuBar() {

    int buttonWidth1 = button1->sizeHint().width();
    int buttonWidth2 = button2->sizeHint().width();
    int requiredWidth = buttonWidth1 + buttonWidth2 + 8;

    // Animate the expansion of the menu bar
    QPropertyAnimation *animation = new QPropertyAnimation(menuBar, "minimumWidth", this);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    if (isExpanded) {
        // Collapse menu bar
        animation->setStartValue(requiredWidth);  // Expanded width
        animation->setEndValue(0);      // Collapsed width
        button1->setVisible(false);     // Hide button when collapsed
        button2->setVisible(false);     // Hide button when collapsed

        toggleButton->setStyleSheet("QPushButton {"
            "border: none;"
            "border-image: url(:/icons/toggle_menu.png) 0 0 0 0 stretch stretch;"
            "}"
        );
    } else {
        // Expand menu bar
        animation->setStartValue(0);    // Collapsed width
        animation->setEndValue(requiredWidth);    // Expanded width
        button1->setVisible(true);      // Show button when expanded
        button2->setVisible(true);      // Show button when expanded

        toggleButton->setStyleSheet("QPushButton {"
            "border: none;"
            "border-image: url(:/icons/toggle_menu_back.png) 0 0 0 0 stretch stretch;"
            "}"
        );
    }

    animation->start();
    isExpanded = !isExpanded;  // Toggle the expanded state
}

void FunctionBar::closeMenuBar() {
    if (not isExpanded) {
        return;
    }
    this->toggleMenuBar();
    disconnect(toggleButton, &QPushButton::clicked, this, &FunctionBar::closeMenuBar);
    connect(toggleButton, &QPushButton::clicked, this, &FunctionBar::expandMenuBar, Qt::UniqueConnection);
}

void FunctionBar::expandMenuBar() {
    if (isExpanded) {
        return;
    }
    this->toggleMenuBar();
    disconnect(toggleButton, &QPushButton::clicked, this, &FunctionBar::expandMenuBar);
    connect(toggleButton, &QPushButton::clicked, this, &FunctionBar::closeMenuBar, Qt::UniqueConnection);
}

QList<QPushButton*> FunctionBar::getAllButtons() const {
    QList<QPushButton*> buttons;
    buttons << toggleButton << button1 << button2;
    return buttons;
}

void FunctionBar::showPaintCornerWidget() {
    if (isScrollbuttonActive) {
        return;
    }
    paintCornerWidget->setVisible(true);
}

void FunctionBar::hidePaintCornerWidget() {
    if (isScrollbuttonActive) {
        return;
    }

    paintCornerWidget->setVisible(false);
}

void FunctionBar::animatePaintRightEdgeWidget() {
    // Delay in milliseconds (e.g., 500ms = 0.5 seconds)
    if (!paintCornerWidget) return; // Guard against widget being deleted during delay

    paintCornerWidgetOpacityEffect = new QGraphicsOpacityEffect(this); // Create the effect
    paintCornerWidget->setGraphicsEffect(paintCornerWidgetOpacityEffect); // Apply to the widget
    paintCornerWidgetOpacityEffect->setOpacity(0.0); // Start fully transparent

    int delayMilliseconds = 200; // You can adjust this value

    QTimer::singleShot(delayMilliseconds, this, [this]() {
        // This code will execute after delayMilliseconds
        if (!paintCornerWidget) return; // Guard against widget being deleted during delay

        QPropertyAnimation *animation = new QPropertyAnimation(paintCornerWidgetOpacityEffect, "opacity", this);
        animation->setDuration(100); // Duration of the fade-in animation
        animation->setStartValue(paintCornerWidgetOpacityEffect->opacity()); // Start from current opacity (should be 0.0 if just made visible)
        animation->setEndValue(1.0);   // Fade to fully opaque
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void FunctionBar::showPaintLeftEdgeWidget() {
    paintLeftEdgeWidget->setVisible(true);
    transparentLeftWidget->setVisible(false);
}

void FunctionBar::hidePaintLeftEdgeWidget() {
    paintLeftEdgeWidget->setVisible(false);
    transparentLeftWidget->setVisible(true);
}

void FunctionBar::hideBothPaintCornerWidget() {
    paintCornerWidget->setVisible(false);

    isScrollbuttonActive = true;
}

void FunctionBar::notHideBothPaintCornerWidget() {
    isScrollbuttonActive = false;
}

void FunctionBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            if (window()->windowHandle()) {
                window()->windowHandle()->startSystemMove();
            }
    #else
            // fallback for old Qt versions
            isDragging = true;
            dragStartPosition = event->globalPos() - window()->frameGeometry().topLeft();
    #endif
            event->accept();
    }
}

void FunctionBar::mouseMoveEvent(QMouseEvent *event) {
    #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        if (isDragging && (event->buttons() & Qt::LeftButton)) {
            window()->move(event->globalPos() - dragStartPosition);
            event->accept();
        }
    #endif
}

void FunctionBar::mouseReleaseEvent(QMouseEvent *event) {
    #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        if (event->button() == Qt::LeftButton) {
            isDragging = false;
            event->accept();
        }
    #endif
}
