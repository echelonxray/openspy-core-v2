#ifndef _MM_QUERY_H
#define _MM_QUERY_H
#include "main.h"
#include <vector>
#include <map>
#include <string>

namespace SB {
	struct sServerListReq;
	class Driver;
};


namespace MM {
	typedef struct {
		uint32_t ip;
		uint16_t port;
	} Address;
	void Init(SB::Driver *driver);
	void Think(); //check for push notifications, etc
	typedef struct {
		Address wan_address;
		Address lan_address;
		OS::GameData game;
		OS::countryRegion region;
		std::map<std::string, std::string> kvFields;
		
	} Server;

	struct ServerListQuery{
		std::vector<std::string> requested_fields;
		std::vector<Server *> list;
	};
	

	void AppendServerEntry(const char *entry_name, ServerListQuery *ret, bool all_keys);
	bool FindAppend_ServKVFields(Server *server, std::string entry_name, std::string key);
	
	struct MM::ServerListQuery GetServers(const SB::sServerListReq *req);
	struct MM::ServerListQuery GetGroups(const SB::sServerListReq *req);
	extern SB::Driver *mp_driver;
	extern redisContext *mp_redis_connection;
};

#endif //_MM_QUERY_H