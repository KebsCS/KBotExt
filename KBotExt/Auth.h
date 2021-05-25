#pragma once

#include <string>

class Auth
{
public:
	int riotPort = 0;
	std::string riotToken;
	std::string riotHeader;

	int leaguePort = 0;
	std::string leagueToken;
	std::string leagueHeader;

	// returns 1 when successfully got port and token
	bool GetRiotClientInfo();

	void MakeRiotHeader();

	// returns 1 when successfully got port and token
	bool GetLeagueClientInfo();

	void MakeLeagueHeader();
private:
	std::wstring GetProcessCommandLine(std::string sProcessName);
};

extern Auth* auth;
