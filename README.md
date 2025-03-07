# beans
Open dependency injection library for modern C++

## Example
BigClass.hpp
```cpp
#pragma once

#include <beans.hpp>
#include "FooBar.hpp"

/// Some big class that needs to foo and bar
class BigClass
{
public:
    int getFooValue() { return m_foo->value(); }
    void bar() { m_bar->doTheBar(); }
private:
    beans::Bean<IFoo> m_foo;
    beans::Bean<IBar> m_bar;
};

```

FooBar.hpp
```cpp
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

```

main.cpp
```cpp
#include <beans.hpp>
#include <iostream>

#include "BigClass.hpp"
#include "FooBar.hpp"

int main(int argc, char** argv)
try
{
    // Register a unique instance for IFoo
    // Mind that foo must not be destroyed while using classes are not destroyed
    // From now on, IFoo beans will point to this instance
    Foo foo(456);
    beans::registerInstance<IFoo, Foo>(&foo);

    // Register implementation
    // From now on, IBar beans will be implemented with Bar
    beans::registerImplementation<IBar, Bar>();

    // Create a BigClass instance. Its beans use the specified implementations
    BigClass x;
    std::cout << x.getFooValue() << std::endl; // "456"
    x.bar(); // Prints "BAR" as defined in the Bar::doTheBar definition

    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return -1;
}
```
