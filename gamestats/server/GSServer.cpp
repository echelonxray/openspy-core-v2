#include <OS/OpenSpy.h>
#include <OS/TaskPool.h>
#include "GSPeer.h"
#include "GSServer.h"
#include "GSDriver.h"

namespace GS {
	Server::Server() : INetServer(){
	}
	void Server::init() {
		mp_gamestats_tasks = GS::InitTasks(this);
	}
	void Server::tick() {
	std::vector<INetDriver *>::iterator it = m_net_drivers.begin();
		while (it != m_net_drivers.end()) {
			INetDriver *driver = *it;
			driver->think(false);
			it++;
		}
		NetworkTick();
	}
	void Server::shutdown() {

	}
}