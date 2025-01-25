#include <cstdint>

extern std::uint8_t mysprite;

struct Test
{
    std::uint8_t someone = 2; 
};


int test_main()
{
    Test t;
    if (t.someone == mysprite) {}
    if (t.someone != mysprite) {}

    std::uint8_t someone;
    if (mysprite == someone) {}
    if (mysprite != someone) {}
    if (someone == mysprite) {}
    if (someone != mysprite) {}

    std::uint8_t mysprite;
    if (mysprite == someone) {}
    if (mysprite != someone) {}
    if (someone == mysprite) {}
    if (someone != mysprite) {}
    

    return 0;
}