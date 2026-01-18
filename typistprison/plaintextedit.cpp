#include "plaintextedit.h"
#include "utils/contextmenuutil.h"

PlaintextEdit::PlaintextEdit(QWidget *parent)
    : QTextEdit(parent), 
      globalFontSize(14), 
      matchStringIndex(-1)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Highlight, QColor("#84e0a5"));
    palette.setColor(QPalette::HighlightedText, QColor("#2C2C2C"));
    this->setPalette(palette);

    // set `Noto Sans Regular` as default font for fictiontextedit 
    QString notoSansRegularFamily = FontManager::instance().getNotoSansMonoFamily();
    QFont font(notoSansRegularFamily, globalFontSize);
    this->setFont(font);
    this->setCursorWidth(2);
    
    // Set up context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &PlaintextEdit::customContextMenuRequested, this, &PlaintextEdit::showContextMenu);
    
    // cursorTimer = new QTimer(this);
    // connect(cursorTimer, &QTimer::timeout, this, &PlaintextEdit::toggleCursorVisibility);
    // cursorTimer->setInterval(750); // Blink every 750 milliseconds
    // cursorTimer->start();

    highlighter = new PlaintextHighlighter(this->document());
}

void PlaintextEdit::load(const QString &text)
{
    // Clear existing content
    clear();
    QFont font  = this->font();
    font.setPointSize(globalFontSize);
    this->setFont(font);

    // Load plain text
    this->setPlainText(text);
}

void PlaintextEdit::keyPressEvent(QKeyEvent *event) {
    // emit keyboardInput();
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            // If user key press "Ctrl" + "+", increase fontsize
            changeFontSize(1);
            return;
        } else if (event->key() == Qt::Key_Minus) {
            changeFontSize(-1);
            return;
        } else if (event->key() == Qt::Key_F) {
            QString selectedText = textCursor().selectedText();
            emit onPlaintextSearch(selectedText);
            return;
        }
    }
    QTextEdit::keyPressEvent(event);
}

void PlaintextEdit::insertFromMimeData(const QMimeData *source) {
    QString plainText = source->text();
    QTextCursor cursor = this->textCursor();
    cursor.insertText(plainText);
}

// void PlaintextEdit::paintEvent(QPaintEvent *event) {
//     QTextEdit::paintEvent(event);

//     // Drawing a custom cursor
//     if (hasFocus()) {
//         QPainter painter(viewport());
//         QTextCursor cursor = textCursor();
        
//         // Get the rectangle representing the cursor's position
//         QRect cursorRect = this->cursorRect();

//         // Adjust the cursor width as desired
//         int cursorWidth = 2;
//         cursorRect.setWidth(cursorWidth);
//         QColor cursorColor;
//         if (cursorVisible) {
//             cursorColor = QColor("#84e0a5");  // Cursor color when visible
//         } else {
//             cursorColor = QColor("#31363F");  // Background color
//         }
//         painter.fillRect(cursorRect, cursorColor);
//     }
// }

// void PlaintextEdit::toggleCursorVisibility() {
//     cursorVisible = !cursorVisible;  // Toggle cursor visibility
//     update();  // Trigger a repaint
// }

// void PlaintextEdit::focusOutEvent(QFocusEvent *event) {
//     QTextEdit::focusOutEvent(event);
//     cursorTimer->stop();  // Stop blinking when focus is lost
//     cursorVisible = true; // Ensure cursor is visible when focus is lost
//     update(); // Trigger a repaint to hide the cursor
// }

void PlaintextEdit::changeFontSize(int delta) {
    QScrollBar *vScrollBar = this->verticalScrollBar();
    int scrollBarPosition = vScrollBar->value();
    float positionRatio = static_cast<float>(scrollBarPosition) / static_cast<float>(vScrollBar->maximum());

    globalFontSize += delta;
    if (globalFontSize < 1) {
        globalFontSize = 1; // Prevent font size from becoming too small
    }

    QTextCursor cursor(this->document());
    cursor.select(QTextCursor::Document);

    QTextCharFormat format;
    format.setFontPointSize(globalFontSize);

    cursor.mergeCharFormat(format);

    int updatedPosition = static_cast<int>(positionRatio * static_cast<float>(vScrollBar->maximum()));
    vScrollBar->setValue(updatedPosition);
}

void PlaintextEdit::focusInEvent(QFocusEvent *e) {
    QTextEdit::focusInEvent(e); // Call base class implementation
    emit focusGained(); // Emit the signal
    // cursorTimer->start(); // Start cursor timer
}

void PlaintextEdit::search(const QString &searchString) {
    // Reset matchStringIndex if searchString has changed
    if (searchString != highlighter->getSearchString()) {
        matchStringIndex = -1;
    }

    QString documentText = this->document()->toPlainText();

    // Convert both document text and search string to lower case for case-insensitive comparison
    QString lowerDocumentText = documentText.toLower();
    QString lowerSearchString = searchString.toLower();

    // Perform the case-insensitive search
    matchStringIndex = lowerDocumentText.indexOf(lowerSearchString, matchStringIndex + 1);
    if (matchStringIndex == -1) {
        // Try searching again if the first search didn't find any match
        matchStringIndex = lowerDocumentText.indexOf(lowerSearchString, matchStringIndex + 1);
        if (matchStringIndex == -1) {
            return; // No match found
        }
    }

    // Update matchStringIndex to account for the position in the original document text
    QTextCursor cursor = this->textCursor();
    cursor.setPosition(matchStringIndex);
    cursor.setPosition(matchStringIndex + searchString.length(), QTextCursor::KeepAnchor);
    this->setTextCursor(cursor);

    highlighter->setSearchString(searchString);
}

void PlaintextEdit::searchPrev(const QString &searchString) {
    QString documentText = this->document()->toPlainText();
    
    // Convert both document text and search string to lower case for case-insensitive comparison
    QString lowerDocumentText = documentText.toLower();
    QString lowerSearchString = searchString.toLower();

    // Find the last index of the search string in the document text, case-insensitively
    int searchStartIndex = (matchStringIndex == -1) ? documentText.length() - 1 : matchStringIndex - 1;
    matchStringIndex = lowerDocumentText.lastIndexOf(lowerSearchString, searchStartIndex);

    if (matchStringIndex == -1) {
        // First time search not found, search the entire document
        matchStringIndex = lowerDocumentText.lastIndexOf(lowerSearchString);
        if (matchStringIndex == -1) {
            // Second time search not found
            return;
        }
    }

    // Set the cursor to the position of the match
    QTextCursor cursor = this->textCursor();
    cursor.setPosition(matchStringIndex);
    cursor.setPosition(matchStringIndex + searchString.length(), QTextCursor::KeepAnchor);
    this->setTextCursor(cursor);
}

void PlaintextEdit::clearSearch() {
    highlighter->setSearchString("");
    matchStringIndex = -1;
}

void PlaintextEdit::showContextMenu(const QPoint &pos) {
    ContextMenuUtil::showContextMenu(this, pos);
}
