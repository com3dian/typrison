#include "ahocorasick.h"

AhoCorasick::AhoCorasick() {
    root = new TrieNode();
}

void AhoCorasick::insert(const std::string& word, int index) {
    TrieNode* curr = root;
    for (char c : word) {
        if (!curr->children.count(c)) {
            curr->children[c] = new TrieNode();
        }
        curr = curr->children[c];
    }
    curr->output.push_back(index);
}

void AhoCorasick::buildFailureLinks() {
    std::queue<TrieNode*> q;
    for (auto& [c, child] : root->children) {
        child->failLink = root;
        q.push(child);
    }
    while (!q.empty()) {
        TrieNode* curr = q.front();
        q.pop();

        for (auto& [c, child] : curr->children) {
            TrieNode* fail = curr->failLink;
            while (fail && !fail->children.count(c)) {
                fail = fail->failLink;
            }
            child->failLink = fail ? fail->children[c] : root;
            child->output.insert(child->output.end(),
                                 child->failLink->output.begin(),
                                 child->failLink->output.end());
            q.push(child);
        }
    }
}

std::vector<std::pair<int, int>> AhoCorasick::search(const std::string& text) {
    TrieNode* curr = root;
    std::vector<std::pair<int, int>> result;
    for (int i = 0; i < text.size(); i++) {
        char c = text[i];
        while (curr && !curr->children.count(c)) {
            curr = curr->failLink;
        }
        curr = curr ? curr->children[c] : root;
        for (int index : curr->output) {
            result.push_back(std::make_pair(index, i));
        }
    }
    return result;
}

void AhoCorasick::remove(const std::string& word) {
    removeWithoutRebuild(word);
    // Rebuild failure links as they might be affected
    buildFailureLinks();
}

void AhoCorasick::removeWithoutRebuild(const std::string& word) {
    TrieNode* curr = root;
    std::vector<std::pair<TrieNode*, char>> path;
    
    // Traverse to the end of the word
    for (char c : word) {
        if (!curr->children.count(c)) {
            return; // Word not found
        }
        path.push_back({curr, c});
        curr = curr->children[c];
    }
    
    // Clear all indices from output
    curr->output.clear();
    
    // If node has no children, remove it and its parents if possible
    if (curr->children.empty()) {
        for (int i = path.size() - 1; i >= 0; i--) {
            TrieNode* parent = path[i].first;
            char c = path[i].second;
            TrieNode* child = parent->children[c];
            
            delete child;
            parent->children.erase(c);
            
            // Stop if parent has other children or is an output node
            if (!parent->children.empty() || !parent->output.empty()) {
                break;
            }
        }
    }
}

void AhoCorasick::removeMultiple(const std::vector<std::string>& words) {
    for (const auto& word : words) {
        removeWithoutRebuild(word);
    }
    // Rebuild failure links only once after all removals
    buildFailureLinks();
}