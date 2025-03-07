#include "calculator.hpp"

#include <beans.hpp>
#include <catch2/catch.hpp>

/// This is a basic mock, but better mocking class can be used with mocking libraries like GMOCK...
struct MockLexer : public ILexer
{
    std::vector<Token> tokens;
    virtual void SetString(std::string str) override {}
    virtual const std::vector<Token>& GetTokens() const override { return tokens; }
};

TEST_CASE("calculator.Lexer")
{
    SECTION("Empty")
    {
        Lexer lex;
        lex.SetString("");
        CHECK(lex.GetTokens().empty());
    }

    SECTION("Standard")
    {
        Lexer lex;
        lex.SetString("(3 + 12) * 2 / -1");
        REQUIRE(lex.GetTokens().size() == 10);
        CHECK(lex.GetTokens()[0].type == Token::T_PAROPEN);     // (
        CHECK(lex.GetTokens()[1].type == Token::T_NUMBER);      // 3
        CHECK(lex.GetTokens()[1].text == "3");
        CHECK(lex.GetTokens()[2].type == Token::T_PLUS);        // +
        CHECK(lex.GetTokens()[3].type == Token::T_NUMBER);      // 12
        CHECK(lex.GetTokens()[3].text == "12");
        CHECK(lex.GetTokens()[4].type == Token::T_PARCLOSE);    // )
        CHECK(lex.GetTokens()[5].type == Token::T_TIMES);       // *
        CHECK(lex.GetTokens()[6].type == Token::T_NUMBER);      // 2
        CHECK(lex.GetTokens()[6].text == "2");
        CHECK(lex.GetTokens()[7].type == Token::T_DIVIDE);      // /
        CHECK(lex.GetTokens()[8].type == Token::T_MINUS);       // -
        CHECK(lex.GetTokens()[9].type == Token::T_NUMBER);      // 1
        CHECK(lex.GetTokens()[9].text == "1");
    }
}

TEST_CASE("calculator.Parser")
{
    SECTION("Number")
    {
        // Initialization (parser uses beans so it must be constructed BEFORE environment unlock)
        beans::LockedEnvironment env;
        MockLexer lexer;
        beans::registerInstance<ILexer>(&lexer);
        Parser parser;
        env.unlock();

        Token t1;
        t1.type = Token::T_NUMBER;
        t1.text = "3.14";
        lexer.tokens = { t1 };

        parser.SetString("..."); // String is not lexed anyway
        const auto& tree = parser.GetTree();
        CHECK(tree.type == SemanticTree::T_NUMBER);
        CHECK(tree.value == 3.14);
        CHECK(tree.children.empty());
    }

    SECTION("Unary minus")
    {
        // Initialization (parser uses beans so it must be constructed BEFORE environment unlock)
        beans::LockedEnvironment env;
        MockLexer lexer;
        beans::registerInstance<ILexer>(&lexer);
        Parser parser;
        env.unlock();

        Token t1;
        t1.type = Token::T_MINUS;
        Token t2;
        t2.type = Token::T_NUMBER;
        t2.text = "2.0";
        lexer.tokens = { t1, t2 };

        parser.SetString("..."); // String is not lexed anyway
        const auto& tree = parser.GetTree();
        CHECK(tree.type == SemanticTree::T_UNARY_MINUS);
        REQUIRE(tree.children.size() == 1);
        CHECK(tree.children[0].type == SemanticTree::T_NUMBER);
        CHECK(tree.children[0].value == 2.0);
    }

    SECTION("Plus")
    {
        // Initialization (parser uses beans so it must be constructed BEFORE environment unlock)
        beans::LockedEnvironment env;
        MockLexer lexer;
        beans::registerInstance<ILexer>(&lexer);
        Parser parser;
        env.unlock();
        
        Token t1;
        t1.type = Token::T_NUMBER;
        t1.text = "2.0";
        Token t2;
        t2.type = Token::T_PLUS;
        Token t3;
        t3.type = Token::T_NUMBER;
        t3.text = "3.0";
        lexer.tokens = { t1, t2, t3 };

        parser.SetString("..."); // String is not lexed anyway
        const auto& tree = parser.GetTree();
        CHECK(tree.type == SemanticTree::T_PLUS);
        REQUIRE(tree.children.size() == 2);
        CHECK(tree.children[0].type == SemanticTree::T_NUMBER);
        CHECK(tree.children[0].value == 2.0);
        CHECK(tree.children[1].type == SemanticTree::T_NUMBER);
        CHECK(tree.children[1].value == 3.0);
    }

    // TODO: complete unit tests here
}