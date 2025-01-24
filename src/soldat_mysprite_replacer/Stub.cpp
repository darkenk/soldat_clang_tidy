int mysprite = 0;

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
    TSpriteSystem::Get().GetSprite(mysprite);
    TSpriteSystem::Get().GetSprite(12);

    auto& system = TSpriteSystem::Get();
    system.GetSprite(mysprite);

    int mysprite = 12;
    system.GetSprite(mysprite);

    return {};
}