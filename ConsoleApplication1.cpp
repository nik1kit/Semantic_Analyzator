#include <iostream>
#include <fstream>
#include "LexicalAnalyzer.h"
#include "SintaksisAnalyzer.h"
#include "Token.h"
#include "TokenList.h"

int main() {
    LexicalAnalyzer lexer("input.txt", "output.txt");
    lexer.analyze();
    return 0;
}