#include <iostream>

#include "beans.hpp"

class IElement
{
public:
    virtual int value() = 0;
};

class Element : public IElement
{
public:
    virtual int value() override { return 123; }
};

class Element2 : public IElement
{
public:
    Element2(int val) : m_val(val) {}
    virtual int value() override { return m_val; }
private:
    int m_val;
};

class BigClass
{
public:
    int getElementValue() { return element->value(); }
private:
    beans::Bean<IElement> element;
};

int main(int argc, char** argv)
try
{
    // First construction with implementation class
    {
        beans::LockedEnvironment locked;
        beans::registerImplementation<IElement, Element>();
        BigClass x;

        std::cout << x.getElementValue() << std::endl;
    }

    // Second construction with implementation instance
    {
        beans::LockedEnvironment locked;
        Element2 element2(456);
        beans::registerInstance<IElement, Element2>(&element2);
        BigClass x;

        std::cout << x.getElementValue() << std::endl;
    }

    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return -1;
}