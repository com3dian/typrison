#ifndef PLAINTEXTHIGHLIGHTER_H
#define PLAINTEXTHIGHLIGHTER_H
#include <QTextBlock>
#include <QBrush>
#include <QColor>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class PlaintextHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit PlaintextHighlighter(QTextDocument *parent = nullptr);

    void setSearchString(const QString &searchString);
    QString getSearchString() const;

protected:
    void highlightBlock(const QString &text) override;

private:
    QString searchString;
    QTextCharFormat searchHighlightFormat;
};

#endif // PLAINTEXTHIGHLIGHTER_H
