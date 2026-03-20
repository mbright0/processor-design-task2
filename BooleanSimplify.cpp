#include "BooleanSimplify.h"
#include <iostream>
#include <algorithm>

std::string termToStr(const SopTerm& t, int n) {
    std::string s;
    
    
    for (int pos = n - 1; pos >= 0; --pos) {
        int bit = 1 << pos;
        if (t.mask & bit) {
            char var = static_cast<char>('A' + (n - 1 - pos));
            s += var;
            if (!(t.value & bit)) s += "'";   
        }
    }
    return s.empty() ? "1" : s;
}

void printSOP(const std::vector<SopTerm>& terms, int n) {
    std::cout << "  F = ";
    for (int i = 0; i < static_cast<int>(terms.size()); ++i) {
        if (i > 0) std::cout << " + ";
        std::cout << termToStr(terms[i], n);
    }
    std::cout << "\n";
}

bool coveredBy(const SopTerm& a, const SopTerm& b) {
    if (a == b) return false;                        
    if ((b.mask & a.mask) != b.mask) return false;   
    return (a.value & b.mask) == (b.value & b.mask); 
}

void algebraicSimplify(const BooleanResult& br, int n) {

    
    if (br.form != "SOP") {
        std::cout << "\n[Algebraic] Step-by-step trace only available for SOP.\n";
        return;
    }

    
    if (n < 2 || n > 4) {
        std::cout << "\n[Algebraic] Trace only shown for 2-4 variables.\n";
        return;
    }

    
    if (br.terms.empty()) {
        std::cout << "\n=== Algebraic Simplification ===\n"
                  << "  F = 0  (no minterms — function is always 0)\n";
        return;
    }
    if (static_cast<int>(br.terms.size()) == (1 << n)) {
        std::cout << "\n=== Algebraic Simplification ===\n"
                  << "  F = 1  (all minterms — function is always 1)\n";
        return;
    }

    
    
    
    std::vector<SopTerm> terms;
    int allMask = (1 << n) - 1;
    for (int idx : br.terms)
        terms.push_back({allMask, idx});

    std::cout << "\n=== Algebraic Simplification (Lecture 12 Theorems) ===\n";
    std::cout << "Start (canonical SOP):\n";
    printSOP(terms, n);

    
    
    
    
    bool changed = true;
    while (changed) {
        changed = false;    
        
        for (int i = 0; i < static_cast<int>(terms.size()) && !changed; ++i) {
            for (int j = i + 1; j < static_cast<int>(terms.size()) && !changed; ++j) {

                
                
                if (terms[i].mask != terms[j].mask) continue;

                
                int diff = terms[i].value ^ terms[j].value;

                
                if (diff == 0 || (diff & (diff - 1)) != 0) continue;

                
                SopTerm merged;
                merged.mask  = terms[i].mask  & ~diff;
                merged.value = terms[i].value & ~diff;

                
                int elimPos = 0;
                for (int p = 0; p < n; ++p)
                    if (diff & (1 << p)) { elimPos = p; break; }
                char elimVar = static_cast<char>('A' + (n - 1 - elimPos));

                std::cout << "  T10 Combining  : "
                          << termToStr(terms[i], n) << " + "
                          << termToStr(terms[j], n)
                          << "  =>  " << termToStr(merged, n)
                          << "  (eliminate " << elimVar << ")\n";

                
                terms[i] = merged;
                terms.erase(terms.begin() + j);

                
                
                std::sort(terms.begin(), terms.end());
                terms.erase(std::unique(terms.begin(), terms.end()), terms.end());

                printSOP(terms, n);
                changed = true;
            }
        }
        if (changed) continue;   
        
        for (int i = 0; i < static_cast<int>(terms.size()) && !changed; ++i) {
            for (int j = 0; j < static_cast<int>(terms.size()) && !changed; ++j) {
                if (i == j) continue;

                if (!coveredBy(terms[i], terms[j])) continue;

                std::cout << "  T9' Covering   : drop "
                          << termToStr(terms[i], n)
                          << "  (covered by "
                          << termToStr(terms[j], n) << ")\n";

                terms.erase(terms.begin() + i);
                printSOP(terms, n);
                changed = true;
            }
        }
        if (changed) continue;   
        
        for (int i = 0; i < static_cast<int>(terms.size()) && !changed; ++i) {
            for (int j = 0; j < static_cast<int>(terms.size()) && !changed; ++j) {
                if (i == j) continue;

                
                int sharedMask   = terms[i].mask & terms[j].mask;
                int conflictBits = sharedMask & (terms[i].value ^ terms[j].value);

                
                if (conflictBits == 0 || (conflictBits & (conflictBits - 1)) != 0)
                    continue;          
                
                int consensusMask  = (terms[i].mask  | terms[j].mask)  & ~conflictBits;
                int consensusValue = (terms[i].value | terms[j].value) & ~conflictBits;
                SopTerm consensus{consensusMask, consensusValue};

                
                for (int k = 0; k < static_cast<int>(terms.size()) && !changed; ++k) {
                    if (k == i || k == j) continue;
                    if (!(terms[k] == consensus)) continue;

                    
                    int cbit = 0;
                    for (int p = 0; p < n; ++p)
                        if (conflictBits & (1 << p)) { cbit = p; break; }
                    char cvar = static_cast<char>('A' + (n - 1 - cbit));

                    std::cout << "  T11 Consensus  : drop "
                              << termToStr(terms[k], n)
                              << "  (consensus of "
                              << termToStr(terms[i], n) << " and "
                              << termToStr(terms[j], n)
                              << " on " << cvar << ")\n";

                    terms.erase(terms.begin() + k);
                    printSOP(terms, n);
                    changed = true;
                }
            }
        }
        
    }

    std::cout << "Result:\n";
    printSOP(terms, n);
}
