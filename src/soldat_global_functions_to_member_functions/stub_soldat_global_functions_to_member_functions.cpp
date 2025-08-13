#include "Stub.hpp"

class SomeClass
{
    void stub_function();
};

void SomeClass::stub_function()
{

}

GlobalStateStub gGlobalStateStub;

static void function()
{

}

void extern_function()
{

}

int nice_function()
{
    return 98;
}

int main()
{
    nice_function();
    extern_function();
}
