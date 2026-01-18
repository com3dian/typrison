#ifndef PLAINTEXTVIEWTAB_H
#define PLAINTEXTVIEWTAB_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollBar>
#include <QPushButton>
#include <QLineEdit>
#include <QMargins>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

#include "plaintextedit.h"
#include "searchWidget.h"
#include "basetextedittab.h"

class PlaintextViewTab : public BaseTextEditTab {
    Q_OBJECT

public:
    explicit PlaintextViewTab(const QString &content, const QString &filePath, QWidget *parent = nullptr);
    QString getTextContent() const override;
    // bool saveContent();

private:
    PlaintextEdit *textEdit;
    QString currentFilePath;
    QScrollBar *vScrollBar;
    QHBoxLayout *globalLayout;
    QVBoxLayout *leftLayout;
    QHBoxLayout *topLeftLayout;
    QHBoxLayout *bottomLeftLayout;

    void setupTextEdit(const QString &content);
    void setupScrollBar();
    void syncScrollBar();
    void activateHighlightMode();
    void deactivateHighlightMode();
    void editContent();

};

#endif // PLIANTEXTVIEWTAB_H
