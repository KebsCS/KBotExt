#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"
#include "Misc.h"
#include "Config.h"

class MiscTab
{
public:

	static std::string LevenshteinDistance(std::vector<std::string>vec, std::string str2)
	{
		int max = 999;
		std::string bestMatch;

		for (const std::string& str1 : vec)
		{
			int l_string_length1 = str1.length();
			int l_string_length2 = str2.length();
			int d[50 + 1][50 + 1];

			int i;
			int j;
			int l_cost;

			for (i = 0; i <= l_string_length1; i++)
			{
				d[i][0] = i;
			}
			for (j = 0; j <= l_string_length2; j++)
			{
				d[0][j] = j;
			}
			for (i = 1; i <= l_string_length1; i++)
			{
				for (j = 1; j <= l_string_length2; j++)
				{
					if (str1[i - 1] == str2[j - 1])
					{
						l_cost = 0;
					}
					else
					{
						l_cost = 1;
					}
					d[i][j] = (std::min)(
						d[i - 1][j] + 1,                  // delete
						(std::min)(d[i][j - 1] + 1,         // insert
							d[i - 1][j - 1] + l_cost)           // substitution
						);
					if ((i > 1) &&
						(j > 1) &&
						(str1[i - 1] == str2[j - 2]) &&
						(str1[i - 2] == str2[j - 1])
						)
					{
						d[i][j] = (std::min)(
							d[i][j],
							d[i - 2][j - 2] + l_cost   // transposition
							);
					}
				}
			}

			if (d[l_string_length1][l_string_length2] <= max)
			{
				max = d[l_string_length1][l_string_length2];
				bestMatch = str1;
			}
		}
		return bestMatch;
	}

	static void Render()
	{
		static bool onOpen = true;
		if (ImGui::BeginTabItem("Misc"))
		{
			static std::string result;

			// Get processes every 5 seconds
			static auto timeBefore = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> timeDuration = std::chrono::high_resolution_clock::now() - timeBefore;
			if (timeDuration.count() > 5000 || onOpen)
			{
				timeBefore = std::chrono::high_resolution_clock::now();
				LCU::GetLeagueProcesses();
			}

			ImGui::Text("Selected process: ");
			ImGui::SameLine();

			std::string comboProcesses = "";
			if (LCU::IsProcessGood())
			{
				comboProcesses = std::to_string(LCU::leagueProcesses[LCU::indexLeagueProcesses].first)
					+ " : " + LCU::leagueProcesses[LCU::indexLeagueProcesses].second;
			}
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 3));
			if (ImGui::BeginCombo("##comboProcesses", comboProcesses.c_str(), 0))
			{
				for (size_t n = 0; n < LCU::leagueProcesses.size(); n++)
				{
					const bool is_selected = (LCU::indexLeagueProcesses == n);
					if (ImGui::Selectable((std::to_string(LCU::leagueProcesses[n].first) + " : " + LCU::leagueProcesses[n].second).c_str(), is_selected))
					{
						LCU::indexLeagueProcesses = n;
						LCU::SetLeagueClientInfo();
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();

			ImGui::Columns(2, 0, false);

			if (ImGui::Button("Launch another client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
					ShellExecuteA(NULL, NULL, std::format("{}LeagueClient.exe", S.leaguePath).c_str(), "--allow-multiple-clients", NULL, SW_SHOWNORMAL);
			}

			if (ImGui::Button("Restart UX"))
			{
				result = LCU::Request("POST", "https://127.0.0.1/riotclient/kill-and-restart-ux", "");
				if (result.find("failed") != std::string::npos)
				{
					if (LCU::SetLeagueClientInfo())
						result = "Rehooked to new league client";
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchLegacyClient();
				}
			}

			if (ImGui::Button("Close client"))
				result = LCU::Request("POST", "https://127.0.0.1/process-control/v1/process/quit", "");

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(2, 0, false);

			if (ImGui::Button("Accept all friend requests"))
			{
				if (MessageBoxA(0, "Are you sure?", "Accepting friend requests", MB_OKCANCEL) == IDOK)
				{
					std::string getFriends = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/friend-requests");

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
								LCU::Request("PUT", req, R"({"direction":"both"})");
							}
							result = "Accepted " + std::to_string(root.size()) + " friend requests";
						}
					}
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Delete all friend requests"))
			{
				if (MessageBoxA(0, "Are you sure?", "Deleting friend requests", MB_OKCANCEL) == IDOK)
				{
					std::string getFriends = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/friend-requests");

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
								LCU::Request("DELETE", req, "");
							}
							result = "Deleted " + std::to_string(root.size()) + " friend requests";
						}
					}
				}
			}

			ImGui::Columns(1);

			static std::vector<std::pair<std::string, int>>items;
			static size_t item_current_idx = 0; // Here we store our selection data as an index.
			const char* combo_label = "**Default";
			if (!items.empty())
				combo_label = items[item_current_idx].first.c_str();

			if (ImGui::Button("Remove all friends"))
			{
				if (MessageBoxA(0, "Are you sure?", "Removing friends", MB_OKCANCEL) == IDOK)
				{
					std::string getFriends = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/friends");

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
								if (root[i]["groupId"].asUInt() == item_current_idx)
								{
									std::string req = "https://127.0.0.1/lol-chat/v1/friends/" + root[i]["pid"].asString();
									LCU::Request("DELETE", req, "");
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
				std::string getGroups = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/friend-groups");
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

				for (size_t n = 0; n < items.size(); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n].first.c_str(), is_selected))
						item_current_idx = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			//if (ImGui::Button("Skip tutorial"))
			//{
			//	http->Request("POST", "https://127.0.0.1/telemetry/v1/events/new_player_experience", R"({"eventName":"hide_screen","plugin":"rcp-fe-lol-new-player-experience","screenName":"npe_tutorial_modules"})", auth->leagueHeader, "", "", auth->leaguePort);
			//	http->Request("PUT", "https://127.0.0.1/lol-npe-tutorial-path/v1/settings", R"({"hasSeenTutorialPath":true,"hasSkippedTutorialPath":true,"shouldSeeNewPlayerExperience":true})", auth->leagueHeader, "", "", auth->leaguePort);
			//	//DELETE https://127.0.0.1:63027/lol-statstones/v1/vignette-notifications HTTP/1.1
			//	// ?
			//}

			// Patched :(
			/*if (ImGui::Button("Free Tristana + Riot Girl skin"))
				result = http->Request("POST", "https://127.0.0.1/lol-login/v1/session/invoke?destination=inventoryService&method=giftFacebookFan&args=[]", "", auth->leagueHeader, "", "", auth->leaguePort);

			ImGui::SameLine();
			Misc::HelpMarker("Relog after pressing the button");
			*/

			ImGui::Separator();

			static int minimapScale = 100;
			ImGui::Text("In-game minimap scale: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 3));
			ImGui::SliderInt("##sliderMinimapScale", &minimapScale, 0, 350, "%d");
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitMinimapScale"))
			{
				result = LCU::Request("PATCH", "https://127.0.0.1/lol-game-settings/v1/game-settings",
					std::format("{{\"HUD\":{{\"MinimapScale\":{:.2f}}}}}", minimapScale / 33.33f));
			}

			ImGui::Separator();

			static std::vector<std::pair<std::string, std::string>>itemsDisenchant = {
	{"Champion shards","CHAMPION_RENTAL"}, {"Champion pernaments","CHAMPION"},
	{"Skin shards","CHAMPION_SKIN_RENTAL"}, {"Skin pernaments", "CHAMPION_SKIN"},
	{"Eternals","STATSTONE_SHARD"},{"Ward shards","WARD_SKIN_RENTAL"},{"Ward pernaments","WARD_SKIN"}
			};
			static size_t itemIndexDisenchant = 0;
			const char* comboDisenchant = itemsDisenchant[itemIndexDisenchant].first.c_str();

			if (ImGui::Button("Disenchant all: "))
			{
				Json::Value root;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				std::string getLoot = LCU::Request("GET", "https://127.0.0.1/lol-loot/v1/player-loot-map", "");

				if (reader->parse(getLoot.c_str(), getLoot.c_str() + static_cast<int>(getLoot.length()), &root, &err))
				{
					if (MessageBoxA(0, "Are you sure?", "Disenchanting loot", MB_OKCANCEL) == IDOK)
					{
						int i = 0;

						for (const std::string& name : root.getMemberNames())
						{
							std::regex regexStr("^" + itemsDisenchant[itemIndexDisenchant].second + "_[\\d]+");

							if (std::regex_match(name, regexStr))
							{
								std::string disenchantCase = itemsDisenchant[itemIndexDisenchant].second == "STATSTONE_SHARD" ? "DISENCHANT" : "disenchant";
								std::string disenchantName = root[name]["type"].asString();

								std::string disenchantUrl = std::format("https://127.0.0.1/lol-loot/v1/recipes/{0}_{1}/craft?repeat=1", disenchantName, disenchantCase);
								std::string disenchantBody = std::format(R"(["{}"])", name).c_str();
								LCU::Request("POST", disenchantUrl, disenchantBody);
								i++;
							}
						}
						result = std::format("Disenchanted {0} {1}", std::to_string(i), itemsDisenchant[itemIndexDisenchant].first);
					}
				}
				else
					result = "Loot not found";
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 3));
			if (ImGui::BeginCombo("##comboDisenchant", comboDisenchant, 0))
			{
				for (size_t n = 0; n < itemsDisenchant.size(); n++)
				{
					const bool is_selected = (itemIndexDisenchant == n);
					if (ImGui::Selectable(itemsDisenchant[n].first.c_str(), is_selected))
						itemIndexDisenchant = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// Getting closest champion name with Levenshtein distance algorithm and getting it's id
			ImGui::Text("Champion name to ID");
			static std::vector<std::string>champNames;
			if (!champSkins.empty() && champNames.empty())
			{
				for (const auto& champ : champSkins)
				{
					champNames.emplace_back(champ.name);
					//std::cout << "('" << champ.name << "', " << champ.key << "), " << std::endl;
					std::cout << champ.name << std::endl;
				}
			}

			static char bufChampionName[50];
			static size_t lastSize = 0;
			static std::string closestChampion;
			static std::string closestId;
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 3));
			ImGui::InputText("##inputChampionName", bufChampionName, IM_ARRAYSIZE(bufChampionName));
			if (strlen(bufChampionName) < 1)
			{
				closestChampion = "";
				closestId = "";
				lastSize = 0;
			}
			else if (lastSize != strlen(bufChampionName))
			{
				lastSize = strlen(bufChampionName);
				closestChampion = LevenshteinDistance(champNames, bufChampionName);

				for (const auto& champ : champSkins)
				{
					if (closestChampion == champ.name)
					{
						closestId = std::to_string(champ.key);
						break;
					}
				}
			}
			ImGui::SameLine();
			ImGui::TextWrapped("%s ID: %s", closestChampion.c_str(), closestId.c_str());

			if (ImGui::Button("Check email of the account"))
				result = LCU::Request("GET", "https://127.0.0.1/lol-email-verification/v1/email");

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
				ImGui::InputTextMultiline("##miscResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 185));
			}

			if (onOpen)
				onOpen = false;

			ImGui::EndTabItem();
		}
		else
		{
			onOpen = true;
		}
	}
};