#ifndef CUSTOMFILESYSTEMMODEL_H
#define CUSTOMFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QIcon>
#include <QTreeView>

class CustomFileSystemModel : public QFileSystemModel {
    Q_OBJECT
public:
    explicit CustomFileSystemModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    QIcon folderIcon;
    QIcon wikiIcon;
};

#endif // CUSTOMFILESYSTEMMODEL_H
