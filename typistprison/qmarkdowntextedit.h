/*
 * MIT License
 *
 * Copyright (c) 2014-2024 Patrizio Bekerle -- <patrizio@bekerle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <QEvent>
#include <QPlainTextEdit>

#include "markdownhighlighter.h"
#include "fontmanager.h"

class QMarkdownTextEdit : public QPlainTextEdit {
    Q_OBJECT
    Q_PROPERTY(
        bool highlighting READ highlightingEnabled WRITE setHighlightingEnabled)


public:
    enum AutoTextOption {
        None = 0x0000,

        // inserts closing characters for brackets and Markdown characters
        BracketClosing = 0x0001,

        // removes matching brackets and Markdown characters
        BracketRemoval = 0x0002
    };

    Q_DECLARE_FLAGS(AutoTextOptions, AutoTextOption)

    explicit QMarkdownTextEdit(QWidget *parent = nullptr,
                               bool initHighlighter = true);
    MarkdownHighlighter *highlighter();
    void setIgnoredClickUrlSchemata(QStringList ignoredUrlSchemata);
    virtual void openUrl(const QString &urlString);
    QString getMarkdownUrlAtPosition(const QString &text, int position);
    void setAutoTextOptions(AutoTextOptions options);
    static bool isValidUrl(const QString &urlString);
    void resetMouseCursor() const;
    void setReadOnly(bool ro);

    void setHighlightingEnabled(bool enabled);
    [[nodiscard]] bool highlightingEnabled() const;

    // extended for typistprison
    void load(const QString& text);
    void changeFontSize(int delta);
    void search(const QString &searchString);
    void searchPrev(const QString &searchString);
    void clearSearch();

public Q_SLOTS:
    void duplicateText();
    void setText(const QString &text);
    void setPlainText(const QString &text);
    void adjustRightMargin();
    void hide();
    bool openLinkAtCursorPosition();
    bool handleBackspaceEntered();
    void centerTheCursor();
    void undo();
    void moveTextUpDown(bool up);

protected:
    QTextCursor _textCursor;
    MarkdownHighlighter *_highlighter = nullptr;
    bool _highlightingEnabled;
    QStringList _ignoredClickUrlSchemata;
    AutoTextOptions _autoTextOptions;
    bool _mouseButtonDown = false;
    bool _centerCursor = false;
    bool _highlightCurrentLine = false;
    QColor _currentLineHighlightColor = QColor();
    uint _debounceDelay = 0;

    bool eventFilter(QObject *obj, QEvent *event) override;
    QMargins viewportMargins();
    bool increaseSelectedTextIndention(
        bool reverse,
        const QString &indentCharacters = QChar::fromLatin1('\t'));
    bool handleTabEntered(bool reverse, const QString &indentCharacters =
                                            QChar::fromLatin1('\t'));
    QMap<QString, QString> parseMarkdownUrlsFromText(const QString &text);
    bool handleReturnEntered();
    bool handleBracketClosing(const QChar openingCharacter,
                              QChar closingCharacter = QChar());
    bool bracketClosingCheck(const QChar openingCharacter,
                             QChar closingCharacter);
    bool quotationMarkCheck(const QChar quotationCharacter);
    void focusOutEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    bool handleCharRemoval(MarkdownHighlighter::RangeType type, int block, int position);
    void resizeEvent(QResizeEvent *event) override;
    bool _handleBracketClosingUsed;

    // extended typrison
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;


Q_SIGNALS:
    void urlClicked(QString url);
    void zoomIn();
    void zoomOut();

    // extended for typist prison
    void onMarkdownSearch(const QString &text);
    void focusGained();
    void onSave();
    void showImageAt(const QString &imagePath, QPoint lastMousePos);
    void hideImage();

private:
    int globalFontSize;
    int matchStringIndex;

    QTimer *timer;
    QPoint lastMousePos;

    void readBlock();

private slots:
    void showContextMenu(const QPoint &pos);
};
