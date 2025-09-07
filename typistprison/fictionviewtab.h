#ifndef FICTIONVIEWTAB_H
#define FICTIONVIEWTAB_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollBar>
#include <QString>
#include <QFile>
#include <QPushButton>
#include <QLineEdit>
#include <QMargins>
#include <QIcon>
#include <QPixmap>
#include <QLabel>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

#include "fictiontextedit.h"
#include "searchWidget.h"
#include "projectmanager.h"
#include "prisonermanager.h"
#include "basetextedittab.h"
#include "utils/hoverbutton.h"
#include "popups/prisonerdialog.h"


class FictionViewTab : public BaseTextEditTab {
    Q_OBJECT

public:
    explicit FictionViewTab(const QString &content,
                            const QString &filePath,
                            QWidget *parent = nullptr,
                            bool isPrisoner = false,
                            ProjectManager *projectManager = nullptr,
                            PrisonerManager *prisonerManager = nullptr);
    QString getTextContent() const override;
    HoverButton *prisonerButton;

private:
    FictionTextEdit *textEdit;
    QScrollBar *vScrollBar;
    HoverButton *sniperButton;
    QHBoxLayout *globalLayout;
    QVBoxLayout *leftLayout;
    QHBoxLayout *topLeftLayout;
    QHBoxLayout *bottomLeftLayout;
    QLabel *wordCountLabel;
    // Add to private member variables
    QWidget *wordCountSpacerRight;  // Change from QSpacerItem* to QWidget*
    QString oldTextContent;
    bool isPrisoner;
    ProjectManager *projectManager;
    PrisonerManager *prisonerManager;

    void setupTextEdit(const QString &content);
    void setupScrollBar();
    void syncScrollBar();
    void activateSniperMode();
    void deactivateSniperMode();
    void editContent();
    bool saveContent() override;
    void updateWordcount();
    int getBaseWordCount();
    void activatePrisonerMode();
    void deactivatePrisonerMode();
    void showWikiFunc(const QString &wikiContent, QPoint lastMousePos);
    void hideWikiFunc();

signals:
    void showWikiAt(const QString &wikiContent, QPoint lastMousePos);
    void hideWiki();
    void activatePrisonerModeSignal(int timeLimit, int wordGoal);
    void deactivatePrisonerModeSignal();
    void tyingProgressChange(qreal progress);

// In the protected or public section of your class declaration:
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // FICTIONVIEWTAB_H
