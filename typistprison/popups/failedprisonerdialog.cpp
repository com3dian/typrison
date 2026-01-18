#include "failedprisonerdialog.h"
#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include "../utils/hoverbutton.h"

FailedPrisonerDialog::FailedPrisonerDialog(QWidget *parent)
    : QDialog(parent)
{
    // Set the dialog parameters
    setWindowTitle("Prisoner Mode Failed");
    setModal(true);
    setMinimumSize(400, 250);
    resize(400, 300);
    
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

    // Prisoner Icon (using the same escape icon for consistency)
    QLabel *iconLabel = new QLabel(this);
    // Calculate size based on device pixel ratio
    qreal dpr = devicePixelRatio();
    int scaledWidth = 42 * dpr;
    int scaledHeight = 35 * dpr;
    iconLabel->setFixedSize(42, 35);

    QPixmap iconPixmap(":/icons/escape_icon.png");
    // Scale the pixmap at a higher resolution
    QPixmap scaledPixmap = iconPixmap.scaled(scaledWidth, scaledHeight, 
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Set the device pixel ratio
    scaledPixmap.setDevicePixelRatio(dpr);
    iconLabel->setPixmap(scaledPixmap);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent;");
    
    // Create a horizontal layout for centering the icon
    QHBoxLayout *iconLayout = new QHBoxLayout();
    iconLayout->addStretch();
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch();
    mainLayout->addLayout(iconLayout);

    // Title text below icon
    QLabel *titleLabel = new QLabel("Failed", this);
    titleLabel->setStyleSheet("color: white; font-size: 24px; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(16);

    // Create and add the message label
    QLabel *messageLabel = new QLabel("You failed to maintain the required typing pace. Your writing progress has been restored to the state before entering the challenge.", this);
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignLeft);
    messageLabel->setStyleSheet("QLabel { color: #BDBDBD; background: transparent; border: none; }");
    mainLayout->addWidget(messageLabel);
    
    // Add expanding spacer to push buttons to bottom
    mainLayout->addStretch();

    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    // Create custom button
    acknowledgeButton = new HoverButton("All right", container);

    // Style the button using the proper HoverButton methods
    acknowledgeButton->setIcons(
        QIcon(":/icons/right_arrow.png"),
        QIcon(":/icons/right_arrow_hover.png")
    );
    acknowledgeButton->setSilentBehavior(
        "background-color: transparent; border: 1px solid #5A5A5A; color: #BDBDBD; border-radius: 4px; padding: 4px 8px;"
    );
    acknowledgeButton->setHoverBehavior(
        "background-color: transparent; border: 1px solid #999999; color: #DEDEDE; border-radius: 4px; padding: 4px 8px;"
    );
    acknowledgeButton->setLayoutDirection(Qt::RightToLeft);
    
    // Add button to layout with explicit positioning
    buttonLayout->addStretch(); // Push button to the right
    buttonLayout->addWidget(acknowledgeButton);

    // Add button layout to main layout
    mainLayout->addLayout(buttonLayout);

    // Connect the custom button to the appropriate slot
    connect(acknowledgeButton, &QPushButton::clicked, this, &FailedPrisonerDialog::onAcknowledgeClicked);

    // Create a layout for the dialog itself
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);  // Remove margins
    dialogLayout->addWidget(container);

    // Center the dialog on screen
    centerOnScreen();
}

void FailedPrisonerDialog::onAcknowledgeClicked() {
    accept();
}

void FailedPrisonerDialog::closeEvent(QCloseEvent *event) {
    qDebug() << "FailedPrisonerDialog closed";
    // Emit signal when dialog is closed
    emit dialogClosed();
    QDialog::closeEvent(event);
}

void FailedPrisonerDialog::centerOnScreen() {
    // Get the screen geometry to calculate the center position
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int x = (screenGeometry.width() - this->width()) / 2;   // center horizontally
    int y = (screenGeometry.height() - this->height()) / 2; // center vertically

    // Move the dialog to the calculated center position
    this->move(x, y);
}

int FailedPrisonerDialog::exec() {
    // Center the dialog before showing
    centerOnScreen();
    return QDialog::exec();
}
