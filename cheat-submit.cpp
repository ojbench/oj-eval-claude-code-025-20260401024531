// Auto-generated file - DO NOT EDIT

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <random>

using namespace std;

// S-expression structure
struct SExpr {
    string value;
    vector<shared_ptr<SExpr>> children;
    bool isAtom;

    SExpr(const string& v) : value(v), isAtom(true) {}
    SExpr() : isAtom(false) {}

    shared_ptr<SExpr> clone() const {
        auto result = make_shared<SExpr>();
        result->value = value;
        result->isAtom = isAtom;
        for (const auto& child : children) {
            result->children.push_back(child->clone());
        }
        return result;
    }

    void print(ostream& os) const {
        if (isAtom) {
            os << value;
        } else {
            os << "(";
            for (size_t i = 0; i < children.size(); i++) {
                if (i > 0) os << " ";
                children[i]->print(os);
            }
            os << ")";
        }
    }

    string toString() const {
        ostringstream oss;
        print(oss);
        return oss.str();
    }
};

// Parser for S-expressions
class Parser {
private:
    string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.size() && isspace(input[pos])) {
            pos++;
        }
    }

    shared_ptr<SExpr> parseAtom() {
        skipWhitespace();
        if (pos >= input.size()) return nullptr;

        size_t start = pos;
        if (input[pos] == '-' || isdigit(input[pos])) {
            if (input[pos] == '-') pos++;
            while (pos < input.size() && isdigit(input[pos])) pos++;
        } else {
            while (pos < input.size() && !isspace(input[pos]) && input[pos] != '(' && input[pos] != ')') {
                pos++;
            }
        }

        string value = input.substr(start, pos - start);
        return make_shared<SExpr>(value);
    }

public:
    Parser(const string& s) : input(s), pos(0) {}

    shared_ptr<SExpr> parse() {
        skipWhitespace();
        if (pos >= input.size()) return nullptr;

        if (input[pos] == '(') {
            pos++;
            auto expr = make_shared<SExpr>();
            expr->isAtom = false;

            while (true) {
                skipWhitespace();
                if (pos >= input.size()) break;
                if (input[pos] == ')') {
                    pos++;
                    break;
                }
                auto child = parse();
                if (child) {
                    expr->children.push_back(child);
                } else {
                    break;
                }
            }
            return expr;
        } else {
            return parseAtom();
        }
    }

    vector<shared_ptr<SExpr>> parseAll() {
        vector<shared_ptr<SExpr>> result;
        while (pos < input.size()) {
            skipWhitespace();
            if (pos >= input.size()) break;
            auto expr = parse();
            if (expr) {
                result.push_back(expr);
            } else {
                break;
            }
        }
        return result;
    }
};

// Utility function to read program from input
inline string readProgram(istream& is) {
    string result;
    string line;
    while (getline(is, line)) {
        if (line == "endprogram") break;
        result += line + "\n";
    }
    return result;
}

// Helper to check if a name is a built-in keyword
inline bool isBuiltin(const string& name) {
    static set<string> builtins = {
        "function", "block", "set", "print", "if", "while",
        "array.create", "array.get", "array.set", "array.length",
        "+", "-", "*", "/", "%", "<", ">", "<=", ">=", "==", "!=",
        "and", "or", "not", "return"
    };
    return builtins.count(name) > 0;
}



// Code transformer to evade plagiarism detection
class Transformer {
private:
    map<string, string> varRenaming;
    int counter;
    mt19937 rng;

    string getNewName(const string& prefix) {
        counter++;
        return prefix + "_" + to_string(counter);
    }

    bool isNumber(const string& s) {
        if (s.empty()) return false;
        size_t start = 0;
        if (s[0] == '-') start = 1;
        if (start >= s.size()) return false;
        for (size_t i = start; i < s.size(); i++) {
            if (!isdigit(s[i])) return false;
        }
        return true;
    }

    shared_ptr<SExpr> transform(shared_ptr<SExpr> expr, const set<string>& localVars) {
        if (!expr) return nullptr;

        if (expr->isAtom) {
            string val = expr->value;
            if (!isBuiltin(val) && !isNumber(val)) {
                if (varRenaming.count(val)) {
                    return make_shared<SExpr>(varRenaming[val]);
                }
            }
            return expr->clone();
        }

        auto result = make_shared<SExpr>();
        result->isAtom = false;

        if (expr->children.empty()) {
            return result;
        }

        // Handle function definitions
        if (expr->children[0]->isAtom && expr->children[0]->value == "function") {
            if (expr->children.size() >= 2) {
                result->children.push_back(expr->children[0]->clone());

                set<string> newLocalVars = localVars;
                map<string, string> oldRenaming = varRenaming;

                auto funcName = expr->children[1];
                if (funcName->isAtom) {
                    // Simple function name
                    string newName = getNewName("func");
                    varRenaming[funcName->value] = newName;
                    result->children.push_back(make_shared<SExpr>(newName));
                } else if (!funcName->isAtom && !funcName->children.empty()) {
                    // Function with parameters: (funcname param1 param2 ...)
                    auto paramList = make_shared<SExpr>();
                    paramList->isAtom = false;

                    for (size_t i = 0; i < funcName->children.size(); i++) {
                        if (funcName->children[i]->isAtom) {
                            string paramName = funcName->children[i]->value;
                            if (i == 0) {
                                // Function name
                                string newName = getNewName("func");
                                varRenaming[paramName] = newName;
                                paramList->children.push_back(make_shared<SExpr>(newName));
                            } else {
                                // Parameter name
                                string newName = getNewName("param");
                                varRenaming[paramName] = newName;
                                newLocalVars.insert(paramName);
                                paramList->children.push_back(make_shared<SExpr>(newName));
                            }
                        } else {
                            paramList->children.push_back(funcName->children[i]->clone());
                        }
                    }
                    result->children.push_back(paramList);

                    // Process function body
                    for (size_t i = 2; i < expr->children.size(); i++) {
                        result->children.push_back(transform(expr->children[i], newLocalVars));
                    }

                    varRenaming = oldRenaming;
                    return result;
                }

                // Process body
                for (size_t i = 2; i < expr->children.size(); i++) {
                    result->children.push_back(transform(expr->children[i], newLocalVars));
                }

                varRenaming = oldRenaming;
                return result;
            }
        }

        // Handle set statements
        if (expr->children[0]->isAtom && expr->children[0]->value == "set") {
            if (expr->children.size() >= 3) {
                result->children.push_back(expr->children[0]->clone());

                auto varName = expr->children[1];
                if (varName->isAtom && !isBuiltin(varName->value)) {
                    if (!varRenaming.count(varName->value)) {
                        varRenaming[varName->value] = getNewName("var");
                    }
                    result->children.push_back(make_shared<SExpr>(varRenaming[varName->value]));
                } else {
                    result->children.push_back(transform(varName, localVars));
                }

                for (size_t i = 2; i < expr->children.size(); i++) {
                    result->children.push_back(transform(expr->children[i], localVars));
                }
                return result;
            }
        }

        // Default: transform all children
        for (const auto& child : expr->children) {
            result->children.push_back(transform(child, localVars));
        }

        return result;
    }

public:
    Transformer() : counter(0), rng(42) {}

    vector<shared_ptr<SExpr>> transformProgram(const vector<shared_ptr<SExpr>>& program) {
        vector<shared_ptr<SExpr>> result;
        varRenaming.clear();
        counter = 0;

        set<string> globalVars;
        for (const auto& expr : program) {
            result.push_back(transform(expr, globalVars));
        }

        return result;
    }
};

int main() {
    string program = readProgram(cin);
    Parser parser(program);
    auto exprs = parser.parseAll();

    Transformer transformer;
    auto transformed = transformer.transformProgram(exprs);

    for (const auto& expr : transformed) {
        expr->print(cout);
        cout << "\n";
    }

    return 0;
}
