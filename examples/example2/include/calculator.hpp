#pragma once

#include <beans.hpp>
#include <string>
#include <sstream>

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

BEANS_DEFAULT_IMPLEMENTATION(ILexer, Lexer)
class Lexer : public ILexer
{
public:
    virtual void SetString(std::string str) override
    {
        m_str = std::move(str);
        Lex();
    }

    virtual const std::vector<Token>& GetTokens() const override
    {
        return m_tokens;
    };

private:
    std::string m_str;
    int m_pos = 0;
    std::vector<Token> m_tokens;

    char PeekChar()
    {
        return (m_pos < m_str.length()) ? m_str.at(m_pos) : 0;
    }

    char PopChar()
    {
        char c = PeekChar();
        ++m_pos;
        return c;
    }

    void LexOne(Token::Type type)
    {
        Token tok;
        tok.pos = m_pos;
        tok.text = std::string(1, PeekChar());
        tok.type = type;
        m_tokens.emplace_back(tok);
        PopChar();
    }

    void LexNumber()
    {
        Token tok;
        tok.pos = m_pos;
        tok.type = Token::T_NUMBER;
        
        std::stringstream ss;
        int pos = m_pos;
        while ((PeekChar() >= '0' && PeekChar() <= '9') || PeekChar() == '.')
        {
            ss << PopChar();
        }
        tok.text = ss.str();

        m_tokens.emplace_back(tok);
    }

    void Lex()
    {
        char c;
        while (c = PeekChar())
        {
            switch (c)
            {
                // Ignore whitespaces
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    PopChar();
                    continue;

                // Number
                case '.':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    LexNumber();
                    break;

                // Single character tokens
                case '(':
                    LexOne(Token::T_PAROPEN);
                    break;
                case ')':
                    LexOne(Token::T_PARCLOSE);
                    break;
                case '+':
                    LexOne(Token::T_PLUS);
                    break;
                case '-':
                    LexOne(Token::T_MINUS);
                    break;
                case '*':
                    LexOne(Token::T_TIMES);
                    break;
                case '/':
                    LexOne(Token::T_DIVIDE);
                    break;
                default:
                    LexOne(Token::T_UNREC);
            }
        }
    }
};

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

class Parser
{
public:
    virtual void SetString(std::string str)
    {
        m_lex->SetString(std::move(str));
        m_tree = ParseExpr();
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
        SemanticTree tree = ParseMinus();
        PopToken(Token::T_NONE); // Check expression parsed completely
        return tree;
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