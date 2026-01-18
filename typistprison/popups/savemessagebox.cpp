#include "savemessagebox.h"
#include <QApplication>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include "../utils/hoverbutton.h"

SaveMessageBox::SaveMessageBox(QWidget *parent)
    : QDialog(parent)
{
    // Set the dialog parameters
    setWindowTitle("Unsaved Changes");
    setModal(true);
    setFixedWidth(400);
    
    // Remove window frame and system title bar
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Create a container widget that will hold all content
    QWidget *container = new QWidget(this);
    container->setStyleSheet("QWidget#container {" // Add an object name selector
                         "    background-color: #1F2020;"  // dark gray background
                         "    border: 1px solid #3A3A3A;"  // subtle border
                         "    border-radius: 9px;"         // rounded corners
                         "}");
    container->setObjectName("container"); // Set the object name to match the selector

    // Create the main layout for the container
    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Create close button widget
    CloseButtonWidget *closeButtonWidget = new CloseButtonWidget(container);
    connect(closeButtonWidget, &CloseButtonWidget::closeButtonClicked, this, [this]() {
        reject();
        close();
    });

    // Add close button to main layout with right alignment
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addStretch();
    topLayout->addWidget(closeButtonWidget);
    mainLayout->addLayout(topLayout);

    // Create and add the message label
    QLabel *messageLabel = new QLabel("The document has been modified. Do you want to save your changes?");
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignLeft);
    messageLabel->setStyleSheet("QLabel { color: #FFFFFF; background: transparent; border: none; }");
    mainLayout->addWidget(messageLabel);
    mainLayout->addSpacing(10);

    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    // Create custom buttons
    saveButton = new HoverButton("Save ", container);
    discardButton = new HoverButton("Discard ", container);

    // Style the buttons using the proper HoverButton methods
    saveButton->setIcons(
        QIcon(":/icons/save_file.png"),
        QIcon(":/icons/save_file_hover.png")
    );
    saveButton->setSilentBehavior(
        "background-color: transparent; border: 1px solid #5A5A5A; color: #BDBDBD; border-radius: 4px; padding: 4px 8px;"
    );
    saveButton->setHoverBehavior(
        "background-color: transparent; border: 1px solid #80bd96; color: #84e0a5; border-radius: 4px; padding: 4px 8px;"
    );
    saveButton->setLayoutDirection(Qt::RightToLeft);

    discardButton->setIcons(
        QIcon(":/icons/discard_file.png"),
        QIcon(":/icons/discard_file_hover.png")
    );
    discardButton->setSilentBehavior(
        "background-color: transparent; border: 1px solid #5A5A5A; color: #BDBDBD; border-radius: 4px; padding: 4px 8px;"
    );
    discardButton->setHoverBehavior(
        "background-color: transparent; border: 1px solid #BA6757; color: #E0715C; border-radius: 4px; padding: 4px 8px;"
    );
    discardButton->setLayoutDirection(Qt::RightToLeft);
    
    // Add buttons to layout with explicit positioning
    buttonLayout->addStretch(); // Push buttons
    buttonLayout->addWidget(discardButton);
    buttonLayout->addWidget(saveButton);

    // Add button layout to main layout
    mainLayout->addLayout(buttonLayout);

    // Connect the custom buttons to the appropriate slots
    connect(saveButton, &QPushButton::clicked, this, &SaveMessageBox::onSaveClicked);
    connect(discardButton, &QPushButton::clicked, this, &SaveMessageBox::onDiscardClicked);

    // Create a layout for the dialog itself
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);  // Remove margins
    dialogLayout->addWidget(container);

    // Center the dialog on screen
    centerOnScreen();
}

void SaveMessageBox::onSaveClicked() {
    // Custom save logic here
    result = Save;
    accept();
}

void SaveMessageBox::onDiscardClicked() {
    result = Discard;
    accept();
}

void SaveMessageBox::centerOnScreen() {
    // Get the screen geometry to calculate the center position
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int x = (screenGeometry.width() - this->width()) / 2;   // center horizontally
    int y = (screenGeometry.height() - this->height()) / 2; // center vertically

    // Move the dialog to the calculated center position
    this->move(x, y);
}

SaveMessageBox::ButtonResult SaveMessageBox::getResult() const {
    return result;
}

int SaveMessageBox::exec() {
    // Center the dialog before showing
    centerOnScreen();
    return QDialog::exec();
}