#include "customtabwidget.h"
#include "fictiontextedit.h"
#include "fictionviewtab.h"
#include "plaintextedit.h"
#include "plaintextviewtab.h"
#include "popups/savemessagebox.h"
#include "markdownviewtab.h"

#include <QVBoxLayout>
#include <QMainWindow>
#include <QStylePainter>
#include <QStyleOptionTab>
#include <QScrollArea>
#include <QPainterPath>
#include <QSpacerItem>
#include <QScrollBar>
#include <QRegularExpression>


CustomTabWidget::CustomTabWidget(QWidget *parent, ProjectManager *projectManager, PrisonerManager *prisonerManager)
    : QTabWidget(parent)
    , untitledCount(1)
    , projectManager(projectManager)
    , prisonerManager(prisonerManager)
{
    setupTabBar();
    setupTabWidget();
    setupStyles();

    setTabPosition(QTabWidget::North);
}

void CustomTabWidget::tabInserted(int index) {
    QTabWidget::tabInserted(index);                                          // Call the base implementation
    emit tabInsertedSignal(index, tabText(index));                           // Emit custom signal
}

void CustomTabWidget::setupTabBar() {
    // // customTabBar = new CustomTabBar();
    // // setTabBar(customTabBar);
    // setTabPosition(QTabWidget::North); // Options: North, South, East, West
    tabBar()->hide();
}

void CustomTabWidget::setupTabWidget() {
    setTabsClosable(true);
    setMovable(true);

    // Connect the signal for closing tabs to the closeWindowIfNoTabs method
    connect(this, &QTabWidget::tabCloseRequested, this, &CustomTabWidget::closeWindowIfNoTabs);
}

void CustomTabWidget::setupStyles() {
    setStyleSheet(
        "QTabWidget::pane {"
                            "    border: 0px;"
                            "    background-color: transparent;"
                            "    margin: 0px;"
                            "}"
    );
}

void CustomTabWidget::createNewTab(const QString &filePath,
                                   bool isUntitled,
                                   int tabIndex) {

    QWidget *newTab;

    if (not filePath.isEmpty()) {
        int tabIndex = this->checkIdenticalOpenedFile(filePath);
        if (tabIndex != -1) {
            this->setCurrentIndex(tabIndex);
            emit tabActivatedSignal(tabIndex );
            return;
        }
    }

    // 
    QString tabName;
    QString content;
    if (isUntitled) {
        tabName = "untitled-" + QString::number(untitledCount++);
        content = "";
    } else {
        // If a file is selected (either from the dialog or the provided path), load the file
        QFile file(filePath);
        tabName = QFileInfo(filePath).fileName();   // get tabName
        QFileInfo fileInfo(file);

        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            content = in.readAll();    //get content
            file.close();
        }
    }
    
    // Create a new tab with the file name as the tab text
    if (tabName.endsWith(".cell.txt") || (isUntitled && filePath.isEmpty())) {
        qDebug() << "CustomTabWidget::createNewTab - Creating FictionViewTab with filePath:" << (filePath.isEmpty() ? "(empty)" : filePath) << "isUntitled:" << isUntitled << "tabName:" << tabName;
        newTab = new FictionViewTab(content, filePath, this, false, projectManager, prisonerManager);
        connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::onChangeFileType,
                this, &CustomTabWidget::updateFileType);
        connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::showWikiAt,
                this, &CustomTabWidget::showWikiAt);
        connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::hideWiki,
                this, &CustomTabWidget::hideWiki);
        connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::activatePrisonerModeSignal,
                this, &CustomTabWidget::activatePrisonerModeFunc);
        connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::deactivatePrisonerModeSignal,
                this, &CustomTabWidget::deactivatePrisonerModeFunc);
    } else if (tabName.endsWith(".md") || (isUntitled && filePath.endsWith(".md"))) {
        newTab = new MarkdownViewTab(content, filePath, this);
        connect(static_cast<MarkdownViewTab*>(newTab), &MarkdownViewTab::showImageAt,
                this, &CustomTabWidget::showImageAt);
        connect(static_cast<MarkdownViewTab*>(newTab), &MarkdownViewTab::hideImage,
                this, &CustomTabWidget::hideImage);

    } else {
        newTab = new PlaintextViewTab(content, filePath, this);
    }

    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeTabName, this, &CustomTabWidget::updateTabTitle);

    if (tabIndex == -1) {
        int newIndex = addTab(newTab, tabName);
    } else {
        int newIndex = insertTab(tabIndex, newTab, tabName); 
        // TODO: set cursor positioin and scroll bar maybe
    }
    
    setCurrentWidget(newTab);
}

void CustomTabWidget::createFictionTab(const QString &filePath, bool isUntitled, int tabIndex) {
    QWidget *newTab;
    QString tabName;
    QString content = "";
    
    if (!filePath.isEmpty() && !isUntitled) {
        if (this->checkIdenticalOpenedFile(filePath) != -1) {
            return;
        }
        
        // Load file content
        QFile file(filePath);
        tabName = QFileInfo(filePath).fileName();
        
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            content = in.readAll();
            file.close();
        }
    } else {
        tabName = "untitled-" + QString::number(untitledCount++);
    }
    
    // Create a fiction view tab
    newTab = new FictionViewTab(content, filePath, this, false, projectManager, prisonerManager);
    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeFileType,
        this, &CustomTabWidget::updateFileType);
    connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::showWikiAt,
            this, &CustomTabWidget::showWikiAt);
    connect(static_cast<FictionViewTab*>(newTab), &FictionViewTab::hideWiki,
            this, &CustomTabWidget::hideWiki);
    
    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeTabName, 
            this, &CustomTabWidget::updateTabTitle);
    
    if (tabIndex == -1) {
        int newIndex = addTab(newTab, tabName);
    } else {
        int newIndex = insertTab(tabIndex, newTab, tabName);
    }
    
    setCurrentWidget(newTab);
}

void CustomTabWidget::createPlainTextTab(const QString &filePath, bool isUntitled, int tabIndex) {
    QWidget *newTab;
    QString tabName;
    QString content = "";
    
    if (!filePath.isEmpty() && !isUntitled) {
        if (this->checkIdenticalOpenedFile(filePath) != -1) {
            return;
        }
        
        // Load file content
        QFile file(filePath);
        tabName = QFileInfo(filePath).fileName();
        
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            content = in.readAll();
            file.close();
        }
    } else {
        tabName = "untitled-" + QString::number(untitledCount++);
    }
    
    // Create a plaintext view tab
    newTab = new PlaintextViewTab(content, filePath, this);
    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeFileType,
        this, &CustomTabWidget::updateFileType);
    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeTabName, 
            this, &CustomTabWidget::updateTabTitle);
    
    if (tabIndex == -1) {
        int newIndex = addTab(newTab, tabName);
    } else {
        int newIndex = insertTab(tabIndex, newTab, tabName);
    }
    
    setCurrentWidget(newTab);
}

void CustomTabWidget::createMarkdownTab(const QString &filePath, bool isUntitled, int tabIndex) {
    QWidget *newTab;
    QString tabName;
    QString content = "";
    QString actualFilePath = filePath;
    
    if (!actualFilePath.isEmpty() && !isUntitled) {
        if (this->checkIdenticalOpenedFile(actualFilePath) != -1) {
            return;
        }
        
        // Load file content
        QFile file(actualFilePath);
        tabName = QFileInfo(actualFilePath).fileName();
        
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            content = in.readAll();
            file.close();
        }
    } else {
        tabName = "untitled-" + QString::number(untitledCount++);
    }
    
    // Create a markdown view tab
    newTab = new MarkdownViewTab(content, actualFilePath, this);
    connect(static_cast<MarkdownViewTab*>(newTab), &MarkdownViewTab::showImageAt,
            this, &CustomTabWidget::showImageAt);
    connect(static_cast<MarkdownViewTab*>(newTab), &MarkdownViewTab::hideImage,
            this, &CustomTabWidget::hideImage);
    
    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeTabName, 
            this, &CustomTabWidget::updateTabTitle);
    connect(static_cast<BaseTextEditTab*>(newTab), &BaseTextEditTab::onChangeFileType,
        this, &CustomTabWidget::updateFileType);
    
    if (tabIndex == -1) {
        int newIndex = addTab(newTab, tabName);
    } else {
        int newIndex = insertTab(tabIndex, newTab, tabName);
    }
    
    setCurrentWidget(newTab);
}

void CustomTabWidget::closeWindowIfNoTabs(int index) {
    onTabCloseRequested(index);
    if (count() == 0) {
        emit lastTabClosed();
    }
}

void CustomTabWidget::showImageAt(const QString &imagePath, QPoint lastMousePos) {
    emit showImageAtSignal(imagePath, lastMousePos);
}

void CustomTabWidget::hideImage() {
    emit hideImageSignal();
}

void CustomTabWidget::showWikiAt(const QString &imagePath, QPoint lastMousePos) {
    emit showWikiAtSignal(imagePath, lastMousePos);
}

void CustomTabWidget::hideWiki() {
    emit hideWikiSignal();
}

void CustomTabWidget::switchToFictionView() {
    QWidget *current = currentWidget();
    if (!current) return;

    // Find the FictionTextEdit in the current tab
    FictionTextEdit *textEdit = current->findChild<FictionTextEdit *>();
    if (textEdit) {
        // Apply desired styles or actions

    }
}

void CustomTabWidget::updateTabTitle(const QString &fileName) {
    int currentIndex = this->currentIndex();
    QString currentTitle = this->tabText(currentIndex);

    QString newTitle;
    // dont remember why but it works
    if (currentTitle.endsWith("*") and fileName.endsWith("*")) {
        return;
    } else if (fileName == "*") {
        newTitle = currentTitle + fileName;
    } else {
        newTitle = fileName;
    }
    this->setTabText(currentIndex, newTitle);

    emit updatedTabTitleSignal(currentIndex, newTitle);
}

void CustomTabWidget::updateFileType(const QString &newFileName) {
    int currentIndex = this->currentIndex();
    QString currentTitle = this->tabText(currentIndex);

    QWidget *currentTab = nullptr;
    currentTab = this->widget(currentIndex);

    QRegularExpression regex("untitled-\\d+\\*");

    QString savedFilePath = static_cast<BaseTextEditTab*>(currentTab)->getCurrentFilePath();
    onTabCloseRequested(currentIndex, false);

    createNewTab(savedFilePath, false);
}

void CustomTabWidget::onTabCloseRequested(int index, bool needAsking) {
    QString currentTitle = this->tabText(index);
    if (currentTitle.endsWith("*") & needAsking) {
        
        // QRect tabBarRect = this->customTabBar->geometry();

        SaveMessageBox msgBox(this);
        int ret = msgBox.exec();

        QWidget *newTab = nullptr;
        QString title;
        QRegularExpression regex("untitled-\\d+\\*");
        bool isSuccessful;

        if (ret == QDialog::Accepted) {
            SaveMessageBox::ButtonResult result = msgBox.getResult();
            switch (result) {
                case SaveMessageBox::Save:
                    // Save the document
                    newTab = this->widget(index);
                    title = this->tabText(index);

                    isSuccessful = static_cast<BaseTextEditTab*>(newTab)->saveContent();
                    if (isSuccessful) {
                        removeTab(index);
                        emit tabClosedFromSyncedTabWidgetSignal(index);
                    }
                    break;
                case SaveMessageBox::Discard:
                    // Discard changes and close the tab
                    removeTab(index);
                    emit tabClosedFromSyncedTabWidgetSignal(index);
                    break;
                case SaveMessageBox::Cancel:
                    // Cancel the close operation
                    // Do nothing to cancel the close operation
                    break;
            }
        } else {
            // Dialog was rejected (should be treated as cancel)
        }
    } else {
        removeTab(index);
        emit tabClosedFromSyncedTabWidgetSignal(index);
    }
}

int CustomTabWidget::checkIdenticalOpenedFile(const QString &givenFilePath)
{
    QString title;
    QWidget *tab = nullptr;
    QString filePath;

    for (int i = 0; i < count(); ++i) {
        title = this->tabText(i);
        tab = this->widget(i);

        filePath = static_cast<BaseTextEditTab*>(tab)->getCurrentFilePath();

        if (filePath == givenFilePath) {
            return i;
        }
    }

    return -1;
}

void CustomTabWidget::handleFileDeleted(const QString &deletedFilePath)
{
    int tabIndex = checkIdenticalOpenedFile(deletedFilePath);

    if (tabIndex != -1) {
        removeTab(tabIndex);
        emit tabClosedFromSyncedTabWidgetSignal(tabIndex);
    }
}

void CustomTabWidget::handleFileRenamed(const QString &originalFilePath, const QString &newFilePath)
{
    int tabIndex = checkIdenticalOpenedFile(originalFilePath);
    if (tabIndex != -1) {

        removeTab(tabIndex);
        emit tabClosedFromSyncedTabWidgetSignal(tabIndex);

        createNewTab(newFilePath, false, tabIndex);
    }
}

void CustomTabWidget::activatePrisonerModeFunc(int timeLimit, int wordGoal) {
    emit activatePrisonerModeSignal(timeLimit, wordGoal);
}

void CustomTabWidget::deactivatePrisonerModeFunc() {
    emit deactivatePrisonerModeSignal();
}

