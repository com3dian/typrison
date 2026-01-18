#ifndef FOLDERTREEVIEWWIDGET_H
#define FOLDERTREEVIEWWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QCursor>
#include "projectmanager.h"
#include "utils/fadeanimationutil.h"

class FolderTreeViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit FolderTreeViewWidget(QWidget *parent = nullptr, const QString folderRoot = "", ProjectManager *projectManager = nullptr);  // Constructor
    void toggleFileTreeView();
    void refresh(const QString &newFolderRoot);
    void handleContextMenuAction(const QString &action, const QModelIndex &index, bool isDir);

signals:
    void showContextMenuInMainWindow(const QStringList &options, const QModelIndex &index, bool isDir);
    void doubleClickedOnFile(const QString &filePath);
    void fileDeleted(const QString &filePath);
    void fileRenamed(const QString &originalFilePath, const QString &newFilePath);

private:
    void setupFileTree();           // Function to set up the file tree
    void setupButton();
    void onItemClicked(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &point);
    void addFile(const QString &targetDir = QString());
    void addFolder(const QString &targetDir = QString());
    void onDoubleClicked(const QModelIndex &index);

    QFileSystemModel *fileModel;    // Pointer to the file system model
    QTreeView *fileTreeView;        // Pointer to the QTreeView
    QWidget *buttonWidget;          // Add this member variable to hold the button widget
    QVBoxLayout *layout;            // Pointer to the QVBoxLayout
    qreal scalingFactor;            // Correctly call devicePixelRatio()
    QString folderRoot;       // project root
    QWidget* overlayWidget;
    ProjectManager *projectManager; // Pointer to the project manager
    
protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};

#endif // FOLDERTREEVIEWWIDGET_H
