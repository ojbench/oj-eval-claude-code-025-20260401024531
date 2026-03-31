#include "eval.h"

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
