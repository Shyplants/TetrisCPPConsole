#include "TetrisClient.h"

//void TetrisClient::PingServer()
//{
//	sp::net::message<GameMsg> msg;
//	msg.header.id = GameMsg::ServerPing;
//
//	std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
//
//	msg << timeNow;
//	Send(msg);
//}
//
//void TetrisClient::MessageAll()
//{
//	sp::net::message<GameMsg> msg;
//	msg.header.id = GameMsg::MessageAll;
//	Send(msg);
//}
