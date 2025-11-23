#include "TetrisServer.h"
#include <iostream>
#include <random>
#include <chrono>

TetrisServer::TetrisServer(uint16_t nPort)
	: sp::net::server_interface<GameMsg>(nPort)
{
}

bool TetrisServer::OnClientConnect(std::shared_ptr<sp::net::connection<GameMsg>> client)
{
	return true;
}

void TetrisServer::OnClientDisconnect(std::shared_ptr<sp::net::connection<GameMsg>> client)
{
	if (client)
	{
		if (m_mapConnectedPlayers.find(client->GetID()) == m_mapConnectedPlayers.end())
		{
			// 존재하지 않은 플레이어
		}
		else
		{
			auto& pd = m_mapConnectedPlayers[client->GetID()];
			std::cout << "[UNGRACEFUL REMOVAL]:" + std::to_string(pd.nUniqueID) + "\n";
			m_mapConnectedPlayers.erase(client->GetID());
			m_vGarbageIDs.push_back(client->GetID());
		}
	}
}

void TetrisServer::OnMessage(std::shared_ptr<sp::net::connection<GameMsg>> client, sp::net::message<GameMsg>& msg)
{
	if (!m_vGarbageIDs.empty())
	{
		for (auto pid : m_vGarbageIDs)
		{
			sp::net::message<GameMsg> msgOut;
			msgOut.header.id = GameMsg::Game_RemovePlayer;
			msgOut << pid;
			std::cout << "Removing " << pid << "\n";
			MessageAllClients(msgOut);
		}

		m_vGarbageIDs.clear();
	}

	switch (msg.header.id)
	{
	case GameMsg::Client_RegisterWithServer:
	{
		sPlayerDescription desc;
		msg >> desc;
		desc.nUniqueID = client->GetID();
		m_mapConnectedPlayers.insert_or_assign(desc.nUniqueID, desc);

		sp::net::message<GameMsg> msgOut;
		msgOut.header.id = GameMsg::Client_AssignID;
		msgOut << desc.nUniqueID;
		MessageClient(client, msgOut);

		break;
	}

	case GameMsg::Client_UnregisterWithServer:
	{

		break;
	}

	case GameMsg::Client_RequestRoomJoin:
	{
		{
			sp::net::message<GameMsg> msgOut;
			msgOut.header.id = GameMsg::Server_RoomJoinAccepted;
			MessageClient(client, msgOut);

			std::cout << "[Client_RequestRoomJoin] Success! [connected player count] : " << std::to_string(m_mapConnectedPlayers.size()) << "\n";
		}

		if (m_mapConnectedPlayers.size() == 2)
		{
			{
				sp::net::message<GameMsg> msgOut;
				msgOut.header.id = GameMsg::Game_SendBagSeed;
				msgOut << GenerateSeed();
				MessageAllClients(msgOut);
			}

			{
				sp::net::message<GameMsg> msgOut;
				msgOut.header.id = GameMsg::Server_AllPlayersReady;
				MessageAllClients(msgOut);
			}
		}

		break;
	}

	case GameMsg::Game_CurMinoState:
	case GameMsg::Game_PreviewMinoState:
	case GameMsg::Game_HoldMinoState:
	case GameMsg::Game_BoardState:
	{
		MessageAllClients(msg, client);
		break;
	}

	case GameMsg::Game_UpdatePlayer:
	{
		MessageAllClients(msg, client);

		break;
	}

	case GameMsg::Game_PlayerDead:
	{
		uint32_t winnerID{ 0 }, loserID{ 0 };

		msg >> loserID;

		if (m_mapConnectedPlayers.size() != 2)
		{
			std::cout << "[Game_PlayerDead] Failed! [connected player count] : " << std::to_string(m_mapConnectedPlayers.size()) << "\n";
		}

		for (auto& kv : m_mapConnectedPlayers)
		{
			if (kv.first != loserID)
			{
				winnerID = kv.first;
				break;
			}
		}

		sp::net::message<GameMsg> msgOut;
		msgOut.header.id = GameMsg::Server_GameOver;
		
		sGameOverInfo info;
		info.nWinnerID = winnerID;
		info.nLoserID = loserID;
		msgOut << info;

		MessageAllClients(msgOut);
		break;
	}

	}
}

void TetrisServer::OnClientValidated(std::shared_ptr<sp::net::connection<GameMsg>> client)
{
	sp::net::message<GameMsg> msg;
	msg.header.id = GameMsg::Client_Accepted;
	client->Send(msg);
}

uint64_t TetrisServer::GenerateSeed()
{
	uint64_t rd = ((uint64_t)std::random_device{}() << 32)
		^ (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count()
		^ (uint64_t)std::hash<std::thread::id>()(std::this_thread::get_id());

	return rd;
}
