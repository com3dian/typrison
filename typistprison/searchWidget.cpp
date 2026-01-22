#include "searchWidget.h"
#include <QPalette>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent), isHovered(false), isOnSearch(false), isSearchPrev(false), isSearchLatter(false)
{
    QScreen *screen = QGuiApplication::primaryScreen(); // Use QGuiApplication to get the primary screen
    scalingFactor = screen->devicePixelRatio(); // Correctly call devicePixelRatio()

    lineEdit = new QLineEdit(this);
    lineEdit->setMinimumWidth(128);
    // lineEdit->setMaximumWidth(480); // maximum width
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lineEdit->setStyleSheet(
            "background-color: transparent; "
            "border: none;"
            "color: #FFFFFF;"
    );
    QPalette palette = lineEdit->palette();
    palette.setColor(QPalette::Highlight, QColor("#84e0a5"));
    palette.setColor(QPalette::HighlightedText, QColor("#2C2C2C"));
    lineEdit->setPalette(palette);

    searchButton = new QPushButton(this);
    searchButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/emptyicon.png) 0 0 0 0 stretch stretch;"
            "}"
    );

    bottomLine = new QWidget(this);
    bottomLine->setStyleSheet("background-color: transparent;");

    connect(lineEdit, &QLineEdit::returnPressed, this, [this]() { handleSearch(lineEdit->text()); });
    connect(searchButton, &QPushButton::clicked, this, [this]() { handleSearch(lineEdit->text()); });

    hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(4, 0, 4, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(lineEdit);
    hLayout->addWidget(searchButton);

    bottomLine->setLayout(hLayout);  // Set hLayout as bottomLine's layout

    vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(bottomLine);  // Only add bottomLine to vLayout

    setLayout(vLayout);

    // Install event filter to detect hover events
    installEventFilter(this);
    // Install event filter to detect line edit changes
    lineEdit->installEventFilter(this);

    connect(lineEdit, &QLineEdit::editingFinished, this, &SearchWidget::updateBottomLine);
    connect(lineEdit, &QLineEdit::textChanged, this, &SearchWidget::updateBottomLine);
}

void SearchWidget::handleSearch(const QString &text)
{
    qDebug() << "handleSearch" << text;
    if (!text.isEmpty()) {
        emit onSearch(text);

        // avoid handleReSearch method to be triggered when search for next match
        QString prevSearchText = lineEdit->text();

        if (prevSearchText != text) {
            disconnect(lineEdit, &QLineEdit::textChanged, this, &SearchWidget::handleReSearch);
            lineEdit->setText(text);
        }

        lineEdit->selectAll();

        searchButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/tab_close.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/tab_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:pressed {"
            "border-image: url(:/icons/tab_hover.png) 0 0 0 0 stretch stretch;"
            "}"
        );
        searchButton->setFixedSize(16, 16);

        disconnect(searchButton, &QPushButton::clicked, this, nullptr);
        connect(searchButton, &QPushButton::clicked, this, &SearchWidget::handleClear);
        connect(lineEdit, &QLineEdit::textChanged, this, &SearchWidget::handleReSearch); // previously not empty and text changed

        isOnSearch = true;
    }
    // Focus to "search" lineEdit
    lineEdit->setFocus();

    updateBottomLine();
}

void SearchWidget::handleClear()
{
    lineEdit->clear();
    lineEdit->clearFocus();
    emit onClear();
    searchButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/search_silent.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/search_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:pressed {"
            "border-image: url(:/icons/search_clicked.png) 0 0 0 0 stretch stretch;"
            "}"
    );
    searchButton->setFixedSize(16, 16);

    disconnect(searchButton, &QPushButton::clicked, this, nullptr);
    connect(searchButton, &QPushButton::clicked, this, [this]() { handleSearch(lineEdit->text()); });
    disconnect(lineEdit, &QLineEdit::textChanged, this, nullptr);
    isOnSearch = false;
    updateBottomLine();
}

void SearchWidget::handleReSearch()
{

    searchButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/search_silent.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/search_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:pressed {"
            "border-image: url(:/icons/search_clicked.png) 0 0 0 0 stretch stretch;"
            "}"
    );
    searchButton->setFixedSize(16, 16);

    emit onClear();
    disconnect(searchButton, &QPushButton::clicked, this, nullptr);
    connect(searchButton, &QPushButton::clicked, this, [this]() { handleSearch(lineEdit->text()); });
    disconnect(lineEdit, &QLineEdit::textChanged, this, nullptr);
    isOnSearch = false;
    updateBottomLine();
}

bool SearchWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineEdit) {
        if (event->type() == QEvent::FocusOut) {
            qDebug() << "focus out";
            // Force the cursor to stop blinking by clearing focus and resetting text
            lineEdit->clearFocus();
            lineEdit->setText("");  // Clear text temporarily
            lineEdit->setCursorPosition(0);  // Position cursor at end
            updateBottomLine();
            return false;  // Allow default focus handling to hide cursor
        }

        // Check if the event is a key event
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            // Convert event to QKeyEvent safely
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent) {
                // Get search text
                QString searchText = lineEdit->text();

                if (keyEvent->key() == Qt::Key_Up && isOnSearch) {
                    // Event filter is intalled to both `this` and `lineEdit`
                    // To avoid triggering `onSearchPrev` signal twice
                    isSearchPrev = not isSearchPrev;
                    if (isSearchPrev) {
                        emit onSearchPrev(searchText);
                        qDebug() << "on search up";
                    }
                    return true;  // Event handled

                } else if (keyEvent->key() == Qt::Key_Down && isOnSearch) {
                    // same as above to prevent triggering `onSearch` signal twice
                    isSearchLatter = not isSearchLatter;
                    if (isSearchLatter) {
                        emit onSearch(searchText);
                        qDebug() << "on search down";
                    }
                    return true;  // Event handled
                } 
                // else {
                //     // isSearchLatter = not isSearchLatter;
                //     // if (isSearchLatter) {
                //     //     emit onSearch(searchText);
                //     //     qDebug() << "on search";
                //     // }
                // }
            }
        }
    } else if (obj == this) {
        if (event->type() == QEvent::Enter) {
            isHovered = true;
            updateBottomLine();
            return true;  // Event handled
        } else if (event->type() == QEvent::Leave) {
            isHovered = false;
            updateBottomLine();
            return true;  // Event handled
        }
    }

    // Let the base class process the event if not handled
    return QWidget::eventFilter(obj, event);
}

void SearchWidget::updateBottomLine()
{
    if (isOnSearch) {
        qDebug() << "is on search";
        bottomLine->setStyleSheet("border: 1px solid #BDBDBD; border-radius: 4px;");

        searchButton->setStyleSheet(
                "QPushButton {"
                "border: none;"
                "border-image: url(:/icons/tab_close.png) 0 0 0 0 stretch stretch;"
                "}"
                "QPushButton:hover {"
                "border-image: url(:/icons/tab_hover.png) 0 0 0 0 stretch stretch;"
                "}"
                "QPushButton:pressed {"
                "border-image: url(:/icons/tab_hover.png) 0 0 0 0 stretch stretch;"
                "}"
        );
        searchButton->setFixedSize(16, 16);
    }
    else if (isHovered || !lineEdit->text().isEmpty() || lineEdit->hasFocus()) {
        qDebug() << lineEdit->text();
        bottomLine->setStyleSheet("border: 1px solid #BDBDBD; border-radius: 4px;");

        searchButton->setStyleSheet(
                "QPushButton {"
                "border: none;"
                "border-image: url(:/icons/search_silent.png) 0 0 0 0 stretch stretch;"
                "}"
                "QPushButton:hover {"
                "border-image: url(:/icons/search_hover.png) 0 0 0 0 stretch stretch;"
                "}"
                "QPushButton:pressed {"
                "border-image: url(:/icons/search_hover.png) 0 0 0 0 stretch stretch;"
                "}"
        );
        searchButton->setFixedSize(16, 16);
    } else {
        qDebug() << "is not on search";
        searchButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/emptyicon.png) 0 0 0 0 stretch stretch;"
            "}"
        );
        searchButton->setFixedSize(16, 16);
        bottomLine->setStyleSheet("border: 1px solid transparent; border-radius: 4px;");
    }
}

void SearchWidget::loseAttention()
{
    if (isOnSearch) {
        return;
    }
    lineEdit->setText("+");
    lineEdit->clear();
    this->updateBottomLine();
    lineEdit->clearFocus();
}