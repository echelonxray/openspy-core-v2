#include <OS/OpenSpy.h>
#include <OS/Buffer.h>
#include <sstream>

#include "../UTPeer.h"
#include "../UTDriver.h"
#include "../UTServer.h"
#include <tasks/tasks.h>
namespace UT {

	void Peer::handle_heartbeat(OS::Buffer buffer) {
		MM::ServerRecord record;
		record.m_address = getAddress();

		std::stringstream ss;
		
		//read unknown properties
		uint8_t num_addresses = buffer.ReadByte();
		ss << "Clients (";
		while(num_addresses--) {
			std::string ip_address = Read_FString(buffer, true);
			ss << ip_address << " ";			
		}
		ss << ") ";

		//unknown data
		buffer.ReadByte(); buffer.ReadInt(); 

		record.m_address.port = htons(buffer.ReadShort());
		ss << " Address: " << record.m_address.ToString();

		//read more unknown properties
		buffer.ReadInt(); buffer.ReadShort();

		record.hostname = Read_FString(buffer, true);
		ss << " Hostname: " << record.hostname;

		record.level = Read_FString(buffer, true);
		ss << " Level: " << record.level;

		record.game_group = Read_FString(buffer, true);
		ss << " Game group: " << record.game_group;

		int num_players = buffer.ReadInt(), max_players = buffer.ReadInt(); /*, unk5 = buffer.ReadInt(); */ 
		buffer.ReadInt(); //unk5
		
		record.num_players = num_players;
		record.max_players = max_players;
		ss << " Players: (" << record.num_players << "/" << record.max_players << ") ";
		
		if(m_client_version >= 3000) {
			buffer.ReadInt();

			record.bot_level = Read_FString(buffer, true);
			ss << "Bot: " << record.bot_level << " ";
		}
		
		uint8_t num_fields = buffer.ReadByte();

		for(int i=0;i<num_fields;i++) {
			std::string field = Read_FString(buffer, true);
			std::string property = Read_FString(buffer, true);

			ss << "(" << field << "," << property << "), ";

			if(field.compare("mutator") == 0) {
				record.m_mutators.push_back(property);
			} else {
				record.m_rules[field] = property;
			}
		} 
		uint8_t num_player_entries = buffer.ReadByte();
		
		ss << " Players (";
		for(int i=0;i<num_player_entries;i++) {
			MM::PlayerRecord player_record;
			player_record.id = buffer.ReadInt();
			player_record.name = Read_FString(buffer, true);

			player_record.ping = buffer.ReadInt();
			player_record.score = buffer.ReadInt();
			player_record.stats_id = buffer.ReadInt();

			ss << player_record.name << "(" << player_record.id << "," << player_record.ping << "," << player_record.score << "," << player_record.stats_id << "),";
			
			record.m_players.push_back(player_record);
		}
		ss << ")";
		OS::LogText(OS::ELogLevel_Info, "[%s] HB: %s", getAddress().ToString().c_str(), ss.str().c_str());

		m_server_address = record.m_address;

		//inject version for UT2003
		ss.str("");
		ss << m_client_version;
		record.m_rules["ServerVersion"] = ss.str();

        TaskScheduler<MM::UTMasterRequest, TaskThreadData> *scheduler = ((UT::Server *)(this->GetDriver()->getServer()))->getScheduler();
        MM::UTMasterRequest req;        
        req.type = MM::UTMasterRequestType_Heartbeat;
		req.peer = this;
		req.peer->IncRef();
		req.callback = NULL;
		req.record = record;
        scheduler->AddRequest(req.type, req);

	}


    //this seems to be more stats related... but here for now, happens on non-stats games as well, just not much data
	void Peer::handle_newserver_request(OS::Buffer recv_buffer) {
		uint32_t unk1 = recv_buffer.ReadInt();

		OS::LogText(OS::ELogLevel_Info, "[%s] Server new server request: %d", getAddress().ToString().c_str(), unk1);

		send_server_id(1234); //init stats backend, generate match id, for now not needed
	}
	void Peer::send_heartbeat_request(uint8_t id, uint32_t code) {
		OS::Buffer send_buffer;
		send_buffer.WriteByte(EServerOutgoingRequest_RequestHeartbeat);
		send_buffer.WriteByte(id);
		send_buffer.WriteInt(code);
		send_packet(send_buffer);
	}
	void Peer::send_server_id(int id) {
		OS::Buffer send_buffer;
		send_buffer.WriteByte(3);
		send_buffer.WriteInt(id);
		send_packet(send_buffer);
	}

}