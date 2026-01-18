#ifndef CUSTOMTREESTYLE_H
#define CUSTOMTREESTYLE_H

#include <QProxyStyle>
#include <QPixmap>

class CustomTreeStyle : public QProxyStyle {
    Q_OBJECT

public:
    CustomTreeStyle(const QString& closedIconPath, const QString& openIconPath);

    // Overriding the drawPrimitive function to customize the branch indicator
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;

private:
    QString closedIconPath;  // Path to the closed folder icon
    QString openIconPath;    // Path to the open folder icon
};

#endif // CUSTOMTREESTYLE_H