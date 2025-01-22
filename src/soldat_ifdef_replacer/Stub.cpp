#define SERVER

#ifdef SERVER
[[maybe_unused]] int global = 0;
#endif // SERVER

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

template <Config::Module M>
class TestClass
{
  void testfunction();
};

template <Config::Module M>
void TestClass<M>::testfunction()
{
  bool t = true;
  #ifdef SERVER
  if (t != true)
  #endif // SERVER
  {
  int nice = 0;
  }
}


#ifdef SERVER
void testfunction_server()
{
    int nice = 0;
}
#endif // SERVER

template <typename M>
void templated_function()
{
    #ifdef SERVER
    int nice = 0;
    #endif // SERVER
}

template <Config::Module M>
void templated_function2()
{
    #ifdef SERVER
    int nice = 0;
    #endif // SERVER
}


void testfunction()
{
    #ifdef SERVER
    int nice = 0;
    #endif // SERVER

    #ifdef SERVER
    int s_nice = 0;
    #else
    int s_bad = 0;
    #endif // SERVER

    #ifdef CLIENT
    int c_nice = 0;
    #else
    int c_bad = 0;
    #endif // CLIENT

    #ifdef SERVER
    #if 0
    int s_nice = 0;
    #endif // 0
    #endif // SERVER
}