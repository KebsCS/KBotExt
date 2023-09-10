#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"
#include "Misc.h"
#include "Config.h"

class misc_tab
{
public:
	static std::string levenshtein_distance(std::vector<std::string> vec, std::string str2)
	{
		size_t max = 999;
		std::string best_match;

		for (const std::string& str1 : vec)
		{
			const size_t str1_len = str1.length();
			const size_t str2_len = str2.length();
			size_t d[50 + 1][50 + 1];

			size_t i;
			size_t j;
			size_t cost;

			for (i = 0; i <= str1_len; i++)
			{
				d[i][0] = i;
			}
			for (j = 0; j <= str2_len; j++)
			{
				d[0][j] = j;
			}
			for (i = 1; i <= str1_len; i++)
			{
				for (j = 1; j <= str2_len; j++)
				{
					if (str1[i - 1] == str2[j - 1])
					{
						cost = 0;
					}
					else
					{
						cost = 1;
					}
					d[i][j] = (std::min)(
						d[i - 1][j] + 1, // delete
						(std::min)(d[i][j - 1] + 1, // insert
						           d[i - 1][j - 1] + cost) // substitution
					);
					if (i > 1 &&
						j > 1 &&
						str1[i - 1] == str2[j - 2] &&
						str1[i - 2] == str2[j - 1]
					)
					{
						d[i][j] = (std::min)(
							d[i][j],
							d[i - 2][j - 2] + cost // transposition
						);
					}
				}
			}

			if (d[str1_len][str2_len] <= max)
			{
				max = d[str1_len][str2_len];
				best_match = str1;
			}
		}
		return best_match;
	}

	static void render()
	{
		static bool on_open = true;
		if (ImGui::BeginTabItem("Misc"))
		{
			static std::string result;

			// Get processes every 5 seconds
			static auto time_before = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration<float, std::milli> time_duration = std::chrono::high_resolution_clock::now() -
				time_before; time_duration.count() > 5000 || on_open)
			{
				time_before = std::chrono::high_resolution_clock::now();
				lcu::get_league_processes();
			}

			ImGui::Text("Selected process: ");
			ImGui::SameLine();

			std::string combo_processes;
			if (lcu::is_process_good())
			{
				combo_processes = std::to_string(lcu::league_processes[lcu::index_league_processes].first)
					+ " : " + lcu::league_processes[lcu::index_league_processes].second;
			}
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 3));
			if (ImGui::BeginCombo("##comboProcesses", combo_processes.c_str(), 0))
			{
				for (size_t n = 0; n < lcu::league_processes.size(); n++)
				{
					const bool is_selected = (lcu::index_league_processes == n);
					if (ImGui::Selectable(
						(std::to_string(lcu::league_processes[n].first) + " : " + lcu::league_processes[n].second).
						c_str(), is_selected))
					{
						lcu::index_league_processes = n;
						lcu::set_league_client_info();
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);

			if (ImGui::Button("Launch another client"))
			{
				if (!std::filesystem::exists(s.league_path))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
					ShellExecuteA(nullptr, nullptr, std::format("{}LeagueClient.exe", s.league_path).c_str(),
					              "--allow-multiple-clients", nullptr, SW_SHOWNORMAL);
			}

			if (ImGui::Button("Restart UX"))
			{
				result = lcu::request("POST", "https://127.0.0.1/riotclient/kill-and-restart-ux", "");
				if (result.find("failed") != std::string::npos)
				{
					if (lcu::set_league_client_info())
						result = "Rehooked to new league client";
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(s.league_path))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					misc::launch_legacy_client();
				}
			}

			if (ImGui::Button("Close client"))
				result = lcu::request("POST", "https://127.0.0.1/process-control/v1/process/quit", "");

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);

			if (ImGui::Button("Accept all friend requests"))
			{
				if (MessageBoxA(nullptr, "Are you sure?", "Accepting friend requests", MB_OKCANCEL) == IDOK)
				{
					std::string get_friends = lcu::request("GET", "https://127.0.0.1/lol-chat/v1/friend-requests");

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (!reader->parse(get_friends.c_str(), get_friends.c_str() + static_cast<int>(get_friends.length()),
					                   &root, &err))
					{
						result = "Failed to parse JSON";
					}
					else
					{
						if (root.isArray())
						{
							for (auto& i : root)
							{
								std::string req = "https://127.0.0.1/lol-chat/v1/friend-requests/" + i["pid"].
									asString();
								lcu::request("PUT", req, R"({"direction":"both"})");
							}
							result = "Accepted " + std::to_string(root.size()) + " friend requests";
						}
					}
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Delete all friend requests"))
			{
				if (MessageBoxA(nullptr, "Are you sure?", "Deleting friend requests", MB_OKCANCEL) == IDOK)
				{
					std::string get_friends = lcu::request("GET", "https://127.0.0.1/lol-chat/v1/friend-requests");

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (!reader->parse(get_friends.c_str(), get_friends.c_str() + static_cast<int>(get_friends.length()),
					                   &root, &err))
					{
						result = "Failed to parse JSON";
					}
					else
					{
						if (root.isArray())
						{
							for (auto& i : root)
							{
								std::string req = "https://127.0.0.1/lol-chat/v1/friend-requests/" + i["pid"].
									asString();
								lcu::request("DELETE", req, "");
							}
							result = "Deleted " + std::to_string(root.size()) + " friend requests";
						}
					}
				}
			}

			ImGui::Columns(1);

			static std::vector<std::pair<std::string, int>> items;
			static size_t item_current_idx = 0; // Here we store our selection data as an index.
			auto combo_label = "**Default";
			if (!items.empty())
				combo_label = items[item_current_idx].first.c_str();

			if (ImGui::Button("Remove all friends"))
			{
				if (MessageBoxA(nullptr, "Are you sure?", "Removing friends", MB_OKCANCEL) == IDOK)
				{
					std::string get_friends = lcu::request("GET", "https://127.0.0.1/lol-chat/v1/friends");

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (!reader->parse(get_friends.c_str(), get_friends.c_str() + static_cast<int>(get_friends.length()),
					                   &root, &err))
					{
						result = "Failed to parse JSON";
					}
					else
					{
						if (root.isArray())
						{
							int i_deleted = 0;
							for (auto& i : root)
							{
								if (i["groupId"].asUInt() == item_current_idx)
								{
									std::string req = "https://127.0.0.1/lol-chat/v1/friends/" + i["pid"].
										asString();
									lcu::request("DELETE", req, "");
									i_deleted++;
								}
							}
							result = "Deleted " + std::to_string(i_deleted) + " friends";
						}
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text(" From folder: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::CalcTextSize(std::string(20, 'W').c_str(), nullptr, true).x);
			if (ImGui::BeginCombo("##comboGroups", combo_label, 0))
			{
				std::string get_groups = lcu::request("GET", "https://127.0.0.1/lol-chat/v1/friend-groups");
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(get_groups.c_str(), get_groups.c_str() + static_cast<int>(get_groups.length()), &root,
				                  &err))
				{
					if (root.isArray())
					{
						items.clear();
						for (auto& i : root)
						{
							std::pair temp = {i["name"].asString(), i["id"].asInt()};
							items.emplace_back(temp);
						}
						std::ranges::sort(items, [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
							return a.second < b.second;
						});
					}
				}


				for (size_t n = 0; n < items.size(); n++)
				{
					const bool is_selected = item_current_idx == n;
					if (ImGui::Selectable(items[n].first.c_str(), is_selected))
						item_current_idx = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();

			static int minimap_scale = 100;
			ImGui::Text("In-game minimap scale: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 3));
			ImGui::SliderInt("##sliderMinimapScale", &minimap_scale, 0, 350, "%d");
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitMinimapScale"))
			{
				result = lcu::request("PATCH", "https://127.0.0.1/lol-game-settings/v1/game-settings",
				                      std::format(R"({{"HUD":{{"MinimapScale":{:.2f}}}}})", minimap_scale / 33.33f));
			}

			ImGui::Separator();

			static std::vector<std::pair<std::string, std::string>> items_disenchant = {
				{"Champion shards", "CHAMPION_RENTAL"}, {"Champion pernaments", "CHAMPION"},
				{"Skin shards", "CHAMPION_SKIN_RENTAL"}, {"Skin pernaments", "CHAMPION_SKIN"},
				{"Eternals", "STATSTONE_SHARD"}, {"Ward shards", "WARD_SKIN_RENTAL"}, {"Ward pernaments", "WARD_SKIN",},
				{"Emotes", "EMOTE"}, {"Icons", "SUMMONER_ICON"}, {"Companions", "COMPANION"}
			};
			static size_t item_index_disenchant = 0;
			const char* combo_disenchant = items_disenchant[item_index_disenchant].first.c_str();

			if (ImGui::Button("Disenchant all: "))
			{
				Json::Value root;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				std::string get_loot = lcu::request("GET", "https://127.0.0.1/lol-loot/v1/player-loot-map", "");

				if (reader->parse(get_loot.c_str(), get_loot.c_str() + static_cast<int>(get_loot.length()), &root, &err))
				{
					if (MessageBoxA(nullptr, "Are you sure?", "Disenchanting loot", MB_OKCANCEL) == IDOK)
					{
						int i = 0;

						for (const std::string& name : root.getMemberNames())
						{
							if (std::regex regex_str("^" + items_disenchant[item_index_disenchant].second + "_[\\d]+"); std::regex_match(
								name, regex_str))
							{
								std::string disenchant_case =
									items_disenchant[item_index_disenchant].second == "STATSTONE_SHARD"
										? "DISENCHANT"
										: "disenchant";
								std::string disenchant_name = root[name]["type"].asString();

								std::string disenchant_url = std::format(
									"https://127.0.0.1/lol-loot/v1/recipes/{0}_{1}/craft?repeat=1", disenchant_name,
									disenchant_case);
								std::string disenchant_body = std::format(R"(["{}"])", name);
								lcu::request("POST", disenchant_url, disenchant_body);
								i++;
							}
						}
						result = std::format("Disenchanted {0} {1}", std::to_string(i),
						                     items_disenchant[item_index_disenchant].first);
					}
				}
				else
					result = "Loot not found";
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 3));
			if (ImGui::BeginCombo("##comboDisenchant", combo_disenchant, 0))
			{
				for (size_t n = 0; n < items_disenchant.size(); n++)
				{
					const bool is_selected = (item_index_disenchant == n);
					if (ImGui::Selectable(items_disenchant[n].first.c_str(), is_selected))
						item_index_disenchant = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Champion name to ID");
			static std::vector<std::string> champ_names;
			if (!champ_skins.empty() && champ_names.empty())
			{
				for (const auto& [key, name, skins] : champ_skins)
				{
					champ_names.emplace_back(name);
					std::cout << name << std::endl;
				}
			}

			static char buf_champion_name[50];
			static size_t last_size = 0;
			static std::string closest_champion;
			static std::string closest_id;
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 3));
			ImGui::InputText("##inputChampionName", buf_champion_name, IM_ARRAYSIZE(buf_champion_name));
			if (strlen(buf_champion_name) < 1)
			{
				closest_champion = "";
				closest_id = "";
				last_size = 0;
			}
			else if (last_size != strlen(buf_champion_name))
			{
				last_size = strlen(buf_champion_name);
				closest_champion = levenshtein_distance(champ_names, buf_champion_name);

				for (const auto& [key, name, skins] : champ_skins)
				{
					if (closest_champion == name)
					{
						closest_id = std::to_string(key);
						break;
					}
				}
			}
			ImGui::SameLine();
			ImGui::TextWrapped("%s ID: %s", closest_champion.c_str(), closest_id.c_str());

			if (ImGui::Button("Check email of the account"))
				result = lcu::request("GET", "https://127.0.0.1/lol-email-verification/v1/email");

			ImGui::Separator();

			if (ImGui::Button("Tournament of Souls - unlock all"))
			{
				lcu::request("POST", "/lol-marketing-preferences/v1/partition/sfm2023", R"({
	"SmallConspiracyFan" : "True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True",
	"SmallGwenPykeFan" : "True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True",
	"SmallJhinFan" : "True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True,True",
	"SmallSettFans" : "True,True,True,True,True,True,True,True,True,True,True,True,True,True,True",
	"SmallShaco" : "True,True",
	"hasNewAbility" : "False",
	"hasNewFanLine" : "False",
	"hasPlayedTutorial" : "True",
	"hasSeenCelebration_Story" : "True",
	"hasSeenLoadoutTutorial" : "True",
	"hasSeenMapTutorial" : "True",
	"loadout_active_e" : "2",
	"loadout_active_q" : "1",
	"loadout_active_r" : "2",
	"loadout_active_w" : "2",
	"numNodesUnlocked" : "20",
	"progress" : "20"
})");

				Json::Value root;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				std::string get_grants = lcu::request("GET", "/lol-rewards/v1/grants");

				if (reader->parse(get_grants.c_str(), get_grants.c_str() + static_cast<int>(get_grants.length()), &root,
				                  &err))
				{
					if (root.isArray())
					{
						for (auto grant : root)
						{
							for (Json::Value& reward : grant["rewardGroup"]["rewards"])
							{
								Json::Value body;
								body["rewardGroupId"] = grant["info"]["rewardGroupId"].asString();
								body["selections"] = {};
								body["selections"].append(reward["id"].asString());

								result += lcu::request(
									"POST", std::format("/lol-rewards/v1/grants/{}/select",
									                    grant["info"]["id"].asString()), body.toStyledString());
							}
						}
					}
				}
			}

			ImGui::SameLine();
			ImGui::help_marker("You need reputation for this to work");

			static Json::StreamWriterBuilder w_builder;
			static std::string s_result_json;
			static char* c_result_json;

			if (!result.empty())
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
					s_result_json = result;
				else
				{
					s_result_json = writeString(w_builder, root);
				}
				result = "";
			}

			if (!s_result_json.empty())
			{
				c_result_json = s_result_json.data();
				ImGui::InputTextMultiline("##miscResult", c_result_json, s_result_json.size() + 1, ImVec2(600, 185));
			}

			if (on_open)
				on_open = false;

			ImGui::EndTabItem();
		}
		else
		{
			on_open = true;
		}
	}
};
