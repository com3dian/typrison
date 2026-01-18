#ifndef SAVEMESSAGEBOX_H
#define SAVEMESSAGEBOX_H

#include <QDialog>
#include <QWidget>
#include "../utils/closebuttonwidget.h"
#include "../utils/hoverbutton.h"

class SaveMessageBox : public QDialog {
    Q_OBJECT // Necessary macro for signals and slots in Qt

public:
    enum ButtonResult {
        Save,
        Discard,
        Cancel
    };

    explicit SaveMessageBox(QWidget *parent = nullptr);
    int exec() override;
    ButtonResult getResult() const;

private slots:
    void onSaveClicked();
    void onDiscardClicked();

private:
    void centerOnScreen();
    
    HoverButton *saveButton;
    HoverButton *discardButton;
    ButtonResult result = Cancel;
};

#endif // SAVEMESSAGEBOX_H