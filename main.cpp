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
                    string newName = getNewName("func");
                    varRenaming[funcName->value] = newName;
                    result->children.push_back(make_shared<SExpr>(newName));
                } else if (!funcName->isAtom && !funcName->children.empty()) {
                    auto paramList = make_shared<SExpr>();
                    paramList->isAtom = false;

                    for (size_t i = 0; i < funcName->children.size(); i++) {
                        if (funcName->children[i]->isAtom) {
                            string paramName = funcName->children[i]->value;
                            if (i == 0) {
                                string newName = getNewName("func");
                                varRenaming[paramName] = newName;
                                paramList->children.push_back(make_shared<SExpr>(newName));
                            } else {
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

                    for (size_t i = 2; i < expr->children.size(); i++) {
                        result->children.push_back(transform(expr->children[i], newLocalVars));
                    }

                    varRenaming = oldRenaming;
                    return result;
                }

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

// Similarity checker for plagiarism detection
class SimilarityChecker {
private:
    double computeTreeSimilarity(shared_ptr<SExpr> e1, shared_ptr<SExpr> e2) {
        if (!e1 || !e2) return 0.0;

        if (e1->isAtom && e2->isAtom) {
            bool e1IsNum = !e1->value.empty() && (isdigit(e1->value[0]) || e1->value[0] == '-');
            bool e2IsNum = !e2->value.empty() && (isdigit(e2->value[0]) || e2->value[0] == '-');

            if (e1IsNum && e2IsNum) {
                return e1->value == e2->value ? 1.0 : 0.5;
            }

            bool e1Builtin = isBuiltin(e1->value);
            bool e2Builtin = isBuiltin(e2->value);

            if (e1Builtin && e2Builtin) {
                return e1->value == e2->value ? 1.0 : 0.0;
            }

            if (!e1Builtin && !e2Builtin) {
                return 0.8;
            }

            return e1->value == e2->value ? 1.0 : 0.0;
        }

        if (e1->isAtom != e2->isAtom) return 0.0;

        if (e1->children.size() != e2->children.size()) {
            return max(0.0, 1.0 - abs((int)e1->children.size() - (int)e2->children.size()) * 0.2);
        }

        double sum = 0.0;
        for (size_t i = 0; i < e1->children.size(); i++) {
            sum += computeTreeSimilarity(e1->children[i], e2->children[i]);
        }

        return sum / max(1.0, (double)e1->children.size());
    }

    void extractFeatures(shared_ptr<SExpr> expr, map<string, int>& features, int depth = 0) {
        if (!expr) return;
        if (depth > 10) return;

        if (expr->isAtom) {
            if (isBuiltin(expr->value)) {
                features["builtin:" + expr->value]++;
            } else {
                bool isNum = !expr->value.empty() && (isdigit(expr->value[0]) || expr->value[0] == '-');
                if (isNum) {
                    features["const"]++;
                } else {
                    features["var"]++;
                }
            }
        } else {
            if (!expr->children.empty() && expr->children[0]->isAtom) {
                features["op:" + expr->children[0]->value]++;
            }
            features["expr_size:" + to_string(expr->children.size())]++;

            for (const auto& child : expr->children) {
                extractFeatures(child, features, depth + 1);
            }
        }
    }

    double computeFeatureSimilarity(const map<string, int>& f1, const map<string, int>& f2) {
        set<string> allFeatures;
        for (const auto& p : f1) allFeatures.insert(p.first);
        for (const auto& p : f2) allFeatures.insert(p.first);

        if (allFeatures.empty()) return 0.5;

        double dotProduct = 0.0;
        double norm1 = 0.0, norm2 = 0.0;

        for (const auto& feat : allFeatures) {
            int v1 = f1.count(feat) ? f1.at(feat) : 0;
            int v2 = f2.count(feat) ? f2.at(feat) : 0;
            dotProduct += v1 * v2;
            norm1 += v1 * v1;
            norm2 += v2 * v2;
        }

        if (norm1 == 0 || norm2 == 0) return 0.5;
        return dotProduct / (sqrt(norm1) * sqrt(norm2));
    }

    void countPatterns(shared_ptr<SExpr> expr, map<string, int>& patterns) {
        if (!expr || expr->isAtom) return;

        if (!expr->children.empty() && expr->children[0]->isAtom) {
            string op = expr->children[0]->value;

            if (op == "function") {
                patterns["function_count"]++;
            } else if (op == "if") {
                patterns["if_count"]++;
            } else if (op == "while") {
                patterns["while_count"]++;
            } else if (op == "block") {
                patterns["block_count"]++;
            } else if (op == "array.create") {
                patterns["array_create_count"]++;
            }
        }

        for (const auto& child : expr->children) {
            countPatterns(child, patterns);
        }
    }

public:
    double computeSimilarity(const vector<shared_ptr<SExpr>>& prog1,
                            const vector<shared_ptr<SExpr>>& prog2) {
        if (prog1.empty() || prog2.empty()) return 0.5;

        double sizeSim = 1.0 - min(1.0, abs((int)prog1.size() - (int)prog2.size()) / (double)max(prog1.size(), prog2.size()));

        double structSim = 0.0;
        size_t minSize = min(prog1.size(), prog2.size());
        size_t maxSize = max(prog1.size(), prog2.size());

        for (size_t i = 0; i < minSize; i++) {
            structSim += computeTreeSimilarity(prog1[i], prog2[i]);
        }
        structSim /= maxSize;

        map<string, int> features1, features2;
        for (const auto& expr : prog1) {
            extractFeatures(expr, features1);
        }
        for (const auto& expr : prog2) {
            extractFeatures(expr, features2);
        }

        double featureSim = computeFeatureSimilarity(features1, features2);

        map<string, int> patterns1, patterns2;
        for (const auto& expr : prog1) {
            countPatterns(expr, patterns1);
        }
        for (const auto& expr : prog2) {
            countPatterns(expr, patterns2);
        }

        double patternSim = computeFeatureSimilarity(patterns1, patterns2);

        double similarity = 0.4 * structSim + 0.3 * featureSim + 0.2 * patternSim + 0.1 * sizeSim;

        similarity = max(0.0, min(1.0, similarity));

        return similarity;
    }
};

int main(int argc, char* argv[]) {
    string mode = "cheat";
    if (argc >= 2) {
        mode = argv[1];
    } else {
        // Try to auto-detect mode by looking ahead at input
        string firstLine;
        getline(cin, firstLine);

        // If we can read input, put it back and try to detect
        if (firstLine.find("endprogram") != string::npos || firstLine.empty()) {
            mode = "anticheat";
        }

        // Put the line back by recreating the input
        string restOfInput;
        string line;
        while (getline(cin, line)) {
            restOfInput += line + "\n";
        }

        // This is a simplified approach - just process as cheat mode by default
        // In production, would need better input handling
        cin.clear();
        istringstream iss(firstLine + "\n" + restOfInput);
        cin.rdbuf(iss.rdbuf());
    }

    if (mode == "cheat") {
        string program = readProgram(cin);
        Parser parser(program);
        auto exprs = parser.parseAll();

        Transformer transformer;
        auto transformed = transformer.transformProgram(exprs);

        for (const auto& expr : transformed) {
            expr->print(cout);
            cout << "\n";
        }
    } else if (mode == "anticheat") {
        string program1 = readProgram(cin);
        string program2 = readProgram(cin);

        string testInputs;
        string line;
        while (getline(cin, line)) {
            testInputs += line + "\n";
        }

        Parser parser1(program1);
        auto exprs1 = parser1.parseAll();

        Parser parser2(program2);
        auto exprs2 = parser2.parseAll();

        SimilarityChecker checker;
        double similarity = checker.computeSimilarity(exprs1, exprs2);

        cout << fixed;
        cout.precision(6);
        cout << similarity << endl;
    } else {
        cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    return 0;
}
