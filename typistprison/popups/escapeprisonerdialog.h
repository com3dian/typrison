#ifndef ESCAPEPRISONERDIALOG_H
#define ESCAPEPRISONERDIALOG_H

#include <QDialog>
#include <QWidget>
#include "../utils/closebuttonwidget.h"
#include "../utils/hoverbutton.h"

class EscapePrisonerDialog : public QDialog {
    Q_OBJECT

public:
    enum ButtonResult {
        Stay,
        Escape,
        Cancel
    };

    explicit EscapePrisonerDialog(QWidget *parent = nullptr);
    int exec() override;
    ButtonResult getResult() const;

private slots:
    void onStayClicked();
    void onEscapeClicked();

private:
    void centerOnScreen();
    
    HoverButton *stayButton;
    HoverButton *escapeButton;
    ButtonResult result = Cancel;
};

#endif // ESCAPEPRISONERDIALOG_H
