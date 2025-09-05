// Test content file
struct NetworkContext;

void clienthandlenewplayer2(NetworkContext *netmessage);

void clienthandlenewplayer2(NetworkContext *netmessage)
{
  
}

void clienthandlenewplayer(NetworkContext *netmessage);

void clienthandlenewplayer(NetworkContext *netmessage)
{
  
}

struct NetworkContext{};
struct SteamNetworkingMessage_t;
using PSteamNetworkingMessage_t = SteamNetworkingMessage_t*;


class NetworkClientImpl
{
public:
    NetworkClientImpl();
    void HandleMessages(PSteamNetworkingMessage_t);
    struct {
        int id;
    } *PacketHeader;

    void RegisterMsgHandler(int, void(*)(NetworkContext*));
};

auto constexpr msgid_custom = 0;
auto constexpr msgid_newplayer = msgid_custom + 17;
auto constexpr msgid_newplayer2 = msgid_custom + 18;

NetworkClientImpl::NetworkClientImpl()
{
  RegisterMsgHandler(msgid_newplayer, clienthandlenewplayer);
  RegisterMsgHandler(msgid_newplayer2, clienthandlenewplayer2);
}