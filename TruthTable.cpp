#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <set>

#include "BooleanTypes.h"
#include "BooleanSimplify.h"

struct TruthTableRow {
    std::vector<int> inputs;
    int output;   
};

class TruthTable {
private:
    int n;
    int requiredRows;
    std::vector<TruthTableRow> table;

    bool inputExists(const std::vector<int>& inputs) const {
        for (const auto& row : table)
            if (row.inputs == inputs) return true;
        return false;
    }

    bool allBinary(const std::vector<int>& values) const {
        for (int v : values)
            if (v != 0 && v != 1) return false;
        return true;
    }

public:
    TruthTable() : n(0), requiredRows(0) {}

    
    
    
    void input() {
        while (true) {
            std::cout << "Enter the number of input variables (n >= 2): ";
            if (!(std::cin >> n)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                std::cout << "  [Error] Please enter a valid integer.\n";
                continue;
            }
            if (n < 2) { std::cout << "  [Error] n must be >= 2.\n"; continue; }
            break;
        }

        requiredRows = static_cast<int>(std::pow(2, n));
        table.clear();

        std::cout << "\nYou need to enter " << requiredRows
                  << " rows (each with " << n << " input bit(s) and 1 output bit).\n";
        std::cout << "For each row, enter " << n + 1
                  << " space-separated values (inputs then output), all 0 or 1.\n";
        std::cout << "Example for n=2: 0 1 1  (inputs: A=0, B=1 -> output=1)\n\n";

        while (static_cast<int>(table.size()) < requiredRows) {
            int remaining = requiredRows - static_cast<int>(table.size());
            std::cout << "Row " << table.size() + 1 << " of " << requiredRows
                      << "  (" << remaining << " remaining) > ";

            std::vector<int> values(n + 1);
            bool readOk = true;
            for (int i = 0; i <= n; ++i) {
                if (!(std::cin >> values[i])) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    std::cout << "  [Error] Invalid input. Please enter integers only.\n";
                    readOk = false;
                    break;
                }
            }
            if (!readOk) continue;

            if (!allBinary(values)) {
                std::cout << "  [Error] All values must be 0 or 1. Row rejected.\n";
                continue;
            }

            std::vector<int> inputs(values.begin(), values.begin() + n);
            int output = values[n];

            if (inputExists(inputs)) {
                std::cout << "  [Error] This input combination already exists. Row rejected.\n";
                continue;
            }

            table.push_back({inputs, output});
            std::cout << "  [OK] Row accepted.\n";
        }

        std::cout << "\nAll " << requiredRows << " rows entered successfully.\n";
    }

    
    
    
    bool validate() const {
        if (static_cast<int>(table.size()) != requiredRows) {
            std::cerr << "[Validation FAIL] Expected " << requiredRows
                      << " rows, got " << table.size() << ".\n";
            return false;
        }
        for (int i = 0; i < static_cast<int>(table.size()); ++i)
            for (int j = i + 1; j < static_cast<int>(table.size()); ++j)
                if (table[i].inputs == table[j].inputs) {
                    std::cerr << "[Validation FAIL] Duplicate at rows "
                              << i + 1 << " and " << j + 1 << ".\n";
                    return false;
                }
        for (int i = 0; i < static_cast<int>(table.size()); ++i)
            if (table[i].output != 0 && table[i].output != 1) {
                std::cerr << "[Validation FAIL] Row " << i + 1 << " bad output.\n";
                return false;
            }
        return true;
    }

    
    
    
    void display() const {
        if (table.empty()) { std::cout << "Truth table is empty.\n"; return; }

        std::cout << "\n--- Truth Table (" << n << " input variable(s)) ---\n";
        for (int i = 0; i < n; ++i) std::cout << "  " << static_cast<char>('A' + i);
        std::cout << "  | Out\n" << std::string(n * 3 + 7, '-') << "\n";

        std::vector<TruthTableRow> sorted = table;
        std::sort(sorted.begin(), sorted.end(),
            [](const TruthTableRow& a, const TruthTableRow& b){
                return a.inputs < b.inputs; });

        for (const auto& row : sorted) {
            for (int v : row.inputs) std::cout << "  " << v;
            std::cout << "  |  " << row.output << "\n";
        }
        std::cout << std::string(n * 3 + 7, '-') << "\n";
    }

    
    int getN()            const { return n; }
    int getRequiredRows() const { return requiredRows; }
    const std::vector<TruthTableRow>& getTable() const { return table; }
};

static int rowIndex(const std::vector<int>& inputs) {
    int idx = 0;
    for (int v : inputs) idx = (idx << 1) | v;
    return idx;
}

static std::string literal(int varIdx, int bit, bool isSOP) {
    std::string s(1, static_cast<char>('A' + varIdx));
    bool complement = isSOP ? (bit == 0) : (bit == 1);
    if (complement) s += "'";
    return s;
}

BooleanResult buildBooleanExpression(const TruthTable& tt) {
    int n = tt.getN();
    const auto& table = tt.getTable();

    
    int choice = 0;
    while (true) {
        std::cout << "\nSelect Boolean form:\n";
        std::cout << "  1) SOP  (Sum of Products  — minterms where output = 1)\n";
        std::cout << "  2) POS  (Product of Sums  — maxterms where output = 0)\n";
        std::cout << "Choice (1 or 2): ";
        if (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "  [Error] Enter 1 or 2.\n";
            continue;
        }
        break;
    }

    bool isSOP       = (choice == 1);
    int  targetOut   = isSOP ? 1 : 0;
    std::string form = isSOP ? "SOP" : "POS";

    
    std::vector<TruthTableRow> sorted = table;
    std::sort(sorted.begin(), sorted.end(),
        [](const TruthTableRow& a, const TruthTableRow& b){
            return rowIndex(a.inputs) < rowIndex(b.inputs); });

    
    std::vector<int> terms;
    for (const auto& row : sorted)
        if (row.output == targetOut)
            terms.push_back(rowIndex(row.inputs));

    
    std::map<int, std::vector<int>> lookup;
    for (const auto& row : sorted)
        lookup[rowIndex(row.inputs)] = row.inputs;

    
    std::string equation;
    if (terms.empty()) {
        equation = isSOP ? "F = 0" : "F = 1";
    } else if (static_cast<int>(terms.size()) == static_cast<int>(std::pow(2, n))) {
        equation = isSOP ? "F = 1" : "F = 0";
    } else {
        std::ostringstream oss;
        oss << "F = ";
        for (int t = 0; t < static_cast<int>(terms.size()); ++t) {
            const std::vector<int>& inp = lookup[terms[t]];
            if (isSOP) {
                if (t > 0) oss << " + ";
                for (int i = 0; i < n; ++i) oss << literal(i, inp[i], true);
            } else {
                if (t > 0) oss << " * ";
                oss << "(";
                for (int i = 0; i < n; ++i) {
                    if (i > 0) oss << " + ";
                    oss << literal(i, inp[i], false);
                }
                oss << ")";
            }
        }
        equation = oss.str();
    }

    
    std::cout << "\n=== " << form << " Canonical Form ===\n";
    std::cout << "Equation  : " << equation << "\n";

    std::string termLabel = isSOP ? "Minterms" : "Maxterms";
    std::cout << termLabel << " : ";
    if (terms.empty()) {
        std::cout << "(none)";
    } else {
        std::cout << (isSOP ? "m(" : "M(");
        for (int i = 0; i < static_cast<int>(terms.size()); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << terms[i];
        }
        std::cout << ")";
    }
    std::cout << "\n";

    return {form, equation, terms};
}

static const std::vector<int> gray1 = {0, 1};
static const std::vector<int> gray2 = {0, 1, 3, 2};   

static std::vector<int> grayToBits2(int g) {
    return { (g >> 1) & 1, g & 1 };
}

static std::map<int, int> buildOutputMap(const TruthTable& tt) {
    std::map<int, int> m;
    for (const auto& row : tt.getTable())
        m[rowIndex(row.inputs)] = row.output;
    return m;
}

static void printKMap2(const std::map<int, int>& out) {
    std::cout << "\n  K-Map (2 variables: A, B)\n\n";
    std::cout << "       B=0  B=1\n";
    std::cout << "      +----+----+\n";
    for (int a : gray1) {
        std::cout << " A=" << a << "  |";
        for (int b : gray1) {
            int idx = (a << 1) | b;
            std::cout << "  " << out.at(idx) << " |";
        }
        std::cout << "\n      +----+----+\n";
    }
}

static void printKMap3(const std::map<int, int>& out) {
    std::cout << "\n  K-Map (3 variables: A, B, C)\n\n";
    std::cout << "         BC=00  BC=01  BC=11  BC=10\n";
    std::cout << "        +------+------+------+------+\n";
    for (int a : gray1) {
        std::cout << "  A=" << a << "   |";
        for (int bc : gray2) {
            auto bits = grayToBits2(bc);
            int idx = (a << 2) | (bits[0] << 1) | bits[1];
            std::cout << "  " << out.at(idx) << "   |";
        }
        std::cout << "\n        +------+------+------+------+\n";
    }
}

static void printKMap4(const std::map<int, int>& out) {
    std::cout << "\n  K-Map (4 variables: A, B, C, D)\n\n";
    std::cout << "            CD=00  CD=01  CD=11  CD=10\n";
    std::cout << "           +------+------+------+------+\n";
    for (int ab : gray2) {
        auto abBits = grayToBits2(ab);
        std::cout << "  AB=" << abBits[0] << abBits[1] << "    |";
        for (int cd : gray2) {
            auto cdBits = grayToBits2(cd);
            int idx = (abBits[0] << 3) | (abBits[1] << 2)
                    | (cdBits[0] << 1) |  cdBits[1];
            std::cout << "  " << out.at(idx) << "   |";
        }
        std::cout << "\n           +------+------+------+------+\n";
    }
}

struct Implicant {
    std::set<int> minterms;
    int mask;
    int value;

    bool operator==(const Implicant& o) const {
        return mask == o.mask && value == o.value;
    }
};

static std::vector<Implicant> findPrimeImplicants(
        const std::vector<int>& minterms, int n) {

    std::vector<Implicant> current;
    for (int m : minterms) {
        Implicant imp;
        imp.minterms.insert(m);
        imp.mask  = (1 << n) - 1;
        imp.value = m;
        current.push_back(imp);
    }

    std::vector<Implicant> primes;

    while (!current.empty()) {
        std::vector<Implicant> next;
        std::vector<bool> merged(current.size(), false);

        for (int i = 0; i < static_cast<int>(current.size()); ++i) {
            for (int j = i + 1; j < static_cast<int>(current.size()); ++j) {
                if (current[i].mask != current[j].mask) continue;
                int diff = current[i].value ^ current[j].value;
                if (diff == 0 || (diff & (diff - 1)) != 0) continue;
                if ((diff & current[i].mask) != diff) continue;

                Implicant m;
                m.minterms = current[i].minterms;
                m.minterms.insert(current[j].minterms.begin(),
                                  current[j].minterms.end());
                m.mask  = current[i].mask & ~diff;
                m.value = current[i].value & ~diff;

                bool dup = false;
                for (const auto& ex : next)
                    if (ex == m) { dup = true; break; }
                if (!dup) next.push_back(m);

                merged[i] = merged[j] = true;
            }
        }

        for (int i = 0; i < static_cast<int>(current.size()); ++i) {
            if (!merged[i]) {
                bool dup = false;
                for (const auto& p : primes)
                    if (p == current[i]) { dup = true; break; }
                if (!dup) primes.push_back(current[i]);
            }
        }
        current = next;
    }
    return primes;
}

static std::string implicantToTerm(const Implicant& imp, int n) {
    std::string term;
    for (int pos = n - 1; pos >= 0; --pos) {
        int careBit = 1 << pos;
        if (imp.mask & careBit) {
            char var = static_cast<char>('A' + (n - 1 - pos));
            int  val = (imp.value >> pos) & 1;
            term += var;
            if (val == 0) term += "'";
        }
    }
    return term.empty() ? "1" : term;
}

static std::vector<Implicant> greedyCover(
        std::vector<Implicant>& primes,
        const std::vector<int>& minterms) {

    std::vector<Implicant> cover;
    std::set<int> uncovered(minterms.begin(), minterms.end());

    for (int m : minterms) {
        std::vector<int> covering;
        for (int pi = 0; pi < static_cast<int>(primes.size()); ++pi)
            if (primes[pi].minterms.count(m)) covering.push_back(pi);

        if (covering.size() == 1) {
            const Implicant& ess = primes[covering[0]];
            bool already = false;
            for (const auto& c : cover) if (c == ess) { already = true; break; }
            if (!already) {
                cover.push_back(ess);
                for (int cm : ess.minterms) uncovered.erase(cm);
            }
        }
    }

    while (!uncovered.empty()) {
        int bestIdx = -1, bestCnt = 0;
        for (int pi = 0; pi < static_cast<int>(primes.size()); ++pi) {
            int cnt = 0;
            for (int m : primes[pi].minterms) if (uncovered.count(m)) ++cnt;
            if (cnt > bestCnt) { bestCnt = cnt; bestIdx = pi; }
        }
        if (bestIdx == -1) break;
        cover.push_back(primes[bestIdx]);
        for (int m : primes[bestIdx].minterms) uncovered.erase(m);
    }

    return cover;
}

void simplifyKMap(const TruthTable& tt) {
    int n = tt.getN();

    if (n < 2 || n > 4) {
        std::cout << "\n[K-Map] Simplification only supported for 2-4 variables "
                  << "(n = " << n << " is out of range).\n";
        return;
    }

    std::map<int, int> outMap = buildOutputMap(tt);

    std::cout << "\n=== Karnaugh Map ===";
    if      (n == 2) printKMap2(outMap);
    else if (n == 3) printKMap3(outMap);
    else             printKMap4(outMap);

    std::vector<int> minterms;
    for (const auto& kv : outMap)
        if (kv.second == 1) minterms.push_back(kv.first);
    std::sort(minterms.begin(), minterms.end());

    std::cout << "\n=== K-Map Simplification (SOP) ===\n";

    if (minterms.empty()) {
        std::cout << "Simplified : F = 0\n";
        return;
    }
    if (static_cast<int>(minterms.size()) == static_cast<int>(std::pow(2, n))) {
        std::cout << "Simplified : F = 1\n";
        return;
    }

    std::vector<Implicant> primes = findPrimeImplicants(minterms, n);
    std::vector<Implicant> cover  = greedyCover(primes, minterms);

    std::string simplified = "F = ";
    for (int i = 0; i < static_cast<int>(cover.size()); ++i) {
        if (i > 0) simplified += " + ";
        simplified += implicantToTerm(cover[i], n);
    }
    std::cout << "Simplified : " << simplified << "\n";

    std::cout << "\nGroups (prime implicants used):\n";
    for (const auto& imp : cover) {
        std::cout << "  " << implicantToTerm(imp, n) << "  <-  minterms {";
        bool first = true;
        for (int m : imp.minterms) {
            if (!first) std::cout << ", ";
            std::cout << m;
            first = false;
        }
        std::cout << "}\n";
    }
}

int main() {
    std::cout << "=== Truth Table Input System ===\n\n";

    TruthTable tt;
    tt.input();

    if (tt.validate()) {
        std::cout << "[Validation PASS] Truth table is valid.\n";
    } else {
        std::cout << "[Validation FAIL] Truth table has errors.\n";
        return 1;
    }

    tt.display();

    
    BooleanResult result = buildBooleanExpression(tt);

    
    
    algebraicSimplify(result, tt.getN());

    
    simplifyKMap(tt);

    return 0;
}
