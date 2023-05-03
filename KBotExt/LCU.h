#pragma once

#include <cpr/cpr.h>

#include "Definitions.h"
#include "Auth.h"

class LCU
{
public:

	static inline cpr::Session session;

	static inline ClientInfo league;
	static inline ClientInfo riot;

	static std::string Request(const std::string& method, const std::string& endpoint, const std::string& body = "");

	static bool SetRiotClientInfo(const ClientInfo& info);
	static bool SetRiotClientInfo();

	static bool SetLeagueClientInfo(const ClientInfo& info);
	static bool SetLeagueClientInfo();

	static bool SetCurrentClientRiotInfo();

	static inline std::vector<std::pair<DWORD, std::string>> leagueProcesses;
	static inline size_t indexLeagueProcesses = 0; // currently selected process
	static void GetLeagueProcesses();
	static bool IsProcessGood();

	static std::string GetStoreHeader();
private:
	static inline bool isCurrentRiotInfoSet = false;
};
