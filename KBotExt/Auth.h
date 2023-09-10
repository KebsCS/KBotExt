#pragma once

#include <string>
#include <vector>

#include "base64.h"

struct client_info
{
	int port = 0;
	std::string token;
	std::string header;
	std::string version;
	std::wstring path;
};

class auth
{
public:
	auth() = default;
	~auth() = default;

	static std::vector<DWORD> get_all_process_ids(const std::wstring& process_name);

	// set riotClient flag when you need Riot Client info but RiotClientUx.exe is closed
	static client_info get_client_info(const DWORD& pid, bool riot_client = false);
	static DWORD get_process_id(const std::wstring& process_name);

	static std::string make_league_header(const client_info& info);
	static std::string make_riot_header(const client_info& info);

private:
	static inline base64 base64_;

	static int get_port(const std::string& cmd_line, bool riot_client = false);
	static std::string get_token(const std::string& cmd_line, bool riot_client = false);

	static std::wstring get_process_command_line(const DWORD& process_id);
	static std::wstring get_process_path(const DWORD& process_id);
	static std::string get_file_version(const std::wstring& file);
};
