#ifndef FAILEDPRISONERDIALOG_H
#define FAILEDPRISONERDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QCloseEvent>
#include "../utils/closebuttonwidget.h"
#include "../utils/hoverbutton.h"

class FailedPrisonerDialog : public QDialog {
    Q_OBJECT

public:
    explicit FailedPrisonerDialog(QWidget *parent = nullptr);
    int exec() override;

signals:
    void dialogClosed();

private slots:
    void onAcknowledgeClicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void centerOnScreen();
    
    HoverButton *acknowledgeButton;
};

#endif // FAILEDPRISONERDIALOG_H
