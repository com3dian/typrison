/*
PlaintextViewTab


*/

#include "plaintextviewtab.h"


PlaintextViewTab::PlaintextViewTab(const QString &content, const QString &filePath, QWidget *parent)
    : BaseTextEditTab(content, filePath, parent), 
      textEdit(new PlaintextEdit(this)), 
      vScrollBar(new QScrollBar(Qt::Vertical, this))
{
    // Remove currentFilePath initialization as it's handled by BaseTextEditTab
    globalLayout = new QHBoxLayout(this);
    leftLayout = new QVBoxLayout();
    topLeftLayout = new QHBoxLayout();
    bottomLeftLayout = new QHBoxLayout();

    globalLayout->setContentsMargins(0, 0, 0, 0);
    globalLayout->setSpacing(0);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    topLeftLayout->setContentsMargins(0, 0, 0, 0);
    topLeftLayout->setSpacing(16);
    bottomLeftLayout->setContentsMargins(0, 0, 0, 0);
    bottomLeftLayout->setSpacing(0);

    QSpacerItem *topLeftSpacerLeft1 = new QSpacerItem(64, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    QSpacerItem *topLeftSpacerLeft2 = new QSpacerItem(64, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *topLeftSpacerRight = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

    SearchWidget *searchWidget = new SearchWidget();
    
    topLeftLayout->addItem(topLeftSpacerLeft1);
    topLeftLayout->addItem(topLeftSpacerLeft2);
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

    bottomLeftLayout->addWidget(textEdit);

    leftLayout->addWidget(topLeftWidget);
    leftLayout->addLayout(bottomLeftLayout);

    globalLayout->addLayout(leftLayout);
    globalLayout->addWidget(vScrollBar);

    setLayout(globalLayout);
    connect(textEdit, &PlaintextEdit::onPlaintextSearch, searchWidget, &SearchWidget::handleSearch);
    connect(textEdit, &PlaintextEdit::focusGained, searchWidget, &SearchWidget::loseAttention);
    connect(textEdit, &PlaintextEdit::textChanged, this, &PlaintextViewTab::editContent);
    connect(searchWidget, &SearchWidget::onSearch, textEdit, &PlaintextEdit::search);
    connect(searchWidget, &SearchWidget::onClear, textEdit, &PlaintextEdit::clearSearch);
    connect(searchWidget, &SearchWidget::onSearchPrev, textEdit, &PlaintextEdit::searchPrev);
}

void PlaintextViewTab::setupTextEdit(const QString &content) {
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->load(content);
    textEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        );
    textEdit->setMinimumWidth(480); // Adjust the minimum width as needed
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTextDocument *doc = textEdit->document();
    QTextFrame *rootFrame = doc->rootFrame();
    QTextFrameFormat format = rootFrame->frameFormat();
    format.setTopMargin(16);
    format.setBottomMargin(16);
    format.setLeftMargin(16);
    format.setRightMargin(16);
    rootFrame->setFrameFormat(format);
}

void PlaintextViewTab::setupScrollBar() {
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
    connect(textEdit->verticalScrollBar(), &QScrollBar::rangeChanged, this, &PlaintextViewTab::syncScrollBar);
}

void PlaintextViewTab::syncScrollBar() {
    QScrollBar* internalScrollBar = textEdit->verticalScrollBar();
    vScrollBar->setRange(internalScrollBar->minimum(), internalScrollBar->maximum());
    vScrollBar->setPageStep(internalScrollBar->pageStep());
    vScrollBar->setValue(internalScrollBar->value());
    vScrollBar->setVisible(internalScrollBar->minimum() != internalScrollBar->maximum());
}

void PlaintextViewTab::editContent() {
    emit onChangeTabName(QFileInfo(currentFilePath).fileName() + "*");
}

QString PlaintextViewTab::getTextContent() const {
    return textEdit->toPlainText();
}