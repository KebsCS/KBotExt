#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include "Includes.h"

struct settings
{
	settings(): hwnd(nullptr)
	{
	}
	;
	HWND hwnd;
	const std::string settings_file = "config.JSON";
	std::string current_debugger; // debugger path
	std::vector<std::string> ignored_versions;

	bool auto_rename = false;
	std::string league_path = "C:/Riot Games/League of Legends/";
	float font_scale = 1.f;
	bool stream_proof = false;
	bool debugger = false;
	bool no_admin = false;

	struct
	{
		int width = 730;
		int height = 530;
	} window;

	struct
	{
		std::string player_name;
	} info_tab;

	struct
	{
		std::string method;
		std::string url_text;
		std::string request_text;
		std::string port;
		std::string header;
	} custom_tab;

	struct
	{
		std::string destination;
		std::string method;
		std::string args;
	} invoke_tab;

	struct
	{
		std::string language = "en_US";
		std::string league_args = "--locale=en_US";
	} login_tab;

	struct
	{
		size_t index_first_role = 0;
		size_t index_second_role = 0;
		size_t index_multi_search = 0;
		bool auto_accept_enabled = false;
		bool instalock_enabled = false;
		bool auto_ban_enabled = false;
		int instalock_id = 0;
		int instalock_delay = 0;
		std::string instant_message;
		int instant_message_delay = 0;
		int instant_message_times = 1;
		int instant_message_delay_times = 0;
		int auto_ban_id = 0;
		int auto_ban_delay = 0;
		bool dodge_on_ban = false;
		int backup_id = 0;
		bool instant_mute = false;
		bool side_notification = false;
	} game_tab;
};

extern settings s;

class config
{
public:
	static void save()
	{
		// if file doesn't exist, create new one with {} so it can be parsed
		if (!std::filesystem::exists(s.settings_file))
		{
			std::ofstream file(s.settings_file);
			file << "{}";
			file.close();
		}

		Json::Value root;
		JSONCPP_STRING err;

		std::ifstream i_file(s.settings_file);

		if (i_file.good())
		{
			if (Json::CharReaderBuilder builder; parseFromStream(builder, i_file, &root, &err))
			{
				root["autoRename"] = s.auto_rename;
				root["leaguePath"] = s.league_path;
				root["debugger"] = s.debugger;
				root["window"]["width"] = s.window.width;
				root["window"]["height"] = s.window.height;
				root["fontScale"] = s.font_scale;
				root["loginTab"]["language"] = s.login_tab.language;
				root["loginTab"]["leagueArgs"] = s.login_tab.league_args;
				root["streamProof"] = s.stream_proof;
				root["noAdmin"] = s.no_admin;

				root["infoTab"]["playerName"] = s.info_tab.player_name;

				root["customTab"]["method"] = s.custom_tab.method;
				root["customTab"]["urlText"] = s.custom_tab.url_text;
				root["customTab"]["requestText"] = s.custom_tab.request_text;
				root["customTab"]["port"] = s.custom_tab.port;
				root["customTab"]["header"] = s.custom_tab.header;
				root["invokeTab"]["destination"] = s.invoke_tab.destination;
				root["invokeTab"]["method"] = s.invoke_tab.method;
				root["invokeTab"]["args"] = s.invoke_tab.args;

				root["gameTab"]["indexFirstRole"] = s.game_tab.index_first_role;
				root["gameTab"]["indexSecondRole"] = s.game_tab.index_second_role;
				root["gameTab"]["indexMultiSearch"] = s.game_tab.index_multi_search;
				root["gameTab"]["autoAcceptEnabled"] = s.game_tab.auto_accept_enabled;
				root["gameTab"]["instalockEnabled"] = s.game_tab.instalock_enabled;
				root["gameTab"]["autoBanEnabled"] = s.game_tab.auto_ban_enabled;
				root["gameTab"]["instalockDelay"] = s.game_tab.instalock_delay;
				root["gameTab"]["instalockId"] = s.game_tab.instalock_id;
				root["gameTab"]["instantMessage"] = s.game_tab.instant_message;
				root["gameTab"]["instantMessageDelay"] = s.game_tab.instant_message_delay;
				root["gameTab"]["instantMessageTimes"] = s.game_tab.instant_message_times;
				root["gameTab"]["instantMessageDelayTimes"] = s.game_tab.instant_message_delay_times;
				root["gameTab"]["autoBanId"] = s.game_tab.auto_ban_id;
				root["gameTab"]["autoBanDelay"] = s.game_tab.auto_ban_delay;
				root["gameTab"]["dodgeOnBan"] = s.game_tab.dodge_on_ban;
				root["gameTab"]["backupId"] = s.game_tab.backup_id;
				root["gameTab"]["instantMute"] = s.game_tab.instant_mute;
				root["gameTab"]["sideNotification"] = s.game_tab.side_notification;

				{
					root["ignoredVersions"] = Json::Value(Json::arrayValue);
					for (const std::string& version : s.ignored_versions)
						root["ignoredVersions"].append(version);
				}

				if (!root.toStyledString().empty())
				{
					std::ofstream o_file(s.settings_file);
					o_file << root.toStyledString() << std::endl;
					o_file.close();
				}
			}
		}
		i_file.close();
	}

	static void load()
	{
		std::fstream file(s.settings_file, std::ios_base::in);
		if (file.good())
		{
			Json::Value root;
			JSONCPP_STRING err;

			if (Json::CharReaderBuilder builder; parseFromStream(builder, file, &root, &err))
			{
				if (auto t = root["autoRename"]; !t.empty())
					s.auto_rename = t.asBool();
				if (auto t = root["leaguePath"]; !t.empty())
					s.league_path = t.asString();
				if (auto t = root["debugger"]; !t.empty())
					s.debugger = t.asBool();
				if (auto t = root["window"]["width"]; !t.empty())
					s.window.width = t.asInt();
				if (auto t = root["window"]["height"]; !t.empty())
					s.window.height = t.asInt();
				if (auto t = root["fontScale"]; !t.empty())
					s.font_scale = t.asFloat();
				if (auto t = root["loginTab"]["language"]; !t.empty())
					s.login_tab.language = t.asString();
				if (auto t = root["loginTab"]["leagueArgs"]; !t.empty())
					s.login_tab.league_args = t.asString();
				if (auto t = root["streamProof"]; !t.empty())
					s.stream_proof = t.asBool();
				if (auto t = root["noAdmin"]; !t.empty())
					s.no_admin = t.asBool();

				if (auto t = root["infoTab"]["playerName"]; !t.empty())
					s.info_tab.player_name = t.asString();

				if (auto t = root["customTab"]["method"]; !t.empty())
					s.custom_tab.method = t.asString();
				if (auto t = root["customTab"]["urlText"]; !t.empty())
					s.custom_tab.url_text = t.asString();
				if (auto t = root["customTab"]["requestText"]; !t.empty())
					s.custom_tab.request_text = t.asString();
				if (auto t = root["customTab"]["port"]; !t.empty())
					s.custom_tab.port = t.asString();
				if (auto t = root["customTab"]["header"]; !t.empty())
					s.custom_tab.header = t.asString();

				if (auto t = root["invokeTab"]["destination"]; !t.empty())
					s.invoke_tab.destination = t.asString();
				if (auto t = root["invokeTab"]["method"]; !t.empty())
					s.invoke_tab.method = t.asString();
				if (auto t = root["invokeTab"]["args"]; !t.empty())
					s.invoke_tab.args = t.asString();

				if (auto t = root["gameTab"]["indexFirstRole"]; !t.empty())
					s.game_tab.index_first_role = t.asUInt();
				if (auto t = root["gameTab"]["indexSecondRole"]; !t.empty())
					s.game_tab.index_second_role = t.asUInt();
				if (auto t = root["gameTab"]["indexMultiSearch"]; !t.empty())
					s.game_tab.index_multi_search = t.asUInt();
				if (auto t = root["gameTab"]["autoAcceptEnabled"]; !t.empty())
					s.game_tab.auto_accept_enabled = t.asBool();
				if (auto t = root["gameTab"]["instalockEnabled"]; !t.empty())
					s.game_tab.instalock_enabled = t.asBool();
				if (auto t = root["gameTab"]["autoBanEnabled"]; !t.empty())
					s.game_tab.auto_ban_enabled = t.asBool();
				if (auto t = root["gameTab"]["instalockDelay"]; !t.empty())
					s.game_tab.instalock_delay = t.asInt();
				if (auto t = root["gameTab"]["instalockId"]; !t.empty())
					s.game_tab.instalock_id = t.asInt();
				if (auto t = root["gameTab"]["instantMessage"]; !t.empty())
					s.game_tab.instant_message = t.asString();
				if (auto t = root["gameTab"]["instantMessageDelay"]; !t.empty())
					s.game_tab.instant_message_delay = t.
						asInt();
				if (auto t = root["gameTab"]["instantMessageTimes"]; !t.empty())
					s.game_tab.instant_message_times = t.
						asInt();
				if (auto t = root["gameTab"]["instantMessageDelayTimes"]; !t.empty())
					s.game_tab.instant_message_delay_times
						= t.asInt();
				if (auto t = root["gameTab"]["autoBanId"]; !t.empty())
					s.game_tab.auto_ban_id = t.asInt();
				if (auto t = root["gameTab"]["autoBanDelay"]; !t.empty())
					s.game_tab.auto_ban_delay = t.asInt();
				if (auto t = root["gameTab"]["dodgeOnBan"]; !t.empty())
					s.game_tab.dodge_on_ban = t.asBool();
				if (auto t = root["gameTab"]["backupId"]; !t.empty())
					s.game_tab.backup_id = t.asInt();
				if (auto t = root["gameTab"]["instantMute"]; !t.empty())
					s.game_tab.instant_mute = t.asBool();
				if (auto t = root["gameTab"]["sideNotification"]; !t.empty())
					s.game_tab.side_notification = t.asBool();

				if (root["ignoredVersions"].isArray() && !root["ignoredVersions"].empty())
				{
					for (const auto& i : root["ignoredVersions"])
					{
						s.ignored_versions.emplace_back(i.asString());
					}
				}
			}
		}
		file.close();
	}
};
