class Stub
{
public:
    void stub_function();
    int x;
};

Stub gStub;

class GlobalStateClientStub
{
public:
    void client_function();
};

GlobalStateClientStub gGlobalStateClientStub;

class GlobalStateStub
{
public:
    void another_function();
    void extern_function();
    int x = 98;
};

GlobalStateStub gGlobalStateStub;

void GlobalStateStub::another_function()
{
    gGlobalStateStub.extern_function();
    gGlobalStateStub.x = 98;
    gGlobalStateStub.extern_function();
    gGlobalStateClientStub.client_function();
    gStub.x = 92;
}



int main()
{
    gGlobalStateStub.x = 128;
    gGlobalStateStub.extern_function();
    gStub.stub_function();
}
