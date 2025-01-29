#include <vector>

namespace Config
{

enum Module
{
  CLIENT_MODULE,
  SERVER_MODULE,
  TEST_MODULE,
  INVALID_MODULE
};

namespace defaults
{

constexpr Module GetModule() noexcept
{
  return INVALID_MODULE;
}

} // namespace defaults

using namespace defaults;

} // namespace Config

template<Config::Module M>
class Sprite{};

template <class T>
class GlobalSubsystem
{
    public:
    static T& Get() {
        static T instance;
        return instance;
    }

};

template<class TSprite = Sprite<Config::CLIENT_MODULE>>
class TSpriteSystem : public GlobalSubsystem<TSpriteSystem<>>
{
    public:
        void func18() {}
        int func19() { return 0; }
        std::vector<int> func20() { return {}; }
};

using SpriteSystem = TSpriteSystem<>;

void func()
{
    auto& sprite_system = SpriteSystem::Get();
    SpriteSystem::Get();
    SpriteSystem::Get().func18();

    TSpriteSystem g;
}

void func2()
{
    auto& sprite_system {SpriteSystem::Get()};
}

void func3()
{
    SpriteSystem::Get().func18();
    SpriteSystem::Get().func18();
}

void func4()
{
    ;
    SpriteSystem::Get().func18();
    auto i = SpriteSystem::Get().func19();

}

void func5()
{

    for (const auto& i: SpriteSystem::Get().func20())
    {
    }
}