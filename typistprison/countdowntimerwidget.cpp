#include "countdowntimerwidget.h"
#include <QApplication>
#include <QScreen>


CountdownTimerWidget::CountdownTimerWidget(QWidget *parent)
    : QWidget(parent)
    , totalSeconds(0)
    , remainingSeconds(0)
    , isActive(false)
    , isUnlimited(false)
    , fadeAnimation(nullptr)
    , opacityEffect(nullptr)
{
    setupUI();
    
    // Initialize timer
    countdownTimer = new QTimer(this);
    countdownTimer->setInterval(1000); // 1 second intervals
    connect(countdownTimer, &QTimer::timeout, this, &CountdownTimerWidget::updateCountdown);
    
    // Initialize opacity effect for fade animation
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);
    
    // Hide widget initially
    setVisible(false);
}

CountdownTimerWidget::~CountdownTimerWidget()
{
    if (countdownTimer) {
        countdownTimer->stop();
    }
    if (fadeAnimation) {
        fadeAnimation->stop();
        // fadeAnimation will be automatically deleted as it's a child of this widget
    }
}

void CountdownTimerWidget::setupUI()
{
    // Create main layout
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(4);
    
    // Create text label for "Challenge starts in:"
    textLabel = new QLabel("Challenge starts in:", this);
    textLabel->setAlignment(Qt::AlignCenter);
    
    // Create time display label
    timeLabel = new QLabel("00:00", this);
    timeLabel->setAlignment(Qt::AlignCenter);
    
    // Style the widget and labels
    setStyleSheet(
        "CountdownTimerWidget {"
        "    background-color: #2c2c2c;"
        "    border: 2px solid #4a4a4a;"
        "    border-radius: 8px;"
        "}"
        "QLabel {"
        "    color: #ffffff;"
        "    background: transparent;"
        "    border: none;"
        "}"
    );
    
    // Style text label (smaller font)
    QFont textFont = textLabel->font();
    textFont.setPointSize(10);
    textFont.setWeight(QFont::Normal);
    textLabel->setFont(textFont);
    textLabel->setStyleSheet("color: #b0b0b0;");
    
    // Set ultra-thin font programmatically for maximum thinness on time label
    QFont timeFont = timeLabel->font();
    timeFont.setFamily("Noto Sans Mono");
    timeFont.setPointSize(24);
    timeFont.setWeight(QFont::Thin);
    timeLabel->setFont(timeFont);
    
    // Add widgets to layout (text label first, then time label)
    mainLayout->addWidget(textLabel);
    mainLayout->addWidget(timeLabel);
    
    // Set fixed size
    setFixedSize(120, 60);
    
    // Make widget stay on top
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void CountdownTimerWidget::showWidget()
{
    setVisible(true);
    raise(); // Bring to front
    updateDisplay();
}

void CountdownTimerWidget::hideWidget()
{
    setVisible(false);
    if (countdownTimer->isActive()) {
        stopCountdown();
    }
}

void CountdownTimerWidget::startCountdown(int seconds) {
    // Handle unlimited mode (seconds <= 0)
    if (seconds <= 0) {
        isUnlimited = true;
        isActive = false;
        
        // Update text label to indicate unlimited mode
        textLabel->setText("Challenge starts in:");
        timeLabel->setText("unlimited");
        timeLabel->setStyleSheet("color: #ffffff;");
        
        // Reset opacity and show widget
        if (opacityEffect) {
            opacityEffect->setOpacity(1.0);
        }
        showWidget();
        
        // Start fade out animation (fade out over 3 seconds)
        if (fadeAnimation) {
            fadeAnimation->stop();
            fadeAnimation->disconnect(); // Disconnect any previous signals
            delete fadeAnimation;
            fadeAnimation = nullptr;
        }
        
        fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
        fadeAnimation->setDuration(5000); // 3 second fade
        fadeAnimation->setStartValue(1.0);
        fadeAnimation->setEndValue(0.0);
        
        // When fade completes, hide widget and emit finished signal
        connect(fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            hideWidget();
            emit countdownFinished();
        });
        
        fadeAnimation->start();
        return;
    }
    
    // Normal countdown mode
    isUnlimited = false;
    totalSeconds = seconds;
    remainingSeconds = seconds;
    isActive = true;
    
    // Reset opacity for normal countdown
    if (opacityEffect) {
        opacityEffect->setOpacity(1.0);
    }
    
    // Stop any existing fade animation
    if (fadeAnimation) {
        fadeAnimation->stop();
    }
    
    updateDisplay();
    showWidget();
    
    // Start the timer
    countdownTimer->start();
}

void CountdownTimerWidget::setCountdownTime(int seconds)
{
    if (seconds <= 0) {
        return;
    }
    
    totalSeconds = seconds;
    remainingSeconds = seconds;
    updateDisplay();
}

void CountdownTimerWidget::stopCountdown() {
    if (countdownTimer->isActive()) {
        countdownTimer->stop();
    }
    isActive = false;
    remainingSeconds = 0;
    updateDisplay();
}

int CountdownTimerWidget::getRemainingTime() const {
    return remainingSeconds;
}

bool CountdownTimerWidget::isCountdownActive() const {
    return isActive && countdownTimer->isActive();
}

void CountdownTimerWidget::updateCountdown() {
    if (remainingSeconds > 0) {
        remainingSeconds--;
        updateDisplay();
        
        // Emit tick signal
        emit countdownTick(remainingSeconds);
    } else {
        // Countdown finished
        countdownTimer->stop();
        isActive = false;
        
        // Hide widget and emit finished signal
        hideWidget();
        emit countdownFinished();
    }
}

void CountdownTimerWidget::updateDisplay()
{
    QString timeText = formatTime(remainingSeconds);
    timeLabel->setText(timeText);
    
    // Calculate color progression from white to #E0715C
    if (totalSeconds > 0) {
        // Calculate progress from 0.0 (start) to 1.0 (end)
        double progress = 1.0 - (double)remainingSeconds / (double)totalSeconds;
        
        // Target color #E0715C (224, 113, 92)
        int targetRed = 224;
        int targetGreen = 113;
        int targetBlue = 92;
        
        // Interpolate color from white (255, 255, 255) to #E0715C (224, 113, 92)
        int red = (int)(255 + (targetRed - 255) * progress);
        int green = (int)(255 + (targetGreen - 255) * progress);
        int blue = (int)(255 + (targetBlue - 255) * progress);
        
        // Ensure values are within valid range
        red = qBound(0, red, 255);
        green = qBound(0, green, 255);
        blue = qBound(0, blue, 255);
        
        // Apply the calculated color
        QString colorStyle = QString("color: rgb(%1, %2, %3);").arg(red).arg(green).arg(blue);
        timeLabel->setStyleSheet(colorStyle);
    } else {
        // Fallback to white if totalSeconds is 0
        timeLabel->setStyleSheet("color: #ffffff;");
    }
}

QString CountdownTimerWidget::formatTime(int seconds) const
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}
