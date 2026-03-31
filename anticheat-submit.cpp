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



// Similarity checker for plagiarism detection
class SimilarityChecker {
private:
    // Compute structural similarity between two expression trees
    double computeTreeSimilarity(shared_ptr<SExpr> e1, shared_ptr<SExpr> e2) {
        if (!e1 || !e2) return 0.0;

        if (e1->isAtom && e2->isAtom) {
            // Check if both are numbers
            bool e1IsNum = !e1->value.empty() && (isdigit(e1->value[0]) || e1->value[0] == '-');
            bool e2IsNum = !e2->value.empty() && (isdigit(e2->value[0]) || e2->value[0] == '-');

            if (e1IsNum && e2IsNum) {
                return e1->value == e2->value ? 1.0 : 0.5;
            }

            // Check if both are builtins
            bool e1Builtin = isBuiltin(e1->value);
            bool e2Builtin = isBuiltin(e2->value);

            if (e1Builtin && e2Builtin) {
                return e1->value == e2->value ? 1.0 : 0.0;
            }

            // Both are identifiers
            if (!e1Builtin && !e2Builtin) {
                return 0.8;  // Identifiers can be renamed
            }

            return e1->value == e2->value ? 1.0 : 0.0;
        }

        if (e1->isAtom != e2->isAtom) return 0.0;

        // Different number of children
        if (e1->children.size() != e2->children.size()) {
            return max(0.0, 1.0 - abs((int)e1->children.size() - (int)e2->children.size()) * 0.2);
        }

        // Compare children
        double sum = 0.0;
        for (size_t i = 0; i < e1->children.size(); i++) {
            sum += computeTreeSimilarity(e1->children[i], e2->children[i]);
        }

        return sum / max(1.0, (double)e1->children.size());
    }

    // Extract features from expression tree
    void extractFeatures(shared_ptr<SExpr> expr, map<string, int>& features, int depth = 0) {
        if (!expr) return;

        if (depth > 10) return;  // Prevent deep recursion

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

    // Compute cosine similarity between feature vectors
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

    // Count specific patterns
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

        // Size similarity
        double sizeSim = 1.0 - min(1.0, abs((int)prog1.size() - (int)prog2.size()) / (double)max(prog1.size(), prog2.size()));

        // Structure similarity
        double structSim = 0.0;
        size_t minSize = min(prog1.size(), prog2.size());
        size_t maxSize = max(prog1.size(), prog2.size());

        for (size_t i = 0; i < minSize; i++) {
            structSim += computeTreeSimilarity(prog1[i], prog2[i]);
        }
        structSim /= maxSize;

        // Feature-based similarity
        map<string, int> features1, features2;
        for (const auto& expr : prog1) {
            extractFeatures(expr, features1);
        }
        for (const auto& expr : prog2) {
            extractFeatures(expr, features2);
        }

        double featureSim = computeFeatureSimilarity(features1, features2);

        // Pattern similarity
        map<string, int> patterns1, patterns2;
        for (const auto& expr : prog1) {
            countPatterns(expr, patterns1);
        }
        for (const auto& expr : prog2) {
            countPatterns(expr, patterns2);
        }

        double patternSim = computeFeatureSimilarity(patterns1, patterns2);

        // Combined similarity with weights
        double similarity = 0.4 * structSim + 0.3 * featureSim + 0.2 * patternSim + 0.1 * sizeSim;

        // Clamp to [0, 1]
        similarity = max(0.0, min(1.0, similarity));

        return similarity;
    }
};

int main() {
    // Read two programs
    string program1 = readProgram(cin);
    string program2 = readProgram(cin);

    // Read test inputs (ignored for now)
    string testInputs;
    string line;
    while (getline(cin, line)) {
        testInputs += line + "\n";
    }

    // Parse programs
    Parser parser1(program1);
    auto exprs1 = parser1.parseAll();

    Parser parser2(program2);
    auto exprs2 = parser2.parseAll();

    // Compute similarity
    SimilarityChecker checker;
    double similarity = checker.computeSimilarity(exprs1, exprs2);

    cout << fixed;
    cout.precision(6);
    cout << similarity << endl;

    return 0;
}
