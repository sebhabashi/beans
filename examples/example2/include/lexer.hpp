#pragma once

#include <beans.hpp>
#include <string>
#include <sstream>

#include "ilexer.hpp"

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

BEANS_DEFAULT_IMPLEMENTATION(ILexer, Lexer)
