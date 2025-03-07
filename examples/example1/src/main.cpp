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
    beans::registerInstance<IFoo>(&foo);

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