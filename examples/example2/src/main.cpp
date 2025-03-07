#include <beans.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "calculator.hpp"

int main(int argc, char** argv)
{
    std::string answer;
    while (true)
    {
        std::cout << "Expression ? (\"exit\" to quit)" << std::endl;
        std::getline(std::cin, answer);
        if (answer == "exit")
            break;
        std::cout << answer << std::endl;
    }
    
    return 0;
}