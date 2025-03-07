#pragma once

#include <iostream>

/// An interface that foo's
class IFoo
{
public:
    virtual int value() = 0;
};

/// A class that foo's and implements IFoo
class Foo : public IFoo
{
public:
    Foo(int val) : m_val(val) {}
    virtual int value() override { return m_val; }
private:
    int m_val;
};

/// An interface that bar's
class IBar
{
public:
    virtual void doTheBar() = 0;
};

/// A class that bar's and implements IBar
class Bar : public IBar
{
public:
    Bar() = default;
    virtual void doTheBar() override { std::cout << "BAR" << std::endl; }
};
