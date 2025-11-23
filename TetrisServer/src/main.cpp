#include "TetrisServer.h"

int main()
{
	TetrisServer server(60000);
	server.Start();

	while (true)
	{
		server.Update(-1, true);
	}

	return 0;
}