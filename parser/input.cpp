#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_set>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

struct Token {
    string type;
    string value;
};

vector<Token> tokens;
int current = 0;

// Language definitions
unordered_set<string> keywords = {
    "int", "float", "char", "double", "void", "return", "if", "else", "while", "for", "switch", "case", "default", "break"
};

unordered_set<char> operators = {'+', '-', '*', '/', '=', '<', '>', '!', '&', '|', '%'};
unordered_set<char> delimiters = {'(', ')', '{', '}', '[', ']', ';', ',', ':'};

// === Lexical Analysis ===
bool isKeyword(const string& s) {
    return keywords.find(s) != keywords.end();
}

bool isOperator(char c) {
    return operators.find(c) != operators.end();
}

bool isDelimiter(char c) {
    return delimiters.find(c) != delimiters.end();
}

void lexicalAnalyse(const string& code) {
    int i = 0;
    while (i < (int)code.length()) {
        if (isspace(code[i])) {
            i++;
            continue;
        }

        if (isalpha(code[i]) || code[i] == '_') {
            string token = "";
            while (i < (int)code.length() && (isalnum(code[i]) || code[i] == '_')) token += code[i++];
            if (isKeyword(token)) tokens.push_back({"KEYWORD", token});
            else tokens.push_back({"IDENTIFIER", token});
        } else if (isdigit(code[i])) {
            string number = "";
            bool dotSeen = false;
            while (i < (int)code.length() && (isdigit(code[i]) || (!dotSeen && code[i] == '.'))) {
                if (code[i] == '.') dotSeen = true;
                number += code[i++];
            }
            tokens.push_back({"NUMBER", number});
        } else if (isOperator(code[i])) {
            string op(1, code[i]);
            if (i + 1 < (int)code.length() && (code[i] == '=' || code[i] == '!' || code[i] == '<' || code[i] == '>') && code[i+1] == '=') {
                op += code[i+1];
                i += 2;
            } else {
                i++;
            }
            tokens.push_back({"OPERATOR", op});
        } else if (isDelimiter(code[i])) {
            tokens.push_back({"DELIMITER", string(1, code[i])});
            i++;
        } else {
            tokens.push_back({"UNKNOWN", string(1, code[i])});
            i++;
        }
    }

    tokens.push_back({"END", ""});
}

// === Syntax Analysis ===
Token peek() {
    return current < (int)tokens.size() ? tokens[current] : Token{"END", ""};
}

Token get() {
    return current < (int)tokens.size() ? tokens[current++] : Token{"END", ""};
}

bool match(const string& type, const string& value = "") {
    if (peek().type == type && (value.empty() || peek().value == value)) {
        get();
        return true;
    }
    return false;
}

void error(const string& message) {
    cout << "Syntax Error: " << message << " near '" << peek().value << "'\n";
    exit(1);
}

void Expression();
void Statement();
void Block();
void Condition();
void Switch();

void Factor() {
    if (peek().type == "NUMBER" || peek().type == "IDENTIFIER") get();
    else if (match("DELIMITER", "(")) {
        Expression();
        if (!match("DELIMITER", ")")) error("Expected ')'");
    } else error("Expected number, identifier, or '('");
}

void Term() {
    Factor();
    while (peek().value == "*" || peek().value == "/") {
        get(); Factor();
    }
}

void Expression() {
    Term();
    while (peek().value == "+" || peek().value == "-") {
        get(); Term();
    }
}

void Condition() {
    Expression();
    if (peek().type == "OPERATOR" && (peek().value == "<" || peek().value == ">" || peek().value == "==" || peek().value == "!=" || peek().value == "<=" || peek().value == ">=")) {
        get(); Expression();
    } else error("Expected relational operator in condition");
}

void DeclarationOrAssignment() {
    // The first token is type keyword
    if (!match("KEYWORD")) error("Expected type keyword");
    if (!match("IDENTIFIER")) error("Expected identifier");
    if (match("OPERATOR", "=")) Expression();
    if (!match("DELIMITER", ";")) error("Expected ';'");
}

void Assignment() {
    if (!match("IDENTIFIER")) error("Expected identifier");
    if (!match("OPERATOR", "=")) error("Expected '='");
    Expression();
    if (!match("DELIMITER", ";")) error("Expected ';'");
}

void IfStatement() {
    if (!match("KEYWORD", "if")) error("Expected 'if'");
    if (!match("DELIMITER", "(")) error("Expected '(' after if");
    Condition();
    if (!match("DELIMITER", ")")) error("Expected ')'");
    Block();
    if (match("KEYWORD", "else")) {
        Block();
    }
}

void WhileLoop() {
    if (!match("KEYWORD", "while")) error("Expected 'while'");
    if (!match("DELIMITER", "(")) error("Expected '(' after while");
    Condition();
    if (!match("DELIMITER", ")")) error("Expected ')'");
    Block();
}

void ForLoop() {
    if (!match("KEYWORD", "for")) error("Expected 'for'");
    if (!match("DELIMITER", "(")) error("Expected '(' after for");

    // For loop init: can be declaration, assignment, or empty
    if (peek().type == "KEYWORD") {
        DeclarationOrAssignment();
    } else if (peek().type == "IDENTIFIER") {
        Assignment();
    } else if (match("DELIMITER", ";")) {
        // empty init
    } else {
        error("Expected initialization in for loop");
    }

    // Condition (optional)
    if (peek().type != "DELIMITER" || peek().value != ";") {
        Condition();
    }
    if (!match("DELIMITER", ";")) error("Expected ';' after for loop condition");

    // Update (optional)
    if (peek().type != "DELIMITER" || peek().value != ")") {
        if (peek().type == "IDENTIFIER") {
            // for update, e.g., i = i + 1
            if (!match("IDENTIFIER")) error("Expected identifier in for update");
            if (!match("OPERATOR", "=")) error("Expected '=' in for update");
            Expression();
        } else {
            error("Expected update expression in for loop");
        }
    }

    if (!match("DELIMITER", ")")) error("Expected ')' to close for loop header");
    Block();
}

void Switch() {
    if (!match("KEYWORD", "switch")) error("Expected 'switch'");
    if (!match("DELIMITER", "(")) error("Expected '(' after switch");
    if (!match("IDENTIFIER")) error("Expected identifier in switch");
    if (!match("DELIMITER", ")")) error("Expected ')'");
    if (!match("DELIMITER", "{")) error("Expected '{'");

    while (peek().type == "KEYWORD" && peek().value == "case") {
        get(); // consume 'case'
        if (peek().type != "NUMBER" && peek().type != "IDENTIFIER") error("Expected case value");
        get(); // consume case value
        if (!match("DELIMITER", ":")) error("Expected ':' after case value");

        while (!(peek().type == "KEYWORD" && (peek().value == "case" || peek().value == "default")) && !(peek().type == "DELIMITER" && peek().value == "}")) {
            Statement();
        }
    }

    if (peek().type == "KEYWORD" && peek().value == "default") {
        get(); // consume 'default'
        if (!match("DELIMITER", ":")) error("Expected ':' after default");
        while (!(peek().type == "DELIMITER" && peek().value == "}")) {
            Statement();
        }
    }

    if (!match("DELIMITER", "}")) error("Expected '}' to end switch");
}

void Block() {
    if (match("DELIMITER", "{")) {
        while (!match("DELIMITER", "}")) {
            Statement();
        }
    } else {
        Statement();
    }
}

void Statement() {
    if (peek().type == "KEYWORD") {
        string kw = peek().value;
        if (kw == "int" || kw == "float" || kw == "char" || kw == "double") DeclarationOrAssignment();
        else if (kw == "void") error("Void type cannot be used for variable declaration");
        else if (kw == "if") IfStatement();
        else if (kw == "while") WhileLoop();
        else if (kw == "for") ForLoop();
        else if (kw == "switch") Switch();
        else if (kw == "break") {
            get();
            if (!match("DELIMITER", ";")) error("Expected ';' after break");
        }
        else error("Unknown keyword");
    } else if (peek().type == "IDENTIFIER") {
        Assignment();
    } else {
        error("Unexpected token");
    }
}

void Program() {
    while (peek().type != "END") {
        Statement();
    }
    cout << "\n✅ Syntax is correct.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./a.out <inputfile>\n";
        return 1;
    }

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cout << "Error opening file.\n";
        return 1;
    }

    string code((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    lexicalAnalyse(code);

    // Prepare JSON token output
    json j;
    for (const auto& token : tokens) {
        if (token.type == "IDENTIFIER" || token.type == "KEYWORD")
            j["symbols"].push_back({{"type", token.type}, {"name", token.value}});
    }

    // Save to JSON
    ofstream out("symbols.json");
    out << j.dump(4);
    out.close();

    return 0;
}
