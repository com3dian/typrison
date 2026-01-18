#ifndef AHOCORASICK_H
#define AHOCORASICK_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <string>
#include <QDebug>

struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    TrieNode* failLink = nullptr;
    std::vector<int> output; // Store indexes of strings ending here
};

class AhoCorasick {
    TrieNode* root;

public:
    AhoCorasick();
    // ~AhoCorasick();

    // Insert a string into the Trie
    void insert(const std::string& word, int index);

    void remove(const std::string& word);
    void removeMultiple(const std::vector<std::string>& words);
    void removeWithoutRebuild(const std::string& word);

    // Build failure links using BFS
    void buildFailureLinks();

    // Search the text for patterns
    std::vector<std::pair<int, int>> search(const std::string& text);

private:
    void deleteTrie(TrieNode* node); // Helper to clean up memory
};

#endif // AHOCORASICK_H