#ifndef PARSER_H
#define PARSER_H

#include "TreeNode.h"
#include <string>
#include <sstream>
#include <iostream>

class Parser {
public:
    static void parseProgram(TreeNode* root, const std::string& line, int& level) {
        std::istringstream lineStream(line);
        std::string word;

        TreeNode* currentNode = root;
        TreeNode* lastDescriptionNode = nullptr;

        while (lineStream >> word) {
            if (word == "PROGRAM") {
                TreeNode* programmNode = currentNode->addSon(word, level, "WordsKey");
                std::string programName;
                if (lineStream >> programName) {
                    programmNode->addSon(programName, level + 1, "Id");
                }
            }
            else if (word == "INTEGER") {
                if (!lastDescriptionNode) {
                    lastDescriptionNode = root->addSon("Descriptions", level);
                }
                TreeNode* descrNode = lastDescriptionNode->addSon("Descr", level + 1);
                descrNode->addSon("Type", level + 2)->addSon(word, level + 3, "WordsKey");

                std::string var;
                bool firstVar = true;  // Флаг для первой переменной
                while (lineStream >> var) {
                    if (var == ",") {
                        // Если запятая, добавляем её отдельно в Symbols_of_Separating
                        TreeNode* SymbolsNode = descrNode->addSon("Varlist", level + 2);
                        SymbolsNode->addSon(var, level + 3, "Symbols_of_Separating");

                    }
                    else {
                        bool flag = false;
                        if (var.back() == ',') {
                            var = var.substr(0, var.size() - 1);
                            flag = true;
                        }
                        // Если это не запятая, добавляем переменную
                        TreeNode* varNode = descrNode->addSon("Varlist", level + 2);
                        varNode->addSon(var, level + 3, "Id");
                        if (flag)
                            varNode->addSon(",", level + 3, "Symbols_of_Separating");
                        firstVar = false;  // После первой переменной запятые будут добавляться
                    }
                }
            }
            else if (word == "OPERATORS") {
                // Создаем новый узел для блока OPERATORS
                TreeNode* operatorsNode = currentNode->addSon("Operators", level);
                if (!operatorsNode) return; // Проверка, что узел создан

                std::string operatorWord;
                while (lineStream >> operatorWord) {
                    // Чтение левой части выражения (переменная, например "c")
                    std::string lhs = operatorWord; // Сохраняем первую переменную как левый операнд

                    // Создаем узел для операции
                    TreeNode* opNode = operatorsNode->addSon("Op", level + 1);
                    if (!opNode) return;

                    // Левый операнд (переменная, например "c")
                    TreeNode* lhsNode = opNode->addSon(lhs, level + 2, "Id");
                    if (!lhsNode) return;

                    // Чтение операции присваивания "="
                    lineStream >> operatorWord;  // Теперь ожидаем знак присваивания "="
                    opNode->addSon(operatorWord, level + 2, "Symbols_of_Operation"); // Операция "="

                    // Правое выражение (например, "a + ( b + 5 ) - x")
                    std::string token;
                    bool expectOperand = true;  // Ожидаем сначала операнд
                    bool inParentheses = false; // Проверка на вложенные скобки

                    // Для правой части создаем новый узел для выражения
                    TreeNode* exprNode = opNode->addSon("Expr", level + 3);
                    if (!exprNode) return;

                    TreeNode* currentExprNode = nullptr;  // Узел для текущего SimpleExpr

                    while (lineStream >> token) {
                        if (token == "(") {
                            // Открывающая скобка - начинаем вложенное выражение
                            inParentheses = true;
                            exprNode->addSon("(", level + 6, "Opening_Bracket");
                            continue;
                        }

                        if (token == ")") {
                            // Закрывающая скобка - заканчиваем вложенное выражение
                            exprNode->addSon(")", level + 6, "Closing_Bracket");
                            inParentheses = false;
                            continue;
                        }

                        if (expectOperand) {
                            // Ожидаем операнд (переменная или константа)
                            if (!currentExprNode) {
                                // Если текущий операнд не был создан, создаем новый SimpleExpr
                                currentExprNode = exprNode->addSon("SimpleExpr", level + 5);
                                if (!currentExprNode) return;
                            }

                            if (token[0] >= '0' && token[0] <= '9') {
                                // Если это число, то константа
                                currentExprNode->addSon(token, level + 6, "Const");
                            }
                            else {
                                // Если это переменная
                                currentExprNode->addSon(token, level + 6, "Id");
                            }
                            expectOperand = false; // Следующей должна быть операция
                        }
                        else {
                            // Ожидаем операцию
                            exprNode->addSon(token, level + 6, "Symbols_of_Operation");
                            expectOperand = true; // Следующим снова должен быть операнд

                            // После операции создаем новый SimpleExpr для следующего операнда
                            currentExprNode = exprNode->addSon("SimpleExpr", level + 5);
                            if (!currentExprNode) return;
                        }
                    }

                    // После обработки присваивания, если встречаем "END", завершить блок
                    if (operatorWord == "END") {
                        operatorsNode->addSon(operatorWord, level + 2, "WordsKey");
                        break; // Выход из цикла после окончания блока операторов
                    }
                }
            }
            else if (word == "FOR") {
                TreeNode* opNode = currentNode->addSon("Operators", level);
                if (!opNode) return; // Проверка, что узел создан
                opNode = opNode->addSon("Op", level + 1);
                if (!opNode) return; // Проверка, что узел создан
                opNode->addSon(word, level + 2, "WordsKey");

                // Обрабатываем выражение после FOR
                TreeNode* exprNode = opNode->addSon("Expr", level + 2);
                if (!exprNode) return; // Проверка, что узел создан

                std::string expr;
                TreeNode* simpleExprNode = nullptr;
                while (lineStream >> expr && expr != "TO") {
                    if (expr == "=") {
                        simpleExprNode = exprNode->addSon("SimpleExpr", level + 3);
                        if (!simpleExprNode) return;
                        simpleExprNode->addSon(expr, level + 4, "Symbols_of_Operation");
                    }
                    else if (expr == "+" || expr == "-" || expr == "*") {
                        if (!simpleExprNode) {
                            simpleExprNode = exprNode->addSon("SimpleExpr", level + 3);
                            if (!simpleExprNode) return;
                        }
                        simpleExprNode->addSon(expr, level + 4, "Symbols_of_Operation");
                    }
                    else {
                        if (!simpleExprNode) {
                            simpleExprNode = exprNode->addSon("SimpleExpr", level + 3);
                            if (!simpleExprNode) return;
                        }
                        simpleExprNode->addSon(expr, level + 4, expr[0] >= '0' && expr[0] <= '9' ? "Const" : "Id");
                    }
                }

                // Обрабатываем выражение после TO
                if (expr == "TO") {
                    opNode->addSon(expr, level + 2, "WordsKey");
                    TreeNode* toExprNode = opNode->addSon("Expr", level + 2);
                    if (!toExprNode) return;

                    simpleExprNode = nullptr;
                    while (lineStream >> expr && expr != "DO") {
                        if (expr == "(") {
                            toExprNode->addSon(expr, level + 3, "Opening_Bracket");
                        }
                        else if (expr == ")") {
                            toExprNode->addSon(expr, level + 3, "Closing_Bracket");
                        }
                        else if (expr == "+" || expr == "-" || expr == "=") {
                            simpleExprNode = toExprNode->addSon("SimpleExpr", level + 3);
                            if (!simpleExprNode) return;
                            simpleExprNode->addSon(expr, level + 4, "Symbols_of_Operation");
                        }
                        else {
                            if (!simpleExprNode) {
                                simpleExprNode = toExprNode->addSon("SimpleExpr", level + 3);
                                if (!simpleExprNode) return;
                            }
                            simpleExprNode->addSon(expr, level + 4, expr[0] >= '0' && expr[0] <= '9' ? "Const" : "Id");
                        }
                    }
                }

                // Если встретился DO, обрабатываем тело цикла
                if (expr == "DO") {
                    opNode->addSon(expr, level + 2, "WordsKey");

                    TreeNode* nestedCycleNode = opNode->addSon("NestedCycle", level + 2);
                    if (!nestedCycleNode) return;

                    TreeNode* nestedOpsNode = nestedCycleNode->addSon("Operators", level + 3);
                    if (!nestedOpsNode) return;

                    std::string nestedExpr;
                    while (lineStream >> nestedExpr) {
                        if (nestedExpr == "FOR") {
                            std::string nestedLine;
                            std::getline(lineStream, nestedLine);
                            nestedLine = "FOR " + nestedLine;
                            int nestedLevel = level + 4; // Локально обновляем уровень
                            parseProgram(nestedOpsNode, nestedLine, nestedLevel);
                            break;
                        }
                        else {
                            // Обработка вложенных выражений
                            std::ostringstream expressionStream;
                            expressionStream << nestedExpr;

                            std::string remainingToken;
                            while (lineStream >> remainingToken && remainingToken != "FOR") {
                                expressionStream << " " << remainingToken;
                            }

                            // Если встретился "FOR", нужно вернуть его обратно в поток
                            if (remainingToken == "FOR") {
                                lineStream.putback('F');
                                for (int i = 0; i < 3; ++i) lineStream.putback(remainingToken[i]); // FOR = 3 символа
                            }

                            // Разделение выражений с помощью splitBySemicolonOrNewline
                            std::vector<std::string> expressions = splitBySemicolonOrNewline(expressionStream.str());
                            for (const auto& expr : expressions) {
                                // Создание Op узла для каждого выражения
                                TreeNode* nestedOpNode = nestedOpsNode->addSon("Op", level + 4);
                                if (!nestedOpNode) return;

                                // Добавление узла Expr для выражения
                                TreeNode* nestedExprNode = nestedOpNode->addSon("Expr", level + 5);
                                if (!nestedExprNode) return;

                                // Разбор выражения
                                std::istringstream exprStream(expr);
                                std::string token;
                                TreeNode* simpleExprNode = nullptr;

                                bool expectOperand = true; // Ожидаем сначала операнд (переменная или константа)

                                while (exprStream >> token) {
                                    if (expectOperand) {
                                        // Ожидаем операнд (переменная или константа)
                                        simpleExprNode = nestedExprNode->addSon("SimpleExpr", level + 6);
                                        if (!simpleExprNode) return;

                                        simpleExprNode->addSon(token, level + 7, token[0] >= '0' && token[0] <= '9' ? "Const" : "Id");
                                        expectOperand = false; // Следующей должна быть операция
                                    }
                                    else {
                                        // Ожидаем операцию

                                        simpleExprNode->addSon(token, level + 7, "Symbols_of_Operation");
                                        expectOperand = true; // Следующим снова должен быть операнд
                                    }
                                }
                            }

                        }
                    }
                }
            }
            else if (word == "END") {
                TreeNode* endNode = currentNode->addSon(word, level, "WordsKey");
                std::string endName;
                if (lineStream >> endName) {
                    endNode->addSon(endName, level + 1, "Id");
                }
            }
            // Обработка других ключевых слов...
        }
    }


    static std::vector<std::string> splitBySemicolonOrNewline(const std::string& line) {
        std::vector<std::string> expressions;
        std::vector<size_t> equal_positions;  // Массив позиций символов '='

        // Находим все позиции символов '=' в строке
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '=') {
                equal_positions.push_back(i);
            }
        }

        // Если в строке только один знак '=', то возвращаем всю строку как одно выражение
        if (equal_positions.size() == 1) {
            expressions.push_back(line);
            return expressions;
        }

        // Разбиваем строку на выражения по позициям '='
        size_t start_pos = 0;
        for (size_t i = 1; i < equal_positions.size(); ++i) {
            // Строка от предыдущего '=' до текущего '='
            expressions.push_back(line.substr(start_pos, equal_positions[i] - start_pos - 2));
            start_pos = equal_positions[i] - 2;  // Новая стартовая позиция после '='
        }

        // Добавляем последнее выражение (после последнего '=')
        expressions.push_back(line.substr(start_pos));

        return expressions;
    }
};

#endif // PARSER_H
