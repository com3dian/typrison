#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QCursor>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QPushButton>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QScreen>
#include <QPoint>
#include <QMessageBox>
#include <QStackedWidget>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>
#include <QTimer>
#include <QNetworkAccessManager>

#include "customtabwidget.h"
#include "utils/fadeanimationutil.h"
#include "ui_mainwindow.h"
#include "customstyle.h"
#include "foldertreeviewwidget.h"
#include "functionbar.h"
#include "projectmanager.h"
#include "basetextedittab.h"
#include "searchWidget.h"
#include "fictionviewtab.h"
#include "markdownviewtab.h"
#include "plaintextviewtab.h"
#include "popups/imageframe.h"
#include "popups/wikiframe.h"
#include "functionbar/menubutton.h"
#include "qmarkdowntextedit.h"
#include "progressborderwidget.h"
#include "prisonermanager.h"
#include "countdowntimerwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

static constexpr int RESIZE_MARGIN = 8;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Enum for tab types
    enum TabType {
        FICTION_TAB,
        PLAINTEXT_TAB,
        MARKDOWN_TAB
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void openFile(const QString &filePath = ""); // Declaration of the openFile slot
    void saveFile();
    void searchFile();
    void openProject(const QString &path = QString()); // Updated openProject declaration
    void saveProject();
    void showMarkdownImage(const QString &imagePath, QPoint pos);
    void hideMarkdownImage();
    void showWiki(const QString &wikiContent, QPoint lastMousePos);
    void hideWiki();
    void openFileTreeView();
    void closeFileTreeView();
    void createAndOpenProject();
private slots:
    void showContextMenu(const QStringList &options, const QModelIndex &index, bool isDir);
    void handleContextMenuSelection(const QString &action);
    void activatePrisonerModeFunc(int timeLimit, int wordGoal);
    void deactivatePrisonerModeFunc();
private:
    Ui::MainWindow *ui;
    int untitledCount;
    CustomTabWidget *customTabWidget;
    FolderTreeViewWidget *folderTreeView;
    QSplitter *centralSplitter;
    float previousSplitterPosition;
    ProjectManager *projectManager;
    PrisonerManager *prisonerManager;
    QPushButton *sidePanelButton;
    QPushButton *button1;
    QPushButton *button2;
    QPushButton *menuToggleButton;
    QPushButton *currentMenuButton = nullptr;  // Track which button's menu is showing
    QFrame *existingFrame = nullptr;
    QFrame *subMenuFrame = nullptr;  // New submenu frame for hover functionality
    FunctionBar *functionBar;
    ImageFrame *imageFrame;
    QLabel *imageLabel;
    WikiFrame *wikiFrame;
    QFrame *contextMenuFrame;
    QModelIndex currentContextMenuIndex;
    bool isContextMenuForDir;
    ProgressBorderWidget *splitterContainer;
    QVBoxLayout *splitterLayout;
    QWidget *splitterTopWidget;
    QNetworkAccessManager *networkManager;
    CountdownTimerWidget *countdownTimerWidget;
    
    // Member variables to store prisoner mode parameters for countdown
    int pendingTimeLimit;
    int pendingWordGoal;
    
    void setupUntitledTab(TabType tabType = FICTION_TAB);
    void setupMenuButtons(FunctionBar *tabBarWidget);
    void createNewTab(const QString &tabName);
    void onLastTabClosed();
    void toggleFileTreeView();
    void minimizeWindow();
    void maximizeWindow();
    void closeWindow();
    void resizeEvent(QResizeEvent *event) override;
    void adjustButtonPosition();
    void adjustCountdownTimerPosition();
    void handleMouseEnterMenuButton(QPushButton *button);
    void handleFocusLeaveMenuButton();
    void setupActions();
    void displayImage(const QPixmap &pixmap, QPoint lastMousePos, const QString &imagePath = "");

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    Qt::Edges edgeAt(const QPoint &pos) const;
    void updateCursor(const Qt::Edges &edges);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void mouseClick();

};
#endif // MAINWINDOW_H