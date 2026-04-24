/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);
    if (!scanner.hasMoreTokens()) return;
    std::string first = scanner.nextToken();
    TokenType tt = scanner.getTokenType(first);
    // Numbered line handling
    if (tt == NUMBER) {
        int lineNumber = stringToInteger(first);
        // fetch rest of line text preserving original formatting after space
        // The input format guarantees proper spacing per README
        // If there are no further tokens, remove the line
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return;
        }
        // reconstruct remaining text from the original line after first space
        // We can get the remainder by trimming original line and removing leading number and a space
        // Simple approach: find first space and take substring after it
        size_t pos = line.find(' ');
        std::string rest = (pos == std::string::npos) ? std::string() : line.substr(pos + 1);
        // Store the full original line to preserve formatting in LIST
        program.addSourceLine(lineNumber, line);
        // parse statement and store
        TokenScanner stmtScanner;
        stmtScanner.ignoreWhitespace();
        stmtScanner.scanNumbers();
        stmtScanner.setInput(rest);
        std::string kw = stmtScanner.nextToken();
        std::string upkw = toUpperCase(kw);
        Statement *stmt = nullptr;
        if (upkw == "REM") {
            stmt = new RemStatement(stmtScanner);
        } else if (upkw == "LET") {
            stmt = new LetStatement(stmtScanner);
        } else if (upkw == "PRINT") {
            stmt = new PrintStatement(stmtScanner);
        } else if (upkw == "INPUT") {
            stmt = new InputStatement(stmtScanner);
        } else if (upkw == "END") {
            if (stmtScanner.hasMoreTokens()) error("SYNTAX ERROR");
            stmt = new EndStatement(stmtScanner);
        } else if (upkw == "GOTO") {
            stmt = new GotoStatement(stmtScanner);
        } else if (upkw == "IF") {
            stmt = new IfStatement(stmtScanner);
        } else {
            error("SYNTAX ERROR");
        }
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    // Immediate commands
    std::string cmd = toUpperCase(first);
    if (cmd == "REM") {
        // ignore rest
        return;
    } else if (cmd == "LET") {
        Statement *s = new LetStatement(scanner);
        s->execute(state, program);
        delete s;
    } else if (cmd == "PRINT") {
        Statement *s = new PrintStatement(scanner);
        s->execute(state, program);
        delete s;
    } else if (cmd == "INPUT") {
        Statement *s = new InputStatement(scanner);
        s->execute(state, program);
        delete s;
    } else if (cmd == "END") {
        // In immediate mode, END is syntax error per demo
        error("SYNTAX ERROR");
    } else if (cmd == "GOTO" || cmd == "IF") {
        // Not allowed in immediate mode per demo
        error("SYNTAX ERROR");
    } else if (cmd == "LIST") {
        // print stored lines in ascending order with original source numbers formatting preserved
        int ln = program.getFirstLineNumber();
        while (ln != -1) {
            std::string src = program.getSourceLine(ln);
            std::cout << src << std::endl;
            ln = program.getNextLineNumber(ln);
        }
    } else if (cmd == "CLEAR") {
        program.clear();
        state.Clear();
    } else if (cmd == "RUN") {
        // execute from first line; keep current variable state
        int current = program.getFirstLineNumber();
        program.setEnd(false);
        while (current != -1) {
            Statement *st = program.getParsedStatement(current);
            if (!st) {
                // parse on the fly from source
                std::string src = program.getSourceLine(current);
                TokenScanner stmtScanner;
                stmtScanner.ignoreWhitespace();
                stmtScanner.scanNumbers();
                // parse from the part after line number
                size_t p = src.find(' ');
                std::string body = (p == std::string::npos) ? std::string() : src.substr(p + 1);
                stmtScanner.setInput(body);
                std::string kw = stmtScanner.nextToken();
                std::string upkw = toUpperCase(kw);
                if (upkw == "REM") st = new RemStatement(stmtScanner);
                else if (upkw == "LET") st = new LetStatement(stmtScanner);
                else if (upkw == "PRINT") st = new PrintStatement(stmtScanner);
                else if (upkw == "INPUT") st = new InputStatement(stmtScanner);
                else if (upkw == "END") st = new EndStatement(stmtScanner);
                else if (upkw == "GOTO") st = new GotoStatement(stmtScanner);
                else if (upkw == "IF") st = new IfStatement(stmtScanner);
                else error("SYNTAX ERROR");
            }
            st->execute(state, program);
            if (program.isEnded()) break;
            int jump = program.consumeRequestedJump();
            if (jump != -1) {
                current = jump;
            } else {
                current = program.getNextLineNumber(current);
            }
        }
    } else if (cmd == "QUIT") {
        exit(0);
    } else if (cmd == "HELP") {
        // optional, ignore
        return;
    } else {
        error("SYNTAX ERROR");
    }
}
