#include "markdownviewtab.h"
#include <QTextBlock>


MarkdownViewTab::MarkdownViewTab(const QString &content, const QString &filePath, QWidget *parent)
    : BaseTextEditTab(content, filePath, parent),
      textEdit(new QMarkdownTextEdit(this)), 
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
    bottomLeftLayout->setContentsMargins(16, 16, 16, 16);
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
    connect(textEdit, &QMarkdownTextEdit::onMarkdownSearch, searchWidget, &SearchWidget::handleSearch);
    connect(textEdit, &QMarkdownTextEdit::focusGained, searchWidget, &SearchWidget::loseAttention);
    connect(textEdit, &QMarkdownTextEdit::onSave, this, &MarkdownViewTab::saveContent);
    connect(textEdit, &QMarkdownTextEdit::textChanged, this, &MarkdownViewTab::editContent);
    connect(textEdit, &QMarkdownTextEdit::showImageAt, this, &MarkdownViewTab::showImageFunc);
    connect(textEdit, &QMarkdownTextEdit::hideImage, this, &MarkdownViewTab::hideImageFunc);
    connect(searchWidget, &SearchWidget::onSearch, textEdit, &QMarkdownTextEdit::search);
    connect(searchWidget, &SearchWidget::onClear, textEdit, &QMarkdownTextEdit::clearSearch);
    connect(searchWidget, &SearchWidget::onSearchPrev, textEdit, &QMarkdownTextEdit::searchPrev);
}

void MarkdownViewTab::setupTextEdit(const QString &content) {
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->load(content);
    textEdit->setStyleSheet(
        "QMarkdownTextEdit {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
    );
    textEdit->setMinimumWidth(400); // Adjust the minimum width as needed
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

void MarkdownViewTab::setupScrollBar() {
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
    connect(textEdit->verticalScrollBar(), &QScrollBar::rangeChanged, this, &MarkdownViewTab::syncScrollBar);
}

void MarkdownViewTab::syncScrollBar() {
    QScrollBar* internalScrollBar = textEdit->verticalScrollBar();
    vScrollBar->setRange(internalScrollBar->minimum(), internalScrollBar->maximum());
    vScrollBar->setPageStep(internalScrollBar->pageStep());
    vScrollBar->setValue(internalScrollBar->value());
    vScrollBar->setVisible(internalScrollBar->minimum() != internalScrollBar->maximum());
}

void MarkdownViewTab::editContent() {
    emit onChangeTabName(QFileInfo(currentFilePath).fileName() + "*");
}

QString MarkdownViewTab::getTextContent() const {
    return textEdit->toPlainText();
}

void MarkdownViewTab::showImageFunc(const QString &imagePath, QPoint lastMousePos) {
    emit showImageAt(imagePath, lastMousePos);
}

void MarkdownViewTab::hideImageFunc() {
    emit hideImage();
}