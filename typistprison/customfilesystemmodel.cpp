/*
file system model for file tree panel
*/

#include "customfilesystemmodel.h"

CustomFileSystemModel::CustomFileSystemModel(QObject *parent)
    : QFileSystemModel(parent)  // Call the base class constructor
{
    // Initialize folder icons
    folderIcon = QIcon(":/icons/folder.png");
    wikiIcon = QIcon(":/icons/wikifile_icon.png");
}

/*
override data method to show different file type with unique color

*/
QVariant CustomFileSystemModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::ForegroundRole) {
        return QColor("#BDBDBD");
    }
    else if (role == Qt::DecorationRole) {
        QString dirName = QFileSystemModel::fileName(index);
        // Check if it's a directory first
        if (isDir(index)) {
            if (dirName == "wiki") {
                return wikiIcon;
            } else {
                return folderIcon;
            }
        }
        // Then handle files
        QString fileName = QFileSystemModel::fileName(index);
        
        if (fileName.endsWith("cell.txt")) {
            return QIcon(":/icons/cell_icon.png");
        } else if (fileName.endsWith(".md")) {
            return QIcon(":/icons/md_icon.png");
        } else if (fileName.endsWith(".txt")) {
            bool allAsterisks = false;
            if (fileName.length() > 4) {
                QString firstHalf = fileName.left(fileName.length() - 4); // remove last 4 characters '.txt'
                allAsterisks = std::all_of(firstHalf.begin(), firstHalf.end(),
                                                [](QChar ch) { return ch == '*'; });
            }
            if (allAsterisks) {
                return QIcon(":/icons/*_icon.png");
            } else {
                return QIcon(":/icons/txt_icon.png");
            }
        } else {
            return QIcon(":/icons/file.png");  // Default icon for other file types
        }
    }
    
    return QFileSystemModel::data(index, role);
}

Qt::ItemFlags CustomFileSystemModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}