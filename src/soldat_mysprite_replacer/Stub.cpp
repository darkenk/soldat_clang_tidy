#include <cstdint>
extern std::uint8_t mysprite;

class Sprite {

};

template <class TSprite>
class TSpriteSystem
{   
public:
    int GetSprite(int spriteId)
    {
        return spriteId;
    }

    static auto& Get()
    {
        static TSpriteSystem instance;
        return instance;
    }

    int GetMySprite()
    {
        return mysprite;
    }
};

int main_test()
{
    TSpriteSystem<Sprite>::Get().GetSprite(mysprite);
    TSpriteSystem<Sprite>::Get().GetSprite(12);

    auto& system = TSpriteSystem<Sprite>::Get();
    system.GetSprite(mysprite);

    int mysprite = 12;
    system.GetSprite(mysprite);

    return {};
}