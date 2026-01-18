#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QTextEdit>
#include <QFont>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>
#include <QContextMenuEvent>
#include <QGraphicsDropShadowEffect>
#include <QWidgetAction>
#include <functional>
#include "plaintexthighlighter.h"
#include "fontmanager.h"
#include "functionbar/menubutton.h"

class PlaintextEdit : public QTextEdit {
    Q_OBJECT

public:
    PlaintextEdit(QWidget *parent = nullptr);

    void load(const QString& text);
    void changeFontSize(int delta);
    void search(const QString &searchString);
    void searchPrev(const QString &searchString);
    void clearSearch();

signals:
    void onPlaintextSearch(const QString &text);
    void focusGained();
    void onSave();
    void keyboardInput();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void insertFromMimeData(const QMimeData *source) override;
    void focusInEvent(QFocusEvent *e) override;
    // void paintEvent(QPaintEvent *event) override;
    // void focusOutEvent(QFocusEvent *event) override;

private slots:
    void showContextMenu(const QPoint &pos);

private:
    // void toggleCursorVisibility();

    PlaintextHighlighter* highlighter;
    int globalFontSize;
    int matchStringIndex;
    // QTimer *cursorTimer;
    // bool cursorVisible;
};

#endif // PLAINTEXTEDIT_H
