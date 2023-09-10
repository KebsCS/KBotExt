#pragma once

#include <cpr/cpr.h>

#include "Definitions.h"
#include "Auth.h"

class lcu
{
public:
	static inline cpr::Session session;

	static inline client_info league;
	static inline client_info riot;

	static std::string request(const std::string& method, const std::string& endpoint, const std::string& body = "");

	static bool set_riot_client_info(const client_info& info);
	static bool set_riot_client_info();

	static bool set_league_client_info(const client_info& info);
	static bool set_league_client_info();

	static bool set_current_client_riot_info();

	static inline std::vector<std::pair<DWORD, std::string>> league_processes;
	static inline size_t index_league_processes = 0; // currently selected process
	static void get_league_processes();
	static bool is_process_good();

	static std::string get_store_header();

private:
	static inline bool is_current_riot_info_set_ = false;
};
