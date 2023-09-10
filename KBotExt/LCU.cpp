#include <json/json.h>
#include <thread>
#include "Utils.h"

#include "LCU.h"

std::string lcu::request(const std::string& method, const std::string& endpoint, const std::string& body)
{
	if (league.port == 0)
		return "Not connected to League";
	std::string s_url = endpoint;
	if (s_url.find("https://127.0.0.1") == std::string::npos)
	{
		if (s_url.find("https://") == std::string::npos && s_url.find("http://") == std::string::npos)
		{
			while (s_url[0] == ' ')
				s_url.erase(s_url.begin());
			if (s_url[0] != '/')
				s_url.insert(0, "/");
			s_url.insert(0, "https://127.0.0.1:" + std::to_string(league.port));
		}
	}
	else if (s_url.find("https://127.0.0.1:") == std::string::npos)
	{
		s_url.insert(strlen("https://127.0.0.1"), ":" + std::to_string(league.port));
	}

	cpr::Response r = {};

	session.SetUrl(s_url);
	session.SetBody(body);

	if (const std::string upper_method = utils::to_upper(method); upper_method == "GET")
	{
		r = session.Get();
	}
	else if (upper_method == "POST")
	{
		r = session.Post();
	}
	else if (upper_method == "OPTIONS")
	{
		r = session.Options();
	}
	else if (upper_method == "DELETE")
	{
		r = session.Delete();
	}
	else if (upper_method == "PUT")
	{
		r = session.Put();
	}
	else if (upper_method == "HEAD")
	{
		r = session.Head();
	}
	else if (upper_method == "PATCH")
	{
		r = session.Patch();
	}

	return r.text;
}

bool lcu::set_riot_client_info(const client_info& info)
{
	riot = info;

	if (riot.port == 0 || riot.token.empty())
		return false;

	riot.header = auth::make_riot_header(info);

	return true;
}

bool lcu::set_riot_client_info()
{
	return set_riot_client_info(auth::get_client_info(auth::get_process_id(L"RiotClientUx.exe")));
}

bool lcu::set_league_client_info(const client_info& info)
{
	is_current_riot_info_set_ = false;

	league = info;

	if (league.port == 0 || league.token.empty())
		return false;

	league.header = auth::make_league_header(info);

	session = cpr::Session();
	session.SetVerifySsl(false);

	session.SetHeader(utils::string_to_header(league.header));

	return true;
}

bool lcu::set_league_client_info()
{
	if (!is_process_good())
		return false;
	return set_league_client_info(auth::get_client_info(league_processes[index_league_processes].first));
}

bool lcu::set_current_client_riot_info()
{
	if (is_current_riot_info_set_)
		return true;

	const bool is_set = set_riot_client_info(auth::get_client_info(league_processes[index_league_processes].first, true));
	if (is_set)
		is_current_riot_info_set_ = true;

	return is_set;
}

void lcu::get_league_processes()
{
	const std::vector<DWORD> all_process_ids = auth::get_all_process_ids(L"LeagueClientUx.exe");
	for (size_t i = 0; i < league_processes.size(); i++)
	{
		bool exists = false;
		for (const DWORD& proc : all_process_ids)
		{
			if (proc == league_processes[i].first)
			{
				exists = true;
				break;
			}
		}
		if (!exists)
		{
			league_processes.erase(league_processes.begin() + i);
			if (i == index_league_processes)
				set_league_client_info();
		}
	}

	for (const DWORD& proc : all_process_ids)
	{
		int found_index = -1;
		for (size_t i = 0; i < league_processes.size(); i++)
		{
			if (league_processes[i].first == proc)
			{
				found_index = static_cast<int>(i);
				break;
			}
		}

		if (found_index != -1 && !league_processes[found_index].second.empty())
			continue;

		client_info current_info = auth::get_client_info(proc);
		current_info.header = auth::make_league_header(current_info);

		size_t current_index = found_index;
		if (found_index == -1)
		{
			current_index = league_processes.size();
			std::pair<DWORD, std::string> temp = {proc, ""};
			league_processes.emplace_back(temp);
		}

		std::thread t([current_index, current_info]() {
			short session_fail_count = 0;
			while (true)
			{
				cpr::Session session;
				session.SetVerifySsl(false);
				session.SetHeader(utils::string_to_header(current_info.header));
				session.SetUrl(std::format("https://127.0.0.1:{}/lol-login/v1/session", current_info.port));
				std::string process_session = session.Get().text;

				// probably legacy client
				if (process_session.find("errorCode") != std::string::npos)
				{
					session_fail_count++;
					if (session_fail_count > 5)
					{
						league_processes[current_index].second = "!FAILED!";
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					continue;
				}

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				if (reader->parse(process_session.c_str(), process_session.c_str() + static_cast<int>(process_session.length()),
				                  &root, &err))
				{
					if (std::string current_summoner_id = root["summonerId"].asString(); !current_summoner_id.empty())
					{
						session.SetUrl(std::format("https://127.0.0.1:{}/lol-summoner/v1/summoners/{}", current_info.port,
						                           current_summoner_id));
						std::string current_summoner = session.Get().text;

						if (reader->parse(current_summoner.c_str(),
						                  current_summoner.c_str() + static_cast<int>(current_summoner.length()), &root, &err))
						{
							league_processes[current_index].second = std::string(
								root["displayName"].asString().substr(0, 25));
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

bool lcu::is_process_good()
{
	return !league_processes.empty() && index_league_processes < league_processes.size();
}

// todo, sync with LCU header, it's almost the same
std::string lcu::get_store_header()
{
	std::string store_url = request("GET", "/lol-store/v1/getStoreUrl");
	std::erase(store_url, '"');

	const std::string access_token = request("GET", "/lol-rso-auth/v1/authorization/access-token");
	const Json::CharReaderBuilder builder;
	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	JSONCPP_STRING err;
	Json::Value root;
	if (reader->parse(access_token.c_str(), access_token.c_str() + static_cast<int>(access_token.length()), &root, &err))
	{
		const std::string store_token = root["token"].asString();
		std::string store_host;
		if (const auto n = store_url.find("https://"); n != std::string::npos)
		{
			store_host = store_url.substr(n + strlen("https://"));
		}
		std::string store_header = "Host: " + store_host + "\r\n" +
			"Connection: keep-alive\r\n" +
			"AUTHORIZATION: Bearer " + store_token + "\r\n" +
			"Accept: application/json" + "\r\n" +
			"Accept-Language: en-US,en;q=0.9" + "\r\n" +
			"Content-Type: application/json" + "\r\n" +
			"Origin: https://127.0.0.1:" + std::to_string(league.port) + "\r\n" +
			"User-Agent: Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) LeagueOfLegendsClient/"
			+
			league.version + " (CEF 91) Safari/537.36" + "\r\n" +
			//X-B3-SpanId:
			//X-B3-TraceId:
			R"(sec-ch-ua: "Chromium";v="91")" + "\r\n" +
			"sec-ch-ua-mobile: ?0" + "\r\n" +
			"Sec-Fetch-Site: same-origin" + "\r\n" +
			"Sec-Fetch-Mode: no-cors" + "\r\n" +
			"Sec-Fetch-Dest: empty" + "\r\n" +
			"Referer: https://127.0.0.1:" + std::to_string(league.port) + "\r\n" +
			"Accept-Encoding: "/*gzip,*/ + "deflate, br";
		return store_header;
	}

	return "";
}

[[maybe_unused]] static void fix_caching_problem()
{
	using t_reg_open_key_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_sub_key, DWORD ul_options,
	                                             REGSAM sam_desired, PHKEY phk_result);
	static t_reg_open_key_ex_a reg_open_key_ex_a;
	reg_open_key_ex_a = reinterpret_cast<t_reg_open_key_ex_a>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "RegOpenKeyExA"));

	using t_reg_query_value_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_value_name,
	                                                LPDWORD lp_reserved, LPDWORD lp_type, LPBYTE lp_data, LPDWORD lpcb_datan);
	static t_reg_query_value_ex_a reg_query_value_ex_a;
	reg_query_value_ex_a = reinterpret_cast<t_reg_query_value_ex_a>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "RegQueryValueExA"));

	using t_reg_set_value_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_value_name, DWORD reserved,
	                                              DWORD dw_type, const BYTE* lp_data, DWORD cb_data);
	static t_reg_set_value_ex_a reg_set_value_ex_a;
	reg_set_value_ex_a = reinterpret_cast<t_reg_set_value_ex_a>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "RegSetValueExA"));

	using t_reg_close_key = LSTATUS(WINAPI*)(HKEY h_ke);
	static t_reg_close_key reg_close_key;
	reg_close_key = reinterpret_cast<t_reg_close_key>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCloseKey"));

	bool b_changed = false;
	HKEY hk_result;
	if (reg_open_key_ex_a(HKEY_LOCAL_MACHINE, R"(Software\Policies\Microsoft\Windows\CurrentVersion\Internet Settings)",
	                      0, KEY_READ | KEY_WRITE, &hk_result) == ERROR_SUCCESS)
	{
		DWORD value;
		DWORD new_value = 0;
		DWORD dw_size = sizeof(value);
		if (const LSTATUS reg_query = reg_query_value_ex_a(hk_result, "DisableCachingOfSSLPages", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value),
		                                                   &dw_size); reg_query == ERROR_SUCCESS)
		{
			if (value == 0x1)
			{
				reg_set_value_ex_a(hk_result, "DisableCachingOfSSLPages", 0, REG_DWORD, reinterpret_cast<LPBYTE>(&new_value), dw_size);
				b_changed = true;
			}
		}
		else if (reg_query == ERROR_FILE_NOT_FOUND)
		{
			reg_set_value_ex_a(hk_result, "DisableCachingOfSSLPages", 0, REG_DWORD, reinterpret_cast<LPBYTE>(&new_value), dw_size);
			b_changed = true;
		}
		reg_close_key(hk_result);
	}

	if (b_changed == true)
	{
		MessageBoxA(
			nullptr, "Restart the program\n\nIf this pop-up window keeps showing up: Open \"Internet Options\", "
			"Go to \"Advanced\" tab and disable \"Do not save encrypted pages to disk\". Press \"Apply\" and \"OK\"",
			"Updated faulty options", MB_OK);
		exit(EXIT_SUCCESS);
	}
}
