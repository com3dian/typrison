#ifndef WIKIFRAME_H
#define WIKIFRAME_H

#include <QFrame>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QDebug>
#include "../qmarkdowntextedit.h"

class WikiFrame : public QFrame {
    Q_OBJECT

private:
    QMarkdownTextEdit* editor;
    QVBoxLayout* layout;

public:
    explicit WikiFrame(QWidget *parent = nullptr);
    void setContent(const QString& content);

protected:
    void showEvent(QShowEvent* event) override;
};

#endif // WIKIFRAME_H