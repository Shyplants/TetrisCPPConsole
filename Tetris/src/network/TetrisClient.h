#pragma once

#include "../common/PacketProtocol.h"

class TetrisClient : public sp::net::client_interface<GameMsg>
{
public:
	uint32_t GetPlayerID() { return m_PlayerID; }
	void SetPlayerID(uint32_t id) { m_PlayerID = id; }

private:
	uint32_t m_PlayerID{ 0 };
};