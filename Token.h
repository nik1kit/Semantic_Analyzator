#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <fstream>

enum class TokenType {
    PROGRAM,        // Program -> Begin Descriptions Operators End
    BEGIN,          // Begin -> PROGRAM Id
    END,            // End -> END Id
    DESCRIPTIONS,   // Descriptions -> Descr | Descr Descriptions
    TYPE,           // Type -> INTEGER
    ID_NAME,        // Идентификатор
    INT_NUM,        // Целое число
    OPERATOR,       // Операторы
    DELIMITER,      // Разделители
    ERROR,          // Ошибки
    UNKNOWN,        // Неизвестный токен
    OP,
    END_LINE,
};

class Token {
public:
    TokenType type;
    std::string lexeme;
    int index;
    Token() : type(TokenType::UNKNOWN), lexeme(""), index(-1) {}

    Token(TokenType t, const std::string& lex, int idx) : type(t), lexeme(lex), index(idx) {}
};


#endif 