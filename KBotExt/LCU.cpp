#include <json/json.h>
#include <thread>
#include "Utils.h"

#include "LCU.h"

static void FixCachingProblem();

std::string LCU::Request(const std::string& method, const std::string& endpoint, const std::string& body)
{
	if (league.port == 0)
		return "Not connected to League";
	std::string sURL = endpoint;
	if (sURL.find("https://127.0.0.1") == std::string::npos)
	{
		if (sURL.find("https://") == std::string::npos && sURL.find("http://") == std::string::npos)
		{
			while (sURL[0] == ' ')
				sURL.erase(sURL.begin());
			if (sURL[0] != '/')
				sURL.insert(0, "/");
			sURL.insert(0, "https://127.0.0.1:" + std::to_string(league.port));
		}
	}
	else if (sURL.find("https://127.0.0.1:") == std::string::npos)
	{
		sURL.insert(strlen("https://127.0.0.1"), ":" + std::to_string(league.port));
	}

	/*wrap::Response r = wrap::HttpsRequest(wrap::Method{ method }, wrap::Url{ sURL }, wrap::Body{ body }, wrap::Header{ league.header }, wrap::Port{ league.port },
		wrap::Timeout{1000});*/

	cpr::Response r;

	LCU::session.SetUrl(sURL);
	LCU::session.SetBody(body);

	const std::string upperMethod = Utils::ToUpper(method);

	if (upperMethod == "GET")
	{
		r = LCU::session.Get();
	}
	else if (upperMethod == "POST")
	{
		r = LCU::session.Post();
	}
	else if (upperMethod == "OPTIONS")
	{
		r = LCU::session.Options();
	}
	else if (upperMethod == "DELETE")
	{
		r = LCU::session.Delete();
	}
	else if (upperMethod == "PUT")
	{
		r = LCU::session.Put();
	}
	else if (upperMethod == "HEAD")
	{
		r = LCU::session.Head();
	}
	else if (upperMethod == "PATCH")
	{
		r = LCU::session.Patch();
	}

	return r.text;
}

bool LCU::SetRiotClientInfo(const ClientInfo& info)
{
	riot = info;

	if (riot.port == 0 || riot.token == "")
		return false;

	riot.header = Auth::MakeRiotHeader(info);

	return true;
}

bool LCU::SetRiotClientInfo()
{
	return SetRiotClientInfo(Auth::GetClientInfo(Auth::GetProcessId(L"RiotClientUx.exe")));
}

bool LCU::SetLeagueClientInfo(const ClientInfo& info)
{
	isCurrentRiotInfoSet = false;

	league = info;

	if (league.port == 0 || league.token == "")
		return false;

	league.header = Auth::MakeLeagueHeader(info);

	session = cpr::Session();
	session.SetVerifySsl(false);

	session.SetHeader(Utils::StringToHeader(league.header));

	return true;
}

bool LCU::SetLeagueClientInfo()
{
	if (!LCU::IsProcessGood())
		return false;
	return SetLeagueClientInfo(Auth::GetClientInfo(LCU::leagueProcesses[LCU::indexLeagueProcesses].first));
}

bool LCU::SetCurrentClientRiotInfo()
{
	if (isCurrentRiotInfoSet)
		return true;

	bool isSet = SetRiotClientInfo(Auth::GetClientInfo(LCU::leagueProcesses[LCU::indexLeagueProcesses].first, true));
	if (isSet)
		isCurrentRiotInfoSet = true;

	return isSet;
}

void LCU::GetLeagueProcesses()
{
	std::vector<DWORD>allProcessIds = Auth::GetAllProcessIds(L"LeagueClientUx.exe");
	// remove unexisting clients
	for (size_t i = 0; i < LCU::leagueProcesses.size(); i++)
	{
		bool exists = false;
		for (const DWORD& proc : allProcessIds)
		{
			if (proc == LCU::leagueProcesses[i].first)
			{
				exists = true;
				break;
			}
		}
		if (!exists)
		{
			LCU::leagueProcesses.erase(LCU::leagueProcesses.begin() + i);
			if (i == LCU::indexLeagueProcesses)
				LCU::SetLeagueClientInfo();
		}
	}

	for (const DWORD& proc : allProcessIds)
	{
		int foundIndex = -1;
		for (size_t i = 0; i < LCU::leagueProcesses.size(); i++)
		{
			if (LCU::leagueProcesses[i].first == proc)
			{
				foundIndex = static_cast<int>(i);
				break;
			}
		}

		if (foundIndex != -1 && !LCU::leagueProcesses[foundIndex].second.empty())
			continue;

		ClientInfo currInfo = Auth::GetClientInfo(proc);
		currInfo.header = Auth::MakeLeagueHeader(currInfo);

		size_t currentIndex = foundIndex;
		if (foundIndex == -1)
		{
			currentIndex = LCU::leagueProcesses.size();
			std::pair<DWORD, std::string>temp = { proc, "" };
			LCU::leagueProcesses.emplace_back(temp);
		}

		std::thread t([currentIndex, currInfo]()
			{
				short sessionFailCount = 0;
				while (true)
				{
					cpr::Session session;
					session.SetVerifySsl(false);
					session.SetHeader(Utils::StringToHeader(currInfo.header));
					session.SetUrl(std::format("https://127.0.0.1:{}/lol-login/v1/session", currInfo.port));
					std::string procSession = session.Get().text;

					// probably legacy client
					if (procSession.find("errorCode") != std::string::npos)
					{
						sessionFailCount++;
						if (sessionFailCount > 5)
						{
							LCU::leagueProcesses[currentIndex].second = "!FAILED!";
							break;
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(300));
						continue;
					}

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;

					if (reader->parse(procSession.c_str(), procSession.c_str() + static_cast<int>(procSession.length()), &root, &err))
					{
						std::string currSummId = root["summonerId"].asString();
						// player has summId when client is loaded
						if (!currSummId.empty())
						{
							session.SetUrl(std::format("https://127.0.0.1:{}/lol-summoner/v1/summoners/{}", currInfo.port, currSummId));
							std::string currSummoner = session.Get().text;

							if (reader->parse(currSummoner.c_str(), currSummoner.c_str() + static_cast<int>(currSummoner.length()), &root, &err))
							{
								LCU::leagueProcesses[currentIndex].second = std::string(root["displayName"].asString().substr(0, 25));
								break;
							}
						}
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
				}
			});
		t.detach();
	}
}

bool LCU::IsProcessGood()
{
	return !LCU::leagueProcesses.empty() && LCU::indexLeagueProcesses < LCU::leagueProcesses.size();
}

// todo, sync with LCU header, it's almost the same
std::string LCU::GetStoreHeader()
{
	std::string storeUrl = LCU::Request("GET", "/lol-store/v1/getStoreUrl");
	storeUrl.erase(std::remove(storeUrl.begin(), storeUrl.end(), '"'), storeUrl.end());

	std::string accessToken = LCU::Request("GET", "/lol-rso-auth/v1/authorization/access-token");
	Json::CharReaderBuilder builder;
	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	JSONCPP_STRING err;
	Json::Value root;
	if (reader->parse(accessToken.c_str(), accessToken.c_str() + static_cast<int>(accessToken.length()), &root, &err))
	{
		std::string storeToken = root["token"].asString();
		std::string storeHost = "";
		auto n = storeUrl.find("https://");
		if (n != std::string::npos)
		{
			storeHost = storeUrl.substr(n + strlen("https://"));
		}
		std::string storeHeader = "Host: " + storeHost + "\r\n" +
			"Connection: keep-alive\r\n" +
			"AUTHORIZATION: Bearer " + storeToken + "\r\n" +
			"Accept: application/json" + "\r\n" +
			"Accept-Language: en-US,en;q=0.9" + "\r\n" +
			"Content-Type: application/json" + "\r\n" +
			"Origin: https://127.0.0.1:" + std::to_string(LCU::league.port) + "\r\n" +
			"User-Agent: Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) LeagueOfLegendsClient/" + LCU::league.version + " (CEF 91) Safari/537.36" + "\r\n" +
			//X-B3-SpanId:
			//X-B3-TraceId:
			"sec-ch-ua: \"Chromium\";v=\"91\"" + "\r\n" +
			"sec-ch-ua-mobile: ?0" + "\r\n" +
			"Sec-Fetch-Site: same-origin" + "\r\n" +
			"Sec-Fetch-Mode: no-cors" + "\r\n" +
			"Sec-Fetch-Dest: empty" + "\r\n" +
			"Referer: https://127.0.0.1:" + std::to_string(LCU::league.port) + "\r\n" +
			"Accept-Encoding: "/*gzip,*/ + "deflate, br";
		return storeHeader;
	}

	return "";
}

// Fixes Wininet error 12158, #80 in GitHub issues
static void FixCachingProblem()
{
	typedef LSTATUS(WINAPI* tRegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
		REGSAM samDesired, PHKEY phkResult);
	static tRegOpenKeyExA RegOpenKeyExA = (tRegOpenKeyExA)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegOpenKeyExA");

	typedef LSTATUS(WINAPI* tRegQueryValueExA)(HKEY hKey, LPCSTR lpValueName,
		LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbDatan);
	static tRegQueryValueExA RegQueryValueExA = (tRegQueryValueExA)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegQueryValueExA");

	typedef LSTATUS(WINAPI* tRegSetValueExA)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
		DWORD dwType, const BYTE* lpData, DWORD cbData);
	static tRegSetValueExA RegSetValueExA = (tRegSetValueExA)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegSetValueExA");

	typedef LSTATUS(WINAPI* tRegCloseKey)(HKEY hKe);
	static tRegCloseKey RegCloseKey = (tRegCloseKey)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCloseKey");

	bool bChanged = false;
	HKEY hkResult;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ | KEY_WRITE, &hkResult) == ERROR_SUCCESS)
	{
		DWORD value;
		DWORD newValue = 0;
		DWORD dwSize = sizeof(value);
		LSTATUS regQuery = RegQueryValueExA(hkResult, "DisableCachingOfSSLPages", 0, NULL, (LPBYTE)&value, &dwSize);
		if (regQuery == ERROR_SUCCESS)
		{
			if (value == 0x1)
			{
				RegSetValueExA(hkResult, "DisableCachingOfSSLPages", 0, REG_DWORD, (LPBYTE)&newValue, dwSize);
				bChanged = true;
			}
		}
		else if (regQuery == ERROR_FILE_NOT_FOUND) // if key doesnt exist, create it
		{
			RegSetValueExA(hkResult, "DisableCachingOfSSLPages", 0, REG_DWORD, (LPBYTE)&newValue, dwSize);
			bChanged = true;
		}
		RegCloseKey(hkResult);
	}

	if (bChanged == true)
	{
		MessageBoxA(NULL, "Restart the program\n\nIf this pop-up window keeps showing up: Open \"Internet Options\", "
			"Go to \"Advanced\" tab and disable \"Do not save encrypted pages to disk\". Press \"Apply\" and \"OK\"",
			"Updated faulty options", MB_OK);
		exit(EXIT_SUCCESS);
	}
}