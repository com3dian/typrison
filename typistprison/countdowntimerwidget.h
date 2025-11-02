#ifndef COUNTDOWNTIMERWIDGET_H
#define COUNTDOWNTIMERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QFont>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class CountdownTimerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CountdownTimerWidget(QWidget *parent = nullptr);
    ~CountdownTimerWidget();

    // Method to show the widget
    void showWidget();
    
    // Method to set time period and start countdown
    void startCountdown(int seconds);
    
    // Method to set time period without starting
    void setCountdownTime(int seconds);
    
    // Method to stop countdown manually
    void stopCountdown();
    
    // Get remaining time
    int getRemainingTime() const;
    
    // Check if countdown is running
    bool isCountdownActive() const;

public slots:
    // Slot to hide the widget
    void hideWidget();

signals:
    // Signal emitted when countdown finishes
    void countdownFinished();
    
    // Signal emitted every second during countdown (optional)
    void countdownTick(int remainingSeconds);

private slots:
    // Internal slot for timer updates
    void updateCountdown();

private:
    void setupUI();
    void updateDisplay();
    QString formatTime(int seconds) const;
    
    QLabel *textLabel;
    QLabel *timeLabel;
    QTimer *countdownTimer;
    QVBoxLayout *mainLayout;
    
    int totalSeconds;
    int remainingSeconds;
    bool isActive;
    bool isUnlimited;
    QPropertyAnimation *fadeAnimation;
    QGraphicsOpacityEffect *opacityEffect;
};

#endif // COUNTDOWNTIMERWIDGET_H
