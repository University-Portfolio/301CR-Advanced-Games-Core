#include "Includes\Core\DefaultNetLayer.h"
#include "Includes\Core\Encoding.h"
#include "Includes\Core\PlayerController.h"
#include <Windows.h>


CLASS_SOURCE(DefaultNetLayer, CORE_API)


DefaultNetLayer::DefaultNetLayer() 
{
	m_connFlags = 0;
	m_password = "";
}

void DefaultNetLayer::OnBegin() 
{
}

void DefaultNetLayer::OnEncodeHandshake(const NetIdentity& host, ByteBuffer& outBuffer)
{
	// Use PC name as player's name
	string playerName;
	TCHAR name[STR_MAX_ENCODE_LEN];
	DWORD count = STR_MAX_ENCODE_LEN;
	if (GetUserName(name, &count))
		playerName = name;
	

	Encode<string>(outBuffer, playerName);
	Encode<string>(outBuffer, m_password);
}

NetResponseCode DefaultNetLayer::OnDecodeHandshake(const NetIdentity& connection, ByteBuffer& inBuffer, OPlayerController*& outPlayer)
{
	string playerName;
	string password;

	if (!Decode(inBuffer, playerName) || !Decode(inBuffer, password))
		return NetResponseCode::BadRequest;

	if (!m_password.empty() && m_password != password)
		return NetResponseCode::BadAuthentication;

	outPlayer->SetPlayerName(playerName);
	return NetResponseCode::Accepted;
}

void DefaultNetLayer::SetPassword(const string& pass) 
{
	m_password = pass;

	// Bit flag
	if (m_password.empty())
		m_connFlags &= ~LAYER_FLAG_PASSWORD;
	else
		m_connFlags |= LAYER_FLAG_PASSWORD;
}