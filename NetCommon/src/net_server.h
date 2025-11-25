#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace sp
{
	namespace net
	{
		template <typename T>
		class server_interface
		{
		public:
			server_interface(uint16_t port)
				: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

			}

			virtual ~server_interface()
			{
				Stop();
			}

			bool Start()
			{
				try
				{
					WaitForClientConnection();

					m_threadContext = std::thread([this]() {m_asioContext.run(); });
				}
				catch (std::exception& e)
				{
					std::cerr << "[SERVER] Exception: " << e.what() << "\n";
					return false;
				}

				std::cout << "[SERVER] Started!\n";
				return true;
			}

			void Stop()
			{
				m_asioContext.stop();

				if (m_threadContext.joinable())
					m_threadContext.join();

				std::cout << "[SERVER] Stopped!\n";
			}

			void WaitForClientConnection()
			{
				m_asioAcceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

							std::shared_ptr<connection<T>> newClient =
								std::make_shared<connection<T>>(connection<T>::owner::server, m_asioContext, std::move(socket), m_qMessagesIn);

							if (OnClientConnect(newClient))
							{
								m_deqConnections.push_back(std::move(newClient));

								m_deqConnections.back()->ConnectToClient(this, nIDCounter++);

								std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";
							}
							else
							{
								std::cout << "[-----] Connection Denied\n";
							}
						}
						else
						{
							std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
						}

						WaitForClientConnection();
					});
			}

			void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{
				if (client && client->IsConnected())
				{
					client->Send(msg);
				}
			}

			void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
			{
				for (auto& client : m_deqConnections)
				{
					if (client && client->IsConnected())
					{
						if (client != pIgnoreClient)
							client->Send(msg);
					}
				}
			}

			void Update(size_t nMaxMessages = -1, bool bWait = false)
			{
				if (bWait)
					m_qMessagesIn.wait();

				size_t nMessageCount = 0;
				while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
				{
					auto msg = m_qMessagesIn.pop_front();

					OnMessage(msg.remote, msg.msg);

					nMessageCount++;
				}

				bool bInvalidClientExists = false;
				for (auto& client : m_deqConnections)
				{
					if (client && !client->IsConnected())
					{
						OnClientDisconnect(client);
						client.reset();

						bInvalidClientExists = true;
					}
				}

				if (bInvalidClientExists)
				{
					m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
				}

				OnUpdate();
			}

		protected:
			std::shared_ptr<connection<T>> GetConnectionByID(uint32_t id)
			{
				for (auto& conn : m_deqConnections)
				{
					if (conn->GetID() == id)
						return conn;
				}
				return nullptr;
			}

		protected:
			virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
			{
				return false;
			}

			virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
			{

			}

			virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
			{

			}

			virtual void OnUpdate()
			{

			}

		protected:

		public:
			virtual void OnClientValidated(std::shared_ptr<connection<T>> client)
			{

			}

		protected:
			tsqueue<owned_message<T>> m_qMessagesIn;
			std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

			asio::io_context m_asioContext;
			std::thread m_threadContext;

			asio::ip::tcp::acceptor m_asioAcceptor;

			uint32_t nIDCounter = 10000;
		};
	}
}