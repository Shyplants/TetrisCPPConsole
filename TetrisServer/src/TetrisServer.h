#pragma once

#include "common/PacketProtocol.h"

class TetrisServer : public sp::net::server_interface<GameMsg>
{
public:
	TetrisServer(uint16_t nPort);

protected:
	bool OnClientConnect(std::shared_ptr<sp::net::connection<GameMsg>> client) override;
	void OnClientDisconnect(std::shared_ptr<sp::net::connection<GameMsg>> client) override;
	void OnMessage(std::shared_ptr<sp::net::connection<GameMsg>> client, sp::net::message<GameMsg>& msg) override;
	void OnClientValidated(std::shared_ptr<sp::net::connection<GameMsg>> client) override;

private:
	uint64_t GenerateSeed();

private:
	std::unordered_map<uint32_t, sPlayerDescription> m_mapConnectedPlayers;
	std::vector<uint32_t> m_vGarbageIDs;
};