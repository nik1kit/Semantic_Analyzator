#include "SintaksisAnalyzer.h"
#include "TokenList.h"
#include <iostream>
#include <string>
#include <cstring> // для memset
#include <stack>

SintaksisAnalyzer::SintaksisAnalyzer() {
    outputFile.open("parsing_tree.txt");
    if (!outputFile.is_open()) {
        throw std::ios_base::failure("Failed to open the file.");
    }

    // Открытие файла ошибок и очистка его содержимого
    std::ofstream errorsFile("errors.txt", std::ios::trunc); // флаг trunc очищает файл
    if (!errorsFile.is_open()) {
        throw std::ios_base::failure("Failed to open errors.txt.");
    }
    // Закрываем файл ошибок после очистки
    errorsFile.close();
}

SintaksisAnalyzer::~SintaksisAnalyzer() {
    if (outputFile.is_open()) {
        outputFile.close();
    }
}

void SintaksisAnalyzer::check_error() {
    if (!is_valid_end) {
        clear_tree();
        error(1234, "", "Отсутствует конец программы(END id_name)");
        throw std::ios_base::failure("An error was encountered in the input file.");
    }

    // Переменные для отслеживания состояния порядка лексем
    bool found_start = false;
    bool found_end = false;
    bool found_description = false;
    bool found_operator = false;

    // Проверяем порядок лексем
    for (int i = 0; i < count_types_lexeme; ++i) {
        const std::string& lexeme = types_lexeme[i];

        if (lexeme == "start") {
            // Убедимся, что "start" встречается только один раз
            if (found_start) {
                clear_tree();
                error(1234, "", "Множественные вхождения start");
                throw std::ios_base::failure("An error was encountered in the input file.");
            }
            found_start = true;
        }
        else if (lexeme == "end") {
            // Убедимся, что "end" встречается только один раз
            if (found_end) {
                clear_tree();
                error(1234, "", "Множественные вхождения end");
                throw std::ios_base::failure("An error was encountered in the input file.");
            }
            found_end = true;
        }
        else if (lexeme == "description") {
            // description должно идти только после start и до оператора или end
            if (!found_start || found_end || found_operator) {
                clear_tree();
                error(1234, "", "Ошибка: описание должно идти после start и до оператора или end");
                throw std::ios_base::failure("An error was encountered in the input file.");
            }
            found_description = true;
        }
        else if (lexeme == "operator") {
            // operator должно идти только после description и до end
            if (!found_start || !found_description || found_end) {
                clear_tree();
                error(1234, "", "Ошибка: оператор должен идти после description и до end");
                throw std::ios_base::failure("An error was encountered in the input file.");
            }
            found_operator = true;
        }
    }
}

bool SintaksisAnalyzer::isValidOperator_for_cylce(const std::string opLine) {
    std::string currentExpression = "";
    std::vector<std::string> expressions;
    int balance = 0; // Для учета вложенных выражений, если появятся скобки или сложные конструкции

    // Разделяем строку на отдельные выражения по пробелам
    for (size_t i = 0; i < opLine.size(); ++i) {
        char c = opLine[i];

        if (c == ' ' && balance == 0) {
            // Когда встречаем пробел на "нулевом уровне", добавляем текущее выражение
            if (!currentExpression.empty()) {
                expressions.push_back(currentExpression);
                currentExpression = "";
            }
        }
        else {
            currentExpression += c;
        }
    }

    // Добавляем последнее выражение
    if (!currentExpression.empty()) {
        expressions.push_back(currentExpression);
    }

    // Проверяем каждое выражение с помощью isValidOperator
    for (const std::string& expr : expressions) {
        if (!isValidOperator(expr, /*count_line=*/0)) { // count_line = 0, так как строка не привязана к конкретной линии
            return false;
        }
    }

    return true; // Все выражения корректны
}



void SintaksisAnalyzer::clear_tree() {
    outputFile.close();
    std::ofstream treeFile("parsing_tree.txt", std::ios::trunc);
    if (!treeFile.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл errors.txt для очистки." << std::endl;
    }
    treeFile.close(); // Закрываем файл после очистки
}

bool SintaksisAnalyzer::isValidIdentifier(const std::string word) {
    for (char c : word) {
        if (!isalpha(c)) { // Если символ не является буквой
            return false;
        }
    }
    return true;
}

// Проверка, является ли строка валидным выражением (пока допускаем только числа)
bool SintaksisAnalyzer::isValidExpression(const std::string expr) {
    return !expr.empty() && std::all_of(expr.begin(), expr.end(), ::isdigit);
}

std::string* SintaksisAnalyzer::split(const std::string line, int& numWords) {
    static const int MAX_WORDS = 1000;  // Максимальное количество слов
    static std::string words[MAX_WORDS];  // Статический массив слов

    // Очистка массива перед использованием
    for (int i = 0; i < MAX_WORDS; i++) {
        words[i].clear();
    }

    size_t start = 0;
    size_t end = line.find(' '); // Находим первый пробел
    numWords = 0;                // Счётчик слов

    // Разделение строки на слова
    while (end != std::string::npos && numWords < MAX_WORDS) {
        std::string word = line.substr(start, end - start);
        if (!word.empty()) { // Добавляем слово, только если оно не пустое
            words[numWords++] = word;
        }
        start = end + 1;
        end = line.find(' ', start); // Находим следующий пробел
    }

    // Добавляем последнее слово, если оно есть
    if (start < line.length()) {
        std::string word = line.substr(start);
        if (!word.empty() && numWords < MAX_WORDS) {
            words[numWords++] = word;
        }
    }

    return words;  // Возвращаем указатель на массив слов
}

// Проверка, является ли строка корректным оператором (пока простая проверка)
bool SintaksisAnalyzer::isValidOperator(const std::string opLine) {
    std::string error_;
    int count_words = 0;
    // Разделяем строку на токены
    std::string* tokens = split(opLine, count_words);
    // Проверяем, что строка не пустая и содержит минимум "id = что-то"
    if (count_words < 3) {
        return false;
    }

    // Проверяем второй токен как оператор присваивания
    if (tokens[1] != "=") {
        return false;
    }

    // Проверяем первый токен как идентификатор
    if (!isValidIdentifier(tokens[0])) {
        return false;
    }

    // Проверяем оставшиеся токены после '='
    bool expectingOperand = true; // Указывает, ожидаем ли операнд или оператор
    int openBrackets = 0;         // Счетчик открывающих скобок

    for (int i = 2; i < count_words; ++i) {
        if (tokens[i] == "(") {
            openBrackets++;
        }
        else if (tokens[i] == ")") {
            openBrackets--;
            // Проверяем, чтобы закрывающая скобка не была вне пары
            if (openBrackets < 0) {
                return false;
            }
        }
        else if (expectingOperand) {
            // Ожидаем операнд: идентификатор или выражение
            if (!isValidExpression(tokens[i]) && !isValidIdentifier(tokens[i])) {
                return false;
            }
            expectingOperand = false; // Следующий токен должен быть оператором
        }
        else {
            // Ожидаем оператор: '+' или '-'
            if (tokens[i] != "+" && tokens[i] != "-") {
                return false;
            }
            expectingOperand = true; // Следующий токен должен быть операндом
        }
    }

    // Если цикл завершился, но последним токеном был оператор, это ошибка
    if (expectingOperand) {
        return false;
    }

    // Если после завершения цикла остались незакрытые скобки, это ошибка
    if (openBrackets > 0) {
        return false;
    }

    // Если дошли до сюда, строка корректна
    return true;
}


// Проверка, является ли строка корректным оператором (пока простая проверка)
// Проверка, является ли строка корректным оператором (пока простая проверка)
bool SintaksisAnalyzer::isValidOperator(const std::string opLine, const int count_line) {
    std::string error_;
    int count_words = 0;
    // Разделяем строку на токены
    std::string* tokens = split(opLine, count_words);
    // Проверяем, что строка не пустая и содержит минимум "id = что-то"
    if (count_words < 3) {
        return false;
    }

    // Проверяем второй токен как оператор присваивания
    if (tokens[1] != "=") {
        return false;
    }

    // Проверяем первый токен как идентификатор
    if (!isValidIdentifier(tokens[0])) {
        error_ = "Id не соответствует правилам, встречен посторонний символ -> " + tokens[0];
        is_error_flag = true;
        error(count_line, opLine, error_);
        return false;
    }

    // Проверяем оставшиеся токены после '='
    bool expectingOperand = true; // Указывает, ожидаем ли операнд или оператор
    int openBrackets = 0;         // Счетчик открывающих скобок

    for (int i = 2; i < count_words; ++i) {
        if (tokens[i] == "(") {
            openBrackets++;
        }
        else if (tokens[i] == ")") {
            openBrackets--;
            // Проверяем, чтобы закрывающая скобка не была вне пары
            if (openBrackets < 0) {
                error_ = "Закрывающая скобка ')' без пары.";
                is_error_flag = true;
                error(count_line, opLine, error_);
                return false;
            }
        }
        else if (expectingOperand) {
            // Ожидаем операнд: идентификатор или выражение
            if (!isValidExpression(tokens[i]) && !isValidIdentifier(tokens[i])) {
                error_ = "Ожидается идентификатор или выражение, встречено -> " + tokens[i];
                is_error_flag = true;
                error(count_line, opLine, error_);
                return false;
            }
            expectingOperand = false; // Следующий токен должен быть оператором
        }
        else {
            // Ожидаем оператор: '+' или '-'
            if (tokens[i] != "+" && tokens[i] != "-") {
                error_ = "Ожидается оператор '+' или '-', встречено -> " + tokens[i];
                is_error_flag = true;
                error(count_line, opLine, error_);
                return false;
            }
            expectingOperand = true; // Следующий токен должен быть операндом
        }
    }

    // Если цикл завершился, но последним токеном был оператор, это ошибка
    if (expectingOperand) {
        error_ = "Строка заканчивается на оператор без операнда.";
        is_error_flag = true;
        error(count_line, opLine, error_);
        return false;
    }

    // Если после завершения цикла остались незакрытые скобки, это ошибка
    if (openBrackets > 0) {
        error_ = "Незакрытая скобка '(' обнаружена.";
        is_error_flag = true;
        error(count_line, opLine, error_);
        return false;
    }

    // Если дошли до сюда, строка корректна
    return true;
}


void SintaksisAnalyzer::building_tree(const int count_line, const std::string line) {
    if (is_error_flag) {
        clear_tree();
        std::cout << "An error has been detected, take a look at the file <errors.txt> to get acquainted." << "\n";
        throw std::ios_base::failure("An error was encountered in the input file.");
    }
    if ( is_cycle(line, count_line) ) {
        if (!is_valid_start) {
            clear_tree();
            error(1234, "", "Отсутствует старт программы(PROGRAM id_name)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        else if (!is_valid_descriptions) {
            clear_tree();
            error(1234, "", "Отсутствуют описания переменных(INTEGER VarList)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        types_lexeme[count_types_lexeme] = "operator";
        count_types_lexeme++;
        is_valid_operator = true;
        //draw_cycle(line, "");

        std::istringstream inputStream(line);
        std::string line_;
        while (std::getline(inputStream, line_)) {
            Parser::parseProgram(root, line_, level);
        }
    }
    else if ( is_start_program(line, count_line) ) {
        types_lexeme[count_types_lexeme] = "start";
        count_types_lexeme++;
        is_valid_start = true;
        //draw_start_program(line);

        std::istringstream inputStream(line);
        std::string line_;
        while (std::getline(inputStream, line_)) {
            Parser::parseProgram(root, line_, level);
        }
    }
    else if ( is_end_program(line, count_line) ) {
        if (!is_valid_start) {
            clear_tree();
            error(1234, "", "Отсутствует старт программы(PROGRAM id_name)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        else if (!is_valid_descriptions) {
            clear_tree();
            error(1234, "", "Отсутствуют описания переменных(INTEGER VarList)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        else if (!is_valid_operator) {
            clear_tree();
            error(1234, "", "Отсутствуют Operators");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        types_lexeme[count_types_lexeme] = "end";
        count_types_lexeme++;
        is_valid_end = true;
        //draw_end_program(line);

        std::istringstream inputStream(line);
        std::string line_;
        while (std::getline(inputStream, line_)) {
            Parser::parseProgram(root, line_, level);
        }
    }
    else if ( is_descriptions(line, count_line) ) {
        if (!is_valid_start) {
            clear_tree();
            error(1234, "", "Отсутствует старт программы(PROGRAM id_name)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        types_lexeme[count_types_lexeme] = "description";
        count_types_lexeme++;
        is_valid_descriptions = true;
        //draw_descriptions(line);

        std::istringstream inputStream(line);
        std::string line_;
        while (std::getline(inputStream, line_)) {
            Parser::parseProgram(root, line_, level);
        }
    }
    else if (isValidOperator(line, count_line)) {
        if (!is_valid_start) {
            clear_tree();
            error(1234, "", "Отсутствует старт программы(PROGRAM id_name)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        else if (!is_valid_descriptions) {
            clear_tree();
            error(1234, "", "Отсутствуют описания переменных(INTEGER VarList)");
            throw std::ios_base::failure("An error was encountered in the input file.");
        }
        types_lexeme[count_types_lexeme] = "operator";
        count_types_lexeme++;
        is_valid_operator = true;
        //draw_operators(line);

        std::istringstream inputStream("OPERATORS " + line);
        std::string line_;
        while (std::getline(inputStream, line_)) {
            Parser::parseProgram(root, line_, level);
        }
    }
    //else if (is_VarList(line)) {
    //    //std::cout << "TYTYTYTYTYTYTYTY" << "\n";
    //    draw_varlist(line);
    //}
    else {
        if (!is_error_flag && line != "") {
            int count_words = 0;
            std::string* splitting_line = split(line, count_words);
            if (count_words == 1) {
                is_error_flag = true;
                error(count_line, splitting_line[0], "Недопускается писать 1 переменную/число");
            }
            else {
                is_error_flag = true;
                error(count_line, line, "Нераспознанные индентификатор, проверьте его корректность.");
            }
        }
    }
    
}

void SintaksisAnalyzer::error(const int count_line, const std::string line, const std::string type_error) {
    // Режим добавления
    std::ofstream errorFile("errors.txt", std::ios::app);

    if (!errorFile.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл errors.txt для записи." << std::endl;
        return;
    }

    // Формируем строку с ошибкой
    errorFile << "Ошибка в строке #" << count_line << " - " << type_error << ": " << line << std::endl;

    errorFile.close();
}

std::vector<std::string> SintaksisAnalyzer::splitBySemicolonOrNewline(const std::string& line) {
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



bool SintaksisAnalyzer::is_cycle(const std::string line, const int count_line) {
    int count_words = 0;
    int is_do = -1;
    int is_to = -1;
    bool flag = false;
    std::string* lines = split(line, count_words); // Разбиваем строку на слова

    // Разбираем строку на ключевые слова
    for (int i = 0; i < count_words; ++i) {
        if (lines[i] == "TO") {
            if (is_to == -1) {
                flag = true;
                is_to = i;  // Присваиваем значение для TO
            }
        }
        else if (lines[i] == "DO") {
            if (is_do == -1) {
                flag = true;
                is_do = i;  // Присваиваем значение для DO
            }
        }
        else if (lines[i] == "for" || lines[i] == "to" || lines[i] == "do") {
            is_error_flag = true;
            error(count_line, line, "Ключевое слово (FOR, TO, DO) нужно писать капсом");
            return false;
        }
    }

    if (flag) {
        if (count_words < 7) { // Минимальная форма: "FOR i = 1 TO i + 10 DO"
            is_error_flag = true;
            error(count_line, line, "Не хватает символов для соответствия правилам лексемы (FOR ID = Expr TO Expr DO Operators)");
            return false;
        }
    }
    else {
        return false;
    }

    // Проверяем структуру "FOR ... TO ... DO"
    if (lines[0] != "FOR" || lines[is_to] != "TO" || lines[is_do] != "DO") {
        is_error_flag = true;
        error(count_line, line, "Не хватает ключевого слова FOR/TO/DO");
        return false;
    }

    // Проверяем выражение перед TO
    bool is_equal = false;
    int start = 0;
    std::vector<std::string> expressionParts1;
    for (int i = 1; i < is_to; ++i) {
        if (lines[i] == "=") is_equal = true;
        expressionParts1.push_back(lines[i]);
    }

    if (is_equal) {
        if (!isValidIdentifier(expressionParts1[0])) {
            is_error_flag = true;
            error(count_line, line, "Невалидная переменная");
        }
        start = 2;
    }

    bool expectingOperand = true;
    int openBrackets = 0;
    std::string error_;

    for (int i = start; i < expressionParts1.size(); ++i) {
        const std::string& token = expressionParts1[i];
        if (token == "(") openBrackets++;
        else if (token == ")") {
            openBrackets--;
            if (openBrackets < 0) {
                error_ = "Закрывающая скобка ')' без пары.";
                is_error_flag = true;
                error(count_line, line, error_);
                return false;
            }
        }
        else if (expectingOperand) {
            if (!isValidExpression(token) && !isValidIdentifier(token)) {
                error_ = "Ожидается идентификатор или выражение, встречено -> " + token;
                is_error_flag = true;
                error(count_line, line, error_);
                return false;
            }
            expectingOperand = false;
        }
        else {
            if (token != "+" && token != "-") {
                error_ = "Ожидается оператор '+' или '-', встречено -> " + token;
                is_error_flag = true;
                error(count_line, line, error_);
                return false;
            }
            expectingOperand = true;
        }
    }

    if (expectingOperand) {
        error_ = "Строка заканчивается на оператор без операнда.";
        is_error_flag = true;
        error(count_line, line, error_);
        return false;
    }

    if (openBrackets > 0) {
        error_ = "Незакрытая скобка '(' обнаружена.";
        is_error_flag = true;
        error(count_line, line, error_);
        return false;
    }

    // Проверяем выражение между TO и DO
    bool is_equal_ = false;
    int start_ = 0;
    std::vector<std::string> expressionParts2;
    for (int i = is_to + 1; i < is_do; ++i) {
        if (lines[i] == "=") is_equal_ = true;
        expressionParts2.push_back(lines[i]);
    }

    if (is_equal_) {
        if (!isValidIdentifier(expressionParts2[0])) {
            is_error_flag = true;
            error(count_line, line, "Невалидная переменная");
        }
        start_ = 2;
    }

    bool expectingOperand_ = true;
    int openBrackets_ = 0;
    std::string error__;

    for (int i = start_; i < expressionParts2.size(); ++i) {
        const std::string& token = expressionParts2[i];
        if (token == "(") openBrackets_++;
        else if (token == ")") {
            openBrackets_--;
            if (openBrackets_ < 0) {
                error__ = "Закрывающая скобка ')' без пары.";
                is_error_flag = true;
                error(count_line, line, error__);
                return false;
            }
        }
        else if (expectingOperand_) {
            if (!isValidExpression(token) && !isValidIdentifier(token)) {
                error__ = "Ожидается идентификатор или выражение, встречено -> " + token;
                is_error_flag = true;
                error(count_line, line, error__);
                return false;
            }
            expectingOperand_ = false;
        }
        else {
            if (token != "+" && token != "-") {
                error__ = "Ожидается оператор '+' или '-', встречено -> " + token;
                is_error_flag = true;
                error(count_line, line, error__);
                return false;
            }
            expectingOperand_ = true;
        }
    }

    if (expectingOperand_) {
        error__ = "Строка заканчивается на оператор без операнда.";
        is_error_flag = true;
        error(count_line, line, error__);
        return false;
    }

    if (openBrackets_ > 0) {
        error__ = "Незакрытая скобка '(' обнаружена.";
        is_error_flag = true;
        error(count_line, line, error__);
        return false;
    }

    // Проверка оператора после DO
    std::string operatorLine = "";
    for (int i = is_do + 1; i < count_words; ++i) {
        operatorLine += lines[i] + " ";
    }
    operatorLine = operatorLine.substr(0, operatorLine.size() - 1); // Убираем лишний пробел

    // Проверка на вложенный цикл
    while (operatorLine.find("FOR") == 0) {  // Если после DO идет новый цикл
        bool result = is_cycle(operatorLine, count_line);
        if (!result) {
            is_error_flag = true;
            error(count_line, line, "Ошибка во вложенном цикле после DO");
            return false;
        }
        operatorLine = operatorLine.substr(operatorLine.find("DO") + 2);  // Убираем этот цикл и продолжаем
    }

    // Разбиваем строку после DO на отдельные выражения
    std::vector<std::string> expressions = splitBySemicolonOrNewline(operatorLine);

    // Проверка каждого выражения
    for (const std::string& expr : expressions) {
        if (!isValidOperator(expr, count_line)) {
            is_error_flag = true;
            error(count_line, line, "Оператор после DO несоответствует правилам.");
            return false;
        }
    }

    return true; // Строка прошла все проверки
}









// Проверяет, соответствует ли строка началу программы (PROGRAM <идентификатор>)
bool SintaksisAnalyzer::is_start_program(const std::string line, const int count_line) {
    int count_words = 0;
    std::string* words = split(line, count_words);

    // Проверка: строка должна состоять ровно из двух слов
    if (count_words != 2) {
        return false;
    }

    // Первое слово должно быть "PROGRAM", второе - валидным идентификатором
    if (words[0] == "PROGRAM" && isValidIdentifier(words[1])) {
        return true;
    }

    return false;
}

// Проверяет, соответствует ли строка завершению программы (END PROGRAM <идентификатор>)
bool SintaksisAnalyzer::is_end_program(const std::string line, const int count_line) {
    int count_words = 0;
    std::string* words = split(line, count_words);

    // Проверка: строка должна состоять ровно из трех слов
    if (count_words != 2) {
        return false;
    }

    // Первое слово должно быть "END", второе - "PROGRAM", третье - валидным идентификатором
    if (words[0] == "END" && isValidIdentifier(words[1])) {
        return true;
    }

    return false;
}

bool SintaksisAnalyzer::is_descriptions(const std::string line, const int count_line) {
    int count_words = 0;
    std::string* words = split(line, count_words);

    // Проверка: строка должна начинаться с ключевого слова "INTEGER"
    if (count_words < 2 || words[0] != "INTEGER") {
        if (words[0] == "INTEGER") {
            is_error_flag = true;
            error(count_line, line, "Нет VarList после INTEGER");
            is_error_flag = true;
        }
        return false;
    }

    // Если строка состоит из ровно двух слов, проверяем только один идентификатор
    if (count_words == 2) {
        // Проверяем валидность идентификатора или выражения
        if (!isValidIdentifier(words[1]) && !isValidExpression(words[1])) {
            is_error_flag = true;
            error(count_line, line, "Недопустимый идентификатор или выражение: " + words[1]);
            return false;
        }
        return true;  // Если проверка прошла, то возвращаем true
    }

    // Если строка содержит несколько переменных, проверяем их по очереди
    for (int i = 1; i < count_words; ++i) {
        // Если это не последний элемент, проверяем, что после него идет запятая
        if (i < count_words - 1 && words[i].back() != ',') {
            is_error_flag = true;
            error(count_line, line, "Пропущена запятая между элементами: " + words[i]);
            return false;
        }

        // Убираем запятую, если она есть в конце слова
        if (words[i].back() == ',') {
            words[i].erase(words[i].size() - 1); // Убираем последнюю запятую
        }

        // Проверяем валидность идентификатора или выражения
        if (!isValidIdentifier(words[i]) && !isValidExpression(words[i])) {
            is_error_flag = true;
            error(count_line, line, "Недопустимый идентификатор или выражение: " + words[i]);
            return false;
        }
    }
    return true;
}








