#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchWidget(QWidget *parent = nullptr);

signals:
    void onSearch(const QString &text);
    void onSearchPrev(const QString &text);
    void onClear();

public slots:
    void handleSearch(const QString &text);
    void handleClear();
    void handleReSearch();
    void updateBottomLine();
    void loseAttention();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLineEdit *lineEdit;
    QPushButton *searchButton;
    QWidget *bottomLine;
    QHBoxLayout *hLayout;
    QVBoxLayout *vLayout;

    bool isHovered;
    bool isOnSearch;
    bool isSearchPrev;
    bool isSearchLatter;

    qreal scalingFactor;
};

#endif // SEARCHWIDGET_H
