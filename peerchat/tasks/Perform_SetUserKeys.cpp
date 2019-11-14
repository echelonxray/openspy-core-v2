#include "tasks.h"
#include <sstream>
#include <server/Server.h>

namespace Peerchat {

	bool Perform_SetUserKeys(PeerchatBackendRequest request, TaskThreadData* thread_data) {
		TaskResponse response;

		std::map<std::string, std::string> broadcast_keys;
		UserSummary user_summary = GetUserSummaryByName(thread_data, request.summary.username);

        std::pair<std::vector<std::pair< std::string, std::string> >::const_iterator, std::vector<std::pair< std::string, std::string> >::const_iterator> iterators = request.channel_modify.kv_data.GetHead();
        std::vector<std::pair< std::string, std::string> >::const_iterator it = iterators.first;
        while (it != iterators.second) {
            std::pair<std::string, std::string> p = *it;
            Redis::Command(thread_data->mp_redis_connection, 0, "HSET user_%d \"custkey_%s\" \"%s\"", user_summary.id, p.first.c_str(), p.second.c_str());
            it++;
        }

		if (request.callback) {
			request.callback(response, request.peer);
		}
		if (request.peer) {
			request.peer->DecRef();
		}
		return true;
	}
}