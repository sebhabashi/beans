#pragma once

#include <beans.hpp>
#include <string>
#include <sstream>

#include "iparser.hpp"
#include "ilexer.hpp"

class Parser : public IParser
{
public:
    virtual void SetString(std::string str)
    {
        m_lex->SetString(std::move(str));
        m_tree = ParseExpr();
        PopToken(Token::T_NONE); // Check expression parsed completely
    }

    virtual const SemanticTree& GetTree() const
    {
        return m_tree;
    }
private:
    beans::Bean<ILexer> m_lex;

    SemanticTree m_tree;
    int m_pos = 0;

    const Token& PeekToken() const
    {
        static Token nullTok = [] {
            Token tok;
            tok.type = Token::T_NONE;
            tok.pos = 0;
            tok.text = "END";
            return tok;
        } ();
        return (m_pos < m_lex->GetTokens().size()) ? m_lex->GetTokens().at(m_pos) : nullTok;
    }

    const Token& PopToken()
    {
        const Token& tok = PeekToken();
        ++m_pos;
        return tok;
    }

    void PopToken(Token::Type type)
    {
        if (PopToken().type != type)
            ThrowUnexpectedToken();
    }

    __declspec(noreturn) void ThrowUnexpectedToken()
    {
        const auto& tok = PeekToken();
        throw std::invalid_argument("Invalid character at position " + std::to_string(tok.pos)
                                    + ": \"" + tok.text + "\"");
    }

    SemanticTree ParseExpr()
    {
        return ParseMinus();
    }

    SemanticTree ParseMinus()
    {
        SemanticTree left = ParsePlus();
        switch (PeekToken().type)
        {
            case Token::T_MINUS:
            {
                PopToken(); // Pop "-"
                SemanticTree right = ParseMinus();
                SemanticTree tree;
                tree.type = SemanticTree::T_BINARY_MINUS;
                tree.children = { left, right };
                return tree;
            }
            default:
                return left;
        }
    }

    SemanticTree ParsePlus()
    {
        SemanticTree left = ParseDivide();
        switch (PeekToken().type)
        {
            case Token::T_PLUS:
            {
                PopToken(); // Pop "+"
                SemanticTree right = ParsePlus();
                SemanticTree tree;
                tree.type = SemanticTree::T_PLUS;
                tree.children = { left, right };
                return tree;
            }
            default:
                return left;
        }
    }

    SemanticTree ParseDivide()
    {
        SemanticTree left = ParseTimes();
        switch (PeekToken().type)
        {
            case Token::T_DIVIDE:
            {
                PopToken(); // Pop "/"
                SemanticTree right = ParseDivide();
                SemanticTree tree;
                tree.type = SemanticTree::T_DIVIDE;
                tree.children = { left, right };
                return tree;
            }
            default:
                return left;
        }
    }

    SemanticTree ParseTimes()
    {
        SemanticTree left = ParsePar();
        switch (PeekToken().type)
        {
            case Token::T_TIMES:
            {
                PopToken(); // Pop "*"
                SemanticTree right = ParseTimes();
                SemanticTree tree;
                tree.type = SemanticTree::T_TIMES;
                tree.children = { left, right };
                return tree;
            }
            default:
                return left;
        }
    }

    SemanticTree ParsePar()
    {
        SemanticTree tree;
        if (PeekToken().type == Token::T_PAROPEN)
        {
            PopToken(); // Pop "("
            tree = ParseExpr();
            PopToken(Token::T_PARCLOSE); // Pop ")"
        }
        else
        {
            tree = ParseNumber();
        }
        return tree;
    }

    SemanticTree ParseNumber()
    {
        SemanticTree tree;
        switch (PeekToken().type)
        {
            case Token::T_MINUS:
            {
                PopToken(); // Pop "-"
                tree.type = SemanticTree::T_UNARY_MINUS;
                tree.children = { ParseNumber() };
                break;
            }
            case Token::T_NUMBER:
            {
                const auto& tok = PopToken();
                tree.type = SemanticTree::T_NUMBER;
                try
                {
                    tree.value = std::stod(tok.text);
                }
                catch (const std::invalid_argument& e)
                {
                    throw std::invalid_argument("Invalid number \"" + tok.text + "\"");
                }
                break;
            }
            default:
                ThrowUnexpectedToken();
        }
        return tree;
    }
};

BEANS_DEFAULT_IMPLEMENTATION(IParser, Parser)