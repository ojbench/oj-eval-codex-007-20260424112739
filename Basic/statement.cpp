/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
using std::string;


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// REM
RemStatement::RemStatement(TokenScanner &scanner) {
    // consume the rest tokens silently
    (void) scanner;
}

void RemStatement::execute(EvalState &state, Program &program) {
    (void) state;
    (void) program;
}

// LET
LetStatement::LetStatement(TokenScanner &scanner) {
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    // parse assignment as an expression allowing '=' handled by parser/exp
    exp = parseExp(scanner);
}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    (void) program;
    if (!exp) return;
    exp->eval(state);
}

// PRINT
PrintStatement::PrintStatement(TokenScanner &scanner) {
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    exp = parseExp(scanner);
}

PrintStatement::~PrintStatement() { delete exp; }

void PrintStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int v = exp ? exp->eval(state) : 0;
    std::cout << v << std::endl;
}

// INPUT
InputStatement::InputStatement(TokenScanner &scanner) {
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    if (!scanner.hasMoreTokens()) error("SYNTAX ERROR");
    string tk = scanner.nextToken();
    TokenType tp = scanner.getTokenType(tk);
    if (tp != WORD) error("SYNTAX ERROR");
    var = tk;
}

void InputStatement::execute(EvalState &state, Program &program) {
    (void) program;
    // prompt " ? " without newline, then read integer with retry on invalid
    while (true) {
        std::cout << " ? ";
        std::cout.flush();
        string s;
        if (!std::getline(std::cin, s)) {
            // EOF; treat as 0
            state.setValue(var, 0);
            return;
        }
        try {
            int val = stringToInteger(trim(s));
            state.setValue(var, val);
            return;
        } catch (ErrorException &e) {
            (void) e;
            std::cout << "INVALID NUMBER" << std::endl;
        }
    }
}

// END
EndStatement::EndStatement(TokenScanner &scanner) {
    (void) scanner;
}

void EndStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.setEnd(true);
}

// GOTO
GotoStatement::GotoStatement(TokenScanner &scanner) {
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    if (!scanner.hasMoreTokens()) error("SYNTAX ERROR");
    string tk = scanner.nextToken();
    if (scanner.getTokenType(tk) != NUMBER) error("SYNTAX ERROR");
    target = stringToInteger(tk);
}

void GotoStatement::execute(EvalState &state, Program &program) {
    (void) state;
    if (!program.hasLine(target)) error("LINE NUMBER ERROR");
    program.requestJump(target);
}

// IF ... THEN line
IfStatement::IfStatement(TokenScanner &scanner) {
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    lhs = readE(scanner, 0);
    if (!scanner.hasMoreTokens()) error("SYNTAX ERROR");
    op = scanner.nextToken();
    if (op != "<" && op != ">" && op != "=" ) error("SYNTAX ERROR");
    rhs = readE(scanner, 0);
    if (!scanner.hasMoreTokens()) error("SYNTAX ERROR");
    string tk = scanner.nextToken();
    if (tk != "THEN") error("SYNTAX ERROR");
    if (!scanner.hasMoreTokens()) error("SYNTAX ERROR");
    tk = scanner.nextToken();
    if (scanner.getTokenType(tk) != NUMBER) error("SYNTAX ERROR");
    target = stringToInteger(tk);
}

IfStatement::~IfStatement() {
    delete lhs;
    delete rhs;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int lv = lhs->eval(state);
    int rv = rhs->eval(state);
    bool cond = false;
    if (op == "<") cond = lv < rv;
    else if (op == ">") cond = lv > rv;
    else if (op == "=") cond = lv == rv;
    if (cond) {
        if (!program.hasLine(target)) error("LINE NUMBER ERROR");
        program.requestJump(target);
    }
}
