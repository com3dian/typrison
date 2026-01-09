#include "fictiontextedit.h"
#include "utils/contextmenuutil.h"
#include <QSet>

FictionTextEdit::FictionTextEdit(QWidget *parent,
                                 ProjectManager *projectManager,
                                 PrisonerManager *prisonerManager)
    : QTextEdit(parent)
    , globalFontSize(14)
    , matchStringIndex(-1)
    , projectManager(projectManager)
    , previousDocumentLength(0)
    , previousCursorPosition(0)
    , previousDocumentText("")
    , isInit(true)
    , scrollAnimation(nullptr)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Highlight, QColor("#84e0a5"));
    palette.setColor(QPalette::HighlightedText, QColor("#2C2C2C"));
    this->setPalette(palette);

    setUndoRedoEnabled(false); // forbid undo
                               // do our settings
    setTopMargin(256);         // set top margin as 256

    isSniperMode = false;      // default not sniperMode

    // set `Noto Sans Regular` as default font for fictiontextedit 
    QString notoSansRegularFamily = FontManager::instance().getNotoSansRegularFamily();
    QFont font(notoSansRegularFamily, globalFontSize);
    this->setFont(font);
    this->setCursorWidth(2);
    
    // Set up context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &FictionTextEdit::customContextMenuRequested, this, &FictionTextEdit::showContextMenu);

    // To be discussed (Between me and me):
    // Do we want a colored blinking cursor (carent)?
    // cursorTimer = new QTimer(this);
    // connect(cursorTimer, &QTimer::timeout, this, &FictionTextEdit::toggleCursorVisibility);
    // cursorTimer->setInterval(750); // Blink every 750 milliseconds
    // cursorTimer->start();
    
    setUndoRedoEnabled(true); // allow undo

    highlighter = new FictionHighlighter(this->document());

    connect(this, &QTextEdit::textChanged, this, &FictionTextEdit::onTextChanged);
    connect(this, &QTextEdit::cursorPositionChanged, this, &FictionTextEdit::updateCursorPosition);

    // image popup
    setMouseTracking(true);
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &FictionTextEdit::readBlock);

    refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(true);
    refreshTimer->setInterval(300); // 300ms delay
    connect(refreshTimer, &QTimer::timeout, this, &FictionTextEdit::refresh);
    
    // Initialize threading support for block computation
    blockSearchWatcher = new QFutureWatcher<int>(this);
    connect(blockSearchWatcher, &QFutureWatcher<int>::finished, this, &FictionTextEdit::onBlockSearchComplete);
    
    // Initialize throttling timer to batch rapid scroll events
    blockSearchThrottleTimer = new QTimer(this);
    blockSearchThrottleTimer->setSingleShot(true);
    blockSearchThrottleTimer->setInterval(50); // 50ms delay to batch rapid calls
    connect(blockSearchThrottleTimer, &QTimer::timeout, this, [this]() {
        // Call the actual async implementation directly to avoid recursion
        this->findBlockClosestToCenterAsyncImpl();
    });

}

void FictionTextEdit::onTextChanged()
{
    refreshTimer->start();
}

void FictionTextEdit::setTopMargin(int margin)
{
    qDebug() << "FictionTextEdit::setTopMargin";
    QTextDocument *doc = this->document();
    QTextFrame *rootFrame = doc->rootFrame();

    QTextFrameFormat format = rootFrame->frameFormat();
    format.setTopMargin(margin);
    rootFrame->setFrameFormat(format);
}

void FictionTextEdit::applyBlockFormatting(QTextBlock &block)
{
    qDebug() << "FictionTextEdit::applyBlockFormatting";
    bool isFirstBlock = block.blockNumber() == 0;

    // Create a cursor at the start of the block
    QTextCursor cursor(block);

    // Set block format
    QTextBlockFormat blockFormat = cursor.blockFormat();
    blockFormat.setTopMargin(0); // Set the top margin to 10 pixels
    blockFormat.setBottomMargin(32); // Set the bottom margin
    // TODO: scale the margin

    blockFormat.setLeftMargin(16);
    blockFormat.setRightMargin(16);
    cursor.setBlockFormat(blockFormat);

    // Set character format
    // Apply the character format to the whole block
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor = applyCharFormatting(cursor);
}

QTextCursor FictionTextEdit::applyCharFormatting(QTextCursor &cursor, bool insertLargeFont)
{
    qDebug() << "FictionTextEdit::applyCharFormatting";
    bool isFirstBlock = cursor.blockNumber() == 0;

    QTextCharFormat charFormat = cursor.charFormat();  // Get current char format

    QFont font = charFormat.font();
    font.setPointSize((isFirstBlock && insertLargeFont) ? (1.6 * globalFontSize) : globalFontSize);  // Adjust font size
    charFormat.setFont(font);

    if (isSniperMode) {
        int centerY = getVisibleCenterY();
        QRectF blockRect = document()->documentLayout()->blockBoundingRect(cursor.block());

        if ((blockRect.top() - 16 <= centerY) && (centerY <= blockRect.bottom() + 16)) {
            charFormat.setForeground(Qt::white);
            // previousCenteredBlock = cursor.block();  // Update previous centered block
        } else {
            QColor customColor("#656565");
            charFormat.setForeground(customColor);
        }
    } else {
        charFormat.setForeground(Qt::white);
    }

    cursor.setCharFormat(charFormat);  // Apply the new char format to the cursor

    return cursor;
}

QTextCursor FictionTextEdit::applyCharFormatting4NextBlock(QTextCursor &cursor)
{
    bool isFirstBlock = cursor.blockNumber() == 0;

    QTextCharFormat charFormat = cursor.charFormat();  // Get current char format

    QFont font = charFormat.font();
    font.setPointSize(globalFontSize);  // Adjust font size
    charFormat.setFont(font);


    // Check if scrollbar exists and if it is at the bottom
    // If yes then this is a special case
    QScrollBar *vScrollBar = verticalScrollBar();
    bool isAtBottom;
    if (vScrollBar) {
        isAtBottom = vScrollBar->value() == vScrollBar->maximum();
    } else {
        isAtBottom = false;
    }

    if (isSniperMode) {
        int centerY = getVisibleCenterY();

        QFontMetricsF fontMetrics(font);

        // Calculate the height of the text (ascent + descent)
        qreal fontHeight = fontMetrics.height();
        qreal totalHeight = fontHeight + 32;

        QRectF blockRect = document()->documentLayout()->blockBoundingRect(cursor.block());
        
        if (isAtBottom) {
            if ((blockRect.bottom() + 16 - totalHeight <= centerY) && (centerY <= blockRect.bottom() + 16)) {
                charFormat.setForeground(Qt::white);
            } else {
                QColor customColor("#656565");
                charFormat.setForeground(customColor);
            }
        } else {
            if ((blockRect.bottom() + 16 <= centerY) && (centerY <= blockRect.bottom() + totalHeight + 16)) {
                charFormat.setForeground(Qt::white);
                // previousCenteredBlock = cursor.block();  // Update previous centered block
            } else {
                QColor customColor("#656565");
                charFormat.setForeground(customColor);
            }
        }
    } else {
        charFormat.setForeground(Qt::white);
    }

    cursor.setCharFormat(charFormat);  // Apply the new char format to the cursor

    return cursor;
}

// void FictionTextEdit::paintEvent(QPaintEvent *event) {
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
//             cursorColor = QColor("#31363F");
//         }
//         painter.fillRect(cursorRect, cursorColor);
//     }
// }

// void FictionTextEdit::toggleCursorVisibility() {
//     cursorVisible = !cursorVisible;  // Toggle cursor visibility
//     update();  // Trigger a repaint
// }

// void FictionTextEdit::focusOutEvent(QFocusEvent *event) {
//     QTextEdit::focusOutEvent(event);
//     cursorTimer->stop();  // Stop blinking when focus is lost
//     cursorVisible = true; // Ensure cursor is visible when focus is lost
//     update(); // Trigger a repaint to hide the cursor
// }

/*

*/
void FictionTextEdit::keyPressEvent(QKeyEvent *event) {
    qDebug() << "FictionTextEdit::keyPressEvent";
    if (isInit) {
        isInit = false;
        this->document()->clearUndoRedoStacks();
    }

    emit keyboardInput();
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            changeFontSize(1);
            return; // Return early to avoid further processing
        } else if (event->key() == Qt::Key_Minus) {
            changeFontSize(-1);
            return; // Return early to avoid further processing
        } else if (event->key() == Qt::Key_F) {
            QString selectedText = textCursor().selectedText();
            emit onFictionEditSearch(selectedText);
            return; // Return early to avoid further processing
        }
    }

    // If the handler is at bottom and user hit enter, attach handler to bottom
    QTextCursor textCursor = this->textCursor();
    QTextBlock firstBlock = document()->firstBlock();
    QTextBlock secondBlock = firstBlock.next();
    bool isAtBottom = false;

    // Check if scrollbar exists and if it is at the bottom
    QScrollBar *vScrollBar = verticalScrollBar();
    if (vScrollBar) {
        isAtBottom = vScrollBar->value() == vScrollBar->maximum();
    }

    // If hit enter
    // Important: Do not change the following if{...}else{...}
    //            Otherwise in empty line hitting ENTER wno't insert new line
    // *** Need Fix ***
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        QTextCursor textCursorSmall = applyCharFormatting4NextBlock(textCursor);

        textCursorSmall.insertBlock();
        QTextBlock newBlock = textCursorSmall.block();
        applyBlockFormatting(newBlock);
        if (isAtBottom) {
            // Attach handler to bottom
            vScrollBar->setValue(vScrollBar->maximum());
        }
    } else {
        QTextEdit::keyPressEvent(event);
    }

    // Update the text color for the centered block
    if (isSniperMode) {
        updateFocusBlock();
    }
}

void FictionTextEdit::load(const QString &text, bool keepCursorPlace)
{
    qDebug() << "FictionTextEdit::load";
    // detach the FictionTextEdit::refresh to prevent slow out loading
    if (projectManager) {
    }

    // Clear existing content
    QTextCursor cursor = this->textCursor();
    int currentCursorPosition = cursor.position();
    int currentScrollValue = verticalScrollBar()->value();
    
    // Select the entire document
    cursor.select(QTextCursor::Document);
    
    // Remove the selected text
    cursor.removeSelectedText();
    
    // ensure the cursor is at the beginning of the document
    cursor.movePosition(QTextCursor::Start);
    this->setTextCursor(cursor);

    // Set font format
    // QTextCursor cursor = this->textCursor();
    QFont font  = this->font();
    font.setPointSize(globalFontSize);

    // Set cursor format
    QTextCharFormat charFormat = cursor.charFormat();
    charFormat.setFont(font);
    cursor.setCharFormat(charFormat);

    if (text.isEmpty()) {
        // Set the modified cursor back to the text edit
        this->setTextCursor(applyCharFormatting(cursor));
        QTextBlock newBlock = cursor.block();
        // Apply formatting
        applyBlockFormatting(newBlock);

    } else {
        // Split the input text by line breaks
        QStringList lines = text.split("\n");

        QTextCursor cursor = this->textCursor();
        cursor.insertText(lines[0]);

        // First Block
        QTextBlock firstBlock = document()->firstBlock();
        applyBlockFormatting(firstBlock);

        // Iterate through each line
        for (int i = 1; i < lines.size(); ++i) {
            // Insert a block for each line
            cursor.insertBlock();
            QTextBlock newBlock = cursor.block();

            // Insert the line text
            cursor.insertText(lines[i]);

            // Apply formatting
            applyBlockFormatting(newBlock);

            // Move to the end of the block for the next iteration
            cursor.movePosition(QTextCursor::EndOfBlock);
        } 

        if (!keepCursorPlace) {
            cursor.movePosition(QTextCursor::Start);
        } else {
            cursor.setPosition(currentCursorPosition);
            verticalScrollBar()->setValue(currentScrollValue);
        }
        
        this->setTextCursor(cursor);
    }
    previousDocumentText = this->toPlainText();

    // attach the FictionTextEdit::refresh to textchanged() signal
    if (projectManager) {
    }

    this->refresh();
}

void FictionTextEdit::insertFromMimeData(const QMimeData *source)
{
    qDebug() << "FictionTextEdit::insertFromMimeData";
    // Get plain text from the source
    QString plainText = source->text();

    // If empty return
    if (plainText.isEmpty()) {
        return;
    }

    // Get the current text cursor
    QTextCursor cursor = this->textCursor();

    QTextBlock firstBlock = this->document()->firstBlock();
    QTextBlock currentBlock = cursor.block();
    if ((cursor.blockNumber() != 0) || (not plainText.contains('\n'))) {
        // direct insert if not in first block
        cursor.insertText(plainText);
        QTextBlock newBlock = cursor.block();
        applyBlockFormatting(newBlock);
    } else {
        // Split the plain text by line breaks
        QStringList lines = plainText.split('\n');
        for (int i = 0; i <lines.size(); ++i){
            if (i == 0) {
                cursor.insertText(lines[i]);
            } else {
                if (i == 1) {
                    cursor = applyCharFormatting(cursor, false);
                }
                cursor.insertBlock();
                QTextBlock newBlock = cursor.block();
                applyBlockFormatting(newBlock);
                cursor.insertText(lines[i]);
            }
        }
    }
}

void FictionTextEdit::changeFontSize(int delta) {
    qDebug() << "FictionTextEdit::changeFontSize";
    // Find the most centered block
    QTextBlock centerBlock = findBlockClosestToCenter();
    QRectF centerBlockRect = document()->documentLayout()->blockBoundingRect(centerBlock);
    int centerBlockTop = centerBlockRect.top();
    int centerBlockBottom = centerBlockRect.bottom();
    int centerBlockMiddle = static_cast<int>(centerBlockTop + (centerBlockBottom - centerBlockTop) / 2);
    // // Get the scroll position to calculate the distance from the top of the document
    // int scrollPosition = verticalScrollBar()->value();

    // // Calculate the relative position of the center block
    // int relativePosition = centerBlockMiddle + scrollPosition;
    // qDebug() << "relativePosition" << relativePosition;


    highlighter->changeFontSize(delta);
    // set default font
    QFont font = this->font();
    font.setPointSize(globalFontSize); // Set default font size
    this->setFont(font);

    globalFontSize += delta;
    if (globalFontSize < 1) {
        globalFontSize = 1; // Prevent font size from becoming too small
    }

    // Select the entire document
    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);

    // Get the current character format of the selected text
    QTextCharFormat format;
    format.setFontPointSize(globalFontSize);

    // Merge the new format with the existing format to preserve colors
    cursor.mergeCharFormat(format);

    // Ensure the first block is formatted correctly
    QTextBlock firstBlock = document()->firstBlock();
    applyBlockFormatting(firstBlock);

    // Update the text color for the centered block
    if (isSniperMode) {
        updateFocusBlock();
    }

    // After changing the font size, scroll to keep the old center block in view
    QRectF newCenterBlockRect = document()->documentLayout()->blockBoundingRect(centerBlock);
    int newCenterBlockTop = newCenterBlockRect.top();
    int newCenterBlockBottom = newCenterBlockRect.bottom();
    int newCenterBlockMiddle = static_cast<int>(newCenterBlockTop + (newCenterBlockBottom - newCenterBlockTop) / 2);

    // Calculate the relative position of the center block
    int halfViewportHeight = (viewport()->height() - 256) / 2;
    verticalScrollBar()->setValue(newCenterBlockMiddle - halfViewportHeight);
}

/*  
Find the center of the visible area 
    
returns: int
            the y value of center
*/
int FictionTextEdit::getVisibleCenterY() {
    QRect visibleRect = viewport()->rect();
    int centerY = visibleRect.top() * 0.58 + visibleRect.bottom() * 0.42;
    // int centerY = visibleRect.center().y();
    int scrollValue = verticalScrollBar()->value();
    centerY += scrollValue;
    return centerY;
}

int FictionTextEdit::checkVisibleCenterBlock(const QTextBlock &block) {
    QRectF blockRect = document()->documentLayout()->blockBoundingRect(block);
    int centerY = getVisibleCenterY();

    if ((blockRect.top() - 16 <= centerY) && (blockRect.bottom() + 16 >= centerY)) {
        return 0;

    } else if (blockRect.top() - 16 > centerY) {
        return blockRect.top() - centerY; // positive value
        // current block is below the middleline

    } else if (blockRect.bottom() + 16 < centerY) {
        return blockRect.bottom() - centerY; // negative value
        // current block is aobve the middleline
    }
    return 0; // make compiler happy
}

/*  
Function to find the block closest to the center of the visible area (SYNCHRONOUS)
This method blocks the UI thread and should only be used when immediate results are needed,
such as in changeFontSize() where the result is needed immediately for scroll positioning.
For UI updates like focus highlighting, use findBlockClosestToCenterAsync() instead.
    
returns: QTextBlock
    block closest to the center of the visible area
*/
QTextBlock FictionTextEdit::findBlockClosestToCenter() {
    qDebug() << "FictionTextEdit::findBlockClosestToCenter";
    QTextDocument *doc = document();
    int centerY = getVisibleCenterY();

    QTextBlock low = doc->firstBlock();
    QTextBlock high = doc->lastBlock();

    if (!low.isValid() || !high.isValid()) {
        return QTextBlock(); // Return an invalid block if there are no blocks
    }

    QRectF lowBlockRect = document()->documentLayout()->blockBoundingRect(low);
    QRectF highBlockRect = document()->documentLayout()->blockBoundingRect(high);
    QRectF midBlockRect;

    float ratio = (centerY - lowBlockRect.top()) / (highBlockRect.top() - lowBlockRect.top());
    if (ratio < 0) {
        ratio = 0;
    } else if (ratio > 1) {
        ratio = 0.99;
    }
    int midBlockNumber = low.blockNumber() + (high.blockNumber() - low.blockNumber()) * ratio;

    // Ensure the binary search terminates properly
    while (low.blockNumber() < high.blockNumber()) {
        if (low.blockNumber() == -1) {
            return doc->firstBlock();
        }

        QTextBlock midBlock = doc->findBlockByNumber(midBlockNumber);
        midBlockRect = document()->documentLayout()->blockBoundingRect(midBlock);
        int position = checkVisibleCenterBlock(midBlock);

        if (position == 0) {
            // if midBlock is at the middle of visible area
            return midBlock;
        } else if (position > 0) {
            // current block is below the middle line
            high = midBlock;
            highBlockRect = document()->documentLayout()->blockBoundingRect(high);

            // base on the ratio try to locate the block insected with middle line
            ratio = (centerY - lowBlockRect.top()) / (highBlockRect.top() - lowBlockRect.top());
            if (ratio < 0) {
                ratio = 0;
            } else if (ratio > 1) {
                ratio = 0.99;
            }

            midBlockNumber = low.blockNumber() + (high.blockNumber() - low.blockNumber()) * ratio;

        } else if (position < 0) {
            // current block is above the middle line
            low = midBlock.next();
            lowBlockRect = document()->documentLayout()->blockBoundingRect(low);

            // base on the ratio try to locate the block insected with middle line
            ratio = (centerY - lowBlockRect.top()) / (highBlockRect.top() - lowBlockRect.top());
            if (ratio < 0) {
                ratio = 0;
            } else if (ratio > 1) {
                ratio = 0.99;
            }

            midBlockNumber = low.blockNumber() + (high.blockNumber() - low.blockNumber()) * ratio;
        }

        // Break condition to prevent infinite loop
        if (low.blockNumber() == high.blockNumber()) {
            break;
        }
    }

    // Now low is the closest block below or at the center
    // Check the block before and after to find the closest one
    QTextBlock closestBlock = low;
    int position = checkVisibleCenterBlock(closestBlock);
    if (position == 0) {
        return closestBlock;
    } else if (position < 0) {
        if (closestBlock.previous().isValid()) {
            return  closestBlock.previous();
        } else {
            return closestBlock;
        }
        
    } else if (position > 0) {
        if (closestBlock.next().isValid()) {
            return closestBlock.next();
        } else {
            return closestBlock;
        }
    }

    return closestBlock; // make compiler happy 
}

/*
UI Thread                    Background Thread
┌─────────────────────────┐  ┌──────────────────────┐
│ Entry Points:           │  │                      │
│ ├─ Sync (immediate)     │  │                      │
│ ├─ Async (direct)       │  │ Worker Thread        │
│ └─ Async (throttled)    │  │ ├─ Binary Search     │
│                         │  │ ├─ Block Calculation │
│ Data Extraction         │  │ └─ Pure Logic        │
│ ├─ Extract block rects  │  │                      │
│ ├─ Get center Y         │  │                      │
│ └─ Create thread-safe   │  │                      │
│   data structures       │  │                      │
│                         │  │                      │
│ Result Handling         │  │                      │
│ ├─ Apply formatting     │  │                      │
│ └─ Update UI            │  │                      │
└─────────────────────────┘  └──────────────────────┘
*/

/*
Function to find the block closest to the center of the visible area (ASYNCHRONOUS)
This method performs the computation in a background thread to avoid blocking the UI.
Use this for UI updates like focus highlighting during scrolling where slight delay is acceptable.
The result is handled in onBlockSearchComplete() when the computation finishes.
*/
void FictionTextEdit::findBlockClosestToCenterAsync() {
    // Direct call to implementation
    findBlockClosestToCenterAsyncImpl();
}

/*
- document()->documentLayout()->blockBoundingRect() must be called in UI thread
- We do one-time extraction of all needed data
- Worker thread gets pure numerical data - no GUI dependencies
*/
void FictionTextEdit::findBlockClosestToCenterAsyncThrottled() {
    // Use throttling to batch rapid calls (e.g., during fast scrolling)
    blockSearchThrottleTimer->start();
}

void FictionTextEdit::findBlockClosestToCenterAsyncImpl() {
    qDebug() << "FictionTextEdit::findBlockClosestToCenterAsyncImpl";
    // Cancel any existing computation to avoid stacking multiple requests
    if (blockSearchWatcher->isRunning()) {
        blockSearchFuture.cancel();
        blockSearchWatcher->waitForFinished();
    }
    
    // Extract thread-safe data from document
    QTextDocument *doc = document();
    DocumentData docData;
    docData.centerY = getVisibleCenterY();
    
    QTextBlock firstBlock = doc->firstBlock();
    QTextBlock lastBlock = doc->lastBlock();
    
    if (!firstBlock.isValid() || !lastBlock.isValid()) {
        return; // No blocks available
    }
    
    docData.firstBlockNumber = firstBlock.blockNumber();
    docData.lastBlockNumber = lastBlock.blockNumber();
    
    // Extract bounding rectangles for all blocks (this must be done in main thread)
    QTextBlock block = firstBlock;
    while (block.isValid()) {
        QRectF blockRect = document()->documentLayout()->blockBoundingRect(block);
        docData.blocks.append(BlockData(
            block.blockNumber(),
            static_cast<int>(blockRect.top()),
            static_cast<int>(blockRect.bottom())
        ));
        
        block = block.next();
        if (block.blockNumber() > lastBlock.blockNumber()) {
            break;
        }
    }
    
    // Start async computation
    blockSearchFuture = QtConcurrent::run(&FictionTextEdit::findBlockClosestToCenterWorker, docData);
    blockSearchWatcher->setFuture(blockSearchFuture);
}

int FictionTextEdit::findBlockClosestToCenterWorker(const DocumentData &data) {
    qDebug() << "FictionTextEdit::findBlockClosestToCenterWorker";
    if (data.blocks.isEmpty()) {
        return -1;
    }
    
    int centerY = data.centerY;
    int lowIndex = 0;
    int highIndex = data.blocks.size() - 1;
    
    // Helper lambda to check visible center block using extracted data
    auto checkVisibleCenterBlockData = [&](const BlockData &blockData) -> int {
        if ((blockData.top - 16 <= centerY) && (blockData.bottom + 16 >= centerY)) {
            return 0;
        } else if (blockData.top - 16 > centerY) {
            return blockData.top - centerY; // positive value
        } else if (blockData.bottom + 16 < centerY) {
            return blockData.bottom - centerY; // negative value
        }
        return 0;
    };
    
    // Estimate initial midpoint using ratio
    const BlockData &lowBlock = data.blocks[lowIndex];
    const BlockData &highBlock = data.blocks[highIndex];
    
    float ratio = static_cast<float>(centerY - lowBlock.top) / (highBlock.top - lowBlock.top);
    if (ratio < 0) {
        ratio = 0;
    } else if (ratio > 1) {
        ratio = 0.99f;
    }
    int midIndex = lowIndex + static_cast<int>((highIndex - lowIndex) * ratio);
    
    // Binary search
    while (lowIndex < highIndex) {
        if (midIndex < 0 || midIndex >= data.blocks.size()) {
            midIndex = lowIndex + (highIndex - lowIndex) / 2;
        }
        
        const BlockData &midBlock = data.blocks[midIndex];
        int position = checkVisibleCenterBlockData(midBlock);
        
        if (position == 0) {
            return midBlock.blockNumber;
        } else if (position > 0) {
            // current block is below the middle line
            highIndex = midIndex;
            const BlockData &newHighBlock = data.blocks[highIndex];
            
            // Update ratio for next iteration
            ratio = static_cast<float>(centerY - lowBlock.top) / (newHighBlock.top - lowBlock.top);
            if (ratio < 0) {
                ratio = 0;
            } else if (ratio > 1) {
                ratio = 0.99f;
            }
            midIndex = lowIndex + static_cast<int>((highIndex - lowIndex) * ratio);
        } else {
            // current block is above the middle line
            lowIndex = midIndex + 1;
            if (lowIndex < data.blocks.size()) {
                const BlockData &newLowBlock = data.blocks[lowIndex];
                
                // Update ratio for next iteration
                ratio = static_cast<float>(centerY - newLowBlock.top) / (highBlock.top - newLowBlock.top);
                if (ratio < 0) {
                    ratio = 0;
                } else if (ratio > 1) {
                    ratio = 0.99f;
                }
                midIndex = lowIndex + static_cast<int>((highIndex - lowIndex) * ratio);
            }
        }
        
        // Break condition to prevent infinite loop
        if (lowIndex == highIndex) {
            break;
        }
    }
    
    // Find the closest block
    if (lowIndex < data.blocks.size()) {
        const BlockData &closestBlock = data.blocks[lowIndex];
        int position = checkVisibleCenterBlockData(closestBlock);
        
        if (position == 0) {
            return closestBlock.blockNumber;
        } else if (position < 0) {
            if (lowIndex > 0) {
                return data.blocks[lowIndex - 1].blockNumber;
            } else {
                return closestBlock.blockNumber;
            }
        } else {
            if (lowIndex + 1 < data.blocks.size()) {
                return data.blocks[lowIndex + 1].blockNumber;
            } else {
                return closestBlock.blockNumber;
            }
        }
    }
    
    return data.firstBlockNumber; // Fallback
}

void FictionTextEdit::onBlockSearchComplete() {
    if (blockSearchWatcher->isCanceled()) {
        return; // Computation was cancelled
    }
    
    int blockNumber = blockSearchWatcher->result();
    if (blockNumber >= 0) {
        QTextDocument *doc = document();
        QTextBlock foundBlock = doc->findBlockByNumber(blockNumber);
        
        if (foundBlock.isValid()) {
            // Update the focus block with the result
            if (isSniperMode) {
                // Set the previous centered block to grey
                if (previousCenteredBlock.isValid()) {
                    applyBlockFormatting(previousCenteredBlock);
                }
                
                // Set the new centered block to white
                newCenteredBlock = foundBlock;
                applyBlockFormatting(newCenteredBlock);
                previousCenteredBlock = newCenteredBlock;
            }
        }
    }
}

void FictionTextEdit::updateFocusBlock() {
    qDebug() << "FictionTextEdit::updateFocusBlock";
    disconnect(this, &QTextEdit::textChanged, this, &FictionTextEdit::refresh);

    int centerY = getVisibleCenterY();

    // Check if the previously centered block is still close to the center
    if (previousCenteredBlock.isValid()) {
        int position = checkVisibleCenterBlock(previousCenteredBlock);
        if (position == 0) {
            // Block is at center, but check if it's actually highlighted
            // Get the first character's foreground color to check current state
            QTextCursor cursor(previousCenteredBlock);
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            QColor currentColor = cursor.charFormat().foreground().color();
            
            // If the block is grey instead of white, re-apply formatting to highlight it
            if (currentColor != Qt::white && currentColor.name() == "#656565") {
                applyBlockFormatting(previousCenteredBlock);
            }
            
            return; // The previous centered block is still close enough, no need to update
        }
    }

    // If there was a previously centered block, set it to grey
    if (previousCenteredBlock.isValid()) {
        applyBlockFormatting(previousCenteredBlock);
    }

    // check the prevous & next blocks
    int positionPrevious = checkVisibleCenterBlock(previousCenteredBlock.previous());
    if (positionPrevious == 0) {
        newCenteredBlock = previousCenteredBlock.previous();
        applyBlockFormatting(newCenteredBlock);
        previousCenteredBlock = newCenteredBlock;
        return;
    }
    int positionNext = checkVisibleCenterBlock(previousCenteredBlock.next());
    if (positionNext == 0) {
        newCenteredBlock = previousCenteredBlock.next();
        applyBlockFormatting(newCenteredBlock);
        previousCenteredBlock = newCenteredBlock;
        return;
    }

    // Find the new centered block asynchronously with throttling for frequent scroll events
    findBlockClosestToCenterAsyncThrottled();
    
    // connect(this, &QTextEdit::textChanged, this, &FictionTextEdit::refresh);
}

void FictionTextEdit::changeGlobalTextColor(const QColor &color)
{
    qDebug() << "FictionTextEdit::changeGlobalTextColor";
    // Select the entire document
    QTextCursor cursor(this->document());
    cursor.select(QTextCursor::Document);

    // Get the current character format of the selected text
    QTextCharFormat format;
    format.setForeground(color);

    // Merge the new format with the existing format to preserve colors
    cursor.mergeCharFormat(format);

    // Ensure the centered block is formatted correctly (async)
    if (isSniperMode) {
        findBlockClosestToCenterAsync();
    }
}

void FictionTextEdit::activateSniperMode() {
    isSniperMode = true;
    QColor customColor("#656565"); // gray color
    changeGlobalTextColor(customColor); // Change all text to gray

    previousCursorBlock = textCursor().block();

    // Update the previously centered block to the new highlight color
    updateFocusBlock();

    // Connect the scroll bar value change to the updateFocusBlock method
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FictionTextEdit::updateFocusBlock);
}

void FictionTextEdit::deactivateSniperMode() {
    isSniperMode = false;
    changeGlobalTextColor(Qt::white); // Change all text to white

    // Reset previousCursorBlock to invalid state
    previousCursorBlock = QTextBlock();
    
    // Stop any ongoing scroll animation
    if (scrollAnimation) {
        scrollAnimation->stop();
        delete scrollAnimation;
        scrollAnimation = nullptr;
    }

    disconnect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FictionTextEdit::updateFocusBlock);
}

void FictionTextEdit::focusInEvent(QFocusEvent *e) {
    QTextEdit::focusInEvent(e); // Call base class implementation
    emit focusGained(); // Emit the signal
    // cursorTimer->start(); // Start cursor timer
}

void FictionTextEdit::search(const QString &searchString) {
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

void FictionTextEdit::searchPrev(const QString &searchString) {
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

void FictionTextEdit::clearSearch() {
    highlighter->setSearchString("");
    matchStringIndex = -1;
}

/*  Update cursor position to be used in FictionTextEdit::refresh()
    
    return None
*/
void FictionTextEdit::updateCursorPosition() {
    QTextCursor cursor = this->textCursor();
    if (isSniperMode) {
        QTextBlock currentBlock = cursor.block();
        // Only auto-scroll if there's no active selection to avoid disrupting multi-line selections
        if (previousCursorBlock.isValid() && currentBlock != previousCursorBlock && !cursor.hasSelection()) {
            this->scrollToCenter(currentBlock);
        }
        previousCursorBlock = currentBlock;
    }
    previousCursorPosition = cursor.position();
}

/*
refresh() used to filter banned words when user input;

┌────────┐
│        │
│        │
│        │
└────────┘
*/
void FictionTextEdit::refresh() {
    qDebug() << "FictionTextEdit::refresh";
    if (projectManager) {
        if (projectManager->isLoadedProject) {
            QString currentDocumentText = this->toPlainText();
            // if no text changes is loaded into document
            if (currentDocumentText == previousDocumentText) {
                return;
            }

            // get cursor position and scroll value
            QTextCursor cursor = this->textCursor();
            int currentCursorPosition = cursor.position();
            // int currentScrollValue = verticalScrollBar()->value();

            // get current document length and compare with previous length;
            int currentDocumentLength = this->document()->characterCount() - 1;

            int startIndex;
            int endIndex;
            int maxiumBannedWordLength = projectManager->getMaxiumBannedWordLength();

            // identify the the type of text change
            int lengthDifference = currentDocumentLength - previousDocumentLength;
            int minCursorIndex = std::min(currentCursorPosition, previousCursorPosition);
            int maxCursorIndex = std::max(currentCursorPosition, previousCursorPosition);

            if ( (currentDocumentText.mid(0, minCursorIndex - 1) == previousDocumentText.mid(0, minCursorIndex - 1))
                 &
                 (currentDocumentText.mid(currentCursorPosition) == previousDocumentText.mid(previousCursorPosition)) ) {
                // If insertion or removal
                // keyboard input; keyboard deletion; keyboard pasting; select and deletion; 

                startIndex = currentCursorPosition - lengthDifference - maxiumBannedWordLength;
                endIndex = currentCursorPosition + maxiumBannedWordLength;
            } else {
                // Perform binary search to find the longest prefix that is a substring in both the
                // previous and current documents.
                int left = 0;                                   // Minimum length of prefix
                int right = minCursorIndex;                     // Maximum length of prefix
                int bestLength = 0;

                while (left <= right) {
                    int mid = left + (right - left) / 2;        // Midpoint for binary search
                    QString prefix = currentDocumentText.left(mid);

                    if (previousDocumentText.contains(prefix)) {
                        bestLength = mid;                       // Update the best length found
                        left = mid + 1;                         // Try for a longer prefix
                    } else {
                        right = mid - 1;                        // Try for a shorter prefix
                    }
                }
                startIndex = bestLength - maxiumBannedWordLength;
                endIndex = maxCursorIndex + maxiumBannedWordLength; // todo: better update
            }

            // make start and end index wthin range of QString  
            if (startIndex < 0) {
                startIndex = 0;                                // Clamp startIndex to 0
            }
            if (endIndex >= currentDocumentLength) {
                endIndex = currentDocumentLength - 1;          // C
            }

            QString subChangedText = currentDocumentText.mid(startIndex, endIndex - startIndex + 1);
            QString filteredText = projectManager->matchBannedWords(subChangedText);

            for (int index = startIndex; index <= endIndex; ++index) {
                // Bounds check to prevent crash
                // Break if we've reached the end of either string to prevent out-of-bounds access
                if ((index - startIndex) >= subChangedText.length() || (index - startIndex) >= filteredText.length()) {
                    break;
                }
                QChar subChangedChar = subChangedText.at(index - startIndex);
                QChar filteredChar = filteredText.at(index - startIndex);

                if (subChangedChar != filteredChar) {

                    // Create a QTextCursor for the QTextEdit
                    QTextCursor cursor = this->textCursor();
                    // Set the cursor position
                    cursor.setPosition(index);
                    // Perform deletion of the character at the cursor
                    cursor.deleteChar();
                    // Insert the "*"
                    cursor.insertText("*");

                }
            }
        }
    }
    // to be used in next FictionTextEdit::refresh
    previousDocumentLength = this->document()->characterCount() - 1;
    QTextCursor cursor = this->textCursor();
    previousCursorPosition = cursor.position();
    previousDocumentText = this->document()->toPlainText();
    return;
}

void FictionTextEdit::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "FictionTextEdit::mouseMoveEvent";
    lastMousePos = event->pos();

    emit hideWiki();

    timer->start(1000);  // Restart timer (1 sec delay)
    QTextEdit::mouseMoveEvent(event);
}

void FictionTextEdit::readBlock() {
    // Get cursor at mouse position
    QTextCursor cursor = cursorForPosition(lastMousePos);
    QTextBlock block = cursor.block();
    QString blockText = block.text();

    if (blockText.isEmpty() || !projectManager) {
        return;
    }
    
    // Get matches from wikiTrie for the block text
    QMap<QString, QList<QPair<int, QString>>> matches = projectManager->matchWikiContent(blockText);
    
    // Get the block's layout
    QTextLayout* layout = block.layout();
    
    // Get the block's position in document coordinates
    QPointF blockPos = document()->documentLayout()->blockBoundingRect(block).topLeft();
    QSet<QString> matchedWikiKeysSet;  // Use QSet to ensure unique keys

    // Iterate through the matches
    for (const QString& wikiKey : matches.keys()) {
        qDebug() << "wikiKey: " << wikiKey;
        qDebug() << "matches[wikiKey]: " << matches[wikiKey];
        qDebug() << "matches[wikiKey].size(): " << matches[wikiKey].size();
        qDebug() << "matches[wikiKey][0].first: " << matches[wikiKey][0].first;
        const QList<QPair<int, QString>>& matchesList = matches[wikiKey];
        
        
        for (const QPair<int, QString>& match : matchesList) {
            int startPos = match.first;
            QString matchedText = match.second;
            int endPos = startPos + matchedText.length();
            qDebug() << "startPos: " << startPos;
            qDebug() << "matchedText: " << matchedText;
            qDebug() << "endPos: " << endPos;
            
            // Iterate through each character in the matched text
            QVector<QRectF> rects;
            for (int pos = startPos; pos < endPos; ++pos) {
                // Get the line containing this character
                QTextLine line = layout->lineForTextPosition(pos);
                if (!line.isValid()) continue;

                // Get x-position for this character
                int relativePos = pos;
                int lineStartPos = line.textStart();
                int posInLine = relativePos - lineStartPos;
                
                // Skip if position is invalid
                if (posInLine < 0 || posInLine >= line.textLength()) continue;

                // Get the block's left margin from its format
                qreal leftMargin = block.blockFormat().leftMargin();

                // Get x-position including margin
                qreal x = line.cursorToX(relativePos);
                qreal nextX;
                
                // Get the right edge of the current character
                if (posInLine + 1 >= line.textLength()) {
                    // For the last character in line, use the line's natural width plus margin
                    nextX = line.naturalTextWidth() + leftMargin;
                } else {
                    // For all other characters, use the next character's position plus margin
                    nextX = line.cursorToX(relativePos + 1);
                }

                // Create rectangle for this character
                QRectF charRect(
                    blockPos.x() + x,
                    blockPos.y() + line.y(),
                    nextX - x,
                    line.height()
                );

                // Only add non-zero width rectangles
                rects.append(charRect);
            }

            // Now merge rectangles that are on the same line
            QVector<QRectF> mergedRects;
            for (const QRectF& charRect : rects) {
                if (mergedRects.isEmpty() || 
                    qAbs(mergedRects.last().top() - charRect.top()) > 1) {
                    // New line, add new rectangle
                    mergedRects.append(charRect);
                } else {
                    // Same line, merge with previous rectangle
                    QRectF& lastRect = mergedRects.last();
                    lastRect.setRight(charRect.right());
                }
            }
            rects = mergedRects;

            // Check if mouse is over any of the rectangles
            for (const QRectF& rect : rects) {
                // Convert document coordinates to viewport coordinates
                QRect viewportRect = viewport()->rect();
                QPointF viewportOffset(horizontalScrollBar()->value(), verticalScrollBar()->value());
                QRectF adjustedRect = rect.translated(-viewportOffset);
                
                // Add padding for easier hit detection
                QRectF hitRect = adjustedRect.adjusted(-5, -5, 5, 5);
                
                if (hitRect.contains(lastMousePos)) {
                    matchedWikiKeysSet.insert(wikiKey);  // Use insert() to ensure uniqueness
                    
                }
            }
        }
    }
    // Convert QSet to QList for getContentByKeys()
    QList<QString> matchedWikiKeys = matchedWikiKeysSet.values();
    // Here you can trigger your tooltip or other UI elements
    QString fullMatchedContent = projectManager->getContentByKeys(matchedWikiKeys);
    if (fullMatchedContent.isEmpty()) {
        return;
    }
    QPoint globalPos = mapToGlobal(lastMousePos);
    emit showWikiAt(fullMatchedContent, globalPos);
}

void FictionTextEdit::showContextMenu(const QPoint &pos) {
    ContextMenuUtil::showContextMenu(this, pos);
}

void FictionTextEdit::scrollToCenter(const QTextBlock &block) {
    if (!block.isValid()) {
        return;
    }

    QScrollBar *vScrollBar = verticalScrollBar();
    if (!vScrollBar) {
        return;
    }

    // Calculate target scroll position
    QRectF blockRect = document()->documentLayout()->blockBoundingRect(block);
    int blockMiddle = static_cast<int>(blockRect.top() + blockRect.height() / 2);
    QRect visibleRect = viewport()->rect();
    int centerY = visibleRect.top() * 0.58 + visibleRect.bottom() * 0.42;
    int targetScrollValue = blockMiddle - centerY;
    
    // Clamp to valid range
    targetScrollValue = qBound(vScrollBar->minimum(), targetScrollValue, vScrollBar->maximum());
    
    // Get current scroll position
    int currentScrollValue = vScrollBar->value();
    
    // If already at target, no need to animate
    if (currentScrollValue == targetScrollValue) {
        return;
    }
    
    // Stop any existing animation
    if (scrollAnimation) {
        scrollAnimation->stop();
        delete scrollAnimation;
    }
    
    scrollAnimation = new QPropertyAnimation(vScrollBar, "value", this);
    scrollAnimation->setDuration(250);
    scrollAnimation->setStartValue(currentScrollValue);
    scrollAnimation->setEndValue(targetScrollValue);
    scrollAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    
    // Connect to finished signal to ensure correct coloring after animation completes
    connect(scrollAnimation, &QPropertyAnimation::finished, this, &FictionTextEdit::updateFocusBlock);
    
    scrollAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    scrollAnimation = nullptr;
}