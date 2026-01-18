#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "utils/ahocorasick.h"
#include <QDir>
#include <QFile>
#include <QObject>
#include <QTimer>
#include <unordered_set>

class ProjectManager : public QObject {
    Q_OBJECT  // Required macro for QObject subclasses

public:
    // Constructor
    explicit ProjectManager(QObject* parent = nullptr);

    // Member Functions
    void open(const QString selectedProjectRoot);
    QString matchBannedWords(QString text);
    int getMaxiumBannedWordLength();
    void parseMarkdownContent(const QString& content, const QString& filePath);
    void printWikiContent(); // New method to print wiki content
    QMap<QString, QList<QPair<int, QString>>> matchWikiContent(const QString& keyword);
    QString getContentByKeys(QList<QString> matchedWikiKeys);

    // Variables
    bool haveBannedWordsFile;
    bool haveWiki;
    bool isLoadedProject;
    
private:
    // Member Variables
    std::vector<std::string> bannedWordsLines;
    AhoCorasick bannedWordsTrie;
    AhoCorasick wikiTrie;
    int maxiumBannedWordLength;
    QTimer* bannedWordsTimer;
    QString currentProjectRoot;
    QMap<QString, QString> wikiContentMap; // Key: filePath::title, Value: content

    void readBannedWords(const QString selectedProjectRoot);
    void readWikiFiles(const QString& selectedProjectRoot);
    
private slots:
    void checkBannedWordsChanges();
};

#endif // PROJECTMANAGER_H
