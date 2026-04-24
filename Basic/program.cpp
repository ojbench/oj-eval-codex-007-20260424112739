/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"



Program::Program() = default;

Program::~Program() = default;

void Program::clear() {
    for (auto &kv : lines) {
        delete kv.second.second;
    }
    lines.clear();
    jumpTo = -1;
    endFlag = false;
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    auto it = lines.find(lineNumber);
    if (it != lines.end()) {
        // replace source, delete old parsed stmt
        if (it->second.second) {
            delete it->second.second;
            it->second.second = nullptr;
        }
        it->second.first = line;
    } else {
        lines.emplace(lineNumber, std::make_pair(line, (Statement *) nullptr));
    }
}

void Program::removeSourceLine(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it != lines.end()) {
        if (it->second.second) delete it->second.second;
        lines.erase(it);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it == lines.end()) return "";
    return it->second.first;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    auto it = lines.find(lineNumber);
    if (it == lines.end()) {
        // invalid per spec
        error("SET PARSED ON NONEXISTENT LINE");
    }
    if (it->second.second) delete it->second.second;
    it->second.second = stmt;
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
   auto it = lines.find(lineNumber);
   if (it == lines.end()) return nullptr;
   return it->second.second;
}

int Program::getFirstLineNumber() {
    if (lines.empty()) return -1;
    return lines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it == lines.end()) return -1;
    ++it;
    if (it == lines.end()) return -1;
    return it->first;
}

void Program::requestJump(int lineNumber) {
    jumpTo = lineNumber;
}

int Program::consumeRequestedJump() {
    int t = jumpTo;
    jumpTo = -1;
    return t;
}

void Program::setEnd(bool ended) {
    endFlag = ended;
}

bool Program::isEnded() const {
    return endFlag;
}

bool Program::hasLine(int lineNumber) const {
    return lines.find(lineNumber) != lines.end();
}

