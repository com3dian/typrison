#ifndef FICTIONTEXTEDIT_H
#define FICTIONTEXTEDIT_H


#include "searchWidget.h"
#include "fontmanager.h"
#include "projectmanager.h"
#include "utils/fictionhighlighter.h"
#include "utils/contextmenuutil.h"
#include "prisonermanager.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QList>
#include <QMimeData>
#include <QPainter>
#include <QPalette>
#include <QTextEdit>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTimer>
#include <QScrollBar>
#include <QString>
#include <QStringMatcher>
#include <QTextImageFormat> // tobe removed
#include <QFuture>
#include <QFutureWatcher>
#include <qtconcurrentrun.h>
#include <QPropertyAnimation>


class FictionTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    FictionTextEdit(
        QWidget *parent = nullptr,
        ProjectManager *projectManager = nullptr,
        PrisonerManager *prisonerManager = nullptr
    );

    bool isSniperMode;

    void load(const QString& text, bool keepCursorPlace = false);
    void setTopMargin(int margin);
    virtual void activateSniperMode();
    virtual void deactivateSniperMode();
    void search(const QString &searchString);
    void searchPrev(const QString &searchString);
    void clearSearch();
    void changeGlobalTextColor(const QColor &color);
    void updateFocusBlock();
    void changeFontSize(int delta);
    void applyBlockFormatting(QTextBlock &block);
    QTextCursor applyCharFormatting(QTextCursor &cursor, bool insertLargeFont = true);
    QTextCursor applyCharFormatting4NextBlock(QTextCursor &cursor);

signals:
    void onFictionEditSearch(const QString &text);
    void focusGained();
    void onSave();
    void keyboardInput();
    void showWikiAt(const QString &wikiContent, QPoint lastMousePos);
    void hideWiki();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void insertFromMimeData(const QMimeData *source) override;
    void focusInEvent(QFocusEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void showContextMenu(const QPoint &pos);

private:
    // Data structure for thread-safe block computation
    struct BlockData {
        int blockNumber;
        int top;
        int bottom;
        bool isValid;
        
        BlockData() : blockNumber(-1), top(0), bottom(0), isValid(false) {}
        BlockData(int num, int t, int b) : blockNumber(num), top(t), bottom(b), isValid(true) {}
    };
    
    struct DocumentData {
        QVector<BlockData> blocks;
        int centerY;
        int firstBlockNumber;
        int lastBlockNumber;
    };

    int getVisibleCenterY();
    int checkVisibleCenterBlock(const QTextBlock &block);
    QTextBlock findBlockClosestToCenter();
    void findBlockClosestToCenterAsync();
    void findBlockClosestToCenterAsyncThrottled();
    void findBlockClosestToCenterAsyncImpl();
    static int findBlockClosestToCenterWorker(const DocumentData &data);
    void onBlockSearchComplete();
    void refresh();
    void onTextChanged();
    void updateCursorPosition();
    void readBlock();
    void scrollToCenter(const QTextBlock &block);
    // void toggleCursorVisibility();

    int globalFontSize;
    QTextBlock previousCenteredBlock;
    QTextBlock newCenteredBlock;
    QTextBlock previousCursorBlock;
    
    FictionHighlighter* highlighter;
    int matchStringIndex;
    ProjectManager *projectManager;
    PrisonerManager *prisonerManager;
    QString previousText;
    int previousDocumentLength;
    int previousCursorPosition;
    QString previousDocumentText;
    
    QTimer *timer;
    QTimer *refreshTimer;
    QPoint lastMousePos;
    bool isInit;
    // Threading support for block computation
    QFutureWatcher<int> *blockSearchWatcher;
    QFuture<int> blockSearchFuture;
    QTimer *blockSearchThrottleTimer;
    
    // Smooth scrolling animation
    QPropertyAnimation *scrollAnimation;

};

#endif // FICTIONTEXTEDIT_H
