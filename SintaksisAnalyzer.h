#ifndef SINTAKSIS_ANALYZER_H
#define SINTAKSIS_ANALYZER_H

#include "Token.h"
#include "TokenList.h"
#include "TreeNode.h"
#include "Parser.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>
#include <vector>
#include <iomanip> // Для std::setw

class SintaksisAnalyzer {
public:
    SintaksisAnalyzer();  // Конструктор
    ~SintaksisAnalyzer();
    //SintaksisAnalyzer(TokenList& tokenList); // Конструктор принимает ссылку на TokenList
    //void collectLine();              // Метод для сбора строки из tokenSequence
    void error(const int count_line, const std::string line, const std::string type_error);                    // Метод для вывода ошибок 
    void building_tree(const int count_line, const std::string line);            // Метод для построения дерева разбора
    std::string* split(const std::string line, int& numWords);

    bool isValidIdentifier(const std::string word); // работает
    bool isValidExpression(const std::string expr); // работает
    bool isValidOperator(const std::string opLine, const int count_line); // работает
    bool isValidOperator(const std::string opLine); // работает
    bool isValidOperator_for_cylce(const std::string opLine); // работает

    bool is_cycle(const std::string line, const int count_line); // работает
    bool is_start_program(const std::string line, const int count_line); // работает 
    bool is_end_program(const std::string line, const int count_line); // работает
    bool is_descriptions(const std::string line, const int count_line); // работает

    
    void clear_tree();
    void check_error();


    void trim(std::string& str) {
        // Удаляем пробелы с начала строки
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));

        // Удаляем пробелы с конца строки
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), str.end());
    }

    void Printing_Tree() {
        std::ofstream outFile("parsing_tree.txt");

        // Проверка на успешное открытие файла
        if (outFile.is_open()) {
            // Печать дерева в файл
            int maxLevel = root->getMaxLevel(root);
            root->printTree(outFile, maxLevel);  // rootNode — это корень вашего дерева
            outFile.close();  // Закрываем файл после записи
        }
        else {
            std::cerr << "Ошибка при открытии файла!" << std::endl;
        }
    }

    void Printing_Specific_Tree(const std::string& Nodename, int indentation = 0) {
        std::ofstream outFile("parsing_tree.txt");

        // Проверка на успешное открытие файла
        if (outFile.is_open()) {
            // Печать дерева в файл
            root->printSpecificNode(outFile, Nodename, indentation);  // rootNode — это корень вашего дерева
            outFile.close();  // Закрываем файл после записи
        }
        else {
            std::cerr << "Ошибка при открытии файла!" << std::endl;
        }
    }

    std::vector<std::string> splitBySemicolonOrNewline(const std::string& str);

    void analyzeTree() {
        root->analyzeTree(root);
    }

    void printCollectedStrings() {
        root->printCollectedStrings(root);
    }


private:
    TreeNode* root = new TreeNode("Program", 0);
    int level = 1;
    std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
        std::string result;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i != 0) result += delimiter;
            result += parts[i];
        }
        return result;
    }

    std::string types_lexeme[1000];
    int count_types_lexeme = 0;
    bool is_valid_start = false;
    bool is_valid_end = false;
    bool is_valid_descriptions = false;
    bool is_valid_operator = false;
    bool is_error_flag = false;
    std::ofstream outputFile;  // Поток для записи в файл
};

#endif
