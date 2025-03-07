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
