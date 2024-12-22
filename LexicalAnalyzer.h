#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

#include <iostream>
#include <fstream>
#include <string>
#include "TokenList.h"
#include "SintaksisAnalyzer.h"
#include "TreeNode.h"

class LexicalAnalyzer {
public:
    SintaksisAnalyzer sintaksis_analyzer;
    std::string lexeme;
    int count_line = 0;
    int count = 0;
    LexicalAnalyzer(const std::string& inputFileName, const std::string& outputFileName);
    ~LexicalAnalyzer();
    void analyze();

    TokenList& getTokenList() {
        return tokenList;
    }

private:
    std::ifstream inputFile;
    std::ofstream outputFile;
    TokenList tokenList;
    const std::string tree = "parsing_tree.txt";

    Token getNextLexeme();
    TokenType getKeywordOrIdentifier(const std::string& lexeme);
    TokenType getNumber(const std::string& lexeme);
    bool isOperator(const std::string& str) const;
    bool isDelimiter(char c) const;
    void handleDelimiters(char c);
    bool isValidIdentifier(const std::string& word) const; // Объявление функции
};

#endif // LEXICALANALYZER_H
