#pragma once

#include <string>

#include "base64.h"

class Auth
{
public:
	Auth() = default;
	~Auth() = default;

	int riotPort = 0;
	std::string riotToken;
	std::string riotHeader;

	int leaguePort = 0;
	std::string leagueToken;
	std::string leagueHeader;

	// returns true on success
	bool GetRiotClientInfo();

	void MakeRiotHeader();

	// returns true on success
	bool GetLeagueClientInfo();

	void MakeLeagueHeader();
private:
	Base64 base64;

	int GetPort(std::string cmdLine);
	std::string GetToken(std::string cmdLine);
	std::wstring GetProcessCommandLine(std::wstring processName);
};

extern Auth* auth;
