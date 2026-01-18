#include "fictionhighlighter.h"
#include <QRegularExpression>
#include <QTextBlock>
#include <QBrush>
#include <QColor>

FictionHighlighter::FictionHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), searchString(""), globalFontSize(14)
{
    // Define the format for search highlighting
    searchHighlightFormat.setBackground(QBrush(QColor("#4F726C"))); // Custom background color

    searchFirstlineHighlightFormat.setBackground(QBrush(QColor("#4F726C")));

    // Define the format for the first line
    firstLineFormat.setFontPointSize(1.6 * globalFontSize); // Larger font size for the first line
    otherLineFormat.setFontPointSize(globalFontSize);
    searchFirstlineHighlightFormat.setFontPointSize(1.6 * globalFontSize); // Larger font size for the first line
}

/* Change font size 
 */
void FictionHighlighter::changeFontSize(int delta)
{   
    // define global font size and 
    // Prevent font size from becoming <= 0
    globalFontSize += delta;
    if (globalFontSize < 1) {
        globalFontSize = 1; // Prevent font size from becoming too small
    }

    // Define the format for the first line & other lines
    firstLineFormat.setFontPointSize(1.6 * globalFontSize);
    otherLineFormat.setFontPointSize(globalFontSize);

    searchFirstlineHighlightFormat.setBackground(QBrush(QColor("#4F726C")));
    searchFirstlineHighlightFormat.setFontPointSize(1.6 * globalFontSize); // Larger font size for the first line

    rehighlight();
}

void FictionHighlighter::setSearchString(const QString &searchString) {
    if (this->searchString == searchString) {
        return;
    }
    this->searchString = searchString;

    rehighlight(); // Trigger a rehighlight whenever the search string changes
}

QString FictionHighlighter::getSearchString() const {
    return searchString;
}

void FictionHighlighter::highlightBlock(const QString &text) {
    QTextBlock block = currentBlock();
    int lineNumber = block.blockNumber();

    // Determine the format based on the line number
    if (lineNumber == 0) {
        // Apply the format only for the first line
        setFormat(0, text.length(), firstLineFormat);
    } else if (lineNumber == 1) {
        setFormat(0, text.length(), otherLineFormat);
    }

    if (searchString.isEmpty()) {
        return;
    }

    // Prepare the regular expression for case-insensitive search
    QString escapedSearchString = QRegularExpression::escape(searchString);
    QRegularExpression expression(escapedSearchString, QRegularExpression::CaseInsensitiveOption);

    int offset = 0;
    while (offset < text.length()) {
        QRegularExpressionMatch match = expression.match(text, offset);
        if (match.hasMatch()) {
            int start = match.capturedStart();
            int length = match.capturedLength();
            if (lineNumber == 0) {
                setFormat(start, length, searchFirstlineHighlightFormat); // Apply search highlight format
            } else {
                setFormat(start, length, searchHighlightFormat); // Apply search highlight format
            }
            offset = start + length; // Move offset to avoid overlapping matches
        } else {
            break;
        }
    }
}
