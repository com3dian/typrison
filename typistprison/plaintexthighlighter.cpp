#include "plaintexthighlighter.h"
#include <QDebug>

PlaintextHighlighter::PlaintextHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), searchString("")
{
    // Define the format for search highlighting
    searchHighlightFormat.setBackground(QBrush(QColor("#4F726C"))); // Custom background color
}

void PlaintextHighlighter::setSearchString(const QString &searchString) {
    this->searchString = searchString;
    qDebug() << "set search string";
    rehighlight(); // Trigger a rehighlight whenever the search string changes
}

QString PlaintextHighlighter::getSearchString() const {
    return searchString;
}

void PlaintextHighlighter::highlightBlock(const QString &text) {
    if (searchString.isEmpty()) {
        return;
    }

    QString escapedSearchString = QRegularExpression::escape(searchString);
    QRegularExpression expression(escapedSearchString, QRegularExpression::CaseInsensitiveOption);

    int offset = 0;
    while (offset < text.length()) {
        QRegularExpressionMatch match = expression.match(text, offset);
        if (match.hasMatch()) {
            int start = match.capturedStart();
            int length = match.capturedLength();
            setFormat(start, length, searchHighlightFormat); // Apply search highlight format
            offset = start + length; // Move offset to avoid overlapping matches
        } else {
            break;
        }
    }
}
