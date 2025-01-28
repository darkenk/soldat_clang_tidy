#include <cstdint>

extern std::uint8_t mysprite;

struct Test
{
    const std::uint8_t someone = 2;

    void test_method()
    {
        if (this->someone != mysprite) {}
        if (this->someone == mysprite) {}
    }

    void test_method2();
};

void Test::test_method2()
{
    if (this->someone == mysprite) {}
    if (this->someone != mysprite) {}
}


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