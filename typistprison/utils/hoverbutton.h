#ifndef HOVERBUTTON_H
#define HOVERBUTTON_H

#include <QPushButton>
#include <QIcon>
#include <QEnterEvent>

class HoverButton : public QPushButton {
    Q_OBJECT
public:
    explicit HoverButton(const QString &text = "", QWidget *parent = nullptr)
        : QPushButton(text, parent)
    {
        // Set default values
        normalIcon = QIcon(":/icons/emptyicon.png");
        hoverIcon = QIcon(":/icons/emptyicon.png");
        
        // Default silent behavior
        silentStyleSheet = 
            "QPushButton {"
            "    padding: 2px 14px 2px 14px;"
            "    color: transparent;"
            "    background-color: transparent;"
            "    border: 1px solid transparent;"
            "    border-radius: 4px;"
            "    text-align: left;"
            "}";
            
        // Default hover behavior
        hoverStyleSheet = 
            "QPushButton {"
            "    padding: 2px 14px 2px 14px;"
            "    color: #FFFFFF;"
            "    background-color: transparent;"
            "    border: 1px solid #BDBDBD;"
            "    border-radius: 4px;"
            "    text-align: left;"
            "}";
        
        setIcon(normalIcon);
        setIconSize(QSize(16, 16));
        setStyleSheet(silentStyleSheet);
    }

    // Method to set silent (normal) behavior
    void setSilentBehavior(const QString &styleSheet, const QIcon &icon = QIcon()) {
        silentStyleSheet = styleSheet;
        if (!icon.isNull()) {
            normalIcon = icon;
        }
        // Apply silent behavior immediately if not hovering
        if (!isHovered) {
            setStyleSheet(silentStyleSheet);
            setIcon(normalIcon);
        }
    }

    // Method to set hover behavior
    void setHoverBehavior(const QString &styleSheet, const QIcon &icon = QIcon()) {
        hoverStyleSheet = styleSheet;
        if (!icon.isNull()) {
            hoverIcon = icon;
        }
        // Apply hover behavior immediately if currently hovering
        if (isHovered) {
            setStyleSheet(hoverStyleSheet);
            setIcon(hoverIcon);
        }
    }

    // Convenience method to set both icons at once
    void setIcons(const QIcon &normalIcon, const QIcon &hoverIcon) {
        this->normalIcon = normalIcon;
        this->hoverIcon = hoverIcon;
        if (!isHovered) {
            setIcon(this->normalIcon);
        } else {
            setIcon(this->hoverIcon);
        }
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        QPushButton::enterEvent(event);
        isHovered = true;
        setIcon(hoverIcon);
        setStyleSheet(hoverStyleSheet);
    }

    void leaveEvent(QEvent *event) override {
        QPushButton::leaveEvent(event);
        isHovered = false;
        setIcon(normalIcon);
        setStyleSheet(silentStyleSheet);
    }

private:
    QIcon normalIcon;
    QIcon hoverIcon;
    QString silentStyleSheet;
    QString hoverStyleSheet;
    bool isHovered = false;
};

#endif // HOVERBUTTON_H