#include "foldertreeviewwidget.h"
#include "customtreestyle.h"
#include "customfilesystemmodel.h"
#include "projectmanager.h"
#include <QTreeView>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QDir>
#include <QScreen>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextBrowser>
#include <QFileDialog>
#include <QClipboard>
#include <QLabel>

FolderTreeViewWidget::FolderTreeViewWidget(QWidget *parent,
                   QString folderRoot,
                   ProjectManager *projectManager)
    : QWidget(parent),
      fileModel(nullptr),
      fileTreeView(nullptr),
      layout(nullptr),
      buttonWidget(nullptr),
      projectManager(projectManager)
{
    this->setStyleSheet(
        "FolderTreeViewWidget {"
        "    background-color: #1F2020;"
        "    border: none;"
        "}"
    );

    // Initialize fileModel with the custom file system model
    fileModel = new CustomFileSystemModel(this);
    fileModel->setRootPath(folderRoot);

    // Initialize layout and fileTreeView
    layout = new QVBoxLayout(this);
    // Set margins and spacing to zero
    layout->setContentsMargins(6, 0, 8, 8);   // margins
    layout->setSpacing(0);                    // No spacing between widgets

    fileTreeView = new QTreeView(this);
    CustomTreeStyle *customStyle = new CustomTreeStyle(":/icons/angle_right.png", ":/icons/angle_down.png");
    fileTreeView->setStyle(customStyle);

    setupButton();
    // Set up file tree view and add it to layout
    setupFileTree();
    this->setLayout(layout);

    // Add overlay widget
    overlayWidget = new QWidget(this);
    overlayWidget->setStyleSheet("background-color: rgba(31, 32, 32, 0.33);"); // Semi-transparent dark overlay
    overlayWidget->setAttribute(Qt::WA_TransparentForMouseEvents); // Allow mouse events to pass through
    overlayWidget->raise(); // Ensure it's on top
    overlayWidget->setGeometry(this->rect()); // Set initial geometry
    
    // Enable mouse tracking for the main widget
    this->setMouseTracking(true);
}

// Add this method to handle resize events
void FolderTreeViewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (overlayWidget) {
        overlayWidget->setGeometry(this->rect());
    }
}

void FolderTreeViewWidget::enterEvent(QEnterEvent* event)
{
    FadeAnimationUtil::fadeOut(overlayWidget, 150);
    QWidget::enterEvent(event);
}

void FolderTreeViewWidget::leaveEvent(QEvent* event)
{
    FadeAnimationUtil::fadeIn(overlayWidget, 150);
    QWidget::leaveEvent(event);
}

// In the refresh method, ensure the overlay is properly handled
void FolderTreeViewWidget::refresh(const QString &newFolderRoot) {
    qDebug() << "REFRESH - Checking fileTreeView:" << (fileTreeView ? "exists" : "NULL");
    if (fileTreeView) {
        delete fileTreeView;
        fileTreeView = nullptr;
    }

    // Update folderRoot with the new path (if provided)
    if (!newFolderRoot.isEmpty()) {
        folderRoot = newFolderRoot;
    }

    // Trigger project manager to reopen the project
    if (projectManager && !folderRoot.isEmpty()) {
        projectManager->open(folderRoot);
    }

    fileTreeView = new QTreeView(this);
    CustomTreeStyle *customStyle = new CustomTreeStyle(":/icons/angle_right.png", ":/icons/angle_down.png");
    fileTreeView->setStyle(customStyle);

    fileModel->setRootPath(folderRoot);
    setupButton();
    setupFileTree();
    
    // Ensure overlay remains on top after refresh
    if (overlayWidget) {
        overlayWidget->raise();
        overlayWidget->setGeometry(this->rect());
    }
}

void FolderTreeViewWidget::setupButton() {

    if (buttonWidget && layout) {
        layout->removeWidget(buttonWidget);
        buttonWidget->hide();
        buttonWidget->deleteLater();
        buttonWidget = nullptr;
    }

    buttonWidget = new QWidget(this);
    buttonWidget->setStyleSheet("background-color: #1F2020;");
    buttonWidget->setFixedHeight(48);
    qDebug() << "folderRoot: " << folderRoot;

    // Add a label before the buttons
    QString path = QDir::toNativeSeparators(folderRoot);
    QString deepestFolder = path.split(QDir::separator()).last();  // Get the last folder name
    QString firstLetter = deepestFolder.isEmpty() ? QString("E") : deepestFolder.at(0);
    QLabel *titleLabel = new QLabel(firstLetter, this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    color: #545555;"
        "    font-size: 10px;"
        "    border: 1px solid #545555;"
        "    border-radius: 2px;"
        "}"
    );
    titleLabel->setFixedSize(16, 16);
    titleLabel->setAlignment(Qt::AlignCenter);  // Center the text both horizontally and vertically

    QSpacerItem *topSpacerMiddle = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);

    QPushButton *newFileButton = new QPushButton(this);
    newFileButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/newfile_silent.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/newfile_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            // "QPushButton:pressed {"
            // "border-image: url(:/icons/newfile_clicked.png) 0 0 0 0 stretch stretch;"
            // "}"
            "   padding: 10px;"
    );
    newFileButton->setFixedSize(13, 13);
    connect(newFileButton, &QPushButton::clicked, this, [this]() {
        addFile(); // Call without parameters (uses default behavior)
    });

    QPushButton *newFolderButton = new QPushButton(this);
    newFolderButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/newfolder_silent.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/newfolder_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            // "QPushButton:pressed {"
            // "border-image: url(:/icons/newfolder_clicked.png) 0 0 0 0 stretch stretch;"
            // "}"
            "   padding: 10px;"
    );
    newFolderButton->setFixedSize(13, 13);
    connect(newFolderButton, &QPushButton::clicked, this, [this]() {
        addFolder(); // Call without parameters (uses default behavior)
    });

    QPushButton *refreshButton = new QPushButton(this);
    refreshButton->setStyleSheet(
            "QPushButton {"
            "border: none;"
            "border-image: url(:/icons/refresh_silent.png) 0 0 0 0 stretch stretch;"
            "}"
            "QPushButton:hover {"
            "border-image: url(:/icons/refresh_hover.png) 0 0 0 0 stretch stretch;"
            "}"
            // "QPushButton:pressed {"
            // "border-image: url(:/icons/refresh_clicked.png) 0 0 0 0 stretch stretch;"
            // "}"
            "   padding: 10px;"
    );
    refreshButton->setFixedSize(13, 13);
    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        QString projectRootPath = fileModel->rootPath();
        this->refresh(projectRootPath);
    });

    // Create a horizontal layout to add buttons
    QHBoxLayout *buttonWidgetLayout = new QHBoxLayout;
    buttonWidgetLayout->setContentsMargins(2, 0, 8, 0);  // Remove margins to fit buttons neatly
    buttonWidgetLayout->setSpacing(12);  // Optional: set spacing between buttons
    
    // Add text to the layout
    buttonWidgetLayout->addWidget(titleLabel);
    buttonWidgetLayout->addItem(topSpacerMiddle);

    // Add buttons to the layout
    buttonWidgetLayout->addWidget(newFileButton);
    buttonWidgetLayout->addWidget(newFolderButton);
    buttonWidgetLayout->addWidget(refreshButton);

    // Set the layout for the widget
    buttonWidget->setLayout(buttonWidgetLayout);

    layout->addWidget(buttonWidget);
}

void FolderTreeViewWidget::setupFileTree() {
    // Set the model for the tree view
    fileTreeView->setModel(fileModel);
    fileTreeView->setRootIndex(fileModel->index(folderRoot));

    // Hide all columns except the first one (file name column)
    for (int i = 1; i < fileModel->columnCount(); ++i) {
        fileTreeView->setColumnHidden(i, true);
    }

    // Hide the header (both horizontal and vertical)
    fileTreeView->header()->hide();
    fileTreeView->setHeaderHidden(true);

    // Disable horizontal scrollbar
    fileTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Customize the appearance of the file tree view
    fileTreeView->setStyleSheet(
        "QTreeView { "
        "    background-color: transparent; "
        "    border: none; "
        "    color: #BDBDBD;"
        "}"
        "QTreeView::item { "
        "    border: none; "
        "    border-radius: 0px; "
        "    padding: 3px 0px; "
        "    margin: 0px; "
        "}"
        "QTreeView::item:hover { "
        "    background-color: #1F2020; "
        "    color: #FFFFFF; "
        "}"
        "QTreeView::item:selected { "
        "    background-color: #1F2020; " //#262626
        "    color: #84e0a5; "
        "}"
        "QTreeView QLineEdit { "              // Editor widget styling
        "    background-color: #2c2c2c; "
        "    color: #FFFFFF; "
        "    border: 1px solid #2c2c2c; "
        "    border-radius: 4px; "
        "    padding: 0px 5px; "
        "    selection-background-color: #84e0a5; "  // Selection color
        "    selection-color: #2c2c2c; "
        "}"
        "QTreeView QLineEdit:focus { "
        "    border: 1px solid #2c2c2c; "    // Accent color for focus state
        "}"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 8px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #262626;"
        "    min-height: 16px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #2C2C2C;"
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

    // Add fileTreeView to the vertical layout
    layout->addWidget(fileTreeView);

    fileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    fileTreeView->setEditTriggers(QTreeView::NoEditTriggers);
    connect(fileTreeView, &QTreeView::customContextMenuRequested, this, &FolderTreeViewWidget::onCustomContextMenu);
    connect(fileTreeView, &QTreeView::doubleClicked, this, &FolderTreeViewWidget::onDoubleClicked);
}

// Toggle visibility of the file tree view
void FolderTreeViewWidget::toggleFileTreeView() {
    // Toggle visibility of the file tree view and button widget
    bool isVisible = fileTreeView->isVisible();
    fileTreeView->setVisible(!isVisible); // Toggle file tree view visibility
    buttonWidget->setVisible(!isVisible);   // Show buttons when tree view is hidden, and hide buttons when tree view is shown
}

void FolderTreeViewWidget::onCustomContextMenu(const QPoint &point) {
    //  get the range of the clicked item
    QRect widgetRect = this->rect();

    // Get the index of the item that was clicked
    QModelIndex index = fileTreeView->indexAt(point);

    if (!index.isValid()) {
        return;  // If no valid item was clicked, return
    }

    // Use the clicked index instead of currentIndex()
    bool isDir = fileModel->isDir(index);
    QStringList menuOptions;
    
    if (isDir) {
        // Directory options
        menuOptions << "Delete" << "Rename" << "New file" << "Copy Path";
    } else {
        // File options
        menuOptions << "Open" << "Delete" << "Rename" << "Copy Path";
    }
    
    emit showContextMenuInMainWindow(menuOptions, index, isDir);
}

// Add these methods to handle the action selected from MainWindow
void FolderTreeViewWidget::handleContextMenuAction(const QString &action, const QModelIndex &index, bool isDir) {
    if (isDir) {
        // Handle directory actions
        if (action == "Delete") {
            QFileInfo fileInfo(fileModel->filePath(index));
            auto answer = QMessageBox::question(nullptr, "Confirm Delete", 
                                  "Are you sure you want to delete '" + fileInfo.fileName() + "'?",
                                  QMessageBox::Yes | QMessageBox::No);
            if (answer == QMessageBox::Yes) {
                QDir dir(fileInfo.filePath());
                if (!dir.removeRecursively()) {
                    QMessageBox::critical(nullptr, "Error", "Failed to delete directory");
                }
            }
        } else if (action == "Rename") {
            fileTreeView->edit(index);
        } else if (action == "New file") {
            QString dirPath = fileModel->filePath(index);
            this->addFile(dirPath);
        } else if (action == "Copy Path") {
            QString dirPath = fileModel->filePath(index);
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(dirPath);
        }
    } else {
        // Handle file actions
        if (action == "Open") {
            QString filePath = fileModel->filePath(index);
            emit doubleClickedOnFile(filePath);
        } else if (action == "Delete") {
            QString filePath = fileModel->filePath(index);
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, 
                "Confirm Delete",
                "Are you sure you want to delete this file?",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                QFile file(filePath);

                if (file.remove()) {
                    emit fileDeleted(filePath);
                    QString projectRootPath = fileModel->rootPath();

                    this->refresh(projectRootPath); // Refresh view
                } else {
                    QMessageBox::warning(this, "Error", "Failed to delete the file");
                }
            }
        } else if (action == "Rename") {
            QString originalFilePath = fileModel->filePath(index);
            fileTreeView->edit(index);
            
            connect(fileTreeView->itemDelegate(), &QAbstractItemDelegate::commitData, this, 
                [this, index, originalFilePath](QWidget *editor) {
                    QString newFilePath = fileModel->filePath(index);
                    qDebug() << "File renamed from:" << originalFilePath << "to:" << newFilePath;
                    
                    // Emit the signal with the new path
                    emit fileRenamed(originalFilePath, newFilePath);
                    
                    // Disconnect to prevent multiple signals
                    disconnect(fileTreeView->itemDelegate(), &QAbstractItemDelegate::commitData, this, nullptr);
                });
        } else if (action == "Copy Path") {
            QString dirPath = fileModel->filePath(index);
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(dirPath);
        }
    }
}

void FolderTreeViewWidget::addFile(const QString &targetDir)
{
    // Get the currently selected index from the file tree
    QModelIndex currentIndex = fileTreeView->currentIndex();

    QString currentDir;
    if (!targetDir.isEmpty()) {
        // Use the provided target directory, converting it to an absolute path
        currentDir = QDir(targetDir).absolutePath();
    } else {
        // Original logic to determine the directory based on the current selection
        if (currentIndex.isValid() && fileModel->isDir(currentIndex)) {
            currentDir = fileModel->fileInfo(currentIndex).absoluteFilePath();
        } else {
            currentDir = fileModel->rootDirectory().absolutePath();
        }
    }

    // Create a default new file name
    QString defaultFileName = "newfile.txt";
    QString newFilePath = QDir(currentDir).filePath(defaultFileName);

    // Ensure the default file doesn't exist already
    int fileCount = 1;
    while (QFile::exists(newFilePath)) {
        defaultFileName = QString("newfile_%1.txt").arg(fileCount++);
        newFilePath = QDir(currentDir).filePath(defaultFileName);
    }

    // Create the file
    QFile newFile(newFilePath);
    if (newFile.open(QIODevice::WriteOnly)) {
        newFile.close();  // Close after creating an empty file
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create the file"));
        return;
    }

    // Force the model to update to show the new file
    // this->refresh(fileModel->rootPath());

    // Get the index of the new file in the model
    QModelIndex newFileIndex = fileModel->index(newFilePath);

    // Make the new file name editable
    fileTreeView->setCurrentIndex(newFileIndex);
    fileTreeView->edit(newFileIndex);

    // Connect to the editingFinished signal for automatic saving
    // connect(fileTreeView->itemDelegate(), &QAbstractItemDelegate::commitData, this, [this, newFilePath](QWidget *editor) {
    //     QModelIndex currentIndex = fileTreeView->currentIndex();
    //     QString newName = fileModel->data(currentIndex, Qt::EditRole).toString();

    //     if (!newName.isEmpty() && newName != QFileInfo(newFilePath).fileName()) {
    //         // Rename the file to the new name
    //         QString newPath = QFileInfo(newFilePath).absolutePath() + "/" + newName;
    //         QFile::rename(newFilePath, newPath);
    //         QString projectRootPath = fileModel->rootPath();
    //         this->refresh(projectRootPath);  // Refresh the view
    //     }
    // });
}

void FolderTreeViewWidget::addFolder(const QString &targetDir)
{
    // Get the target directory path
    QModelIndex currentIndex = fileTreeView->currentIndex();
    QString currentDir;

    if (!targetDir.isEmpty()) {
        currentDir = QDir(targetDir).absolutePath();
    } else {
        if (currentIndex.isValid() && fileModel->isDir(currentIndex)) {
            currentDir = fileModel->fileInfo(currentIndex).absoluteFilePath();
        } else {
            currentDir = fileModel->rootDirectory().absolutePath();
        }
    }

    // Create default folder name
    QString defaultFolderName = tr("newfolder");
    QString newFolderPath = QDir(currentDir).filePath(defaultFolderName);

    // Handle existing folders
    int folderCount = 1;
    while (QDir(newFolderPath).exists()) {
        defaultFolderName = tr("newfolder %1").arg(folderCount++);
        newFolderPath = QDir(currentDir).filePath(defaultFolderName);
    }

    // Create the directory
    QDir dir;
    if (!dir.mkdir(newFolderPath)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create folder"));
        return;
    }

    // Get index and make editable
    QModelIndex newFolderIndex = fileModel->index(newFolderPath);
    fileTreeView->setCurrentIndex(newFolderIndex);
    fileTreeView->edit(newFolderIndex);
}

void FolderTreeViewWidget::onDoubleClicked(const QModelIndex &index)
{
    if (fileModel->isDir(index)) {
        return;
    }
    QString clickedfilePath = fileModel->filePath(index);
    emit doubleClickedOnFile(clickedfilePath);
}

// void FolderTreeViewWidget::refresh(const QString &newFolderRoot) {
//     if (fileTreeView) {
//         delete fileTreeView;
//         fileTreeView = nullptr;
//     }

//     // Update folderRoot with the new path (if provided)
//     if (!newFolderRoot.isEmpty()) {
//         folderRoot = newFolderRoot;
//     }

//     fileTreeView = new QTreeView(this);
//     CustomTreeStyle *customStyle = new CustomTreeStyle(":/icons/angle_right.png", ":/icons/angle_down.png");
//     fileTreeView->setStyle(customStyle);

//     qDebug() << "folderRoot" << folderRoot;
//     fileModel->setRootPath(folderRoot);
//     setupButton();
//     setupFileTree();
// }