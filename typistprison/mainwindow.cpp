/*
The main window widget holding everthing in the app.

+-mainwindow----
|
|   +-mainContainer (qwidget)----
|   |
|   |   +-splitterContainer (progressborderwidget)----
    |   |
    |   |  +-splitterTopWidget (QWidget)----
        |  |      (invisible by default)
        |  +------------------------------------------

           +-centralSplitter (QSplitter)---------
           |
           |   +-folderTreeView-+   +-editorWidget (QWidget)----
           |   |                |   |                        

*/


#include "mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

// Add this to your MainWindow constructor initialization list
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , untitledCount(1)
    , previousSplitterPosition(0.166)
    , imageFrame(nullptr)
    , wikiFrame(nullptr)
    , contextMenuFrame(nullptr)
    , subMenuFrame(nullptr)
    , splitterContainer(nullptr)
    , prisonerManager(new PrisonerManager(this))
    , networkManager(new QNetworkAccessManager(this))
{
    // Hide window original traffic lights (close, minimize, maximize buttons)
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    
    projectManager = new ProjectManager();
    ui->setupUi(this);
    
    this->addAction(ui->actionOpen_File);   // add action for open file
    this->addAction(ui->actionNew_File);   // add action for new file

    // Create the main container widget
    QWidget *mainContainer = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create a wrapper for the splitter (to ensure full expansion)
    splitterContainer = new ProgressBorderWidget(mainContainer, prisonerManager);
    splitterContainer->setObjectName("splitterContainer");  // Add this line
    splitterContainer->setStyleSheet(
        "QWidget#splitterContainer {"  // Target only this specific widget
        "    background-color: #1F2020;"
        "    border-radius: 9px;"
        "    border: 1px solid #393B3B;"
        "}"
    );

    splitterLayout = new QVBoxLayout(splitterContainer);
    splitterLayout->setContentsMargins(8, 1, 8, 8);
    splitterLayout->setSpacing(0);

    splitterTopWidget = new QWidget(this);
    splitterTopWidget->setStyleSheet(
        "QWidget {"
        "    background-color: transparent;"
        "}"
    );
    splitterTopWidget->setFixedHeight(7);
    splitterTopWidget->setVisible(false);

    // Create the splitter
    centralSplitter = new QSplitter(splitterContainer);
    centralSplitter->setStyleSheet(
    "QSplitter { background-color: transparent;}"  // Background of the splitter itself
    "QSplitter::handle {"
    "    background-color: #1F2020;"
    "}"
    );
    centralSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Make the handle always visible by preventing complete collapse
    centralSplitter->setHandleWidth(0);  // Set the physical handle width to match CSS
    centralSplitter->setCursor(Qt::ArrowCursor);

    folderTreeView = new FolderTreeViewWidget(this, "", projectManager);
    centralSplitter->addWidget(folderTreeView);

    QWidget *editorWidget = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout;
    editorLayout->setContentsMargins(0, 0, 0, 0); // Remove margins
    editorLayout->setSpacing(0); // Remove spacing
    customTabWidget = new CustomTabWidget(this, projectManager, prisonerManager);

    // Double holder for tab widget
    QWidget *customTabWidgetHolder = new QWidget(this);
    customTabWidgetHolder->setAttribute(Qt::WA_TranslucentBackground);
    customTabWidgetHolder->setStyleSheet(
        "QWidget {"
        "    background-color: #2c2c2c;"
        "    border: none;"
        "    border-radius: 4px;"
        "}"
    );
    // Create layout for the holder
    QVBoxLayout *tabWidgetLayout = new QVBoxLayout(customTabWidgetHolder);
    tabWidgetLayout->setContentsMargins(0, 0, 0, 0); // Bottom margin
    tabWidgetLayout->setSpacing(0); // Optional, to avoid extra spacing
    tabWidgetLayout->addWidget(customTabWidget);

    functionBar = new FunctionBar(this, customTabWidget);
    setupMenuButtons(functionBar);
    connect(functionBar, &FunctionBar::minimalButtonClicked, this, &MainWindow::minimizeWindow);
    connect(functionBar, &FunctionBar::maximalButtonClicked, this, &MainWindow::maximizeWindow);
    connect(functionBar, &FunctionBar::closeButtonClicked, this, &MainWindow::closeWindow);

    editorLayout->addWidget(functionBar);
    editorLayout->addWidget(customTabWidgetHolder);
    editorWidget->setLayout(editorLayout);

    centralSplitter->addWidget(editorWidget);
    centralSplitter->setCollapsible(1, false);
    centralSplitter->setSizes(QList<int>({0, centralSplitter->size().width()}));
    centralSplitter->handle(1)->setEnabled(false);  // Disable handle dragging

    // Add splitter to its container layout
    splitterLayout->addWidget(splitterTopWidget);
    splitterLayout->addWidget(centralSplitter);
    splitterContainer->setLayout(splitterLayout);
    splitterContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add splitter container to main layout
    mainLayout->addWidget(splitterContainer);

    // Set main container as the central widget
    setCentralWidget(mainContainer);
    setMouseTracking(true);
    mainContainer->setMouseTracking(true);
    
    // ---- Create the Floating Side Panel Button ----
    sidePanelButton = new QPushButton(this);
    sidePanelButton->setStyleSheet(
        "QPushButton {"
        "border: none;"
        "border-image: url(:/icons/sidebar_silent.png) 0 0 0 0 stretch stretch;"
        "}"
        "QPushButton:hover {"
        "border-image: url(:/icons/sidebar_hover.png) 0 0 0 0 stretch stretch;"
        "}"
        "QPushButton:pressed {"
        "border-image: url(:/icons/sidebar_clicked.png) 0 0 0 0 stretch stretch;"
        "}"
    );
    sidePanelButton->setFixedSize(32, 32);
    sidePanelButton->raise(); // Bring to front
    sidePanelButton->setVisible(false);

    // Move the button to the bottom-left corner dynamically
    adjustButtonPosition();

    // Connect the customTabWidget's lastTabClosed signal to the MainWindow's slot to close the window
    connect(customTabWidget, &CustomTabWidget::lastTabClosed, this, &MainWindow::onLastTabClosed);
    connect(customTabWidget, &CustomTabWidget::showImageAtSignal, this, &MainWindow::showMarkdownImage);
    connect(customTabWidget, &CustomTabWidget::hideImageSignal, this, &MainWindow::hideMarkdownImage);
    connect(customTabWidget, &CustomTabWidget::showWikiAtSignal, this, &MainWindow::showWiki);
    connect(customTabWidget, &CustomTabWidget::hideWikiSignal, this, &MainWindow::hideWiki);
    connect(customTabWidget, &CustomTabWidget::activatePrisonerModeSignal, this, &MainWindow::activatePrisonerModeFunc);
    connect(customTabWidget, &CustomTabWidget::deactivatePrisonerModeSignal, this, &MainWindow::deactivatePrisonerModeFunc);

    connect(folderTreeView, &FolderTreeViewWidget::doubleClickedOnFile, this, &MainWindow::openFile);
    connect(folderTreeView, &FolderTreeViewWidget::fileDeleted, customTabWidget, &CustomTabWidget::handleFileDeleted);
    connect(folderTreeView, &FolderTreeViewWidget::fileRenamed, customTabWidget, &CustomTabWidget::handleFileRenamed);
    connect(folderTreeView, &FolderTreeViewWidget::showContextMenuInMainWindow, this, &MainWindow::showContextMenu);

    // set up side panel button
    sidePanelButton->setEnabled(false);
    connect(sidePanelButton, &QPushButton::clicked, this, &MainWindow::toggleFileTreeView);
    
    setupUntitledTab(FICTION_TAB);
    setupActions();

    qApp->installEventFilter(this);
}

/*
(Part of) Short cuts definition.

the other half is in `keypress` method in each textedit classes.
*/
void MainWindow::setupActions() {
    // Create actions for shortcuts instead of relying on invisible buttons
    QAction* actionOpenFile = new QAction("Open File", this);
    QAction* actionNewFile = new QAction("New File", this);
    QAction* actionSaveFile = new QAction("Save File", this);
    QAction* actionSearchFile = new QAction("Search File", this);
    QAction* actionOpenProject = new QAction("Open Project", this);
    QAction* actionSaveAllProject = new QAction("Save All Project", this);

    // Set shortcuts for the actions
    actionOpenFile->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    actionNewFile->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    actionSaveFile->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

    // Add actions to the main window so they work globally
    this->addAction(actionOpenFile);
    this->addAction(actionNewFile);
    this->addAction(actionSaveFile);
    this->addAction(actionSearchFile);
    this->addAction(actionOpenProject);
    this->addAction(actionSaveAllProject);

    // Connect signals to the appropriate slots
    connect(actionOpenFile, &QAction::triggered, this, [this]() {
        openFile("");
    });
    connect(actionNewFile, &QAction::triggered, this, [this]() {
        setupUntitledTab(FICTION_TAB);
    });
    connect(actionSaveFile, &QAction::triggered, this, [this]() {
        saveFile();
    });
    connect(actionSearchFile, &QAction::triggered, this, [this]() {
        searchFile();
    });
    connect(actionOpenProject, &QAction::triggered, this, [this]() {
        openProject();
    });
    connect(actionSaveAllProject, &QAction::triggered, this, [this]() {
        saveProject();
    });
}

void MainWindow::createAndOpenProject()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select or Create Project Directory"),
                                                      QDir::homePath(),
                                                      QFileDialog::ShowDirsOnly);
    
    if (!dirPath.isEmpty()) {
        // Prompt for folder name and create directory
        bool ok;
        QString folderName = QInputDialog::getText(this, tr("Create Project"), 
            tr("Enter project folder name:"), QLineEdit::Normal, "", &ok);
        
        if (ok && !folderName.isEmpty()) {
            QDir dir(dirPath);
            dir.mkdir(folderName);
            QString projectPath = dirPath + '/' + folderName;
            
            // Open the project
            openProject(projectPath);
        }
    }
}

// ---- Override resizeEvent to Keep Button in Bottom-Left ----
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    adjustButtonPosition();
}

// ---- Adjust the Button's Position Based on Window Size ----
void MainWindow::adjustButtonPosition() {
    int margin = 16; // Adjust margin for spacing
    int buttonX = margin; 
    int buttonY = height() - sidePanelButton->height() - margin; // Bottom left corner
    sidePanelButton->move(buttonX, buttonY);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuButtons(FunctionBar *functionBar) {
    QList<QPushButton*> buttonList = functionBar->getAllButtons();
    if (buttonList.size() >= 3) {
        menuToggleButton = buttonList.at(0);
        button1 = buttonList.at(1);
        button2 = buttonList.at(2);
    }
    button1->installEventFilter(this);
    button2->installEventFilter(this);
}

void MainWindow::setupUntitledTab(TabType tabType)
{
    // Create the default untitled tab based on the specified type
    switch (tabType) {
    case FICTION_TAB:
        customTabWidget->createFictionTab();
        break;
    case PLAINTEXT_TAB:
        customTabWidget->createPlainTextTab();
        break;
    case MARKDOWN_TAB:
        customTabWidget->createMarkdownTab();
        break;
    default:
        customTabWidget->createFictionTab();
        break;
    }
}

void MainWindow::openFile(const QString &filePath)  // No default argument here
{
    QString selectedFilePath = filePath;

    // If no filePath is provided, open a file dialog to select a file
    if (selectedFilePath.isEmpty()) {
        selectedFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    }

    // If a file is selected (either from the dialog or the provided path), load the file
    if (!selectedFilePath.isEmpty()) {
        // Create a new tab with the file name as the tab text
        customTabWidget->createNewTab(selectedFilePath);
    }
}

void MainWindow::saveFile() {
    // Get the current active tab from customTabWidget
    QWidget *currentTab = customTabWidget->currentWidget();
    
    // Check if there is an active tab
    if (currentTab) {
        // Cast to BaseTextEditTab since all tab types inherit from it
        BaseTextEditTab *textEditTab = qobject_cast<BaseTextEditTab*>(currentTab);
        
        // If the cast is successful, save the content
        if (textEditTab) {
            textEditTab->saveContent();
        }
    }
}

void MainWindow::searchFile()
{
    // Get the current tab widget
    QWidget* currentWidget = customTabWidget->currentWidget();
    if (!currentWidget)
        return;

    // Cast to BaseTextEditTab since all tab types inherit from it
    BaseTextEditTab* textEditTab = qobject_cast<BaseTextEditTab*>(currentWidget);
    if (textEditTab) {
        // Set focus to the text edit
        textEditTab->setFocus();
        
        // Find the search widget within the tab and give it focus
        SearchWidget* searchWidget = textEditTab->findChild<SearchWidget*>();
        
        // Try to get selected text
        QString selectedText;
        if (auto* fictionTab = qobject_cast<FictionViewTab*>(textEditTab)) {
            if (auto* textEdit = fictionTab->findChild<FictionTextEdit*>()) {
                selectedText = textEdit->textCursor().selectedText();
            }
        } else if (auto* markdownTab = qobject_cast<MarkdownViewTab*>(textEditTab)) {
            if (auto* textEdit = markdownTab->findChild<QMarkdownTextEdit*>()) {
                selectedText = textEdit->textCursor().selectedText();
            }
        } else if (auto* plaintextTab = qobject_cast<PlaintextViewTab*>(textEditTab)) {
            if (auto* textEdit = plaintextTab->findChild<PlaintextEdit*>()) {
                selectedText = textEdit->textCursor().selectedText();
            }
        }
        
        if (searchWidget) {
            searchWidget->handleSearch(selectedText);
        }
    }
}

void MainWindow::openProject(const QString &path)
{
    QString projectPath = path;
    QString selectedProjectRoot;
    if (projectPath.isEmpty()) {
        selectedProjectRoot = QFileDialog::getExistingDirectory(
            this, 
            tr("Open Project Folder"), 
            QDir::homePath(), 
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    } else {
        selectedProjectRoot = projectPath;
    }

    if (selectedProjectRoot.isEmpty()) {
        QMessageBox::warning(this, tr("No Folder Selected"), tr("No project folder was selected."));
        return;
    }
    
    // project manager load
    projectManager->open(selectedProjectRoot);
    
    // Re-initialize folderTreeView
    folderTreeView->refresh(selectedProjectRoot);

    // Enable splitter handle dragging
    centralSplitter->handle(1)->setEnabled(true);

    openFileTreeView();
    sidePanelButton->setVisible(true);
    sidePanelButton->setEnabled(true);
    return;
}


void MainWindow::saveProject() {
    for (int i = 0; i < customTabWidget->count(); ++i) {
        QWidget *tab = customTabWidget->widget(i);
        BaseTextEditTab *textEditTab = qobject_cast<BaseTextEditTab*>(tab);
        if (textEditTab) {
            textEditTab->saveContent();
        }
    }
}

void MainWindow::onLastTabClosed() {
    close();
}

void MainWindow::toggleFileTreeView() {
    QList<int> sizes = centralSplitter->sizes(); // Get current splitter sizes
    
    // Only calculate the previousSplitterPosition if sizes[0] is non-zero (i.e., left widget is visible)
    if (sizes[0] != 0) {
        // Save the current splitter position before hiding the left widget
        previousSplitterPosition = static_cast<float>(sizes[0]) / centralSplitter->size().width();

        // Hide the left widget and adjust the splitter size
        // folderTreeView->toggleFileTreeView();
        centralSplitter->setSizes(QList<int>({0, centralSplitter->size().width()})); // Right widget takes over
        sidePanelButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/sidebar_silent.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/sidebar_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:pressed {"
            "border-image: url(:/icons/sidebar_clicked.png) 0 0 0 0 stretch stretch;"
            "}"
        );
    } else {
        if (previousSplitterPosition == 0) {
            previousSplitterPosition = 0.166;
        }
        // Restore sizes based on the stored previousSplitterPosition
        centralSplitter->setSizes(
            QList<int>({static_cast<int>(centralSplitter->size().width() * previousSplitterPosition), 
                        static_cast<int>(centralSplitter->size().width() * (1 - previousSplitterPosition))}));

        // Reset previousSplitterPosition after restoring
        previousSplitterPosition = 0.0;
        sidePanelButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/sidebar_triggerred.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/sidebar_triggerred.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:pressed {"
            "border-image: url(:/icons/sidebar_triggerred.png) 0 0 0 0 stretch stretch;"
            "}"
        );
    }
}

void MainWindow::minimizeWindow() {
    this->showMinimized();
    splitterContainer->setStyleSheet(
        "QWidget#splitterContainer {"  // Target only this specific widget
        "    background-color: #1F2020;"
        "    border-radius: 8px;"
        "    border: 1px solid #393B3B;"
        "}"
    );
}

void MainWindow::maximizeWindow() {
    if (isFullScreen()) {
        this->showNormal();
        splitterContainer->setFullScreen(false);
    } else {
        this->showFullScreen();
        splitterContainer->setFullScreen(true);
    }
}

void MainWindow::closeWindow() {
    this->close();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Check if the event is for one of our menu bar buttons
    if (obj == button1 || obj == button2)
    {
        if (event->type() == QEvent::Enter)
        {
            handleMouseEnterMenuButton(static_cast<QPushButton *>(obj));
        }
    }
    
    // Now, handle all mouse press events in the application
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        
        // Convert the global position to the main window's coordinates.
        QPoint posInMainWindow = this->mapFromGlobal(mouseEvent->globalPos());
        if (this->rect().contains(posInMainWindow)) {
            
            // Hide wiki and markdown image windows on any click in the main window
            hideMarkdownImage();
            hideWiki();

            // If the drop-down frame is visible, handle it
            if (existingFrame && existingFrame->isVisible()) {
                if (obj == menuToggleButton) {
                    return true;
                }
                // Convert mouse position to MainWindow coordinates
                QPoint posInMainWindow = this->mapFromGlobal(mouseEvent->globalPos());
                
                // Check if click is inside the frame
                if (existingFrame->geometry().contains(posInMainWindow)) {
                    
                    return false;
                }

                if (subMenuFrame && subMenuFrame->isVisible()) {
                    if (subMenuFrame->geometry().contains(posInMainWindow)) {
                        return false;
                    }
                }

                if (button1->geometry().contains(posInMainWindow) or button2->geometry().contains(posInMainWindow)) {
                    return true;
                }

                if (menuToggleButton->geometry().contains(posInMainWindow)) {
                    handleFocusLeaveMenuButton();
                    functionBar->closeMenuBar();
                    return true;
                } else {
                    emit mouseClick();
                    handleFocusLeaveMenuButton();
                    disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
                    return true;
                }
                
            }
            if (contextMenuFrame && contextMenuFrame->isVisible()) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                QPoint posInMainWindow = this->mapFromGlobal(mouseEvent->globalPos());
                
                if (!contextMenuFrame->geometry().contains(posInMainWindow)) {
                    contextMenuFrame->hide();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

/*
Show a drop down list of options when mouse entering menu buttons.
[File☚] [Project]
+--------+
| New... |
| Open   |
| Save   |

| Search |
+--------+
menu buttons are the (file, project) buttons in (top left) function bar.
*/
void MainWindow::handleMouseEnterMenuButton(QPushButton *button) {
    // Remove any existing frame if present.
    if (existingFrame) {
        existingFrame->setParent(nullptr);
        existingFrame->deleteLater();
        existingFrame = nullptr;
    }

    if (subMenuFrame) {
        subMenuFrame->setVisible(false);
        subMenuFrame->setParent(nullptr);
        subMenuFrame->deleteLater();
        subMenuFrame = nullptr;
    }

    // Create a new frame for the hovered button.
    QFrame *newFrame = new QFrame(this);
    newFrame->setFrameShape(QFrame::NoFrame); // No default frame margins
    newFrame->setStyleSheet(
        "QFrame {   "
        "    margin: 0px; "
        "    padding: 0px; "
        "    background-color: #1F2020;"
        "    border: 1px solid #1F2020; "
        "    border-radius: 4px; "
        "}"
        "QMenuButton { "
        "    margin: 0px; "
        "    padding: 0px; "
        "    border: none; "
        "    background-color: transparent; "
        "}  "
    );

    // Set up the layout for the frame with no margins and spacing between buttons.
    QVBoxLayout *frameLayout = new QVBoxLayout(newFrame);
    frameLayout->setContentsMargins(10, 4, 6, 6);
    frameLayout->setSpacing(0);

    #ifdef Q_OS_MAC
        #define CONTROL_SYMBOL "⌘ "  // Mac Command key
    #else
        #define CONTROL_SYMBOL "Ctrl+"  // Windows & Linux use "Ctrl"
    #endif

    if (button == button1) {
        // Create buttons for Button 1's frame.
        MenuButton *frameButtonNew = new MenuButton("New...", "+", this);
        connect(frameButtonNew, &MenuButton::clicked, this, [this]() {
            emit mouseClick();
            handleFocusLeaveMenuButton();
            disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
            setupUntitledTab(FICTION_TAB);
        });
        
        // Connect hover signals for submenu
        connect(frameButtonNew, &MenuButton::hovered, this, [this, frameButtonNew]() {
            if (subMenuFrame && subMenuFrame->isVisible()) {
                subMenuFrame->setVisible(false);
            }

            // Create submenu frame on hover
            subMenuFrame = new QFrame(this);
            subMenuFrame->setFrameShape(QFrame::StyledPanel);
            subMenuFrame->setStyleSheet(
                "QFrame {   "
                "    margin: 0px; "
                "    padding: 0px; "
                "    background-color: #1F2020;"
                "    border: 1px solid #1F2020; "
                "    border-radius: 4px; "
                "}"
                "QMenuButton { "
                "    margin: 0px; "
                "    padding: 0px; "
                "    border: none; "
                "    background-color: transparent; "
                "}  "
            );
            
            QVBoxLayout *subMenuLayout = new QVBoxLayout(subMenuFrame);
            subMenuLayout->setContentsMargins(10, 4, 6, 6);
            subMenuLayout->setSpacing(0);
            
            // Create submenu buttons
            MenuButton *newTextFileButton = new MenuButton(".txt", "", subMenuFrame);
            MenuButton *newCellFileButton = new MenuButton(".cell.txt", CONTROL_SYMBOL "N", subMenuFrame);
            MenuButton *newMarkdownButton = new MenuButton(".md", "", subMenuFrame);

            // Connect submenu button signals
            connect(newTextFileButton, &MenuButton::clicked, this, [this]() {
                emit mouseClick();
                handleFocusLeaveMenuButton();
                disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
                setupUntitledTab(PLAINTEXT_TAB); // Use existing function for now
            });

            connect(newCellFileButton, &MenuButton::clicked, this, [this]() {
                emit mouseClick();
                handleFocusLeaveMenuButton();
                disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
                setupUntitledTab(FICTION_TAB); // Use existing function for now
            });
            
            connect(newMarkdownButton, &MenuButton::clicked, this, [this]() {
                emit mouseClick();
                handleFocusLeaveMenuButton();
                disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
                setupUntitledTab(MARKDOWN_TAB); // Use existing function for now
            });
            
            // Set fixed sizes for submenu buttons
            newTextFileButton->setFixedSize(128, 28);
            newCellFileButton->setFixedSize(128, 28);
            newMarkdownButton->setFixedSize(128, 28);
            
            // Add buttons to submenu layout
            subMenuLayout->addWidget(newTextFileButton);
            subMenuLayout->addWidget(newCellFileButton);
            subMenuLayout->addWidget(newMarkdownButton);
            
            // Add proper spacing between buttons and set alignment
            subMenuLayout->setSpacing(0);  // Add spacing between buttons
            subMenuLayout->setAlignment(Qt::AlignLeft);  // Align buttons to the left
            
            // Force the layout to calculate the proper size
            subMenuLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
            
            // Create and configure the drop shadow effect
            QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
            shadow->setBlurRadius(24);   // Increase for a softer, larger shadow
            shadow->setColor(QColor("#1F2020")); // Shadow color
            shadow->setOffset(0, 0);     // Zero offset to get shadow on all sides

            // Apply the effect to your frame
            subMenuFrame->setGraphicsEffect(shadow);

            // Position the submenu to the right of the button
            // get layout margins
            int marginRight = subMenuFrame->layout()->contentsMargins().right() + 7;
            int marginTop = subMenuFrame->layout()->contentsMargins().top();
            
            // Position the submenu to the right of the button
            QPoint buttonTopRight = frameButtonNew->mapToGlobal(QPoint(
                frameButtonNew->width() + marginRight, 
                -marginTop - 1));
            subMenuFrame->move(this->mapFromGlobal(buttonTopRight));
            
            // Show the submenu
            subMenuFrame->show();
        });
        
        connect(frameButtonNew, &MenuButton::notHovered, this, [this]() {
            // Use a timer to allow moving the mouse to the submenu
            QTimer::singleShot(200, this, [this]() {
                // Check if mouse is over the submenu
                if (subMenuFrame && !subMenuFrame->underMouse()) {
                    subMenuFrame->hide();
                }
            });
        });
        
        // Open file button
        MenuButton *frameButtonOpen = new MenuButton("Open", CONTROL_SYMBOL "O", newFrame);
        connect(frameButtonOpen, &MenuButton::clicked, this, [this]() {
            emit mouseClick();
            handleFocusLeaveMenuButton();
            disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
            openFile("");
        });            
        
        // Save file button
        MenuButton *frameButtonSave = new MenuButton("Save", CONTROL_SYMBOL "S", newFrame);
        connect(frameButtonSave, &MenuButton::clicked, this, [this]() {
            emit mouseClick();
            handleFocusLeaveMenuButton();
            disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
            saveFile();
        });

        MenuButton *frameButtonSearch = new MenuButton("Search", "", newFrame);
        connect(frameButtonSearch, &QPushButton::clicked, this, [this]() {
            emit mouseClick();
            handleFocusLeaveMenuButton();
            disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
            searchFile();
        });
        
        frameButtonNew->setFixedSize(128, 28);
        frameButtonOpen->setFixedSize(128, 28);
        frameButtonSave->setFixedSize(128, 28);
        frameButtonSearch->setFixedSize(128, 28);

        frameLayout->addWidget(frameButtonNew);
        frameLayout->addWidget(frameButtonOpen);
        frameLayout->addWidget(frameButtonSave);
        frameLayout->addWidget(frameButtonSearch);
    }
    else if (button == button2) {
        MenuButton *frameButtonNew = new MenuButton("New", "", newFrame);
        connect(frameButtonNew, &MenuButton::clicked, this, [this]() {
            emit mouseClick();
            handleFocusLeaveMenuButton();
            disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
            createAndOpenProject();
        });

        // Create buttons for Button 2's frame.
        MenuButton *frameButtonOpen = new MenuButton("Open", "", newFrame);
        connect(frameButtonOpen, &MenuButton::clicked, this, [this]() {
            emit mouseClick();
            handleFocusLeaveMenuButton();
            disconnect(this, &MainWindow::mouseClick, functionBar, &FunctionBar::closeMenuBar);
            openProject();
        });
        MenuButton *frameButtonSaveAll = new MenuButton("Save All", "", newFrame);
        MenuButton *frameButtonSwitch = new MenuButton("Switch...", "", newFrame);

        // Set fixed sizes and unique background colors.
        frameButtonOpen->setFixedSize(128, 28);
        frameButtonSaveAll->setFixedSize(128, 28);
        frameButtonSwitch->setFixedSize(128, 28);
        frameButtonNew->setFixedSize(128, 28);

        frameLayout->addWidget(frameButtonNew);
        frameLayout->addWidget(frameButtonOpen);
        frameLayout->addWidget(frameButtonSaveAll);
        frameLayout->addWidget(frameButtonSwitch);
    }

    // Resize the frame to fit its contents.
    newFrame->adjustSize();

    // Position the frame just below the button (adjusting as needed).
    QPoint buttonBottomLeft = button->mapToGlobal(QPoint(0, button->height() + 14));
    newFrame->move(this->mapFromGlobal(buttonBottomLeft));

    // Show the new frame and save it in the member variable.
    newFrame->setVisible(true);
    existingFrame = newFrame;

    // Create and configure the drop shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(24);   // Increase for a softer, larger shadow
    shadow->setColor(QColor("#1F2020")); // Shadow color
    shadow->setOffset(0, 0);     // Zero offset to get shadow on all sides

    // Apply the effect to your frame
    newFrame->setGraphicsEffect(shadow);

    connect(this, &MainWindow::mouseClick,
        functionBar, &FunctionBar::closeMenuBar,
        Qt::UniqueConnection);
}

/*
Remove the drop down option list when mouse leaving menu buttons.
menu buttons are the (file, project) buttons in (top left) function bar.
*/
void MainWindow::handleFocusLeaveMenuButton() {
    if (existingFrame) {
        existingFrame->setVisible(false);
    }
    if (subMenuFrame) {
        subMenuFrame->setVisible(false);
    }
}

/*
Find the image in .md file.
triggered by QMarkdownTextEdit::readBlock -> (signal) showImageAt(imagePath, globalPos);

![img☚](path/to/img.png)
     +------------------------+
     |                        |
     | ʕ•̫͡•ʕ*̫͡*ʕ•͓͡•ʔ-̫͡-ʕ•̫͡•ʔ*̫͡*ʔ-̫͡-ʔ |
     |                        |
     +------------------------+

this method will try to read the image from file-system or download from internet.
*/
void MainWindow::showMarkdownImage(const QString &imagePath, QPoint lastMousePos) {
    // Check if imagePath is a URL
    QUrl url(imagePath);
    if (url.isValid() && (url.scheme() == "http" || url.scheme() == "https")) {
        // Download image from URL
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        
        QNetworkReply *reply = networkManager->get(request);
        
        // Store the mouse position for when the download completes
        connect(reply, &QNetworkReply::finished, this, [this, reply, lastMousePos]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    displayImage(pixmap, lastMousePos);
                } else {
                    qDebug() << "Failed to load image from downloaded data";
                }
            } else {
                qDebug() << "Failed to download image:" << reply->errorString();
            }
            reply->deleteLater();
        });
    } else {
        // Handle local file path
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            displayImage(pixmap, lastMousePos);
        } else {
            qDebug() << "Failed to load local image:" << imagePath;
        }
    }
}

/*
Show a floating window that displays image.

![img☚](path/to/img.png)
     +------------------------+
     |                        |
     | ʕ•̫͡•ʕ*̫͡*ʕ•͓͡•ʔ-̫͡-ʕ•̫͡•ʔ*̫͡*ʔ-̫͡-ʔ |
     |                        |
     +------------------------+

max size for image frame is [800, 600]
*/
void MainWindow::displayImage(const QPixmap &pixmap, QPoint lastMousePos) {
    // Define maximum dimensions for the scaled image
    const int MAX_WIDTH = 800;
    const int MAX_HEIGHT = 600;

    const int EDGE_PADDING = 8;
    const int TOP_PADDING = 48;

    int windowMaxWidth = qMin(this->width() - 2*EDGE_PADDING, MAX_WIDTH);
    int windowMaxHeight = qMin(this->height() - EDGE_PADDING - TOP_PADDING, MAX_HEIGHT);

        // Calculate the scaling factor to fit within max dimensions while maintaining aspect ratio
        QSize originalSize = pixmap.size();
        double scaleFactor = 0.2 + 0.8/((originalSize.width()+originalSize.height())/128+1);
        qDebug() << "scaleFactor: " << scaleFactor;
        
        // Calculate scaled size
        QSize scaledSize = originalSize * scaleFactor;
        
        // If scaled size exceeds max dimensions, adjust scale factor
        if (scaledSize.width() > windowMaxWidth || scaledSize.height() > windowMaxHeight) {
            double widthRatio = static_cast<double>(windowMaxWidth) / static_cast<double>(originalSize.width());
            double heightRatio = static_cast<double>(windowMaxHeight) / static_cast<double>(originalSize.height());
            scaleFactor = qMin(0.2, qMin(widthRatio, heightRatio));
        }

        // Scale the image with the calculated factor
        QPixmap scaledPixmap = pixmap.scaled(originalSize * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Create ImageFrame if it doesn't exist
        if (!imageFrame) {
            imageFrame = new ImageFrame(this);
        }

        // Set the image to the frame
        imageFrame->setImage(scaledPixmap);

        // Resize the frame to fit the scaled image
        QSize imageSize = scaledPixmap.size();
        imageFrame->resize(imageSize);

        // Position the frame relative to the mouse position with offset
        QPoint popupPos = this->mapFromGlobal(lastMousePos - QPoint(8, 8));

        // Ensure the frame stays within window bounds
        QSize windowSize = this->size();

        // Adjust x-coordinate if needed
        if (popupPos.x() + imageSize.width() > windowSize.width() - EDGE_PADDING) {
            popupPos.setX(windowSize.width() - imageSize.width() - EDGE_PADDING);
        }
        if (popupPos.x() < EDGE_PADDING) {
            popupPos.setX(EDGE_PADDING);
        }

        // Adjust y-coordinate if needed
        if (popupPos.y() + imageSize.height() > windowSize.height() - EDGE_PADDING) {
            popupPos.setY(windowSize.height() - imageSize.height() - EDGE_PADDING);
        }
        if (popupPos.y() < TOP_PADDING) {
            popupPos.setY(TOP_PADDING);
        }

        imageFrame->move(popupPos);

        // Create and configure the drop shadow effect
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(24);   // Increase for a softer, larger shadow
        shadow->setColor(QColor("#1F2020"));    // Shadow color
        shadow->setOffset(0, 0);     // Zero offset to get shadow on all sides

        // Apply the effect to your frame
        imageFrame->setGraphicsEffect(shadow);

        imageFrame->show();
}

void MainWindow::hideMarkdownImage() {
    if (imageFrame) {
        imageFrame->hide();  // Hide the frame
    }
}

/*
Show wiki content in a popup frame.
*/
void MainWindow::showWiki(const QString &wikiContent, QPoint lastMousePos) {
    // Create WikiFrame if it doesn't exist
    if (!wikiFrame) {
        wikiFrame = new WikiFrame(this);
    }

    // Set the wiki content
    wikiFrame->setContent(wikiContent);

    // Position the frame relative to the mouse position with offset
    QPoint popupPos = this->mapFromGlobal(lastMousePos - QPoint(8, 8));

    // Ensure the frame stays within window bounds
    QSize frameSize = wikiFrame->size();
    QSize windowSize = this->size();

    // Adjust x-coordinate if needed
    if (popupPos.x() + frameSize.width() > windowSize.width()) {
        popupPos.setX(windowSize.width() - frameSize.width());
    }
    if (popupPos.x() < 0) {
        popupPos.setX(0);
    }

    // Adjust y-coordinate if needed
    if (popupPos.y() + frameSize.height() > windowSize.height()) {
        popupPos.setY(windowSize.height() - frameSize.height());
    }
    if (popupPos.y() < 0) {
        popupPos.setY(0);
    }

    wikiFrame->move(popupPos);
    wikiFrame->show();
}

void MainWindow::hideWiki() {
    if (wikiFrame) {
        wikiFrame->hide();
    }
}

// Add these methods to MainWindow class
void MainWindow::showContextMenu(const QStringList &options, const QModelIndex &index, bool isDir) {
    // Store the index and type for later use
    currentContextMenuIndex = index;
    isContextMenuForDir = isDir;
    
    // Create a frame for the context menu if it doesn't exist
    if (!contextMenuFrame) {
        contextMenuFrame = new QFrame(this);
        contextMenuFrame->setFrameShape(QFrame::NoFrame);
        contextMenuFrame->setStyleSheet(
            "QFrame {"
            "    background-color: #2c2c2c;"
            "    border-radius: 5px;"
            "    padding: 4px 4px 6px 4px;"
            "    border: 1px solid #1F2020;"
            "}"
            "QPushButton {"
            "    background-color: transparent;"  // Explicitly transparent
            "    padding: 0px 24px 0px 4px;"
            "    color: #BDBDBD;"
            "    text-align: left;"
            "    margin: 0px;"
            "    border: none;"  // Remove any default borders
            "}"
            "QPushButton:hover {"
            "    color: #FFFFFF;"
            "    background-color: transparent;"  // Keep transparent on hover
            "}"
        );
        
        // Add drop shadow effect
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(24);
        shadow->setColor(QColor("#1F2020"));
        shadow->setOffset(0, 0);
        contextMenuFrame->setGraphicsEffect(shadow);
    }
    
    // Clear any existing layout
    if (contextMenuFrame->layout()) {
        QLayoutItem *item;
        while ((item = contextMenuFrame->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete contextMenuFrame->layout();
    }
    
    // Create a new layout
    QVBoxLayout *layout = new QVBoxLayout(contextMenuFrame);
    layout->setContentsMargins(6, 2, 6, 2);
    layout->setSpacing(3);
    
    // Add buttons for each option
    for (const QString &option : options) {
        QPushButton *button = new QPushButton(option, contextMenuFrame);
        button->setFixedSize(104, 25);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, [this, option]() {
            handleContextMenuSelection(option);
        });
        layout->addWidget(button);
    }
    
    // Position the frame at the cursor position
    QPoint pos = this->mapFromGlobal(QCursor::pos());
    contextMenuFrame->move(pos);
    
    // Size the frame appropriately
    contextMenuFrame->adjustSize();
    
    // Show the frame
    contextMenuFrame->raise();
    contextMenuFrame->show();
    
    // Install event filter to close the menu when clicked outside
    qApp->installEventFilter(this);
}

void MainWindow::handleContextMenuSelection(const QString &action) {
    // Hide the context menu frame
    if (contextMenuFrame) {
        contextMenuFrame->hide();
    }
    
    // Forward the action to the folder tree view
    folderTreeView->handleContextMenuAction(action, currentContextMenuIndex, isContextMenuForDir);
}

void MainWindow::openFileTreeView() {
    QList<int> sizes = centralSplitter->sizes();
    if (sizes[0] == 0) {  // Only open if it's currently closed
        toggleFileTreeView();
    }
}

void MainWindow::closeFileTreeView() {
    QList<int> sizes = centralSplitter->sizes();
    if (sizes[0] != 0) {  // Only close if it's currently open
        toggleFileTreeView();
    }
}

void MainWindow::activatePrisonerModeFunc(int timeLimit, int wordGoal) {
    // Close side panel
    closeFileTreeView();
    
    // Make handler non-draggable
    centralSplitter->handle(1)->setEnabled(false);
    
    // Hide custom tab bar
    functionBar->setVisible(false);
    
    // Hide side panel button
    sidePanelButton->setVisible(false);
    splitterTopWidget->setVisible(true);

    // make the window maximum
    this->showFullScreen();
    splitterContainer->setFullScreen(true);
    splitterContainer->startTimerProgress(timeLimit, wordGoal);
}

void MainWindow::deactivatePrisonerModeFunc() {
    // Show customtabbar
    functionBar->setVisible(true);

    // First ensure window is not maximized
    this->showNormal();
    splitterContainer->setFullScreen(false);
    splitterContainer->clearTimerProgress();
    prisonerManager->clear();

    // Restore side panel
    if (projectManager->isLoadedProject) {
        // openFileTreeView();
        // Show side panel button
        sidePanelButton->setVisible(true);
    }
    
    // Make handler draggable again
    centralSplitter->handle(1)->setEnabled(true);
    splitterTopWidget->setVisible(false);
}

/*
cursor behavior near edges of window

resizing window in linux
*/

Qt::Edges MainWindow::edgeAt(const QPoint &pos) const {
        Qt::Edges edges;
        QRect r = rect();
        if (pos.x() <= RESIZE_MARGIN) {
            edges |= Qt::LeftEdge;
        } else if (pos.x() >= r.width() - RESIZE_MARGIN) {
            edges |= Qt::RightEdge;
        }
        
        if (pos.y() >= r.height() - RESIZE_MARGIN) {
            edges |= Qt::BottomEdge;
        }
        return edges;
    }

void MainWindow::updateCursor(const Qt::Edges &edges) {
    // Get the widget under the cursor
    QWidget* widgetUnderCursor = QApplication::widgetAt(QCursor::pos());
    
    // Only show resize cursors if we have valid edges and window handle
    if (edges != Qt::Edges() && windowHandle()) {
        // Only change cursor if we're directly over the splitterContainer
        if (widgetUnderCursor == splitterContainer) {
            if (edges == (Qt::LeftEdge) || edges == (Qt::RightEdge)) {
                splitterContainer->setCursor(Qt::SizeHorCursor);
            } else if (edges == (Qt::BottomEdge)) {
                splitterContainer->setCursor(Qt::SizeVerCursor);
            }
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    #ifdef Q_OS_LINUX
        if (event->button() == Qt::LeftButton) {
            Qt::Edges e = edgeAt(event->pos());
            // Only attempt resize if we have valid edges
            if (e != Qt::Edges() && windowHandle()) {
                windowHandle()->startSystemResize(e);
                return; // do not pass to base
            }
        }
    #endif
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    #ifdef Q_OS_LINUX
        // only change cursor if not dragging
        if (!(event->buttons() and Qt::LeftButton)) {
            updateCursor(edgeAt(event->pos()));
        }
    #endif
    QMainWindow::mouseMoveEvent(event);
}
