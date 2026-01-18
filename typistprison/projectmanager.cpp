#include "projectmanager.h"
#include <QDebug>
#include <QTimer>
#include <QDirIterator>
#include <QRegularExpression>

/*
This class manage the project(folder) of typrison, including:
- banned words
- wiki folder or file

TODO: have a easy method to handle when banned words file is changed
and wiki folder or file is changed.
*/

// Constructor
ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent)
    , isLoadedProject(false)
    , haveBannedWordsFile(false)
    , maxiumBannedWordLength(0)
    , bannedWordsTimer(new QTimer(this))
    , currentProjectRoot("")
{
    // Set up timer to check for banned words file changes
    connect(bannedWordsTimer, &QTimer::timeout, this, &ProjectManager::checkBannedWordsChanges);
}

void ProjectManager::open(const QString selectedProjectRoot) {
    currentProjectRoot = selectedProjectRoot;
    readBannedWords(selectedProjectRoot);
    readWikiFiles(selectedProjectRoot);

    isLoadedProject = true;
    
    // Start the timer to check for banned words changes every second
    bannedWordsTimer->start(1000);

    return;
}

void ProjectManager::readBannedWords(const QString selectedProjectRoot) {
    maxiumBannedWordLength = 0;

    // Step 1: Locate all .txt files
    QDir directory(selectedProjectRoot);
    QStringList txtFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

    // Step 2: Identify the file with the longest sequence of '*'
    QString bestFile;
    int maxStars = 0;

    for (const QString& fileName : txtFiles) {
        bool allAsterisks = false;
        if (fileName.length() > 4) {
            QString firstHalf = fileName.left(fileName.length() - 4); // remove last 4 characters
            allAsterisks = std::all_of(firstHalf.begin(), firstHalf.end(),
                                            [](QChar ch) { return ch == '*'; });
        }
        
        if (allAsterisks) {
            int starCount = fileName.count('*');
            if (starCount > maxStars) {
                maxStars = starCount;
                bestFile = fileName;
            }
        }
    }

    // Step 3: Read the contents of the selected file
    if (!bestFile.isEmpty()) {
        QFile file(directory.filePath(bestFile));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open file:" << bestFile;
            return;
        }

        QTextStream in(&file);
        std::vector<std::string> lines;
        std::unordered_set<std::string> uniqueLines;
        while (!in.atEnd()) {
            QString line = in.readLine();
            std::string lineStr = line.toStdString();

            // Check if the line is already present
            if (uniqueLines.find(lineStr) == uniqueLines.end()) {
                lines.push_back(lineStr);
                uniqueLines.insert(lineStr); // Mark as added
            }
        }
        file.close();

        bannedWordsLines = lines;

        QString bannedWordsQstringList;
        for (const auto& str : bannedWordsLines) {
            bannedWordsQstringList.append(QString::fromUtf8(str.c_str()));
            QString banneWordQSR = QString::fromUtf8(str.c_str());
            maxiumBannedWordLength = std::max(static_cast<int>(banneWordQSR.length()), maxiumBannedWordLength);
        }

        AhoCorasick trie;
        for (size_t i = 0; i < lines.size(); ++i) {
            trie.insert(lines[i], i);
        }
        trie.buildFailureLinks();

        bannedWordsTrie = trie;
        haveBannedWordsFile = true;
    } else {
        haveBannedWordsFile = false;
    }
}

int ProjectManager::getMaxiumBannedWordLength() {
    return maxiumBannedWordLength;
}

QString ProjectManager::matchBannedWords(QString text) {
    // If no banned words file is found, return original text
    if (!haveBannedWordsFile) {
        return text;
    }

    std::string stdStringText = text.toStdString();
    std::vector<std::pair<int, int>> matches = bannedWordsTrie.search(stdStringText);

    int backspace = 0;

    for (const auto& match : matches) {
        int patternIndex = match.first;
        int utf8BannedWordLength = bannedWordsLines[patternIndex].size();
        int end = match.second - backspace;
        int start = end - utf8BannedWordLength + 1;

        // Replace the range [start, end] with proper number of asterisks
        stdStringText.erase(start, utf8BannedWordLength);

        // get the length of banned word
        int actualBannedWordLength = QString::fromUtf8(bannedWordsLines[patternIndex].c_str()).length();
        std::string replacementAsterisks(actualBannedWordLength, '*');
        stdStringText.insert(start, replacementAsterisks);
        
        // update backspace
        backspace += utf8BannedWordLength - actualBannedWordLength;
    }
    QString filteredText = QString::fromStdString(stdStringText); 

    return filteredText;
}

void ProjectManager::checkBannedWordsChanges() {
    if (!isLoadedProject || currentProjectRoot.isEmpty()) {
        return;
    }

    // Step 1: Locate all .txt files
    QDir directory(currentProjectRoot);
    QStringList txtFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

    // Step 2: Identify the file with the longest sequence of '*'
    QString bestFile;
    int maxStars = 0;

    for (const QString& fileName : txtFiles) {
        int starCount = fileName.count('*');
        if (starCount > maxStars) {
            maxStars = starCount;
            bestFile = fileName;
        }
    }

    // If no banned words file found, return
    if (bestFile.isEmpty()) {
        if (haveBannedWordsFile) {
            // We had a banned words file before, but now it's gone
            haveBannedWordsFile = false;
            bannedWordsLines.clear();
            // Create a new empty trie
            AhoCorasick trie;
            bannedWordsTrie = trie;
            maxiumBannedWordLength = 0;
        }
        return;
    }

    // Read the current contents of the file
    QFile file(directory.filePath(bestFile));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for checking changes:" << bestFile;
        return;
    }

    QTextStream in(&file);
    std::vector<std::string> currentLines;
    while (!in.atEnd()) {
        QString line = in.readLine();
        currentLines.push_back(line.toStdString());
    }
    file.close();

    // If this is the first time or the file has changed, update
    if (!haveBannedWordsFile || currentLines != bannedWordsLines) {
        
        // Find removed lines
        std::vector<std::string> removedLines;
        for (const auto& oldLine : bannedWordsLines) {
            if (std::find(currentLines.begin(), currentLines.end(), oldLine) == currentLines.end()) {
                removedLines.push_back(oldLine);
            }
        }
        
        // Find new lines
        std::vector<std::string> newLines;
        for (const auto& newLine : currentLines) {
            if (std::find(bannedWordsLines.begin(), bannedWordsLines.end(), newLine) == bannedWordsLines.end()) {
                newLines.push_back(newLine);
            }
        }
        
        // Remove words that are no longer in the file
        if (!removedLines.empty()) {
            bannedWordsTrie.removeMultiple(removedLines);
        }
        
        // Add new words to the trie
        for (size_t i = 0; i < newLines.size(); ++i) {
            // Find the index in the current lines
            auto it = std::find(currentLines.begin(), currentLines.end(), newLines[i]);
            if (it != currentLines.end()) {
                int index = std::distance(currentLines.begin(), it);
                bannedWordsTrie.insert(newLines[i], index);
            }
        }
        
        // If there were any changes, rebuild failure links
        if (!removedLines.empty() || !newLines.empty()) {
            bannedWordsTrie.buildFailureLinks();
        }
        
        // Update the stored lines
        bannedWordsLines = currentLines;
        
        // Recalculate maximum banned word length
        maxiumBannedWordLength = 0;
        for (const auto& str : bannedWordsLines) {
            QString banneWordQSR = QString::fromUtf8(str.c_str());
            maxiumBannedWordLength = std::max(static_cast<int>(banneWordQSR.length()), maxiumBannedWordLength);
        }
        
        haveBannedWordsFile = true;
    }
}


void ProjectManager::readWikiFiles(const QString& selectedProjectRoot) {
    QDir projectDir(selectedProjectRoot);
    QDir wikiDir(projectDir.filePath("wiki"));

    QStringList mdFiles;
    wikiContentMap.clear(); // Clear previous content
    // wikiTrie.clear();


    // Check if the "wiki" directory exists
    if (wikiDir.exists()) {
        // Use entryInfoList with QDir::AllEntries to get both files and directories
        // Use QDir::NoDotAndDotDot to exclude "." and ".." entries
        wikiDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        wikiDir.setNameFilters(QStringList() << "*.md");
        QFileInfoList entries = wikiDir.entryInfoList();
        
        // First, add all .md files in the current directory
        for (const QFileInfo& entry : entries) {
            if (entry.isFile()) {
                mdFiles.append(entry.filePath());
            }
        }
        
        // Then recursively search subdirectories
        QDirIterator it(wikiDir.path(), QStringList() << "*.md", QDir::Files, 
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            // Skip files already added from the root directory
            if (!mdFiles.contains(filePath)) {
                mdFiles.append(filePath);
            }
        }
    }

    // If no .md files in "wiki", check for wiki.md in root
    if (mdFiles.isEmpty()) {
        QFile wikiFile(projectDir.filePath("wiki.md"));
        if (wikiFile.exists()) {
            mdFiles.append(projectDir.filePath("wiki.md"));
        }
    }

    // If still no .md files, do nothing
    if (mdFiles.isEmpty()) {
        haveWiki = false;
        return;
    }

    haveWiki = true;

    // Process each markdown file
    for (const QString& mdFile : mdFiles) {
        QFile file(mdFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            
            // Parse the markdown content to extract titles and sections
            parseMarkdownContent(content, mdFile);
            
        } else {
            qWarning() << "Could not open file:" << mdFile;
        }
    }
    
    // **Loop over `wikiContentMap` and insert section names into the Trie**
    int index = 0;
    for (const auto &section : wikiContentMap.keys()) {
        wikiTrie.insert(section.toStdString(), index);
        ++index;
    }

    haveWiki = !wikiContentMap.isEmpty();
}

void ProjectManager::parseMarkdownContent(const QString& content, const QString& filePath) {
    QStringList lines = content.split('\n');
    
    // Create vectors to store titles and content at different levels (0-5 for h1-h6)
    QVector<QString> currentTitles(6); // Array to store titles at different levels
    QVector<QString> currentContents(6); // Array to store content at different levels
    int newTitleLevel = 0;
    bool isCodeBlock = false; // Flag to check if current block is a code block
    
    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i];
        
        // Check if this line starts or ends a code block (```), toggle the flag
        if (line.trimmed().startsWith("```")) {
            isCodeBlock = !isCodeBlock;
            
            // Add this line to all active section contents
            for (int j = 0; j < 6; j++) {
                if (!currentTitles[j].isEmpty()) {
                    currentContents[j] += line + "\n";
                }
            }
            continue;
        }
        
        // If we're inside a code block, just add the line to all active sections
        // and don't try to match it as a heading
        if (isCodeBlock) {
            for (int j = 0; j < 6; j++) {
                if (!currentTitles[j].isEmpty()) {
                    currentContents[j] += line + "\n";
                }
            }
            continue;
        }
        
        // Check if this line is a title (starts with # to ######)
        QRegularExpression titleRegex("^(#{1,6})\\s+(.+)$");
        QRegularExpressionMatch match = titleRegex.match(line);
        
        if (match.hasMatch()) {
            // Extract the new title and its level
            QString hashMarks = match.captured(1);
            newTitleLevel = hashMarks.length() - 1;
            QString newTitle = match.captured(2).trimmed();
            
            // If we already have content at or higher than the current level, save it
            // equal or more hash marks
            for (int j = newTitleLevel; j < 6; j++) {
                if (!currentContents[j].isEmpty()) {
                    // Trim any trailing whitespace from the content
                    currentContents[j] = currentContents[j].trimmed();

                    // Store the title and content in the map
                    QString mapKey = currentTitles[j];
                    if (wikiContentMap.contains(mapKey)) {
                        // If key exists, append the new content
                        wikiContentMap[mapKey] = wikiContentMap[mapKey] + "\n" + currentContents[j];
                    } else {
                        // If key doesn't exist, create new entry
                        wikiContentMap[mapKey] = currentContents[j];
                    }
                    
                    // Clear the title and content for this level
                    currentTitles[j].clear();
                    currentContents[j].clear();
                }
            }

            for (int j = 0; j < newTitleLevel; j++) {
                if (!currentTitles[j].isEmpty()) {
                    currentContents[j] += line + "\n";
                }
            }
            
            // Update the current title level and set the new title
            currentTitles[newTitleLevel] = newTitle;
            
            // Clear any existing content at this level
            currentContents[newTitleLevel].clear();
            currentContents[newTitleLevel] += line + "\n";
        } else {
            // If we have a current title, add this line to its content
            for (int j = 0; j < 6; j++) {
                if (!currentTitles[j].isEmpty()) {
                    currentContents[j] += line + "\n";
                }
            }
        }
    }

    // Don't forget to add the last sections
    for (int j = 0; j < 6; j++) {
        if (!currentTitles[j].isEmpty()) {
            currentContents[j] = currentContents[j].trimmed();
            // QString mapKey = filePath + "::" + currentTitles[j];
            QString mapKey = currentTitles[j];

            if (wikiContentMap.contains(mapKey)) {
                // If key exists, append the new content
                wikiContentMap[mapKey] = wikiContentMap[mapKey] + "\n" + currentContents[j];
            } else {
                // If key doesn't exist, create new entry
                wikiContentMap[mapKey] = currentContents[j];
            }
        }
    }
}

// Add this method after readWikiFiles

void ProjectManager::printWikiContent() {
    if (!haveWiki || wikiContentMap.isEmpty()) {
        return;
    }
    
    qDebug() << "=== Wiki Content Debug ===";
    // Iterate through all keys in the map
    for (auto it = wikiContentMap.constBegin(); it != wikiContentMap.constEnd(); ++it) {
        // Print the key
        qDebug() << "Key:" << it.key();
        
        // Print a preview of the content (first 100 chars)
        QString contentPreview = it.value();
        if (contentPreview.length() > 100) {
            contentPreview = contentPreview.left(100) + "...";
        }
        qDebug() << "Content:" << contentPreview;
        qDebug() << "-------------------";
    }
    qDebug() << "=== End Wiki Debug ===";
}

QMap<QString, QList<QPair<int, QString>>> ProjectManager::matchWikiContent(const QString& text) {
    QMap<QString, QList<QPair<int, QString>>> matches;

    if (!haveWiki || wikiContentMap.isEmpty()) {
        return matches;
    }

    // Create a mapping from UTF-8 positions to QString positions
    QByteArray utf8Text = text.toUtf8();
    QVector<int> positionMap;
    positionMap.reserve(utf8Text.size() + 1);
    
    // Build the position mapping
    int qstringPos = 0;
    for (int i = 0; i < text.length(); ++i) {
        QByteArray charUtf8 = text.mid(i, 1).toUtf8();
        for (int j = 0; j < charUtf8.size(); ++j) {
            positionMap.append(i);
        }
        qstringPos++;
    }
    // Add one more for the end position
    positionMap.append(text.length());
    
    // Convert QString to std::string for AhoCorasick
    std::string stdStringText = text.toStdString();
    
    // Get matches from the trie
    std::vector<std::pair<int, int>> trieMatches = wikiTrie.search(stdStringText);
    
    // Process matches
    for (const auto& match : trieMatches) {
        int patternIndex = match.first;  // Index of the matched pattern
        int utf8Position = match.second;  // Position in the UTF-8 text where match ends
        
        // Map the UTF-8 position back to QString position
        int qstringPosition = (utf8Position < positionMap.size()) ? positionMap[utf8Position] : text.length() - 1;
        
        // Get the wiki content key from the pattern index
        QString wikiKey = wikiContentMap.keys()[patternIndex];
        
        // Get the matched text
        QString matchedText = wikiContentMap.keys()[patternIndex];
        int matchLength = matchedText.length();
        int startPos = qstringPosition - matchLength + 1;
        
        // Ensure startPos is not negative
        startPos = qMax(0, startPos);
        
        // If this section doesn't have a list yet, create one
        if (!matches.contains(wikiKey)) {
            matches[wikiKey] = QList<QPair<int, QString>>();
        }
        
        // Add the match to the list
        matches[wikiKey].append(qMakePair(startPos, matchedText));
    }

    return matches;
}

QString ProjectManager::getContentByKeys(QList<QString> matchedWikiKeys) {
    QString content;
    
    // Check if we have any wiki content
    if (!haveWiki || wikiContentMap.isEmpty()) {
        return content;
    }
    
    for (const auto& wikiKey : matchedWikiKeys) {
        content += "--------------------------\n" + wikiContentMap[wikiKey];
    }
    return content;
}