#ifndef TREENODE_H
#define TREENODE_H

#include "Postfix.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <stack>
#include <set>

class TreeNode {
private:
    PostfixConverter ToPostfix;
    std::string data;
    std::string type; 
    std::vector<TreeNode*> children;
    int level;

public:
    TreeNode(const std::string& nodeName, int nodeLevel, const std::string& nodeType = "")
        : data(nodeName), level(nodeLevel), type(nodeType) {}

    ~TreeNode() {
        for (TreeNode* child : children) {
            delete child;
        }
    }

    TreeNode* addSon(const std::string& nodeName, int nodeLevel, const std::string& nodeType = "") {
        TreeNode* newNode = new TreeNode(nodeName, nodeLevel, nodeType);
        children.push_back(newNode);
        return newNode;
    }

    std::string getData() const { return data; }

    std::string getType() const { return type; }

    const std::vector<TreeNode*>& getChildren() const { return children; }

    int getLevel() const { return level; }

    int getMaxLevel(const TreeNode* node, int currentLevel = 1) const {
        int maxLevel = currentLevel;  
        for (const TreeNode* child : node->children) {
            int childLevel = getMaxLevel(child, currentLevel + 1);
            maxLevel = std::max(maxLevel, childLevel); 
        }
        return maxLevel;
    }



    void printTree(std::ofstream& outFile, int maxLevel, int currentLevel = 1, int indentation = 0) const {
        if (currentLevel > maxLevel) {
            return;
        }

        for (int i = 0; i < indentation; ++i) outFile << " ";

        outFile << data;
        if (!type.empty()) outFile << " [" << type << "]";
        outFile << std::endl;

        for (const TreeNode* child : children) {
            child->printTree(outFile, maxLevel, currentLevel + 1, indentation + 2);
        }
    }


    void printSpecificNode(std::ofstream& outFile, const std::string& nodeName, int indentation = 5) {
        // ���� ��� �������� ���� ��������� � �������
        indentation = getMaxLevel(this);
        if (data == nodeName) {
            // ������� ������� ���� � ��� ���������
            this->printTree(outFile, indentation);
            return;
        }

        // ���������� ���� � �������� �����
        for (TreeNode* child : children) {
            child->printSpecificNode(outFile, nodeName, indentation);
        }
    }







    static std::vector<std::string> splitByEqualSign(const std::string& line) {
        std::vector<std::string> expressions;
        std::vector<size_t> equal_positions;

        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '=') {
                equal_positions.push_back(i);
            }
        }

        if (equal_positions.size() == 1) {
            expressions.push_back(line);
            return expressions;
        }

        size_t start_pos = 0;
        for (size_t i = 0; i < equal_positions.size(); ++i) {
            size_t end_pos = (i + 1 < equal_positions.size()) ? equal_positions[i + 1] - 1 : line.size();
            expressions.push_back(line.substr(start_pos, end_pos - start_pos));
            start_pos = end_pos;
        }

        return expressions;
    }

    static void processForToDo(TreeNode* forNode, const std::string& line, int level) {
        std::istringstream lineStream(line);
        std::string token;

        while (lineStream >> token) {
            if (token == "TO") {
                forNode->addSon(token, level, "WordsKey");
            }
            else if (token == "DO") {
                forNode->addSon(token, level, "WordsKey");
                TreeNode* nestedCycleNode = forNode->addSon("NestedCycle", level);
                if (!nestedCycleNode) return;
                nestedCycleNode->addSon("Operators", level + 1);
            }
            else {
            }
        }
    }

    // ������� ��� �������� ������ �������� � ������ � ����� ������
    std::string trim(const std::string& str) {
        // �������� �������� � ������ ������
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) {
            return ""; // ������ ������, ���� ������ �������
        }

        // �������� �������� � ����� ������
        size_t last = str.find_last_not_of(' ');

        // ���������� ���������� ������
        return str.substr(first, last - first + 1);
    }

    void printCollectedStrings(const TreeNode* root) {
        if (root == nullptr) return;
        std::string text;
        // ���� ����� ��� ���� ������� �����
        std::vector<std::string> collectedStrings = collectChildStrings(root);

        // ����� ����������
        //std::cout << "Collected strings for children of \"" << root->getData() << "\":" << std::endl;
        for (const std::string& str : collectedStrings) {
            text += str + "\n";
            //std::cout << str << std::endl;
        }
        ToPostfix.processFile(text);
    }



    // ������ ���������� ������

    std::vector<std::string> collectChildStrings(const TreeNode* node, int indentation = 0) {
        std::vector<std::string> result;

        if (node == nullptr) return result;

        // ������ �������� ����, ������� �� ����� �������� � ������
        std::set<std::string> excludedKeywords = {
            "Expr", "SimpleExpr", "Operators", "WordsKey", "Symbols_of_Operation",
            "Symbols_of_Separating", "Const", "Opening_Bracket", "Closing_Bracket",
            "Id", "Type", "Varlist", "NestedCycle", "Descr", "Descriptions", "Op"
        };

        // �������� �� ���� �������� �������� ����
        for (const TreeNode* child : node->getChildren()) {
            std::string collectedString;

            // ��������� ������� ��� �������� ������ (���� �����)
            collectedString.append(indentation, ' ');

            // ��������� ������ �������� ��������� ����, ���� ��� �� ����������� �������� �����
            if (excludedKeywords.find(child->getData()) == excludedKeywords.end()) {
                // ���� ������ �� �����, ��������� ������ ����� ����� ���������
                if (!collectedString.empty()) {
                    collectedString += " ";
                }
                
                collectedString += trim(child->getData());
            }

            // ���� � ��������� ���� ���� �������, ���������� ��������� ��
            if (!child->getChildren().empty()) {
                std::vector<std::string> childStrings = collectChildStrings(child, indentation + 4);
                for (const std::string& str : childStrings) {
                    // ��������� �������� �������� �����, �������� �� ��������
                    if (!collectedString.empty()) {
                        collectedString += " ";
                    }
                    collectedString += trim(str);
                }
            }
            //ToPostfix.processFile(collectedString);
            result.push_back(collectedString); // ��������� ������ � ���������
        }

        return result;
    }





    void checkProgramEndIds(const TreeNode* root) {
        if (root == nullptr) return;

        std::string programId;
        std::string endId;

        // ������� ��� ������ �������������� ����� ����� PROGRAM � END
        auto findIdAfterKeyword = [](const TreeNode* node, const std::string& keyword) -> std::string {
            for (const TreeNode* child : node->getChildren()) {
                if (child->getData() == keyword) {
                    // ���� ��������� �������� ���� � ����� "Id"
                    for (const TreeNode* subChild : child->getChildren()) {
                        if (subChild->getType() == "Id") {
                            return subChild->getData();
                        }
                    }
                }
            }
            return ""; // ������������� �� ������
            };

        // ����� ��������������� PROGRAM � END
        programId = findIdAfterKeyword(root, "PROGRAM");
        endId = findIdAfterKeyword(root, "END");
        std::ofstream errorFile("errors.txt", std::ios::app);

        if (!errorFile.is_open()) {
            std::cerr << "������: �� ������� ������� ���� errors.txt ��� ������." << std::endl;
            return;
        }
        // �������� ����������
        if (programId.empty() || endId.empty()) {
            std::cout << "Warning: Missing PROGRAM or END identifier." << std::endl;
            errorFile << "Warning: Missing PROGRAM or END identifier." << std::endl;
            errorFile.close();
        }
        else if (programId != endId) {
            errorFile << "Error: PROGRAM identifier \"" << programId
                << "\" does not match END identifier \"" << endId << "\"." << std::endl;
            std::cout << "Error: PROGRAM identifier \"" << programId
                << "\" does not match END identifier \"" << endId << "\"." << std::endl;
            errorFile.close();
        }
        else {
            errorFile << "PROGRAM and END identifiers match: \"" << programId << "\"." << std::endl;
            std::cout << "PROGRAM and END identifiers match: \"" << programId << "\"." << std::endl;
            errorFile.close();
        }
    }

    bool check_povtor(std::unordered_set<std::string>& declaredVars, std::string id) {
        for (auto child : declaredVars) {
            if (child == id) {
                return false;
            }
        }
        return true;
    }

    void collectDeclaredVariables(const TreeNode* node, std::unordered_set<std::string>& declaredVars) {
        if (node == nullptr) return;
        // ���� ��� ���� ���� Varlist, �������� ��� ���������� (Id) �� ����
        if (node->getData() == "Varlist") {
            for (const TreeNode* child : node->getChildren()) {
                if (child->getType() == "Id") {
                    if (!check_povtor(declaredVars, child->getData())) {
                        std::cout << child->getData() << " - redeclaring a variable\n";
                    }
                    declaredVars.insert(child->getData());
                }
            }
        }

        // ���������� ������� �������� ����
        for (const TreeNode* child : node->getChildren()) {
            collectDeclaredVariables(child, declaredVars);
        }
    }


    void checkVariableUsage(const TreeNode* node,
        const std::unordered_set<std::string>& declaredVars,
        std::unordered_set<std::string>& undeclaredVars) {
        if (node == nullptr) return;
        if (node->getData() == "PROGRAM" || node->getData() == "END") {
            return;
        }
        const std::string& nodeName = node->getData();
        const std::string& nodeType = node->getType();

        // ���������� �������� ����� � ���� "PROGRAM" � "END"
        static const std::unordered_set<std::string> ignoredWords = {
            "PROGRAM", "END", "FOR", "TO", "DO", "=", "+", "-", "*", "/", "(", ")", ","
        };

        if (ignoredWords.find(nodeName) != ignoredWords.end()) {
            // ���������� ������� ����, �� ���������� ��������� ��� �������� ����
            for (const TreeNode* child : node->getChildren()) {
                checkVariableUsage(child, declaredVars, undeclaredVars);
            }
            return;
        }

        // ���� ��� ���� � ����� "Id", ��������� ��������� �� ����������
        if (nodeType == "Id") {
            if (declaredVars.find(nodeName) == declaredVars.end()) {
                undeclaredVars.insert(nodeName);
            }
        }

        // ���������� ������� �������� ����
        for (const TreeNode* child : node->getChildren()) {
            checkVariableUsage(child, declaredVars, undeclaredVars);
        }
    }


    void analyzeTree(const TreeNode* root) {
        if (root == nullptr) return;

        checkProgramEndIds(root);

        // �������� ��� ����������� ����������
        std::unordered_set<std::string> declaredVars;
        collectDeclaredVariables(root, declaredVars);

        // ��������� ������������� ����������
        std::unordered_set<std::string> undeclaredVars;
        checkVariableUsage(root, declaredVars, undeclaredVars);
        std::ofstream errorFile("errors.txt", std::ios::app);

        if (!errorFile.is_open()) {
            std::cerr << "������: �� ������� ������� ���� errors.txt ��� ������." << std::endl;
            return;
        }
        // ������� ���������
        if (undeclaredVars.empty()) {
            std::cout << "All variables are properly declared." << std::endl;
            errorFile << "All variables are properly declared." << std::endl;
            errorFile.close();
        }
        else {
            std::cout << "The following variables are used but not declared:" << std::endl;
            errorFile << "The following variables are used but not declared:" << std::endl;
            for (const std::string& var : undeclaredVars) {
                std::cout << "  " << var << std::endl;
                errorFile << "  " << var << std::endl;
            }

            errorFile.close();
        }
    }
};

#endif // TREENODE_H