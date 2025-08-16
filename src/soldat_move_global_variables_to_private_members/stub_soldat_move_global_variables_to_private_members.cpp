struct GlobalStateClient
{
  const char* joinpassword;
  const char* joinport;
  const char* joinip;
  const char* basedirectory;
  int number;
  private:
};

constexpr auto kConst = 9;
static int v;
static int z = 12;
int m = 98;
namespace
{
    int d;
}

GlobalStateClient gGlobalStateClient;

void function();

void nice_function()
{
  static int static_var = 9;
}

void another_function()
{
    nice_function();
}