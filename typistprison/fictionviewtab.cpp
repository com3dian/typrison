/*
FictionViewTab

mot
*/

#include "fictionviewtab.h"
#include <QTabWidget>
#include <QEnterEvent>
#include <QEvent>


FictionViewTab::FictionViewTab(const QString &content,
                               const QString &filePath,
                               QWidget *parent,
                               bool isPrisoner,
                               ProjectManager *projectManager,
                               PrisonerManager *prisonerManager)
    : BaseTextEditTab(content, filePath, parent), 
      vScrollBar(new QScrollBar(Qt::Vertical, this)),
      wordCountSpacerRight(nullptr),
      oldTextContent(""),
      isPrisoner(isPrisoner),
      projectManager(projectManager),
      prisonerManager(prisonerManager),
      prisonerInitialContent("")
{
    // Remove currentFilePath initialization as it's handled by BaseTextEditTab
    globalLayout = new QHBoxLayout(this);
    leftLayout = new QVBoxLayout();
    topLeftLayout = new QHBoxLayout();
    bottomLeftLayout = new QHBoxLayout();

    alphabeticRegex = QRegularExpression("\\b\\w+\\b");
    cjkRegex = QRegularExpression("[\\p{Han}\\p{Hiragana}\\p{Katakana}]");

    // Add wordcount label
    wordCountLabel = new QLabel(this);
    wordCountLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    wordCountLabel->setStyleSheet("QLabel { color: #BDBDBD; background-color: #1F2020; border-radius: 4px; }");
    QFont font = wordCountLabel->font();  // Get the current font of the QLabel
    // font.setBold(true);                   // Set the font to bold
    // Apply the font to the QLabel
    wordCountLabel->setFont(font);
    wordCountLabel->setContentsMargins(8, 5, 8, 8);
    wordCountLabel->setVisible(false);  // Hide the label by default
    
    // Create a holder widget for the word count label
    QWidget *wordCountHolder = new QWidget(this);
    QHBoxLayout *wordCountLayout = new QHBoxLayout(wordCountHolder);
    wordCountLayout->setContentsMargins(0, 0, 0, 8);
    wordCountLayout->setSpacing(0);
    
    // Add spacer to push the label to the right
    QSpacerItem *wordCountSpacerLeft = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    wordCountLayout->addItem(wordCountSpacerLeft);
    wordCountLayout->addWidget(wordCountLabel);
    
    // Replace spacer item with widget
    wordCountSpacerRight = new QWidget(this);
    wordCountSpacerRight->setFixedWidth(8);
    wordCountSpacerRight->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    wordCountSpacerRight->setStyleSheet("background: transparent; background-color: transparent");
    wordCountLayout->addWidget(wordCountSpacerRight);
    
    // Set the holder widget's policy to expand
    wordCountHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    globalLayout->setContentsMargins(0, 0, 0, 0);
    globalLayout->setSpacing(0);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    topLeftLayout->setContentsMargins(0, 0, 0, 0);
    topLeftLayout->setSpacing(16);
    bottomLeftLayout->setContentsMargins(0, 0, 0, 0);
    bottomLeftLayout->setSpacing(0);

    QSpacerItem *topLeftSpacerLeft1 = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    QSpacerItem *topLeftSpacerLeft2 = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *topLeftSpacerRight = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

    SearchWidget *searchWidget = new SearchWidget();

    // add prisoner button
    prisonerButton = new HoverButton("Prisoner", this);
    prisonerButton->setIcons(QIcon(":/icons/emptyicon.png"), QIcon(":/icons/prisoner.png"));
    prisonerButton->setLayoutDirection(Qt::RightToLeft);

    // sniper button
    sniperButton = new HoverButton("Sniper", this);
    sniperButton->setIcons(QIcon(":/icons/emptyicon.png"), QIcon(":/icons/sniper.png"));
    sniperButton->setLayoutDirection(Qt::RightToLeft);
    
    textEdit = new FictionTextEdit(this, projectManager, prisonerManager);
    sniperButton->setVisible(true);
    
    topLeftLayout->addItem(topLeftSpacerLeft1);
    topLeftLayout->addItem(topLeftSpacerLeft2);
    topLeftLayout->addWidget(prisonerButton);
    topLeftLayout->addWidget(sniperButton);
    topLeftLayout->addWidget(searchWidget);
    topLeftLayout->addItem(topLeftSpacerRight);

    QWidget *topLeftWidget = new QWidget(this);
    topLeftWidget->setLayout(topLeftLayout);

    QMargins margins = topLeftWidget->contentsMargins();
    margins.setTop(8); // Set the desired top margin
    topLeftWidget->setContentsMargins(margins);

    setupTextEdit(content);
    setupScrollBar();
    syncScrollBar();

    // QSpacerItem *bottomLeftSpacer = new QSpacerItem(64, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    // bottomLeftSpacer->setMaxWidth(160);
    QWidget *spacerWidgetLeft = new QWidget();
    spacerWidgetLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // add stupid fucking word count
    QLayout *spaceAndCounterLayout = new QVBoxLayout();
    QWidget *spaceAndCounterWidget = new QWidget(this);
    spaceAndCounterWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    spaceAndCounterLayout->setSpacing(0);
    spaceAndCounterLayout->setContentsMargins(0, 0, 0, 0);
    spaceAndCounterWidget->setContentsMargins(0, 0, 0, 0);
    spaceAndCounterWidget->setStyleSheet("background-color: transparent; border-radius: 6px;" );

    QWidget *spacerWidgetRight = new QWidget();
    spacerWidgetRight->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

    spaceAndCounterLayout->addWidget(spacerWidgetRight);
    QSpacerItem *verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    spaceAndCounterLayout->addItem(verticalSpacer);
    spaceAndCounterLayout->addWidget(wordCountHolder);
    spaceAndCounterWidget->setLayout(spaceAndCounterLayout);

    QWidget *textEditWidget = new QWidget();
    QVBoxLayout *textEditLayout = new QVBoxLayout;
    textEditLayout->addWidget(textEdit);

    // Set the layout's margins to zero
    textEditLayout->setContentsMargins(0, 0, 0, 0);
    textEditWidget->setLayout(textEditLayout);

    bottomLeftLayout->addWidget(spacerWidgetLeft);
    bottomLeftLayout->addWidget(textEdit);
    bottomLeftLayout->addWidget(spaceAndCounterWidget);

    leftLayout->addWidget(topLeftWidget);
    leftLayout->addLayout(bottomLeftLayout);

    globalLayout->addLayout(leftLayout);
    globalLayout->addWidget(vScrollBar);

    setLayout(globalLayout);
    connect(sniperButton, &QPushButton::clicked, this, &FictionViewTab::activateSniperMode);
    connect(textEdit, &FictionTextEdit::textChanged, this, &FictionViewTab::editContent);
    connect(textEdit, &FictionTextEdit::onFictionEditSearch, searchWidget, &SearchWidget::handleSearch);
    connect(textEdit, &FictionTextEdit::focusGained, searchWidget, &SearchWidget::loseAttention);
    connect(searchWidget, &SearchWidget::onSearch, textEdit, &FictionTextEdit::search);
    connect(searchWidget, &SearchWidget::onClear, textEdit, &FictionTextEdit::clearSearch);
    connect(searchWidget, &SearchWidget::onSearchPrev, textEdit, &FictionTextEdit::searchPrev);
    connect(textEdit, &FictionTextEdit::textChanged, this, &FictionViewTab::updateWordcount);
    connect(textEdit, &FictionTextEdit::showWikiAt, this, &FictionViewTab::showWikiFunc);
    connect(textEdit, &FictionTextEdit::hideWiki, this, &FictionViewTab::hideWikiFunc);
    connect(prisonerButton, &QPushButton::clicked, this, &FictionViewTab::activatePrisonerMode);
    
    // Install event filters for hover detection on buttons
    prisonerButton->installEventFilter(this);
    sniperButton->installEventFilter(this);
}

void FictionViewTab::setupTextEdit(const QString &content) {
    qDebug() << "FictionViewTab::setupTextEdit";
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->load(content);
    textEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        );

    // Set initial minimum width, but we'll adjust this dynamically
    textEdit->setMinimumWidth(360); // minimum width
    textEdit->setMaximumWidth(960); // maximum width
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTextDocument *doc = textEdit->document();
    QTextFrame *rootFrame = doc->rootFrame();
    QTextFrameFormat format = rootFrame->frameFormat();
    format.setTopMargin(128);
    format.setBottomMargin(256);
    format.setLeftMargin(16);
    format.setRightMargin(16);
    rootFrame->setFrameFormat(format);
}

void FictionViewTab::setupScrollBar() {
    qDebug() << "FictionViewTab::setupScrollBar";
    vScrollBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    vScrollBar->setStyleSheet(
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 8px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #262626;"
        "    min-height: 32px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #1f2020;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
        "    height: 0px;"
        "    width: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
        );

    connect(vScrollBar, &QScrollBar::valueChanged, textEdit->verticalScrollBar(), &QScrollBar::setValue);
    connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged, vScrollBar, &QScrollBar::setValue);
    connect(textEdit->verticalScrollBar(), &QScrollBar::rangeChanged, this, &FictionViewTab::syncScrollBar);
}

void FictionViewTab::syncScrollBar() {
    qDebug() << "FictionViewTab::syncScrollBar";
    QScrollBar* internalScrollBar = textEdit->verticalScrollBar();
    vScrollBar->setRange(internalScrollBar->minimum(), internalScrollBar->maximum());
    vScrollBar->setPageStep(internalScrollBar->pageStep());
    vScrollBar->setValue(internalScrollBar->value());
    bool isScrollBarVisible = internalScrollBar->minimum() != internalScrollBar->maximum();
    vScrollBar->setVisible(isScrollBarVisible);
    wordCountSpacerRight->setVisible(!isScrollBarVisible);
}

void FictionViewTab::activateSniperMode() {
    qDebug() << "FictionViewTab::activateSniperMode";
    textEdit->activateSniperMode();
    disconnect(textEdit, &QTextEdit::textChanged, this, &FictionViewTab::updateWordcount);
    this->updateWordcount(); // update word count immediately after switching mode
    if(wordCountLabel->isVisible()) {
        wordCountLabel->setVisible(false);
    }
    disconnect(sniperButton, &QPushButton::clicked, this, &FictionViewTab::activateSniperMode);
    connect(sniperButton, &QPushButton::clicked, this, &FictionViewTab::deactivateSniperMode);
}

void FictionViewTab::deactivateSniperMode() {
    qDebug() << "FictionViewTab::deactivateSniperMode";
    textEdit->deactivateSniperMode();
    connect(textEdit, &QTextEdit::textChanged, this, &FictionViewTab::updateWordcount);
    if(!wordCountLabel->isVisible()) {
        wordCountLabel->setVisible(true);
    }
    disconnect(sniperButton, &QPushButton::clicked, this, &FictionViewTab::deactivateSniperMode);
    connect(sniperButton, &QPushButton::clicked, this, &FictionViewTab::activateSniperMode);
}

/*
Save the content into `currentFilePath` file.
if `currentFilePath` is empty, popup a file-saving dialog; and then save.

in prisoner mode, if goal is not reached, do nothing.
if the goal is reached, this method will be triggered by the prisoner manager to save the content.
*/
bool FictionViewTab::saveContent() {
    qDebug() << "FictionViewTab::saveContent";
    // Identify the caller
    QObject *senderObj = QObject::sender();
    QString callerInfo;

    // Try to get tab information from parent CustomTabWidget
    // Note: QTabWidget internally uses QStackedWidget, so we need to traverse up the parent chain
    QString tabInfo = "Unknown";
    QWidget *currentParent = parentWidget();
    QTabWidget *tabWidget = nullptr;
    QString parentChain = "";

    if (isPrisoner) { // if in prisoner mode and not succeeded, do nothing
        bool isSucceeded = this->prisonerManager->isGoalReached();
        if (!isSucceeded) {
            return false;
        }
    }

    if (currentFilePath.isEmpty()) {
        // If no file path is provided, prompt the user to select a save location
        QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Text Files (*.txt);;All Files (*)");
        if (fileName.isEmpty()) {
            // If the user cancels the save dialog, do nothing
            return false;
        }

        currentFilePath = fileName;
        
        QFile file(currentFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Save Error", "Unable to open file for writing.");
            return false;
        }

        QTextStream out(&file);
        out << textEdit->toPlainText();
        file.close();

        emit onChangeFileType(fileName);

        return true;
    }

    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Error", "Unable to open file for writing.");
        return false;
    }

    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();

    emit onChangeTabName(QFileInfo(currentFilePath).fileName());
    return true;
}

void FictionViewTab::editContent() {
    qDebug() << "FictionViewTab::editContent";
    emit onChangeTabName(QFileInfo(currentFilePath).fileName() + "*");
}

int FictionViewTab::getBaseWordCount() {
    qDebug() << "FictionViewTab::getBaseWordCount";
    QString text = textEdit->toPlainText();
    if (text.isEmpty()) {
        return 0;
    }

    int wordCount = 0;

    QRegularExpressionMatchIterator i = alphabeticRegex.globalMatch(text);
    while (i.hasNext()) {
        i.next();
        wordCount++;
    }

    i = cjkRegex.globalMatch(text);
    while (i.hasNext()) {
        i.next();
        wordCount++;
    }

    return wordCount;
}

void FictionViewTab::updateWordcount() {
    qDebug() << "FictionViewTab::updateWordcount";
    // if word count label is not visible, make it visible
    if (!wordCountLabel->isVisible()) {
        wordCountLabel->setVisible(true);
    }
    // prevent lagging in snipper mode
    QString newTextContent = textEdit->toPlainText();
    if (newTextContent == oldTextContent) {
        return;
    }

    int wordCount = this->getBaseWordCount();
    wordCountLabel->setText(QString::number(wordCount) + " words");

    if (isPrisoner && prisonerManager) {
        // update typist progress
        prisonerManager->updateTypingProgress(wordCount);
    }
}

void FictionViewTab::activatePrisonerMode() {
    qDebug() << "FictionViewTab::activatePrisonerMode";
    // create a dialog for setting goal and time limit
    // if user click cancel, do nothing
    // if user click ok, save the content and activate prisoner mode

    // Save content before showing dialog
    if (!saveContent()) {
        return;  // If save failed or was cancelled, don't proceed
    }
    isPrisoner = true;

    int dialogWordGoal;
    int dialogTimeLimit;

    // Create and show the prisoner dialog
    PrisonerDialog dialog(this);
    connect(&dialog, &PrisonerDialog::prisonerSettings,
            this, [&](int wordGoal, int timeLimit) {
        // Handle the prisoner settings here

        dialogWordGoal = wordGoal;
        dialogTimeLimit = timeLimit;
        // Store these values or use them as needed
    });

    if (dialog.exec() != QDialog::Accepted) {
        return;  // User cancelled the dialog
    }

    // pass the word count to prisonermanager to start progress bar
    int baseWordCount = this->getBaseWordCount();
    if (prisonerManager) {
        prisonerManager->setWordGoal(dialogWordGoal, baseWordCount);
    }

    emit activatePrisonerModeSignal(dialogTimeLimit, dialogWordGoal);

    disconnect(prisonerButton, &QPushButton::clicked, this, &FictionViewTab::activatePrisonerMode);
    connect(prisonerButton, &QPushButton::clicked, this, &FictionViewTab::deactivatePrisonerMode);

    // get all text edit content
    prisonerInitialContent = textEdit->toPlainText();

    connect(prisonerManager,
            &PrisonerManager::prisonerModeFailed,
            this,
            &FictionViewTab::failedPrisonerMode);
    connect(prisonerManager,
            &PrisonerManager::prisonerModeSucceeded,
            this,
            &FictionViewTab::saveContent);
}

/*
deactivate the prisoner mode by popup a dialog to confirm user want to escape
triggered when user click prisoner button
*/
void FictionViewTab::deactivatePrisonerMode() {
    qDebug() << "FictionViewTab::deactivatePrisonerMode";
    // Prevent multiple dialogs
    if (activeEscapeDialog) {
        return;
    }

    // Show escape confirmation dialog
    blockDeactivationEscape = true;
    activeEscapeDialog = new EscapePrisonerDialog(this);
    connect(activeEscapeDialog, &QObject::destroyed, this, [this]() {
        activeEscapeDialog = nullptr;
        blockDeactivationEscape = false;
    });
    
    int result = activeEscapeDialog->exec();
    
    if (result == QDialog::Accepted && activeEscapeDialog->getResult() == EscapePrisonerDialog::Escape) {
        this->escapePrisonerMode();
    } else {
        // User chose "Stay Focused" or cancelled - ensure isPrisoner remains true
        isPrisoner = true;
    }
    // If rejected or "Stay Focused" was clicked, do nothing (stay in prisoner mode)
    activeEscapeDialog->deleteLater();
}

void FictionViewTab::failedPrisonerMode() {
    qDebug() << "FictionViewTab::failedPrisonerMode";
    isPrisoner = false;
    // Block deactivation-triggered escape while showing failure dialog
    blockDeactivationEscape = true;
    // Show failed prisoner mode dialog
    FailedPrisonerDialog *failedDialog = new FailedPrisonerDialog(this);
    
    // Connect the dialog closed signal to trigger escape
    connect(failedDialog, &FailedPrisonerDialog::dialogClosed, this, [this]() {
        this->escapePrisonerMode();
    });
    
    int result = failedDialog->exec();
    
    // Always proceed with escape since this is a failure, not a choice
    // Handle both Accepted and Rejected cases
    if (result == QDialog::Accepted || result == QDialog::Rejected) {
        this->escapePrisonerMode();
    }
    
    failedDialog->deleteLater();
    blockDeactivationEscape = false;
    disconnect(prisonerManager,
            &PrisonerManager::prisonerModeFailed,
            this,
            &FictionViewTab::failedPrisonerMode);
}

/*
escape the prisoner mode
*/
void FictionViewTab::escapePrisonerMode() {
    qDebug() << "FictionViewTab::escapePrisonerMode";
    // User confirmed escape
    isPrisoner = false;
    
    // IMPORTANT: Check goal status BEFORE emitting signal, because the signal
    // will trigger MainWindow::deactivatePrisonerModeFunc() which calls
    // prisonerManager->clear(), resetting goalReached to false
    bool goalWasReached = false;
    if (prisonerManager) {
        goalWasReached = prisonerManager->isGoalReached();
    }

    emit deactivatePrisonerModeSignal();
    connect(prisonerButton, &QPushButton::clicked, this, &FictionViewTab::activatePrisonerMode);
    disconnect(prisonerButton, &QPushButton::clicked, this, &FictionViewTab::deactivatePrisonerMode);

    // Disconnect prisoner manager signals since we're exiting prisoner mode
    disconnect(prisonerManager,
            &PrisonerManager::prisonerModeFailed,
            this,
            &FictionViewTab::failedPrisonerMode);
    disconnect(prisonerManager,
            &PrisonerManager::prisonerModeSucceeded,
            this,
            &FictionViewTab::saveContent);

    // if goal not reached, clear the progress in prisoner mode
    if (!goalWasReached) {
        // Skip sniper mode deactivation - load() will reset colors anyway
        // Just set the flag to avoid expensive changeGlobalTextColor()
        bool wasSniperMode = textEdit->isSniperMode;
        if (wasSniperMode) {
            textEdit->isSniperMode = false; // Prevent sniper formatting during load
        }
        
        // Disconnect textChanged signals to prevent infinite loop during load
        disconnect(textEdit, &FictionTextEdit::textChanged, this, &FictionViewTab::editContent);
        disconnect(textEdit, &FictionTextEdit::textChanged, this, &FictionViewTab::updateWordcount);
        
        // Block ALL signals during load to prevent any signal processing
        textEdit->setUpdatesEnabled(false);
        textEdit->blockSignals(true);
        textEdit->load(prisonerInitialContent);
        textEdit->blockSignals(false);
        textEdit->setUpdatesEnabled(true);
        
        // Properly deactivate sniper mode after load if it was active
        if (wasSniperMode) {
            textEdit->deactivateSniperMode();
        }
        
        // Reconnect textChanged signals
        connect(textEdit, &FictionTextEdit::textChanged, this, &FictionViewTab::editContent);
        connect(textEdit, &FictionTextEdit::textChanged, this, &FictionViewTab::updateWordcount);
    }
    
    // Always update word count after escaping prisoner mode
    // Update cached content first, then force word count update
    oldTextContent = ""; // Clear cache to force update
    updateWordcount();
    oldTextContent = textEdit->toPlainText(); // Set cache after update
}

QString FictionViewTab::getTextContent() const {
    return textEdit->toPlainText();
}

bool FictionViewTab::isInPrisonerMode() const {
    return isPrisoner;
}

bool FictionViewTab::isDeactivationEscapeBlocked() const {
    qDebug() << "FictionViewTab::isDeactivationEscapeBlocked";
    return blockDeactivationEscape;
}

void FictionViewTab::resizeEvent(QResizeEvent *event) {
    qDebug() << "FictionViewTab::resizeEvent";
    QWidget::resizeEvent(event);
    
    // Calculate a proportional minimum width based on the tab's width
    // For example, make it 40% of the tab width, but not less than 360px
    int newMinWidth = qMin(960, qMax(360, static_cast<int>(width() * 0.72)));
    
    // Update the textEdit's minimum width
    textEdit->setMinimumWidth(newMinWidth);
}

void FictionViewTab::showWikiFunc(const QString &wikiContent, QPoint lastMousePos) {
    qDebug() << "FictionViewTab::showWikiFunc";
    emit showWikiAt(wikiContent, lastMousePos);
}

void FictionViewTab::hideWikiFunc() {
    qDebug() << "FictionViewTab::hideWikiFunc";
    emit hideWiki();
}

void FictionViewTab::showInstructionPopup(const QString &text, QWidget *button) {
    // Create InstructionFrame if it doesn't exist
    if (!instructionFrame) {
        instructionFrame = new InstructionFrame(this);
    }

    // Set the instruction text
    instructionFrame->setText(text);

    // Get button position in global coordinates
    QPoint buttonGlobalPos = button->mapToGlobal(QPoint(0, 0));
    QPoint buttonPos = this->mapFromGlobal(buttonGlobalPos);
    
    // Position the frame below the button with a small offset
    QPoint popupPos = buttonPos + QPoint(0, button->height() + 8);

    // Ensure the frame stays within window bounds
    QSize frameSize = instructionFrame->sizeHint();
    QSize windowSize = this->size();

    // Adjust x-coordinate if needed
    if (popupPos.x() + frameSize.width() > windowSize.width()) {
        popupPos.setX(windowSize.width() - frameSize.width());
    }
    if (popupPos.x() < 0) {
        popupPos.setX(0);
    }

    // Adjust y-coordinate if needed (show above button if below doesn't fit)
    if (popupPos.y() + frameSize.height() > windowSize.height()) {
        popupPos.setY(buttonPos.y() - frameSize.height() - 8);
    }
    if (popupPos.y() < 0) {
        popupPos.setY(0);
    }

    instructionFrame->move(popupPos);
    instructionFrame->show();
    instructionFrame->raise();
}

void FictionViewTab::hideInstructionPopup() {
    if (instructionFrame) {
        instructionFrame->hide();
    }
}

bool FictionViewTab::eventFilter(QObject *obj, QEvent *event) {
    if (obj == prisonerButton) {
        QString instructionText;
        if (event->type() == QEvent::Enter) {
            if (this->isInPrisonerMode()) {
                instructionText = 
                    "Escape Prisoner Mode: Unsaved progress will be lost "
                    "if you haven't completed your goal.";
            } else {
                instructionText = 
                    "Prisoner Mode: Your progress will be saved first, "
                    "then the editor locks. Set a word goal and/or time "
                    "limit. Unlock by completing the challenge.";
            }
            
            showInstructionPopup(instructionText, prisonerButton);
            return false; // Let the event continue to HoverButton
        } else if (event->type() == QEvent::Leave) {
            hideInstructionPopup();
            return false;
        }
    } else if (obj == sniperButton) {
        if (event->type() == QEvent::Enter) {
            QString instructionText = 
                "Sniper Mode: Highlights your current paragraph and centers it.";
            showInstructionPopup(instructionText, sniperButton);
            return false; // Let the event continue to HoverButton
        } else if (event->type() == QEvent::Leave) {
            hideInstructionPopup();
            return false;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}