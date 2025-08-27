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
};

NetworkClientImpl::NetworkClientImpl()
{
  
}

void clienthandleplayerslist(NetworkContext*);
void clienthandleunaccepted(NetworkContext*);
void clienthandlenewplayer(NetworkContext*);

auto constexpr msgid_custom = 0;
auto constexpr msgid_heartbeat = msgid_custom + 2;
auto constexpr msgid_playerslist = msgid_custom + 16;
auto constexpr msgid_newplayer = msgid_custom + 17;
auto constexpr msgid_unaccepted = msgid_custom + 44;

void NetworkClientImpl::HandleMessages(PSteamNetworkingMessage_t IncomingMsg)
{
  NetworkContext nc{};

  switch (PacketHeader->id)
  {
  case msgid_playerslist:
    clienthandleplayerslist(&nc);
    break;

  case msgid_unaccepted:
    clienthandleunaccepted(&nc);
    break;

  case msgid_newplayer:
    clienthandlenewplayer(&nc);
  }
}