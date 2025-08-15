struct GlobalStateClient
{
  const char* joinpassword;
  const char* joinport;
  const char* joinip;
  const char* basedirectory;
  int number;
};

GlobalStateClient gGlobalStateClient{
  .joinpassword{},
  .joinport = "23073",
  .joinip = "127.0.0.1",
  .basedirectory{},
  .number = 98,
};