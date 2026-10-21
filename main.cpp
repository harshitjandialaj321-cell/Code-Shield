
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

using HashValue = uint64_t;

enum class Language {
    C, CPP, PYTHON, UNKNOWN
};

Language detectLanguage(const string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == string::npos)
        return Language::UNKNOWN;

    string ext = path.substr(dot);

    transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) {
            return static_cast<char>(tolower(c));
        });

    if (ext == ".c")
        return Language::C;

    if (ext == ".cpp" || ext == ".cc" ||
        ext == ".cxx" || ext == ".h" ||
        ext == ".hpp" || ext == ".hh" ||
        ext == ".hxx")
        return Language::CPP;

    if (ext == ".py")
        return Language::PYTHON;

    return Language::UNKNOWN;
}

string languageName(Language lang) {
    switch (lang) {
        case Language::C: return "C";
        case Language::CPP: return "C++";
        case Language::PYTHON: return "Python";
        default: return "Unknown";
    }
}

class CodeFile {
public:
    static bool read(const string& path, string& content) {
        ifstream input(path.c_str(), ios::binary);

        if (!input)
            return false;

        ostringstream buffer;
        buffer << input.rdbuf();

        if (input.bad())
            return false;

        content = buffer.str();
        return true;
    }
};

const unordered_set<string> C_KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "inline", "int", "long",
    "register", "restrict", "return", "short", "signed",
    "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while",
    "_Bool", "_Complex", "_Imaginary"
};

const unordered_set<string> CPP_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto",
    "bitand", "bitor", "bool", "break", "case", "catch",
    "char", "char8_t", "char16_t", "char32_t", "class",
    "compl", "concept", "const", "consteval", "constexpr",
    "constinit", "const_cast", "continue", "co_await",
    "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit",
    "export", "extern", "false", "float", "for", "friend",
    "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "noexcept", "not", "not_eq", "nullptr",
    "operator", "or", "or_eq", "private", "protected", "public",
    "register", "reinterpret_cast", "requires", "return", "short",
    "signed", "sizeof", "static", "static_assert", "static_cast",
    "struct", "switch", "template", "this", "thread_local",
    "throw", "true", "try", "typedef", "typeid", "typename",
    "union", "unsigned", "using", "virtual", "void", "volatile",
    "wchar_t", "while", "xor", "xor_eq", "override", "final"
};

const unordered_set<string> PYTHON_KEYWORDS = {
    "False", "None", "True", "and", "as", "assert", "async",
    "await", "break", "class", "continue", "def", "del",
    "elif", "else", "except", "finally", "for", "from",
    "global", "if", "import", "in", "is", "lambda",
    "nonlocal", "not", "or", "pass", "raise", "return",
    "try", "while", "with", "yield"
};

const unordered_set<string>& getKeywords(Language lang) {
    if (lang == Language::C)
        return C_KEYWORDS;

    if (lang == Language::PYTHON)
        return PYTHON_KEYWORDS;

    return CPP_KEYWORDS;
}

class CodePreprocessor {
public:
    static string removeComments(
        const string& source, Language lang) {

        string result;
        result.reserve(source.size());

        size_t i = 0;

        while (i < source.size()) {
            char c = source[i];

            if (lang == Language::PYTHON && c == '#') {
                while (i < source.size() && source[i] != '\n')
                    ++i;
                continue;
            }

            size_t quotePos = i;

            if (lang == Language::PYTHON) {
                while (quotePos < source.size() &&
                       quotePos - i < 2) {
                    char p = source[quotePos];

                    if (p == 'r' || p == 'R' ||
                        p == 'f' || p == 'F' ||
                        p == 'b' || p == 'B' ||
                        p == 'u' || p == 'U') {
                        ++quotePos;
                    } else {
                        break;
                    }
                }
            }

            if (quotePos < source.size() &&
                (source[quotePos] == '"' ||
                 source[quotePos] == '\'')) {

                char quote = source[quotePos];

                bool triple =
                    lang == Language::PYTHON &&
                    quotePos + 2 < source.size() &&
                    source[quotePos + 1] == quote &&
                    source[quotePos + 2] == quote;

                while (i < quotePos)
                    result += source[i++];

                size_t delimiter = triple ? 3 : 1;

                for (size_t p = 0;
                     p < delimiter && i < source.size(); ++p) {
                    result += source[i++];
                }

                while (i < source.size()) {
                    if (source[i] == '\\' &&
                        i + 1 < source.size()) {
                        result += source[i++];
                        result += source[i++];
                        continue;
                    }

                    bool closing = false;

                    if (triple) {
                        closing =
                            i + 2 < source.size() &&
                            source[i] == quote &&
                            source[i + 1] == quote &&
                            source[i + 2] == quote;
                    } else {
                        closing = source[i] == quote;
                    }

                    if (closing) {
                        for (size_t p = 0; p < delimiter; ++p)
                            result += source[i++];
                        break;
                    }

                    result += source[i++];
                }

                continue;
            }

            if (lang != Language::PYTHON &&
                i + 1 < source.size() &&
                c == '/' && source[i + 1] == '/') {

                i += 2;

                while (i < source.size() && source[i] != '\n')
                    ++i;

                continue;
            }

            if (lang != Language::PYTHON &&
                i + 1 < source.size() &&
                c == '/' && source[i + 1] == '*') {

                i += 2;

                while (i + 1 < source.size() &&
                       !(source[i] == '*' &&
                         source[i + 1] == '/')) {
                    if (source[i] == '\n')
                        result += '\n';
                    ++i;
                }

                if (i + 1 < source.size())
                    i += 2;

                continue;
            }

            result += source[i++];
        }

        return result;
    }
};

class Tokenizer {
private:
    static bool isIdentifierStart(char c) {
        unsigned char u = static_cast<unsigned char>(c);
        return isalpha(u) || c == '_' || c == '$';
    }

    static bool isIdentifierPart(char c) {
        unsigned char u = static_cast<unsigned char>(c);
        return isalnum(u) || c == '_' || c == '$';
    }

public:
    static vector<string> tokenize(
        const string& code, Language lang) {

        static const vector<string> operators = {
            "%:%:", "<=>", ">>=", "<<=", "->*", "...",
            "##", "::", "->", ".*", "++", "--", "<<", ">>",
            "<=", ">=", "==", "!=", "&&", "||", "+=", "-=",
            "*=", "/=", "%=", "&=", "|=", "^=", "=>", "**",
            "//", ":=", "??", "?."
        };

        vector<string> tokens;
        const auto& keywords = getKeywords(lang);

        size_t i = 0;

        while (i < code.size()) {
            unsigned char uc =
                static_cast<unsigned char>(code[i]);

            char c = code[i];

            if (isspace(uc)) {
                ++i;
                continue;
            }

            size_t quotePos = i;

            if (lang == Language::PYTHON) {
                while (quotePos < code.size() &&
                       quotePos - i < 2) {
                    char p = code[quotePos];

                    if (p == 'r' || p == 'R' ||
                        p == 'f' || p == 'F' ||
                        p == 'b' || p == 'B' ||
                        p == 'u' || p == 'U') {
                        ++quotePos;
                    } else {
                        break;
                    }
                }
            }

            if (quotePos < code.size() &&
                (code[quotePos] == '"' ||
                 code[quotePos] == '\'')) {

                char quote = code[quotePos];

                bool triple =
                    lang == Language::PYTHON &&
                    quotePos + 2 < code.size() &&
                    code[quotePos + 1] == quote &&
                    code[quotePos + 2] == quote;

                i = quotePos + (triple ? 3 : 1);

                while (i < code.size()) {
                    if (code[i] == '\\' &&
                        i + 1 < code.size()) {
                        i += 2;
                        continue;
                    }

                    if (triple &&
                        i + 2 < code.size() &&
                        code[i] == quote &&
                        code[i + 1] == quote &&
                        code[i + 2] == quote) {
                        i += 3;
                        break;
                    }

                    if (!triple && code[i] == quote) {
                        ++i;
                        break;
                    }

                    ++i;
                }

                tokens.push_back("STRING_LITERAL");
                continue;
            }

            if (isIdentifierStart(c)) {
                size_t start = i++;

                while (i < code.size() &&
                       isIdentifierPart(code[i])) {
                    ++i;
                }

                string word = code.substr(start, i - start);

                if (keywords.count(word))
                    tokens.push_back("KW_" + word);
                else
                    tokens.push_back("IDENTIFIER");

                continue;
            }

            if (isdigit(uc) ||
                (c == '.' && i + 1 < code.size() &&
                 isdigit(static_cast<unsigned char>(code[i + 1])))) {

                ++i;

                while (i < code.size()) {
                    unsigned char n =
                        static_cast<unsigned char>(code[i]);

                    if (isalnum(n) || code[i] == '_' ||
                        code[i] == '.') {
                        ++i;
                    } else if (
                        (code[i] == '+' || code[i] == '-') &&
                        i > 0 &&
                        (code[i - 1] == 'e' ||
                         code[i - 1] == 'E' ||
                         code[i - 1] == 'p' ||
                         code[i - 1] == 'P')) {
                        ++i;
                    } else {
                        break;
                    }
                }

                tokens.push_back("NUMBER");
                continue;
            }

            bool matched = false;

            for (const string& op : operators) {
                if (code.compare(i, op.size(), op) == 0) {
                    tokens.push_back(op);
                    i += op.size();
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                tokens.push_back(string(1, c));
                ++i;
            }
        }

        return tokens;
    }
};

class KGramGenerator {
public:
    static vector<string> generate(
        const vector<string>& tokens, size_t k) {

        vector<string> grams;

        if (k == 0 || tokens.size() < k)
            return grams;

        grams.reserve(tokens.size() - k + 1);

        for (size_t i = 0; i + k <= tokens.size(); ++i) {
            string gram;

            for (size_t j = 0; j < k; ++j) {
                gram += to_string(tokens[i + j].size());
                gram += ':';
                gram += tokens[i + j];
                gram += '|';
            }

            grams.push_back(gram);
        }

        return grams;
    }
};

class HashGenerator {
public:
    static HashValue hash(const string& text) {
        HashValue value = 14695981039346656037ULL;

        for (unsigned char c : text) {
            value ^= static_cast<HashValue>(c);
            value *= 1099511628211ULL;
        }

        return value;
    }

    static vector<HashValue> generate(
        const vector<string>& grams) {

        vector<HashValue> hashes;
        hashes.reserve(grams.size());

        for (const string& gram : grams)
            hashes.push_back(hash(gram));

        return hashes;
    }
};

class Winnowing {
public:
    static vector<HashValue> select(
        const vector<HashValue>& hashes, size_t windowSize) {

        vector<HashValue> fingerprints;

        if (hashes.empty() || windowSize == 0)
            return fingerprints;

        size_t w = min(windowSize, hashes.size());
        size_t lastSelected = numeric_limits<size_t>::max();

        for (size_t start = 0;
             start <= hashes.size() - w; ++start) {

            size_t minIndex = start;

            for (size_t j = start + 1; j < start + w; ++j) {
                if (hashes[j] <= hashes[minIndex])
                    minIndex = j;
            }

            if (minIndex != lastSelected) {
                fingerprints.push_back(hashes[minIndex]);
                lastSelected = minIndex;
            }
        }

        return fingerprints;
    }
};

class SimilarityChecker {
public:
    static double calculate(
        const vector<HashValue>& first,
        const vector<HashValue>& second) {

        unordered_set<HashValue> set1(first.begin(), first.end());
        unordered_set<HashValue> set2(second.begin(), second.end());

        if (set1.empty() || set2.empty())
            return -1.0;

        size_t intersection = 0;

        for (HashValue value : set1) {
            if (set2.count(value))
                ++intersection;
        }

        unordered_set<HashValue> unionSet = set1;
        unionSet.insert(set2.begin(), set2.end());

        if (unionSet.empty())
            return -1.0;

        return 100.0 * static_cast<double>(intersection) /
               static_cast<double>(unionSet.size());
    }
};

class CodeShield {
private:
    static void printSummary(
        const string& label,
        const vector<string>& tokens,
        const vector<string>& grams,
        const vector<HashValue>& fingerprints) {

        cout << label << " tokens: " << tokens.size() << '\n';
        cout << label << " k-grams: " << grams.size() << '\n';
        cout << label << " fingerprints: "
             << fingerprints.size() << '\n';
    }

public:
    static int run() {
        cout << "============================================\n";
        cout << "       CODE SHIELD - PLAGIARISM DETECTOR\n";
        cout << "============================================\n";
        cout << "Supported: C, C++ and Python\n\n";

        string path1, path2;

        cout << "Enter first source file path: ";
        getline(cin >> ws, path1);

        cout << "Enter second source file path: ";
        getline(cin, path2);

        Language lang1 = detectLanguage(path1);
        Language lang2 = detectLanguage(path2);

        if (lang1 == Language::UNKNOWN ||
            lang2 == Language::UNKNOWN) {
            cerr << "Error: Unsupported file extension.\n";
            cerr << "Supported: .c, .cpp, .cc, .cxx, .h, "
                 << ".hpp, .hh, .hxx, .py\n";
            return 1;
        }

        string source1, source2;

        if (!CodeFile::read(path1, source1)) {
            cerr << "Cannot read first file: " << path1 << '\n';
            return 1;
        }

        if (!CodeFile::read(path2, source2)) {
            cerr << "Cannot read second file: " << path2 << '\n';
            return 1;
        }

        if (source1.empty() || source2.empty()) {
            cerr << "Error: One or both files are empty.\n";
            return 1;
        }

        cout << "\nFile 1 language: " << languageName(lang1) << '\n';
        cout << "File 2 language: " << languageName(lang2) << '\n';

        if (lang1 != lang2) {
            cout << "Warning: Different languages detected. "
                 << "Results may be less reliable.\n";
        }

        const size_t K = 5;
        const size_t WINDOW = 4;

        string clean1 =
            CodePreprocessor::removeComments(source1, lang1);

        string clean2 =
            CodePreprocessor::removeComments(source2, lang2);

        vector<string> tokens1 = Tokenizer::tokenize(clean1, lang1);
        vector<string> tokens2 = Tokenizer::tokenize(clean2, lang2);

        vector<string> grams1 = KGramGenerator::generate(tokens1, K);
        vector<string> grams2 = KGramGenerator::generate(tokens2, K);

        vector<HashValue> hashes1 = HashGenerator::generate(grams1);
        vector<HashValue> hashes2 = HashGenerator::generate(grams2);

        vector<HashValue> fingerprints1 =
            Winnowing::select(hashes1, WINDOW);

        vector<HashValue> fingerprints2 =
            Winnowing::select(hashes2, WINDOW);

        cout << "\n--------------- RESULTS -------------------\n";

        printSummary("File 1", tokens1, grams1, fingerprints1);
        printSummary("File 2", tokens2, grams2, fingerprints2);

        double similarity =
            SimilarityChecker::calculate(fingerprints1, fingerprints2);

        cout << fixed << setprecision(2);

        if (similarity < 0.0) {
            cout << "\nSimilarity: Not enough fingerprints "
                 << "to compare reliably.\n";
            cout << "Use longer files containing at least "
                 << K << " tokens each.\n";
        } else {
            cout << "\nSimilarity: " << similarity << "%\n";

            if (similarity >= 70.0) {
                cout << "Result: HIGH similarity. "
                     << "Manual review recommended.\n";
            } else if (similarity >= 40.0) {
                cout << "Result: MODERATE similarity. "
                     << "Review shared sections.\n";
            } else {
                cout << "Result: LOW similarity based "
                     << "on selected fingerprints.\n";
            }
        }

        cout << "\nNote: Similarity is not final proof "
             << "of plagiarism.\n";
        cout << "-------------------------------------------\n";

        return 0;
    }
};

int main() {
    return CodeShield::run();
}
