#ifndef CONTEXTMENUUTIL_H
#define CONTEXTMENUUTIL_H

#include <QPoint>
#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QMenu>
#include <QWidgetAction>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QPropertyAnimation>
#include <functional>
#include "../functionbar/menubutton.h"


/*
Method for good looking context menu (menu at right click).
Used in FictionTextEdit, QMarkdownTextEdit, PlaintextEdit.
*/
class ContextMenuUtil {
public:
    // Static utility function that works with both QTextEdit and QPlainTextEdit
    template<typename TextEditType>
    static void showContextMenu(TextEditType* textEdit, const QPoint& pos) {
        // Create the menu
        QMenu* menu = new QMenu(textEdit);
        menu->setAttribute(Qt::WA_TranslucentBackground);
        menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint);
        
        // Add drop shadow effect
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(menu);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 80));
        shadow->setOffset(0, 0);
        menu->setGraphicsEffect(shadow);
        
        // Helper function to create MenuButton action
        auto createMenuButtonAction = [&](const QString &text,
                                          const QString &shortcut = QString(),
                                          bool enabled = true,
                                          std::function<void()> callback = nullptr) -> QWidgetAction* {
            MenuButton* button = new MenuButton(text, enabled ? shortcut : "", menu);
            button->setEnabled(enabled);
            button->setFixedHeight(28);
            button->setFixedWidth(128);
            
            QWidgetAction* action = new QWidgetAction(menu);
            action->setDefaultWidget(button);
            
            // Mac-specific fix: Force initial repaint to ensure proper text rendering
            #ifdef Q_OS_MAC
            QTimer::singleShot(0, button, [button]() {
                button->update();
                button->repaint();
            });
            #endif
            
            if (callback) {
                QObject::connect(button, &MenuButton::mouseClick, [menu, callback]() {
                    menu->close();
                    callback();
                });
            }
            
            return action;
        };
        
        // Get document and cursor state (works for both QTextEdit and QPlainTextEdit)
        bool hasUndo = textEdit->document()->isUndoAvailable();
        bool hasRedo = textEdit->document()->isRedoAvailable();
        bool hasSelection = textEdit->textCursor().hasSelection();

        #ifdef Q_OS_MAC
            #define CONTROL_SYMBOL "⌘ "  // Mac Command key
        #else
            #define CONTROL_SYMBOL "Ctrl+"  // Windows & Linux use "Ctrl"
        #endif
        
        // Create menu items using MenuButton
        menu->addAction(createMenuButtonAction("Undo", QString(CONTROL_SYMBOL)+"Z", hasUndo, [textEdit]() { textEdit->undo(); }));
        menu->addAction(createMenuButtonAction("Redo", QString(CONTROL_SYMBOL)+"Y", hasRedo, [textEdit]() { textEdit->redo(); }));
        
        menu->addSeparator();
        
        menu->addAction(createMenuButtonAction("Cut", QString(CONTROL_SYMBOL)+"X", hasSelection, [textEdit]() { textEdit->cut(); }));
        menu->addAction(createMenuButtonAction("Copy", QString(CONTROL_SYMBOL)+"C", hasSelection, [textEdit]() { textEdit->copy(); }));
        menu->addAction(createMenuButtonAction("Paste", QString(CONTROL_SYMBOL)+"V", true, [textEdit]() { textEdit->paste(); }));
        
        menu->addSeparator();
        
        menu->addAction(createMenuButtonAction("Select All", QString(CONTROL_SYMBOL)+"A", true, [textEdit]() { textEdit->selectAll(); }));
        
        // Style the menu
        menu->setStyleSheet(
            "QMenu {"
            "    background-color: #1F2020;"
            "    border: 1px solid #1F2020;"
            "    border-radius: 5px;"
            "    padding-left: 12px;"
            "    padding-right: 12px;"
            "    padding-top: 4px;"
            "    padding-bottom: 4px;"
            "}"
            "QMenu::separator {"
            "    height: 2px;"
            "    background-color: #262626;"
            "    margin: 2px 0px 0px 0px;"
            "}"
        );
        
        // Set initial opacity to 0 for fade-in effect
        menu->setWindowOpacity(0.0);
        
        // Create fade-in animation
        QPropertyAnimation *fadeIn = new QPropertyAnimation(menu, "windowOpacity");
        fadeIn->setDuration(100);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::InOutQuad);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
        
        // Show the menu at the cursor position (this blocks until menu closes)
        menu->exec(textEdit->mapToGlobal(pos));
        
        // Note: fade-out would happen here, but menu->exec() blocks until closed
        // The menu will disappear immediately on close, which is acceptable behavior
        
        // Clean up
        menu->deleteLater();
    }
};

#endif // CONTEXTMENUUTIL_H 