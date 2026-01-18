#include "fadeanimationutil.h"
#include <QAbstractAnimation>

void FadeAnimationUtil::fadeIn(QWidget *widget, int duration, QEasingCurve::Type easing)
{
    fadeInWithCallback(widget, nullptr, duration, easing);
}

void FadeAnimationUtil::fadeOut(QWidget *widget, int duration, QEasingCurve::Type easing)
{
    fadeOutWithCallback(widget, nullptr, duration, easing);
}

void FadeAnimationUtil::fadeInWithCallback(QWidget *widget, 
                                          std::function<void()> callback,
                                          int duration,
                                          QEasingCurve::Type easing)
{
    if (!widget) return;
    
    // Extract and store shadow effect parameters if present
    ShadowParams shadowParams = extractShadowParams(widget);
    
    // Create fresh opacity effect (Qt will delete the old effect when we set the new one)
    QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(fadeEffect);
    fadeEffect->setOpacity(0.0);
    
    // Show the widget
    widget->show();
    
    // Create fade-in animation
    QPropertyAnimation *animation = new QPropertyAnimation(fadeEffect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(easing);
    
    // Restore shadow effect when fade completes
    QObject::connect(animation, &QPropertyAnimation::finished, [widget, shadowParams, callback]() {
        // Restore shadow effect
        restoreShadowEffect(widget, shadowParams);
        
        // Call callback if provided
        if (callback) {
            callback();
        }
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void FadeAnimationUtil::fadeOutWithCallback(QWidget *widget, 
                                           std::function<void()> callback,
                                           int duration,
                                           QEasingCurve::Type easing)
{
    if (!widget) return;
    
    // Extract and store shadow effect parameters if present
    ShadowParams shadowParams = extractShadowParams(widget);
    
    // Create fresh opacity effect
    QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(fadeEffect);
    fadeEffect->setOpacity(1.0);
    
    // Create fade-out animation
    QPropertyAnimation *animation = new QPropertyAnimation(fadeEffect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->setEasingCurve(easing);
    
    // Hide widget when animation finishes
    QObject::connect(animation, &QPropertyAnimation::finished, [widget, shadowParams, callback]() {
        widget->hide();
        
        // Restore shadow effect for next time
        restoreShadowEffect(widget, shadowParams);
        
        // Call callback if provided
        if (callback) {
            callback();
        }
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

FadeAnimationUtil::ShadowParams FadeAnimationUtil::extractShadowParams(QWidget *widget)
{
    ShadowParams params;
    
    if (!widget) return params;
    
    // Check if widget has a shadow effect
    QGraphicsEffect *existingEffect = widget->graphicsEffect();
    QGraphicsDropShadowEffect *shadowEffect = qobject_cast<QGraphicsDropShadowEffect*>(existingEffect);
    
    if (shadowEffect) {
        params.hadShadow = true;
        params.blurRadius = shadowEffect->blurRadius();
        params.color = shadowEffect->color();
        params.offset = shadowEffect->offset();
    }
    
    return params;
}

void FadeAnimationUtil::restoreShadowEffect(QWidget *widget, const ShadowParams &params)
{
    if (!widget) return;
    
    if (params.hadShadow) {
        // Recreate the shadow effect
        QGraphicsDropShadowEffect *newShadow = new QGraphicsDropShadowEffect(widget);
        newShadow->setBlurRadius(params.blurRadius);
        newShadow->setColor(params.color);
        newShadow->setOffset(params.offset);
        widget->setGraphicsEffect(newShadow);
    } else {
        widget->setGraphicsEffect(nullptr);
    }
}
