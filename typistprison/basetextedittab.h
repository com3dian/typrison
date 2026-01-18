#ifndef BASETEXTEDITTAB_H
#define BASETEXTEDITTAB_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>

class BaseTextEditTab : public QWidget {
    Q_OBJECT

public:
    BaseTextEditTab(const QString &content, const QString &filePath, QWidget *parent = nullptr);
    virtual ~BaseTextEditTab();
    QString getCurrentFilePath() const;
    void setFilePath(const QString &path);

protected:
    QString currentFilePath;
    QString content;
    virtual QString getTextContent() const = 0;
    // virtual QString setTextContent(const QString &text) = 0;

public slots:
    virtual bool saveContent();

signals:
    void onChangeTabName(const QString &fileName);
    void onChangeFileType(const QString &path);
};

#endif // BASETEXTEDITTAB_H