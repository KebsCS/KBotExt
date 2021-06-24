#pragma once

#include <filesystem>

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Auth.h"
#include "Misc.h"
#include "Settings.h"

class MiscTab
{
public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Misc"))
		{
			static std::string result;

			ImGui::Columns(2, 0, false);

			if (ImGui::Button("Launch another client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invadlid path, change it in Settings tab";
				}
				else
					ShellExecuteA(NULL, NULL, std::format("{}LeagueClient.exe", S.leaguePath).c_str(), "--allow-multiple-clients", NULL, SW_SHOWNORMAL);
			}

			if (ImGui::Button("Restart UX"))
			{
				result = http->Request("POST", "https://127.0.0.1/riotclient/kill-and-restart-ux", "", auth->leagueHeader, "", "", auth->leaguePort);
				if (result.find("failed") != std::string::npos)
				{
					if (auth->GetLeagueClientInfo())
						result = "Rehooked to new league client";
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invadlid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchLegacyClient();
				}
			}

			if (ImGui::Button("Close client"))
				result = http->Request("POST", "https://127.0.0.1/process-control/v1/process/quit", "", auth->leagueHeader, "", "", auth->leaguePort);

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(2, 0, false);

			if (ImGui::Button("Accept all friend requests"))
			{
				if (MessageBoxA(0, "Are you sure?", 0, MB_OKCANCEL) == IDOK)
				{
					std::string getFriends = http->Request("GET", "https://127.0.0.1/lol-chat/v1/friend-requests", "", auth->leagueHeader, "", "", auth->leaguePort);

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (!reader->parse(getFriends.c_str(), getFriends.c_str() + static_cast<int>(getFriends.length()), &root, &err))
					{
						result = "Failed to parse JSON";
					}
					else
					{
						if (root.isArray())
						{
							for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
							{
								std::string req = "https://127.0.0.1/lol-chat/v1/friend-requests/" + root[i]["pid"].asString();
								http->Request("PUT", req, R"({"direction":"both"})", auth->leagueHeader, "", "", auth->leaguePort);
							}
							result = "Accepted " + std::to_string(root.size()) + " friend requests";
						}
					}
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Delete all friend requests"))
			{
				if (MessageBoxA(0, "Are you sure?", 0, MB_OKCANCEL) == IDOK)
				{
					std::string getFriends = http->Request("GET", "https://127.0.0.1/lol-chat/v1/friend-requests", "", auth->leagueHeader, "", "", auth->leaguePort);

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (!reader->parse(getFriends.c_str(), getFriends.c_str() + static_cast<int>(getFriends.length()), &root, &err))
					{
						result = "Failed to parse JSON";
					}
					else
					{
						if (root.isArray())
						{
							for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
							{
								std::string req = "https://127.0.0.1/lol-chat/v1/friend-requests/" + root[i]["pid"].asString();
								http->Request("DELETE", req, "", auth->leagueHeader, "", "", auth->leaguePort);
							}
							result = "Deleted " + std::to_string(root.size()) + " friend requests";
						}
					}
				}
			}

			ImGui::Columns(1);

			static std::vector<std::pair<std::string, int>>items;
			static int item_current_idx = 0; // Here we store our selection data as an index.
			const char* combo_label = "**Default";
			if (!items.empty())
				combo_label = items[item_current_idx].first.c_str();

			if (ImGui::Button("Remove all friends"))
			{
				if (MessageBoxA(0, "Are you sure?", 0, MB_OKCANCEL) == IDOK)
				{
					std::string getFriends = http->Request("GET", "https://127.0.0.1/lol-chat/v1/friends", "", auth->leagueHeader, "", "", auth->leaguePort);

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (!reader->parse(getFriends.c_str(), getFriends.c_str() + static_cast<int>(getFriends.length()), &root, &err))
					{
						result = "Failed to parse JSON";
					}
					else
					{
						if (root.isArray())
						{
							int iDeleted = 0;
							for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i)
							{
								if (root[i]["groupId"].asInt() == item_current_idx)
								{
									std::string req = "https://127.0.0.1/lol-chat/v1/friends/" + root[i]["pid"].asString();
									http->Request("DELETE", req, "", auth->leagueHeader, "", "", auth->leaguePort);
									iDeleted++;
								}
							}
							result = "Deleted " + std::to_string(iDeleted) + " friends";
						}
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text(" From folder: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			if (ImGui::BeginCombo("##comboGroups", combo_label, 0))
			{
				std::string getGroups = http->Request("GET", "https://127.0.0.1/lol-chat/v1/friend-groups", "", auth->leagueHeader, "", "", auth->leaguePort);
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(getGroups.c_str(), getGroups.c_str() + static_cast<int>(getGroups.length()), &root, &err))
				{
					if (root.isArray())
					{
						items.clear();
						for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
						{
							std::pair<std::string, int > temp = { root[i]["name"].asString(), root[i]["id"].asInt() };
							items.emplace_back(temp);
						}
						std::sort(items.begin(), items.end(), [](std::pair<std::string, int > a, std::pair<std::string, int >b) {return a.second < b.second; });
					}
				}

				for (int n = 0; n < items.size(); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n].first.c_str(), is_selected))
						item_current_idx = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Free Tristana + Riot Girl skin"))
				result = http->Request("POST", "https://127.0.0.1/lol-login/v1/session/invoke?destination=inventoryService&method=giftFacebookFan&args=[]", "", auth->leagueHeader, "", "", auth->leaguePort);

			ImGui::SameLine();
			Misc::HelpMarker("Relog after pressing the button");

			static Json::StreamWriterBuilder wBuilder;
			static std::string sResultJson;
			static char* cResultJson;

			if (!result.empty())
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
					sResultJson = result;
				else
				{
					sResultJson = Json::writeString(wBuilder, root);
				}
				result = "";
			}

			if (!sResultJson.empty())
			{
				cResultJson = &sResultJson[0];
				ImGui::InputTextMultiline("##miscResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 200));
			}

			ImGui::EndTabItem();
		}
	}
};