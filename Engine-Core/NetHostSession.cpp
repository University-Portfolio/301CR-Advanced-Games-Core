#include "Includes\Core\NetHostSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"


NetHostSession::NetHostSession(const Engine* engine, const NetIdentity identity) :
	NetSession(engine, identity)
{
	bIsHost = true;
}

NetHostSession::~NetHostSession()
{
	for (NetClient* c : m_clients)
		delete c;

	LOG("NetHostSession closed.");
}

bool NetHostSession::Start() 
{
	const NetIdentity& host = GetSessionIdentity();

	if (!m_TcpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). TCP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		return false;
	}
	if (!m_UdpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). UDP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		return false;
	}

	bIsConnected = true;

	LOG("Host net session openned on (%s:%i)", host.ip.toString().c_str(), host.port);
	return true;
}

void NetHostSession::Update(const float& deltaTime)
{
	// Read from remote
	{
		std::vector<RawNetPacket> packets;

		// Fetch TCP packets
		if (m_TcpSocket.Poll(packets))
		{
			for (RawNetPacket& p : packets)
			{
				p.buffer.Flip();
				LOG("r %i", p.buffer.Size());
				ForwardToClient(p, m_TcpSocket);
			}
		}

		// Fetch UDP packets
		packets.clear();
		if (m_UdpSocket.Poll(packets))
		{
			// Fetch UDP packets
			for (RawNetPacket& p : packets)
			{
				p.buffer.Flip();
				ForwardToClient(p, m_UdpSocket);
			}
		}
	}

}

NetResponseCode NetHostSession::VerifyHandshake(NetClient* client, RawNetPacket& packet, NetSocket& socket)
{
	// Check message header
	uint16 requestType;
	{
		Version engineVersion;
		Version gameVersion;

		// Invalid header
		if (!Decode<Version>(packet.buffer, engineVersion) ||
			!Decode<Version>(packet.buffer, gameVersion) ||
			!Decode<uint16>(packet.buffer, requestType))
		{
			ByteBuffer response;
			Encode<uint16>(response, NetResponseCode::BadRequest);
			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::BadRequest;
		}

		// Invalid versions
		if (m_engine->GetVersionNo() != engineVersion || m_engine->GetGame()->GetVersionNo() != gameVersion)
		{
			ByteBuffer response;
			Encode<uint16>(response, NetResponseCode::BadVersions);
			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::BadVersions;
		}

		// TODO - Ban checks
		// TODO - Password checks
		// TODO - Whitelist checks
		// TODO - Server is full checks
	}


	// Perform any additional checks/payload filling
	ByteBuffer response;
	NetRequestType request = (NetRequestType)requestType;

	switch (request)
	{
	// User is just pinging the server to get a response
	case Ping:
		Encode<uint16>(response, NetResponseCode::Responded);

		socket.SendTo(response.Data(), response.Size(), packet.source);
		return NetResponseCode::Responded;


	// The user is attempting to connect to the server as a player
	case Connect:
		Encode<uint16>(response, NetResponseCode::Accepted);

		socket.SendTo(response.Data(), response.Size(), packet.source);
		return NetResponseCode::Accepted;


	// The user is querying for server details
	case Query:
		Encode<uint16>(response, NetResponseCode::Responded);
		Encode<uint16>(response, m_clients.size());		// Players connected 
		Encode<uint16>(response, maxPlayerCount);		// Player limit
		Encode<string>(response, "Unnamed Server");		// TODO - Server name
		Encode<uint8>(response, 0);						// TODO - Bitflags

		socket.SendTo(response.Data(), response.Size(), packet.source);
		return NetResponseCode::Responded;
	}

	return NetResponseCode::Unknown;
}

NetClient* NetHostSession::GetClient(const NetIdentity& identity) 
{
	NetClient* client = m_clientLookup[identity];
	if (client != nullptr)
		return client;

	return nullptr;
}

void NetHostSession::ForwardToClient(RawNetPacket& packet, NetSocket& socket) 
{
	const NetIdentity identity = packet.source;
	NetClient* client = GetClient(identity);
	

	// Perform handshake verification
	if (client == nullptr && socket.GetSocketType() == SocketType::TCP)
	{
		client = new NetClient(identity);

		// If hand shake succeeds, track client
		if (VerifyHandshake(client, packet, socket) == NetResponseCode::Accepted)
		{
			m_clients.emplace_back(client);
			m_clientLookup[identity] = client;
			LOG("Client connected from %s:%i", identity.ip.toString().c_str(), identity.port);
		}
		else
			delete client;
		return;
	}


	// Invalid client (Probably spam/invalid traffic)
	if (client == nullptr)
		return;


	// TODO - Actually do stuff with the packets
}