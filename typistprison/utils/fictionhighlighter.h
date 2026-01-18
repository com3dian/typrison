#ifndef SEARCHHIGHLIGHTER_H
#define SEARCHHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class FictionHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit FictionHighlighter(QTextDocument *parent = nullptr);

    void setSearchString(const QString &searchString);
    QString getSearchString() const;

    void changeFontSize(int delta);

protected:
    void highlightBlock(const QString &text) override;

private:
    int globalFontSize;
    QString searchString;

    // Formats for highlighting and font sizes
    QTextCharFormat searchHighlightFormat;
    QTextCharFormat searchFirstlineHighlightFormat;
    QTextCharFormat firstLineFormat;
    QTextCharFormat otherLineFormat;
};

#endif // SEARCHHIGHLIGHTER_H
