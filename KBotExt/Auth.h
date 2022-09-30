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
	std::string riotVersion;
	std::wstring riotPath;

	int leaguePort = 0;
	std::string leagueToken;
	std::string leagueHeader;
	std::string leagueVersion;
	std::wstring leaguePath;

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

	DWORD GetProcessId(std::wstring processName);
	std::wstring GetProcessCommandLine(DWORD processId);
	std::wstring GetProcessPath(DWORD processId);
	std::string GetFileVersion(std::wstring file);
};

extern Auth* auth;
