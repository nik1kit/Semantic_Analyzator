#ifndef POSTFIX_H
#define POSTFIX_H

#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <string>
#include <algorithm>
#include <fstream>

using namespace std;

class PostfixConverter {
public:
    // Конструктор по умолчанию
    PostfixConverter() {}

    // Проверка, является ли строка оператором
    bool isOperator(const string& token) {
        return token == "+" || token == "-" || token == "*" || token == "/" || token == "=";
    }

    // Определение приоритета операторов
    int precedence(const string& op) {
        if (op == "+" || op == "-") return 1;
        if (op == "*" || op == "/") return 2;
        return 0; // Присваивание — низкий приоритет
    }

    // Преобразование инфиксного выражения в постфиксное
    vector<string> infixToPostfix(const vector<string>& tokens) {
        stack<string> operators;
        vector<string> postfix;

        for (const string& token : tokens) {
            if (isalnum(token[0])) {
                postfix.push_back(token);
            }
            else if (token == "(") {
                operators.push(token);
            }
            else if (token == ")") {
                while (!operators.empty() && operators.top() != "(") {
                    postfix.push_back(operators.top());
                    operators.pop();
                }
                if (!operators.empty() && operators.top() == "(") {
                    operators.pop();
                }
            }
            else if (isOperator(token)) {
                while (!operators.empty() && operators.top() != "(" &&
                    precedence(operators.top()) >= precedence(token)) {
                    postfix.push_back(operators.top());
                    operators.pop();
                }
                operators.push(token);
            }
        }

        while (!operators.empty()) {
            postfix.push_back(operators.top());
            operators.pop();
        }

        return postfix;
    }

    // Разделение строки на токены
    vector<string> tokenize(const string& line) {
        vector<string> tokens;
        string token;
        for (char ch : line) {
            if (isspace(ch)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else if (ch == '(' || ch == ')') {
                if (!token.empty()) tokens.push_back(token);
                tokens.push_back(string(1, ch));
                token.clear();
            }
            else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=') {
                if (!token.empty()) tokens.push_back(token);
                tokens.push_back(string(1, ch));
                token.clear();
            }
            else {
                token += ch;
            }
        }
        if (!token.empty()) tokens.push_back(token);
        return tokens;
    }

    // Функция для удаления лишних пробелов с начала и конца строки
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) {
            return "";
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, last - first + 1);
    }

    // Обработка строки с выражением
    string processExpression(const string& line) {
        vector<string> tokens = tokenize(line);
        vector<string> postfix = infixToPostfix(tokens);

        ostringstream oss;
        for (const string& token : postfix) {
            oss << token << " ";
        }
        return oss.str();
    }

    // Обработка цикла FOR
    string processForLoop(const string& line, int& indent) {
        istringstream iss(line);
        string token, var, start, condition, body, temp;

        getline(iss, token, ' ');
        iss >> var;
        iss >> token;
        iss >> start;

        getline(iss, token, 'D');
        size_t posTo = token.find("TO");
        if (posTo != string::npos) {
            condition = token.substr(posTo + 3);
            trim(condition);
        }

        getline(iss, temp, 'O');
        getline(iss, body);

        vector<string> conditionTokens = tokenize(condition);
        vector<string> conditionPostfix = infixToPostfix(conditionTokens);

        vector<string> bodyTokens = tokenize(body);
        ostringstream bodyProcessed;

        size_t i = 0;
        while (i < bodyTokens.size()) {
            if (bodyTokens[i] == "FOR") {
                size_t nestedStart = i;
                int nestedCount = 1;
                while (nestedCount > 0 && i < bodyTokens.size()) {
                    ++i;
                    if (bodyTokens[i] == "FOR") ++nestedCount;
                    else if (bodyTokens[i] == "DO") --nestedCount;
                }

                string nestedLoop;
                for (size_t j = nestedStart; j <= i; ++j) {
                    nestedLoop += bodyTokens[j] + " ";
                }

                int nestedIndent = indent;
                bodyProcessed << processForLoop(nestedLoop, nestedIndent) << " ";
                indent = nestedIndent;
            }
            else {
                string expression;
                while (i < bodyTokens.size() && bodyTokens[i] != "FOR" && bodyTokens[i] != ";") {
                    expression += bodyTokens[i] + " ";
                    ++i;
                }
                --i;
                bodyProcessed << processExpression(expression) << " ";
            }
            ++i;
        }

        ostringstream oss;
        int m1_index = indent++;
        int m2_index = indent++;
        string m1 = "m" + to_string(m1_index);
        string m2 = "m" + to_string(m2_index);

        oss << var << " " << start << " = ";
        oss << m1 << " DEFL ";
        oss << var << " ";
        for (const string& token : conditionPostfix) {
            oss << token << " ";
        }
        oss << "< " << m2 << " BF ";
        oss << bodyProcessed.str();
        oss << var << " " << var << " 1 + = " << m1 << " BRL ";
        oss << m2 << " DEFL";

        return oss.str();
    }

    // Подсчет количества переменных в строке
    int countVariables(const string& line) {
        return count(line.begin(), line.end(), ',') + 1;
    }

    // Обработка входного файла
    void processFile(const string& input) {
        // Открываем файл для записи
        ofstream outFile("postfix.txt");
        if (!outFile.is_open()) {
            cerr << "Error: Unable to open file postfix.txt for writing." << endl;
            return;
        }

        istringstream iss(input);
        string line;

        while (getline(iss, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.find("FOR") != string::npos) {
                int indent = 1;
                outFile << processForLoop(line, indent) << endl;
            }
            else if (line.find("INTEGER") != string::npos) {
                string vars = line.substr(line.find("INTEGER") + 8); // Получаем всё после "INTEGER"
                vars = trim(vars); // Удаляем лишние пробелы в начале и конце строки

                // Разделяем переменные по запятым
                vector<string> varList;
                stringstream ss(vars);
                string var;
                while (getline(ss, var, ',')) {
                    var = trim(var); // Удаляем пробелы вокруг каждой переменной
                    if (!var.empty()) {
                        varList.push_back(var);
                    }
                }

                // Формируем результат
                outFile << "INTEGER ";
                for (const auto& v : varList) {
                    outFile << v << " ";
                }
                outFile << varList.size() + 1 << " DECL" << endl; // Выводим количество переменных
            }
            else if (line.find('=') != string::npos) {
                outFile << processExpression(line) << endl;
            }
        }

        // Закрываем файл
        outFile.close();
    }

};


#endif