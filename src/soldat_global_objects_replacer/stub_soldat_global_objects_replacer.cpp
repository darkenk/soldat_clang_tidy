#include "Client.hpp"
#include "Stub.hpp"

std::string stub_var = {};
bool boolean_var = false;
bool boolean_var2;
std::string stub_var_no_init;

static int static_variable;
extern int extern_global_variable;

int main ()
{
    static int static_variable_inside_function = 98;
    nice_variable = 98;
    another_nice_variable = 92;
}