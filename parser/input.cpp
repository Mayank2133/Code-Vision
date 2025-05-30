#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_set>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

struct ParseNode {
    string name;
    vector<ParseNode> children;
};

json nodeToJson(const ParseNode &node) {
    json j;
    j["name"] = node.name;
    j["children"] = json::array(); // Always create the children field
    for (const auto& child : node.children) {
            j["children"].push_back(nodeToJson(child));
        }
    return j;
}

struct Token {
    string type;
    string value;
};

vector<Token> tokens;
int current = 0;
ParseNode parseTreeRoot;

// Language definitions
unordered_set<string> keywords = {
    // Original keywords
    "int", "float", "char", "double", "void", "return", "if", "else", 
    "while", "for", "switch", "case", "default", "break",
    
    // Added C++ keywords
    "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel",
    "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", 
    "bool", "catch", "class", "co_await", "co_return", "co_yield",
    "compl", "concept", "const", "consteval", "constexpr", "constinit",
    "const_cast", "continue", "decltype", "delete", "do", "dynamic_cast",
    "enum", "explicit", "export", "extern", "false", "friend", "goto",
    "import", "inline", "long", "module", "mutable", "namespace", "new",
    "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq",
    "private", "protected", "public", "reflexpr", "register", "reinterpret_cast",
    "requires", "short", "signed", "sizeof", "static", "static_assert",
    "static_cast", "struct", "synchronized", "template", "this", "thread_local",
    "throw", "true", "try", "typedef", "typeid", "typename", "union", 
    "unsigned", "using", "virtual", "volatile", "wchar_t", "xor", "xor_eq",
    "cout", "cin"
    // C++20 concepts
    "requires",
    
    // C++23 keywords
    "char8_t", "char16_t", "char32_t", "constexpr", "consteval", 
    "constinit", "typename"
};
unordered_set<char> operators = {
    // Arithmetic
    '+', '-', '*', '/', '%',
    // Comparison/Logical
    '=', '<', '>', '!', '&', '|',
    // Bitwise
    '^', '~',
    // Other
    '?'
};

unordered_set<char> delimiters = {
    // Brackets
    '(', ')', '{', '}', '[', ']',
    // Statement/expression
    ';', ',', ':', 
    // Preprocessor
    '#',
    // Member access
    '.',
    // Literal delimiters
    '\'', '"'
};
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

        // Handle string literals
        if (code[i] == '"') {
            string str = "";
            i++; // Skip opening "
            while (i < code.length() && code[i] != '"') {
                str += code[i++];
            }
            if (i < code.length()) i++; // Skip closing "
            tokens.push_back({"STRING", str});
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
             // Check for multi-character operators: << or >>
            if (i + 1 < (int)code.length() && (code[i] == '<' || code[i] == '>') && code[i+1] == code[i]) {
                op += code[i+1];
                i += 2;
            }
            else if (i + 1 < (int)code.length() && (code[i] == '=' || code[i] == '!' || code[i] == '<' || code[i] == '>') && code[i+1] == '=') {
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

ParseNode Expression();
ParseNode Statement();
ParseNode Block();
ParseNode Condition();
ParseNode Switch();
ParseNode FunctionDefinition();
ParseNode Factor() {
    ParseNode node{"Factor"};
    if (peek().type == "NUMBER" || peek().type == "IDENTIFIER" || peek().type == "STRING") {
        Token t = get();
        node.children.push_back(ParseNode{t.value});
    } else if (match("DELIMITER", "(")) {
        node.children.push_back(Expression());
        if (!match("DELIMITER", ")")) error("Expected ')'");
    } else {
        error("Expected number, identifier, string, or '('");
    }
    return node;
}

ParseNode Term() {
    ParseNode node{"Term"};
    node.children.push_back(Factor());
    
    while (peek().value == "*" || peek().value == "/") {
        Token op = get();
        ParseNode opNode{"Operator: " + op.value};
        node.children.push_back(opNode);
        node.children.push_back(Factor());
    }
    return node;
}

ParseNode Expression() {
    ParseNode node{"Expression"};
    node.children.push_back(Term());
    
    while (peek().value == "+" || peek().value == "-") {
        Token op = get();
        ParseNode opNode{"Operator: " + op.value};
        node.children.push_back(opNode);
        node.children.push_back(Term());
    }
    return node;
}

ParseNode Condition() {
    ParseNode node{"Condition"};
    node.children.push_back(Expression());
    if (peek().type == "OPERATOR" && (peek().value == "<" || peek().value == ">" || peek().value == "==" || peek().value == "!=" || peek().value == "<=" || peek().value == ">=")) {
        Token op = get();
        ParseNode opNode{"Operator: " + op.value};
        node.children.push_back(opNode);
        node.children.push_back(Expression());
    } else error("Expected relational operator in condition");
    return node;
}

ParseNode DeclarationOrAssignment() {
    ParseNode node{"DeclarationOrAssignment"};
    // The first token is type keyword
    if (!match("KEYWORD")) error("Expected type keyword");
    node.children.push_back(ParseNode{"Type"});
    if (!match("IDENTIFIER")) error("Expected identifier");
    node.children.push_back(ParseNode{"Identifier"});

    if (match("OPERATOR", "=")){
        node.children.push_back(ParseNode{"="});
        node.children.push_back(Expression());
    }
    if (!match("DELIMITER", ";")) error("Expected ';'");
    return node;
}

ParseNode Assignment() {
    ParseNode node{"Assignment"};
    if (!match("IDENTIFIER")) error("Expected identifier");
    node.children.push_back(ParseNode{"Identifier"});
    if (!match("OPERATOR", "=")) error("Expected '='");
    node.children.push_back(ParseNode{"="});

    node.children.push_back(Expression());
    if (!match("DELIMITER", ";")) error("Expected ';'");
    return node;
}

ParseNode IfStatement() {
    ParseNode node{"IfStatement"};
    if (!match("KEYWORD", "if")) error("Expected 'if'");
    node.children.push_back({"if"});
    if (!match("DELIMITER", "(")) error("Expected '(' after if");
    node.children.push_back(Condition());
    if (!match("DELIMITER", ")")) error("Expected ')'");
    node.children.push_back(Block());
    if (match("KEYWORD", "else")) {
        node.children.push_back({"else"});
        node.children.push_back(Block());
    }
    return node;
}
ParseNode WhileLoop() {
    ParseNode node{"WhileLoop"};
    if (!match("KEYWORD", "while")) error("Expected 'while'");
    node.children.push_back({"while"});
    if (!match("DELIMITER", "(")) error("Expected '(' after while");
    node.children.push_back(Condition());
    if (!match("DELIMITER", ")")) error("Expected ')'");
    node.children.push_back(Block());
    return node;
}

ParseNode ForLoop() {
    ParseNode node{"ForLoop"};
    if (!match("KEYWORD", "for")) error("Expected 'for'");
    node.children.push_back({"for"});
    if (!match("DELIMITER", "(")) error("Expected '(' after for");

    if (peek().type == "KEYWORD") {
        node.children.push_back(DeclarationOrAssignment());
    } else if (peek().type == "IDENTIFIER") {
        node.children.push_back(Assignment());
    } else if (match("DELIMITER", ";")) {
        node.children.push_back({"EmptyInit"});
    } else {
        error("Expected for loop initialization");
    }

    if (peek().type != "DELIMITER" || peek().value != ";") {
        node.children.push_back(Condition());
    }
    if (!match("DELIMITER", ";")) error("Expected ';' after condition");

    if (peek().type != "DELIMITER" || peek().value != ")") {
        if (peek().type == "IDENTIFIER") {
            node.children.push_back(Assignment());
        } else {
            error("Expected update expression");
        }
    }
    if (!match("DELIMITER", ")")) error("Expected ')' after for");
    node.children.push_back(Block());
    return node;
}




ParseNode SwitchStatement() {
    ParseNode node{"SwitchStatement"};
    if (!match("KEYWORD", "switch")) error("Expected 'switch'");
    node.children.push_back({"switch"});
    if (!match("DELIMITER", "(")) error("Expected '(' after switch");
    node.children.push_back(Expression());
    if (!match("DELIMITER", ")")) error("Expected ')' after switch expression");
    if (!match("DELIMITER", "{")) error("Expected '{' after switch") ;

    while (peek().type == "KEYWORD" && peek().value == "case") {
        get();
        ParseNode caseNode{"case"};
        if (!(peek().type == "NUMBER" || peek().type == "IDENTIFIER")) error("Expected case value");
        Token val = get();
        caseNode.children.push_back({val.value});
        if (!match("DELIMITER", ":")) error("Expected ':' after case value");
        while (!(peek().type == "KEYWORD" && (peek().value == "case" || peek().value == "default")) && !(peek().type == "DELIMITER" && peek().value == "}")) {
            caseNode.children.push_back(Statement());
        }
        node.children.push_back(caseNode);
    }

    if (peek().type == "KEYWORD" && peek().value == "default") {
        get();
        ParseNode defaultNode{"default"};
        if (!match("DELIMITER", ":")) error("Expected ':' after default");
        while (!(peek().type == "DELIMITER" && peek().value == "}")) {
            defaultNode.children.push_back(Statement());
        }
        node.children.push_back(defaultNode);
    }

    if (!match("DELIMITER", "}")) error("Expected '}' to end switch block");

    return node;
}
ParseNode Block() {
    ParseNode node{"Block"};
    if (match("DELIMITER", "{")) {
        while (!match("DELIMITER", "}")) {
             node.children.push_back(Statement());
        }
    } else {
        node.children.push_back(Statement());
    }
    return node;
}

ParseNode StreamStatement() {
    Token streamToken = get(); // consume 'cout' or 'cin'
    ParseNode node{"StreamStatement: " + streamToken.value};

    string expectedOp = (streamToken.value == "cout") ? "<<" : ">>";

    do {
        if (!match("OPERATOR", expectedOp)) {
            error("Expected '" + expectedOp + "' after " + streamToken.value);
        }
        ParseNode expr = Expression();
        node.children.push_back(expr);
    } while (peek().value == expectedOp);

    if (!match("DELIMITER", ";")) {
        error("Expected ';' after " + streamToken.value + " statement");
    }

    return node;
}


ParseNode Statement() {
    ParseNode node{"Statement"};

     
    if (peek().type == "KEYWORD") {
        string kw = peek().value;

        if (kw == "int" || kw == "float" || kw == "char" || kw == "double") {
             node.children.push_back(DeclarationOrAssignment());
        }
        else if (kw == "cout" || kw == "cin") {
            node.children.push_back(StreamStatement());

        }
        else if (kw == "void") {
            error("Void type cannot be used for variable declaration");
        }
        else if (kw == "if") {
            node.children.push_back(IfStatement());
        }
        else if (kw == "while") {
            node.children.push_back(WhileLoop());
        }
        else if (kw == "for") {
            node.children.push_back(ForLoop());
        }
        else if (kw == "switch") {
             node.children.push_back(SwitchStatement());
        }
        else if (kw == "return") {
            get(); // consume 'return'
            ParseNode returnNode{"return"};
            if (!(peek().type == "DELIMITER" && peek().value == ";")) {
                returnNode.children.push_back(Expression());  // return value
            }
            if (!match("DELIMITER", ";")) {
                error("Expected ';' after return");
            }
            node.children.push_back(returnNode);

        }
        else if (kw == "break") {
            get(); // consume 'break'
            if (!match("DELIMITER", ";")) {
                error("Expected ';' after break");
            }
        }
        else {
            error("Unknown keyword");
        }
    }
    else if (peek().type == "IDENTIFIER") {
        node.children.push_back(Assignment());
    }
    else {
        error("Unexpected token");
    }
    return node;
}

ParseNode FunctionDefinition() {
    ParseNode node{"FunctionDefinition"};
    if (!match("KEYWORD")) error("Expected return type");
    if (!match("IDENTIFIER")) error("Expected function name");
    if (!match("DELIMITER", "(")) error("Expected '(' after function name");
    if (!(peek().type == "DELIMITER" && peek().value == ")")) {
        do {
            if (!match("KEYWORD")) error("Expected parameter type");
            if (!match("IDENTIFIER")) error("Expected parameter name");
        } while (match("DELIMITER", ","));
    }
    if (!match("DELIMITER", ")")) error("Expected ')' after parameters");
    ParseNode blockNode = Block();
    node.children.push_back(blockNode);

    return node;
}

ParseNode Program() {
    ParseNode node{"Program"};
    while (peek().type != "END") {
        if (peek().type == "KEYWORD") {
            string val = peek().value;
            Token next1 = (current+1 < (int)tokens.size()) ? tokens[current+1] : Token{"",""};
            Token next2 = (current+2 < (int)tokens.size()) ? tokens[current+2] : Token{"",""};
            if ((val == "int" || val == "float" || val == "char" || val == "double" || val == "void") &&
                next1.type == "IDENTIFIER" && next2.type == "DELIMITER" && next2.value == "(") {
                node.children.push_back(FunctionDefinition());
            } else {
                node.children.push_back(Statement());

            }
        } else {
            node.children.push_back(Statement());

        }
    }
    cout << "\n✅ Syntax is correct.\n";
    return node;
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
    ParseNode root = Program();
    json treeJson = nodeToJson(root);
    ofstream outTree("parse_tree.json");
    outTree << treeJson.dump(4);
    outTree.close();

    // Prepare JSON token output
    json j;
    for (const auto& token : tokens) {
        if (token.type == "IDENTIFIER" || token.type == "KEYWORD" || token.type == "OPERATOR" || token.type == "DELIMITER")
            j["symbols"].push_back({{"type", token.type}, {"name", token.value}});
    }

    // Save to JSON
    ofstream out("symbols.json");
    out << j.dump(4);
    out.close();

    return 0;
}
