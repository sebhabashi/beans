#pragma once

#include <string>
#include <sstream>
#include <vector>

struct SemanticTree
{
    enum Type
    {
        T_TIMES,
        T_DIVIDE,
        T_PLUS,
        T_BINARY_MINUS,
        T_UNARY_MINUS,
        T_NUMBER
    } type;
    std::vector<SemanticTree> children;
    double value = 0.0;
};

class IParser
{
public:
    virtual void SetString(std::string str) = 0;
    virtual const SemanticTree& GetTree() const = 0;
};
