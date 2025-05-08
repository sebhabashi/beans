#pragma once

#include <beans.hpp>
#include <string>
#include <sstream>

#include "iparser.hpp"

class Calculator
{
public:
    double Eval(const std::string& expr)
    {
        m_parser->SetString(expr);
        const SemanticTree& tree = m_parser->GetTree();
        return Eval(tree);
    }
private:
    beans::Bean<IParser> m_parser;

    double Eval(const SemanticTree& tree)
    {
        switch (tree.type)
        {
            case SemanticTree::T_NUMBER:
                return tree.value;
            case SemanticTree::T_UNARY_MINUS:
                return -Eval(tree.children[0]);
            case SemanticTree::T_DIVIDE:
                return Eval(tree.children[0]) / Eval(tree.children[1]);
            case SemanticTree::T_TIMES:
                return Eval(tree.children[0]) * Eval(tree.children[1]);
            case SemanticTree::T_PLUS:
                return Eval(tree.children[0]) + Eval(tree.children[1]);
            case SemanticTree::T_BINARY_MINUS:
                return Eval(tree.children[0]) - Eval(tree.children[1]);
            default:
                throw std::invalid_argument("Invalid tree type " + std::to_string(int(tree.type)));
        }
    }
};