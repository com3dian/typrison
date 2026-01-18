#ifndef FADEANIMATIONUTIL_H
#define FADEANIMATIONUTIL_H

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <functional>

/**
 * @brief Utility class for applying fade in/out animations to QWidget-based objects
 * 
 * This class provides static methods for easily adding fade animations to any QWidget.
 * It automatically handles:
 * - Creating and managing QGraphicsOpacityEffect
 * - Preserving and restoring QGraphicsDropShadowEffect (if present)
 * - Auto-showing widgets on fade-in
 * - Auto-hiding widgets on fade-out
 * - Cleaning up animation objects
 */
class FadeAnimationUtil
{
public:
    /**
     * @brief Fades in a widget from transparent to opaque
     * 
     * @param widget The widget to fade in (will be shown automatically)
     * @param duration Animation duration in milliseconds (default: 200)
     * @param easing Easing curve type (default: InOutQuad)
     */
    static void fadeIn(QWidget *widget, 
                      int duration = 200, 
                      QEasingCurve::Type easing = QEasingCurve::InOutQuad);
    
    /**
     * @brief Fades out a widget from opaque to transparent
     * 
     * @param widget The widget to fade out (will be hidden automatically when done)
     * @param duration Animation duration in milliseconds (default: 200)
     * @param easing Easing curve type (default: InOutQuad)
     */
    static void fadeOut(QWidget *widget, 
                       int duration = 200, 
                       QEasingCurve::Type easing = QEasingCurve::InOutQuad);
    
    /**
     * @brief Fades in a widget with a callback function
     * 
     * @param widget The widget to fade in
     * @param callback Function to call when fade-in completes
     * @param duration Animation duration in milliseconds (default: 200)
     * @param easing Easing curve type (default: InOutQuad)
     */
    static void fadeInWithCallback(QWidget *widget, 
                                  std::function<void()> callback,
                                  int duration = 200,
                                  QEasingCurve::Type easing = QEasingCurve::InOutQuad);
    
    /**
     * @brief Fades out a widget with a callback function
     * 
     * @param widget The widget to fade out
     * @param callback Function to call when fade-out completes
     * @param duration Animation duration in milliseconds (default: 200)
     * @param easing Easing curve type (default: InOutQuad)
     */
    static void fadeOutWithCallback(QWidget *widget, 
                                   std::function<void()> callback,
                                   int duration = 200,
                                   QEasingCurve::Type easing = QEasingCurve::InOutQuad);

private:
    // Structure to hold shadow effect parameters
    struct ShadowParams {
        bool hadShadow = false;
        int blurRadius = 24;
        QColor color = QColor("#1F2020");
        QPointF offset = QPointF(0, 0);
    };
    
    /**
     * @brief Extracts shadow effect parameters from a widget
     */
    static ShadowParams extractShadowParams(QWidget *widget);
    
    /**
     * @brief Restores shadow effect to a widget
     */
    static void restoreShadowEffect(QWidget *widget, const ShadowParams &params);
};

#endif // FADEANIMATIONUTIL_H
