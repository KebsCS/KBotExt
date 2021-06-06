#pragma once

#include <filesystem>
#include <fstream>
#include <thread>

#include "Auth.h"
#include "HTTP.h"

class Misc
{
public:

	static void LaunchLegacyClient()
	{
		if (!std::filesystem::exists("C:/Riot Games/League of Legends/LoL Companion"))
		{
			std::filesystem::create_directory("C:/Riot Games/League of Legends/LoL Companion");
		}
		if (!std::filesystem::exists("C:/Riot Games/League of Legends/LoL Companion/system.yaml"))
		{
			std::ifstream infile("C:/Riot Games/League of Legends/system.yaml");
			std::ofstream outfile("C:/Riot Games/League of Legends/LoL Companion/system.yaml");
			std::string content = "";
			int i;

			for (i = 0; infile.eof() != true; i++)
				content += infile.get();

			infile.close();
			size_t pos = content.find("riotclient:");
			content = content.substr(0, pos + 11);

			outfile << content;
			outfile.close();
		}

		if (::FindWindowA(0, "League of Legends"))
		{
			http->Request("POST", "https://127.0.0.1/process-control/v1/process/quit", "", auth->leagueHeader, "", "", auth->leaguePort);

			// wait for client to close (maybe theres a better method of doing that)
			std::this_thread::sleep_for(std::chrono::milliseconds(4500));
		}

		ShellExecute(NULL, L"open", L"\"C:\\Riot Games\\League of Legends\\LeagueClient.exe\"", L"--system-yaml-override=\"C:\\Riot Games\\League of Legends\\LoL Companion\\system.yaml\"", NULL, SW_SHOWNORMAL);
	}

	static void CheckVersion()
	{
		std::string getLatest = http->Request("GET", "https://api.github.com/repos/KebsCS/KBotExt/releases/latest");

		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(getLatest.c_str(), getLatest.c_str() + static_cast<int>(getLatest.length()), &root, &err))
		{
			std::string latestName = root["tag_name"].asString();
			if (latestName != "1.2.3")
			{
				if (MessageBoxA(0, "Open download website?", "New version available", MB_YESNO | MB_SETFOREGROUND) == IDYES)
				{
					ShellExecute(0, 0, L"https://github.com/KebsCS/KBotExt/releases/latest", 0, 0, SW_SHOW);
				}
			}
		}
	}

	static std::string GetCurrentPatch()
	{
		std::string result = http->Request("GET", "https://ddragon.leagueoflegends.com/api/versions.json");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			return root[0].asString();
		}
		return "0.0.0";
	}

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static void ArrowButtonDisabled(const char* id, ImGuiDir dir)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::ArrowButton(id, dir);
		ImGui::PopStyleVar();
	}
};