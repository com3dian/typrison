#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H

#include <QTabWidget>
#include <QTextEdit>
#include <QTabBar>
#include <QStylePainter>
#include <QPainterPath>
#include <QStyleOptionTab>
#include <QHBoxLayout>
#include <QPoint>

#include "projectmanager.h"
#include "prisonermanager.h"


class CustomTabWidget : public QTabWidget {
    Q_OBJECT

public:
    CustomTabWidget(QWidget *parent = nullptr, ProjectManager *projectManager = nullptr, PrisonerManager *prisonerManager = nullptr);

    void createNewTab(const QString &filePath, bool isUntitled = false, int tabIndex = -1);
    void createFictionTab(const QString &filePath = "", bool isUntitled = true, int tabIndex = -1);
    void createPlainTextTab(const QString &filePath = "", bool isUntitled = true, int tabIndex = -1);
    void createMarkdownTab(const QString &filePath = "", bool isUntitled = true, int tabIndex = -1);
    void switchToFictionView();

public slots:
    void updateTabTitle(const QString &fileName);
    void updateFileType(const QString &previousFileName);
    void closeWindowIfNoTabs(int index);
    void onTabCloseRequested(int index, bool needAsking = true);
    void handleFileDeleted(const QString &deletedFilePath);
    void handleFileRenamed(const QString &originalFilePath, const QString &newFilePath);
    void showImageAt(const QString &imagePath, QPoint lastMousePos);
    void hideImage();
    void showWikiAt(const QString &wikiContent, QPoint lastMousePos);
    void hideWiki();
    void activatePrisonerModeFunc(int timeLimit, int wordGoal);
    void deactivatePrisonerModeFunc();

private:
    int untitledCount;
    ProjectManager *projectManager;
    PrisonerManager *prisonerManager;

    void setupTabWidget();
    void setupTabBar();
    void setupStyles();
    void applyFictionViewStyles(QTextEdit *textEdit);
    void applyEditorViewStyles(QTextEdit *textEdit);
    int checkIdenticalOpenedFile(const QString &givenFilePath);

private slots:
    
    
protected:
    void tabInserted(int index) override;

signals:
    void lastTabClosed();
    void tabInsertedSignal(int index, const QString &label);
    void tabClosedFromSyncedTabWidgetSignal(int index);
    void updatedTabTitleSignal(int currentIndex, QString newTitle);
    void tabActivatedSignal(int index);
    void showImageAtSignal(const QString &imagePath, QPoint lastMousePos);
    void hideImageSignal();
    void showWikiAtSignal(const QString &imagePath, QPoint lastMousePos);
    void hideWikiSignal();
    void activatePrisonerModeSignal(int timeLimit, int wordGoal);
    void deactivatePrisonerModeSignal();

};

#endif // CUSTOMTABWIDGET_H
