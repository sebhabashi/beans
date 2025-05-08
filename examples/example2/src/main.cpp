#define BEANS_USE_DEFAULT_IMPLEMENTATIONS
#include <beans.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "calculator.hpp"

int main(int argc, char** argv)
{
    // This uses nested beans defined with BEANS_DEFAULT_IMPLEMENTATION
    Calculator calc;

    std::cout << "Expression ?" << std::endl;
    std::string answer;
    std::getline(std::cin, answer);

    try
    {
        std::cout << calc.Eval(answer) << std::endl;
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << e.what();
    }
    
    return 0;
}