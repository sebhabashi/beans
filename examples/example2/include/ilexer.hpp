#pragma once

#include <beans.hpp>
#include <string>
#include <sstream>
#include <vector>

struct Token
{
    /// Type of token
    enum Type {
        T_NONE,     ///< None (serves for parsing phase)
        T_UNREC,    ///< Unrecognized
        T_NUMBER,   ///< Number
        T_PAROPEN,  ///< (
        T_PARCLOSE, ///< )
        T_PLUS,     ///< +
        T_MINUS,    ///< -
        T_TIMES,    ///< *
        T_DIVIDE,   ///< /
    } type = T_UNREC;

    int pos = 0; ///< Position of the start of the token in the string
    std::string text; ///< Text of the token
};

class ILexer
{
public:
    virtual void SetString(std::string str) = 0;
    virtual const std::vector<Token>& GetTokens() const = 0;
};