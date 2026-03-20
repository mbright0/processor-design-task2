#ifndef BOOLEAN_SIMPLIFY_H
#define BOOLEAN_SIMPLIFY_H

#include "BooleanTypes.h"
#include <string>
#include <vector>

struct SopTerm {
    int mask;
    int value;

    bool operator==(const SopTerm& o) const {
        return mask == o.mask && value == o.value;
    }
    bool operator<(const SopTerm& o) const {
        if (mask != o.mask) return mask < o.mask;
        return value < o.value;
    }
};

std::string termToStr(const SopTerm& t, int n);

void printSOP(const std::vector<SopTerm>& terms, int n);

bool coveredBy(const SopTerm& a, const SopTerm& b);

void algebraicSimplify(const BooleanResult& br, int n);

#endif 
