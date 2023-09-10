#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"
#include "Utils.h"
#include "Misc.h"

class game_tab
{
	static inline bool on_open_ = true;

public:
	static void render()
	{
		if (ImGui::BeginTabItem("Game"))
		{
			static std::string result;
			static std::string custom;

			static std::vector<std::pair<long, std::string>> gamemodes;

			if (on_open_)
			{
				if (gamemodes.empty())
				{
					std::string get_queues = lcu::request("GET", "/lol-game-queues/v1/queues");
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (reader->parse(get_queues.c_str(), get_queues.c_str() + static_cast<int>(get_queues.length()),
					                  &root, &err))
					{
						if (root.isArray())
						{
							for (auto& i : root)
							{
								if (i["queueAvailability"].asString() != "Available")
									continue;

								long id;
								id = i["id"].asInt();
								std::string name = i["name"].asString();
								name += " " + std::to_string(id);
								std::pair temp = {id, name};
								gamemodes.emplace_back(temp);
							}

							std::ranges::sort(gamemodes, [](auto& left, auto& right) {
								return left.first < right.first;
							});
						}
					}
				}
			}

			static std::vector<std::string> firstPosition = {
				"UNSELECTED", "TOP", "JUNGLE", "MIDDLE", "BOTTOM", "UTILITY", "FILL"
			};
			static std::vector<std::string> secondPosition = {
				"UNSELECTED", "TOP", "JUNGLE", "MIDDLE", "BOTTOM", "UTILITY", "FILL"
			};

			static int game_id = 0;

			ImGui::Columns(4, nullptr, false);

			if (ImGui::Button("Blind pick"))
				game_id = blind_pick;

			if (ImGui::Button("Draft pick"))
				game_id = draft_pick;

			if (ImGui::Button("Solo/Duo"))
				game_id = solo_duo;

			if (ImGui::Button("Flex"))
				game_id = flex;

			ImGui::NextColumn();

			if (ImGui::Button("ARAM"))
				game_id = aram;

			if (ImGui::Button("ARURF"))
				game_id = arurf;

			if (ImGui::Button("Nexus Blitz"))
				game_id = nexus_blitz;

			if (ImGui::Button("ARURF 1V1 (PBE)"))
				game_id = 901;

			/*if (ImGui::Button("URF"))
				gameID = 318;*/

			ImGui::NextColumn();

			if (ImGui::Button("TFT Normal"))
				game_id = tft_normal;

			if (ImGui::Button("TFT Ranked"))
				game_id = tft_ranked;

			if (ImGui::Button("TFT Hyper Roll"))
				game_id = tft_hyper_roll;

			if (ImGui::Button("TFT Double Up"))
				game_id = tft_double_up;

			ImGui::NextColumn();

			if (ImGui::Button("TFT Tutorial"))
				game_id = tft_tutorial;

			if (ImGui::Button("Practice Tool"))
			{
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":1},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
			}

			if (ImGui::Button("Practice Tool 5v5"))
			{
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
			}

			if (ImGui::Button("Clash"))
				game_id = clash;

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(4, nullptr, false);

			if (ImGui::Button("Tutorial 1"))
				game_id = tutorial1;

			if (ImGui::Button("Tutorial 2"))
				game_id = tutorial2;

			if (ImGui::Button("Tutorial 3"))
				game_id = tutorial3;

			ImGui::NextColumn();

			if (ImGui::Button("Intro Bots"))
				game_id = intro_bots;

			if (ImGui::Button("Beginner Bots"))
				game_id = beginner_bots;

			if (ImGui::Button("Intermediate Bots"))
				game_id = intermediate_bots;

			ImGui::NextColumn();

			if (ImGui::Button("Custom Blind"))
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"CLASSIC","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";

			if (ImGui::Button("Custom ARAM"))
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"ARAM","gameMutator":"","gameServerRegion":"","mapId":12,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";

			//"id" 1- blind 2- draft -4 all random 6- tournament draft

			static int index_gamemodes = -1;
			auto label_gamemodes = "All Gamemodes";
			if (index_gamemodes != -1)
				label_gamemodes = gamemodes[index_gamemodes].second.c_str();

			if (ImGui::BeginCombo("##combolGamemodes", label_gamemodes, 0))
			{
				for (size_t n = 0; n < gamemodes.size(); n++)
				{
					const bool is_selected = (index_gamemodes == static_cast<int>(n));
					if (ImGui::Selectable(gamemodes[n].second.c_str(), is_selected))
						index_gamemodes = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();

			if (ImGui::Button("Create##gamemode"))
			{
				game_id = gamemodes[index_gamemodes].first;
			}

			ImGui::NextColumn();

			static std::vector<std::pair<int, std::string>> bot_champs;
			static size_t index_bots = 0; // Here we store our selection data as an index.
			auto label_bots = "Bot";
			if (!bot_champs.empty())
				label_bots = bot_champs[index_bots].second.c_str();
			if (ImGui::BeginCombo("##comboBots", label_bots, 0))
			{
				if (bot_champs.empty())
				{
					std::string get_bots = lcu::request(
						"GET", "https://127.0.0.1/lol-lobby/v2/lobby/custom/available-bots");
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (reader->parse(get_bots.c_str(), get_bots.c_str() + static_cast<int>(get_bots.length()), &root,
					                  &err))
					{
						if (root.isArray())
						{
							for (auto& i : root)
							{
								std::pair<int, std::string> temp = {i["id"].asInt(), i["name"].asString()};
								bot_champs.emplace_back(temp);
							}
							std::ranges::sort(bot_champs,
							                  [](std::pair<int, std::string> a, std::pair<int, std::string> b) {
								                  return a.second < b.second;
							                  });
						}
					}
				}

				for (size_t n = 0; n < bot_champs.size(); n++)
				{
					const bool is_selected = (index_bots == n);
					if (ImGui::Selectable(bot_champs[n].second.c_str(), is_selected))
						index_bots = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			std::vector<std::string> difficulties = {"NONE", "EASY", "MEDIUM", "HARD", "UBER", "TUTORIAL", "INTRO"};
			static size_t index_difficulty = 0; // Here we store our selection data as an index.

			if (const char* label_difficulty = difficulties[index_difficulty].c_str(); ImGui::BeginCombo("##comboDifficulty", label_difficulty, 0))
			{
				for (size_t n = 0; n < difficulties.size(); n++)
				{
					const bool is_selected = (index_difficulty == n);
					if (ImGui::Selectable(difficulties[n].c_str(), is_selected))
						index_difficulty = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			static int bot_team = 0;

			if (ImGui::Button("Add bot##addBot"))
			{
				if (bot_champs.empty())
				{
					MessageBoxA(nullptr, "Pick the bot's champion first", "Adding bots failed", MB_OK);
				}
				else
				{
					std::string team = bot_team ? R"(,"teamId":"200"})" : R"(,"teamId":"100"})";
					std::string body = R"({"botDifficulty":")" + difficulties[index_difficulty] + R"(","championId":)" +
						std::to_string(bot_champs[index_bots].first) + team;
					result = lcu::request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", body);
				}
			}
			ImGui::SameLine();
			ImGui::RadioButton("Blue", &bot_team, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Red", &bot_team, 1);

			ImGui::Columns(1);

			if (game_id != 0 || !custom.empty())
			{
				std::string body;
				if (custom.empty())
				{
					body = R"({"queueId":)" + std::to_string(game_id) + "}";
				}
				else
				{
					body = custom;
					custom = "";
				}
				if (game_id == draft_pick || game_id == solo_duo || game_id == flex)
				{
					result = lcu::request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", body);
					lcu::request("PUT", "/lol-lobby/v1/lobby/members/localMember/position-preferences",
					             R"({"firstPreference":")" + firstPosition[s.game_tab.index_first_role]
					             + R"(","secondPreference":")" + secondPosition[s.game_tab.index_second_role] + "\"}");
				}
				else
				{
					result = lcu::request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", body);
				}

				game_id = 0;
			}

			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 7));
			if (const char* label_first_position = firstPosition[s.game_tab.index_first_role].c_str(); ImGui::BeginCombo(
				"##comboFirstPosition", label_first_position, 0))
			{
				for (size_t n = 0; n < firstPosition.size(); n++)
				{
					const bool is_selected = (s.game_tab.index_first_role == n);
					if (ImGui::Selectable(firstPosition[n].c_str(), is_selected))
						s.game_tab.index_first_role = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			/*		ImGui::SameLine();
					ImGui::Text("Primary");*/

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 7));
			if (const char* second_label_position = secondPosition[s.game_tab.index_second_role].c_str(); ImGui::BeginCombo(
				"##comboSecondPosition", second_label_position, 0))
			{
				for (size_t n = 0; n < secondPosition.size(); n++)
				{
					const bool is_selected = (s.game_tab.index_second_role == n);
					if (ImGui::Selectable(secondPosition[n].c_str(), is_selected))
						s.game_tab.index_second_role = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			//ImGui::SameLine();
			//ImGui::Text("Secondary");

			ImGui::SameLine();

			if (ImGui::Button("Pick roles"))
			{
				result = lcu::request("PUT", "/lol-lobby/v1/lobby/members/localMember/position-preferences",
				                      R"({"firstPreference":")" + firstPosition[s.game_tab.index_first_role]
				                      + R"(","secondPreference":")" + secondPosition[s.game_tab.index_second_role] +
				                      "\"}");
			}
			ImGui::SameLine();
			ImGui::help_marker(
				"If you are already in a lobby you can use this button to pick the roles, or start a new lobby with the buttons above");

			ImGui::NextColumn();

			if (ImGui::Button("Change runes"))
			{
				result = change_runes_opgg();
			}

			ImGui::SameLine();
			ImGui::Columns(2, nullptr, false);

			ImGui::Checkbox("Blue/Red Side notification", &s.game_tab.side_notification);

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(3, nullptr, false);
			if (ImGui::Button("Start queue"))
			{
				result = lcu::request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search");
			}
			ImGui::NextColumn();

			if (ImGui::Button("Dodge"))
			{
				result = lcu::request(
					"POST",
					R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])",
					"");
			}
			ImGui::SameLine();
			ImGui::help_marker("Dodges lobby instantly, you still lose LP, but you don't have to restart the client");
			ImGui::NextColumn();

			static std::vector<std::string> items_multi_search = {
				"OP.GG", "U.GG", "PORO.GG", "Porofessor.gg"
			};
			const char* selected_multi_search = items_multi_search[s.game_tab.index_multi_search].c_str();

			if (ImGui::Button("Multi-Search"))
			{
				result = multi_search(items_multi_search[s.game_tab.index_multi_search]);
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 6));
			if (ImGui::BeginCombo("##comboMultiSearch", selected_multi_search, 0))
			{
				for (size_t n = 0; n < items_multi_search.size(); n++)
				{
					const bool is_selected = (s.game_tab.index_multi_search == n);
					if (ImGui::Selectable(items_multi_search[n].c_str(), is_selected))
						s.game_tab.index_multi_search = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(3, nullptr, false);
			ImGui::Checkbox("Auto accept", &s.game_tab.auto_accept_enabled);

			ImGui::NextColumn();
			if (ImGui::Button("Invite everyone to lobby"))
			{
				std::string get_friends = lcu::request("GET", "https://127.0.0.1/lol-chat/v1/friends");

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(get_friends.c_str(), get_friends.c_str() + static_cast<int>(get_friends.length()), &root,
				                  &err))
				{
					if (root.isArray())
					{
						for (auto& i : root)
						{
							std::string friends_summoner_id = i["summonerId"].asString();
							std::string invite_body = "[{\"toSummonerId\":" + friends_summoner_id + "}]";
							lcu::request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/invitations", invite_body);
						}
						result = "Invited friends to lobby";
					}
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Refund last purchase"))
			{
				if (MessageBoxA(nullptr, "Are you sure?", "Refunding last purchase", MB_OKCANCEL) == IDOK)
				{
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root_purchase_history;

					cpr::Header store_header = utils::string_to_header(lcu::get_store_header());

					std::string store_url = lcu::request("GET", "/lol-store/v1/getStoreUrl");
					std::erase(store_url, '"');

					std::string purchase_history = Get(cpr::Url{store_url + "/storefront/v3/history/purchase"},
					                                   cpr::Header{store_header}).text;
					if (reader->parse(purchase_history.c_str(),
					                  purchase_history.c_str() + static_cast<int>(purchase_history.length()),
					                  &root_purchase_history, &err))
					{
						std::string account_id = root_purchase_history["player"]["accountId"].asString();
						std::string transaction_id = root_purchase_history["transactions"][0]["transactionId"].asString();
						result = Post(cpr::Url{store_url + "/storefront/v3/refund"}, cpr::Header{store_header},
						              cpr::Body{
							              "{\"accountId\":" + account_id + R"(,"transactionId":")" + transaction_id +
							              R"(","inventoryType":"CHAMPION","language":"en_US"})"
						              }).text;
					}
					else
					{
						result = purchase_history;
					}
				}
			}
			ImGui::SameLine();
			ImGui::help_marker(
				"Buy a champion, pick it during a game and click this button before the game ends, no refund token will be used to refund it");

			ImGui::Columns(1);

			ImGui::Separator();

			//ImGui::Columns(2, 0, false);

			ImGui::Text("Instant message:");
			ImGui::SameLine();
			static char buf_instant_message[500];
			std::ranges::copy(s.game_tab.instant_message, buf_instant_message);
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 6));
			ImGui::InputText("##inputInstantMessage", buf_instant_message, IM_ARRAYSIZE(buf_instant_message));
			s.game_tab.instant_message = buf_instant_message;

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 7));
			ImGui::SliderInt("Delay##sliderInstantMessageDelay", &s.game_tab.instant_message_delay, 0, 10000, "%d ms");
			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 10));
			ImGui::SliderInt("Time(s)##sliderInstantMessageTimes", &s.game_tab.instant_message_times, 1, 10, "%d");

			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 7));
			ImGui::SliderInt("Delay between##sliderInstantMessageDelayTimes", &s.game_tab.instant_message_delay_times, 0,
			                 4000, "%d ms");

			ImGui::Separator();

			static bool is_still_being_fetched = true;
			if (!champ_skins.empty())
				is_still_being_fetched = false;

			static ImGui::combo_auto_select_data instalock_combo_data;

			if (on_open_)
			{
				std::vector<std::pair<int, std::string>> instalock_champs = get_instalock_champs();

				if (!instalock_champs.empty())
				{
					std::vector<std::string> instalock_champs_names;
					instalock_champs_names.reserve(instalock_champs.size());

					std::string selected_champ = champ_id_to_name(s.game_tab.instalock_id);
					std::ranges::copy(selected_champ, instalock_combo_data.input);

					for (size_t i = 0; i < instalock_champs.size(); i++)
					{
						instalock_champs_names.emplace_back(instalock_champs[i].second);
						if (instalock_combo_data.input == instalock_champs[i].second)
						{
							instalock_combo_data.index = i;
						}
					}
					instalock_combo_data.items = instalock_champs_names;
				}
			}

			ImGui::Checkbox("Instalock", &s.game_tab.instalock_enabled);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 6));
			if (combo_auto_select("##comboInstalock", instalock_combo_data))
			{
				if (instalock_combo_data.index != -1)
				{
					for (const auto& [key, name, skins] : champ_skins)
					{
						if (instalock_combo_data.input == name)
						{
							s.game_tab.instalock_id = key;
						}
					}
				}
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 6));

			ImGui::SliderInt("Delay##sliderInstalockDelay", &s.game_tab.instalock_delay, 0, 10000, "%d ms");

			ImGui::SameLine();

			ImGui::Checkbox("Dodge on champion ban", &s.game_tab.dodge_on_ban);

			ImGui::SameLine();
			ImGui::help_marker("Ignores backup pick");

			static std::string chosen_backup = "Backup pick \t\t\tChosen: " + misc::champ_id_to_name(s.game_tab.backup_id) +
				"###AnimatedBackup";
			static int last_backup_id = 0;
			if ((last_backup_id != s.game_tab.backup_id) && !is_still_being_fetched)
			{
				last_backup_id = s.game_tab.backup_id;
				chosen_backup = "Backup pick \t\t\tChosen: " + misc::champ_id_to_name(s.game_tab.backup_id) +
					"###AnimatedBackup";
			}
			if (ImGui::CollapsingHeader(chosen_backup.c_str()))
			{
				ImGui::Text("None");
				ImGui::SameLine();
				ImGui::RadioButton("##noneBackupPick", &s.game_tab.backup_id, 0);
				for (std::vector<std::pair<int, std::string>> instalock_champs = get_instalock_champs(); const auto& champ : instalock_champs)
				{
					char bufchamp[128];
					sprintf_s(bufchamp, "##Select %s", champ.second.c_str());
					ImGui::Text("%s", champ.second.c_str());
					ImGui::SameLine();
					ImGui::RadioButton(bufchamp, &s.game_tab.backup_id, champ.first);
				}
			}

			ImGui::Checkbox("Auto ban", &s.game_tab.auto_ban_enabled);
			ImGui::SameLine();

			static ImGui::combo_auto_select_data autoban_combo_data;
			if (on_open_)
			{
				if (!champ_skins.empty())
				{
					std::vector<std::string> autoban_champs_names;
					autoban_champs_names.reserve(champ_skins.size());

					std::string selected_champ = champ_id_to_name(s.game_tab.auto_ban_id);
					std::ranges::copy(selected_champ, autoban_combo_data.input);

					for (size_t i = 0; i < champ_skins.size(); i++)
					{
						autoban_champs_names.emplace_back(champ_skins[i].name);

						if (autoban_combo_data.input == champ_skins[i].name)
						{
							autoban_combo_data.index = i;
						}
					}
					autoban_combo_data.items = autoban_champs_names;
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 6));
			if (combo_auto_select("##comboAutoban", autoban_combo_data))
			{
				if (autoban_combo_data.index != -1)
				{
					for (const auto& [key, name, skins] : champ_skins)
					{
						if (autoban_combo_data.input == name)
						{
							s.game_tab.auto_ban_id = key;
						}
					}
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(s.window.width / 6));
			ImGui::SliderInt("Delay##sliderautoBanDelay", &s.game_tab.auto_ban_delay, 0, 10000, "%d ms");

			ImGui::SameLine();

			ImGui::Checkbox("Instant Mute", &s.game_tab.instant_mute);

			ImGui::SeparatorText("Free ARAM Boost");

			static std::string store_token;
			static cpr::Header store_header;
			static std::string account_id;
			static std::string boosted;
			static int owned_rp;
			if (ImGui::Button("Is boost available for this account?"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				std::string get_wallet = lcu::request("GET", "https://127.0.0.1/lol-inventory/v1/wallet/RP");
				if (reader->parse(get_wallet.c_str(), get_wallet.c_str() + static_cast<int>(get_wallet.length()), &root,
				                  &err))
				{
					owned_rp = root["RP"].asInt();
				}

				std::string get_session = lcu::request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(get_session.c_str(), get_session.c_str() + static_cast<int>(get_session.length()), &root,
				                  &err))
				{
					account_id = root["accountId"].asString();
				}

				if (!check_jwt(account_id) && owned_rp < 95)
				{
					int time_left = 0;
					std::string temp = get_old_jwt(account_id, time_left);
					time_left = static_cast<int>(60 * 60 * 24 + time_left - time(nullptr));
					int minutes = time_left / 60 - 60 * (time_left / (60 * 60));
					boosted = "Boost available, time left on this account: " + std::to_string(time_left / (60 * 60)) +
						":" + std::to_string(minutes);
				}
				else
				{
					// get owned champions
					std::vector<int> owned_champions;
					std::string get_champions = lcu::request(
						"GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION");
					if (reader->parse(get_champions.c_str(),
					                  get_champions.c_str() + static_cast<int>(get_champions.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto obj : root)
							{
								if (obj["ownershipType"].asString() == "OWNED")
								{
									owned_champions.emplace_back(obj["itemId"].asInt());
								}
							}
						}
					}

					std::vector<std::pair<int, int>> champs_to_buy; // price, id
					std::string get_catalog = lcu::request("GET", "https://127.0.0.1/lol-store/v1/catalog");
					if (reader->parse(get_catalog.c_str(), get_catalog.c_str() + static_cast<int>(get_catalog.length()),
					                  &root, &err))
					{
						if (root.isArray())
						{
							for (auto obj : root)
							{
								if (obj["inventoryType"].asString() == "CHAMPION")
								{
									if (obj["sale"].empty() == true)
									{
										for (Json::Value::ArrayIndex i = 0; i < obj["prices"].size(); i++)
										{
											if (auto price = obj["prices"][i]; price["currency"].asString() == "RP")
											{
												champs_to_buy.emplace_back(price["cost"].asInt(), obj["itemId"].asInt());
											}
										}
									}
									else
									{
										for (Json::Value::ArrayIndex i = 0; i < obj["sale"]["prices"].size(); i++)
										{
											if (auto sale = obj["sale"]["prices"][i]; sale["currency"].asString() == "RP")
											{
												champs_to_buy.emplace_back(sale["cost"].asInt(), obj["itemId"].asInt());
											}
										}
									}
								}
							}
						}
					}

					int id_to_buy = 0;
					int price_to_buy = 0;

					for (auto [fst, snd] : champs_to_buy)
					{
						bool found = false;
						for (int id : owned_champions)
						{
							if (snd == id)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							if (((owned_rp - fst) > 0) && ((owned_rp - fst) < 95))
							{
								price_to_buy = fst;
								id_to_buy = snd;
								break;
							}
						}
					}
					if (id_to_buy != 0 && price_to_buy != 0)
					{
						boosted = "Boost is available for this account";
					}
					else
					{
						boosted = "Boost IS NOT available for this account";
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Free ARAM/ARURF boost"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				std::string get_wallet = lcu::request("GET", "https://127.0.0.1/lol-inventory/v1/wallet/RP");
				if (reader->parse(get_wallet.c_str(), get_wallet.c_str() + static_cast<int>(get_wallet.length()), &root,
				                  &err))
				{
					owned_rp = root["RP"].asInt();
				}

				// get accountId
				std::string get_session = lcu::request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(get_session.c_str(), get_session.c_str() + static_cast<int>(get_session.length()), &root,
				                  &err))
				{
					account_id = root["accountId"].asString();
				}

				bool b_need_new_jwt = true;
				if (!check_jwt(account_id))
				{
					b_need_new_jwt = false;
					if (owned_rp >= 95)
						b_need_new_jwt = true;
				}

				// get owned champions
				std::vector<int> owned_champions;
				std::string get_champions = lcu::request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION");
				if (reader->parse(get_champions.c_str(), get_champions.c_str() + static_cast<int>(get_champions.length()),
				                  &root, &err))
				{
					if (root.isArray())
					{
						for (auto obj : root)
						{
							if (obj["ownershipType"].asString() == "OWNED")
							{
								owned_champions.emplace_back(obj["itemId"].asInt());
							}
						}
					}
				}

				std::vector<std::pair<int, int>> champs_to_buy;
				std::string get_catalog = lcu::request("GET", "https://127.0.0.1/lol-store/v1/catalog");
				if (reader->parse(get_catalog.c_str(), get_catalog.c_str() + static_cast<int>(get_catalog.length()), &root,
				                  &err))
				{
					if (root.isArray())
					{
						for (auto obj : root)
						{
							if (obj["inventoryType"].asString() == "CHAMPION")
							{
								if (obj["sale"].empty() == true)
								{
									for (Json::Value::ArrayIndex i = 0; i < obj["prices"].size(); i++)
									{
										if (auto price = obj["prices"][i]; price["currency"].asString() == "RP")
										{
											champs_to_buy.emplace_back(price["cost"].asInt(), obj["itemId"].asInt());
										}
									}
								}
								else
								{
									for (Json::Value::ArrayIndex i = 0; i < obj["sale"]["prices"].size(); i++)
									{
										if (auto sale = obj["sale"]["prices"][i]; sale["currency"].asString() == "RP")
										{
											champs_to_buy.emplace_back(sale["cost"].asInt(), obj["itemId"].asInt());
										}
									}
								}
							}
						}
					}
				}

				int id_to_buy = 0;
				int price_to_buy = 0;

				for (auto [fst, snd] : champs_to_buy)
				{
					bool found = false;
					for (int id : owned_champions)
					{
						if (snd == id)
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						if (owned_rp - fst > 0 && owned_rp - fst < 95)
						{
							price_to_buy = fst;
							id_to_buy = snd;
							break;
						}
					}
				}
				if ((id_to_buy != 0 && price_to_buy != 0) || !b_need_new_jwt)
				{
					std::string get_store_url = lcu::request("GET", "https://127.0.0.1/lol-store/v1/getStoreUrl");
					std::erase(get_store_url, '"');

					Json::CharReaderBuilder builder2;
					const std::unique_ptr<Json::CharReader> reader2(builder2.newCharReader());
					JSONCPP_STRING err2;
					Json::Value root2;

					// get signedWalletJwt
					std::string signed_wallet_jwt = lcu::request(
						"GET", "https://127.0.0.1/lol-inventory/v1/signedWallet/RP");
					if (reader2->parse(signed_wallet_jwt.c_str(),
					                   signed_wallet_jwt.c_str() + static_cast<int>(signed_wallet_jwt.length()), &root2,
					                   &err2))
					{
						signed_wallet_jwt = root2["RP"].asString();
						if (b_need_new_jwt)
						{
							save_jwt(account_id, signed_wallet_jwt, static_cast<unsigned int>(time(nullptr)));
						}
						else
						{
							int time_left = 0;
							signed_wallet_jwt = get_old_jwt(account_id, time_left);
						}

						Json::CharReaderBuilder builder3;
						const std::unique_ptr<Json::CharReader> reader3(builder3.newCharReader());
						JSONCPP_STRING err3;
						Json::Value root3;

						// get Bearer token for store
						std::string authorizations = lcu::request(
							"GET", "https://127.0.0.1/lol-rso-auth/v1/authorization/access-token");
						if (reader3->parse(authorizations.c_str(),
						                   authorizations.c_str() + static_cast<int>(authorizations.length()), &root3,
						                   &err3))
						{
							store_token = root3["token"].asString();
							store_header = utils::string_to_header(lcu::get_store_header());
							if (b_need_new_jwt)
							{
								// buy a champion
								std::string purchase_body = R"({"accountId":)" + account_id +
									R"(,"items":[{"inventoryType":"CHAMPION","itemId":)" + std::to_string(id_to_buy)
									+ R"(,"ipCost":null,"rpCost":)" + std::to_string(price_to_buy) +
									R"(,"quantity":1}]})";
								std::string purchase_url = get_store_url + "/storefront/v3/purchase?language=en_US";
								std::string purchase = Post(cpr::Url{purchase_url}, cpr::Body{purchase_body},
								                            cpr::Header{store_header}).text;
								boosted = "Bought " + champ_id_to_name(id_to_buy) +
									" - dont play this champion, or you wont be able to refund RP";
							}

							std::this_thread::sleep_for(std::chrono::seconds(1));

							// boost with signedWalletJwt
							std::string boost_url =
								R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","activateBattleBoostV1","{\"signedWalletJwt\":\")"
								+ signed_wallet_jwt + R"(\"}"])";
							lcu::request("POST", boost_url);
						}
					}
				}
				else
				{
					boosted = "Ineligible for boost on this account";
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Refund RP"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				// get accountId
				std::string get_session = lcu::request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(get_session.c_str(), get_session.c_str() + static_cast<int>(get_session.length()), &root,
				                  &err))
				{
					account_id = root["accountId"].asString();
				}

				std::string get_store_url = lcu::request("GET", "https://127.0.0.1/lol-store/v1/getStoreUrl");
				std::erase(get_store_url, '"');

				std::string authorizations = lcu::request(
					"GET", "https://127.0.0.1/lol-rso-auth/v1/authorization/access-token");
				if (reader->parse(authorizations.c_str(),
				                  authorizations.c_str() + static_cast<int>(authorizations.length()), &root, &err))
				{
					store_token = root["token"].asString();
					store_header = utils::string_to_header(lcu::get_store_header());
				}

				std::string history_url = get_store_url + "/storefront/v3/history/purchase?language=en_US";
				std::string get_history = Get(cpr::Url{history_url}, cpr::Header{store_header}).text;
				if (reader->parse(get_history.c_str(), get_history.c_str() + static_cast<int>(get_history.length()), &root,
				                  &err))
				{
					if (root["transactions"].isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < root["transactions"].size(); i++)
						{
							if (auto transaction = root["transactions"][i]; transaction["refundable"].asBool() == true)
							{
								if (transaction["inventoryType"].asString() == "CHAMPION")
								{
									if (transaction["currencyType"].asString() == "RP")
									{
										if (transaction["requiresToken"].asBool() == false)
										{
											std::string refund_url = get_store_url + "/storefront/v3/refund";
											std::string refund_body = R"({"accountId":)" + account_id +
												R"(,"transactionId":")" + transaction["transactionId"].asString() +
												R"(","inventoryType":"CHAMPION","language":"en_US"})";
											Post(cpr::Url{refund_url}, cpr::Body{refund_body},
											     cpr::Header{store_header});
											boosted = "Refunded";
										}
									}
								}
							}
						}
					}
				}
			}

			ImGui::SameLine();
			ImGui::help_marker("Instructions:\n"
				"You have to wait at least 1 hour after finishing your last game, otherwise the RP you used for boost will get consumed\n"
				"The longer you wait, the better. On a single token you can boost unlimited amount of times in 24h\n"
				"The exploit stores your RP, buys a champion with RP so you're left with <95 and then boosts using the stored RP\n"
				"Don't play with the champion the boost bought, or you wont be able to get your RP back\n");

			ImGui::TextWrapped(boosted.c_str());

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
				ImGui::InputTextMultiline("##gameResult", c_result_json, s_result_json.size() + 1, ImVec2(600, 300));
			}

			if (on_open_)
				on_open_ = false;

			ImGui::EndTabItem();
		}
		else
		{
			on_open_ = true;
		}
	}

	static std::vector<std::pair<int, std::string>> get_instalock_champs()
	{
		std::vector<std::pair<int, std::string>> temp;

		std::string result = lcu::request("GET", "https://127.0.0.1/lol-champions/v1/owned-champions-minimal");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			if (root.isArray())
			{
				for (auto& i : root)
				{
					if (i["freeToPlay"].asBool() == true || i["ownership"]["owned"].asBool() == true ||
						(i["ownership"].isMember("xboxGPReward") && i["ownership"]["xboxGPReward"].asBool() == true))
					{
						std::string load_screen_path = i["baseLoadScreenPath"].asString();
						size_t name_start = load_screen_path.find("ASSETS/Characters/") + strlen("ASSETS/Characters/");
						std::string champ_name = load_screen_path.substr(name_start, load_screen_path.find('/', name_start) - name_start);

						std::pair champ = {i["id"].asInt(), champ_name};
						temp.emplace_back(champ);
					}
				}
			}
		}
		std::ranges::sort(temp, [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) { return a.second < b.second; });
		return temp;
	}

	static void instant_message(const bool instant_mute = false, const bool side_notification = false)
	{
		auto start = std::chrono::system_clock::now();
		while (true)
		{
			auto now = std::chrono::system_clock::now();
			std::chrono::duration<double> diff = now - start;
			if (diff.count() > 10) // took 10 seconds and still didn't connect to chat
			{
				return;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;

			lcu::set_current_client_riot_info();
			std::string get_chat = Get(cpr::Url{
				                           std::format("https://127.0.0.1:{}/chat/v5/participants/champ-select",
				                                       lcu::riot.port)
			                           },
			                           cpr::Header{utils::string_to_header(lcu::riot.header)},
			                           cpr::VerifySsl{false}).text;
			if (!reader->parse(get_chat.c_str(), get_chat.c_str() + static_cast<int>(get_chat.length()), &root, &err))
			{
				continue;
			}

			const auto participantsArr = root["participants"];
			if (!participantsArr.isArray())
			{
				continue;
			}
			if (participantsArr.size() <= 1)
			{
				continue;
			}

			const std::string cid = participantsArr[0]["cid"].asString();

			if (instant_mute || side_notification)
			{
				std::string champ_select = lcu::request("GET", "/lol-champ-select/v1/session");
				Json::Value root_c_select;
				if (!champ_select.empty() && champ_select.find("RPC_ERROR") == std::string::npos)
				{
					if (reader->parse(champ_select.c_str(), champ_select.c_str() + static_cast<int>(champ_select.length()),
					                  &root_c_select, &err))
					{
						if (instant_mute)
						{
							int local_player_cell_id = root_c_select["localPlayerCellId"].asInt();
							for (Json::Value::ArrayIndex i = 0; i < root_c_select["myTeam"].size(); i++)
							{
								Json::Value player = root_c_select["myTeam"][i];
								if (player["cellId"].asInt() == local_player_cell_id)
									continue;

								lcu::request("POST", "/lol-champ-select/v1/toggle-player-muted",
								             std::format(
									             R"({{"summonerId":{0},"puuid":"{1}","obfuscatedSummonerId":{2},"obfuscatedPuuid":"{3}"}})",
									             player["summonerId"].asString(), player["puuid"].asString(),
									             player["obfuscatedSummonerId"].asString(),
									             player["obfuscatedPuuid"].asString()));
							}
						}

						if (side_notification)
						{
							if (root_c_select["myTeam"].isArray() && !root_c_select["myTeam"].empty())
							{
								std::string notify = "You are on the ";
								if (root_c_select["myTeam"][0]["team"].asInt() == 1)
									notify += "Blue Side";
								else
									notify += "Red Side";
								lcu::request("POST", std::format("/lol-chat/v1/conversations/{}/messages", cid),
								             R"({"body":")" + notify + R"(","type":"celebration"})");
							}
						}
					}
				}
			}

			if (s.game_tab.instant_message.empty())
				return;

			const std::string request = "https://127.0.0.1/lol-chat/v1/conversations/" + cid + "/messages";
			const std::string body = R"({"type":"chat", "body":")" + std::string(s.game_tab.instant_message) + R"("})";

			std::this_thread::sleep_for(std::chrono::milliseconds(s.game_tab.instant_message_delay));

			now = std::chrono::system_clock::now();
			int num_of_sent = 0;
			while (true)
			{
				for (; num_of_sent < s.game_tab.instant_message_times; num_of_sent++)
				{
					if (std::string error = lcu::request("POST", request, body); error.find("errorCode") != std::string::npos)
					{
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(s.game_tab.instant_message_delay_times));
				}

				if (num_of_sent >= s.game_tab.instant_message_times)
				{
					return;
				}

				diff = now - start;
				if (diff.count() > 10) // took 10 seconds and still not all messages sent
				{
					return;
				}
			}
		}
	}

	static void auto_accept()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (FindWindowA("RiotWindowClass", "League of Legends (TM) Client"))
			{
				// game is running, auto accept is not needed
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}

			if (!FindWindowA("RCLIENT", "League of Legends"))
			{
				continue;
			}

			if (s.game_tab.auto_accept_enabled || (s.game_tab.auto_ban_enabled && s.game_tab.auto_ban_id) ||
				(s.game_tab.dodge_on_ban && s.game_tab.instalock_enabled) ||
				(s.game_tab.instalock_enabled && s.game_tab.instalock_id) ||
				!s.game_tab.instant_message.empty())
			{
				Json::Value root_search;
				Json::Value root_champ_select;
				Json::Value root_session;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;

				cpr::Session session;
				session.SetVerifySsl(false);
				session.SetHeader(utils::string_to_header(lcu::league.header));
				session.SetUrl(std::format("https://127.0.0.1:{}/lol-lobby/v2/lobby/matchmaking/search-state",
				                           lcu::league.port));

				std::string get_search_state = session.Get().text;
				if (!reader->parse(get_search_state.c_str(),
				                   get_search_state.c_str() + static_cast<int>(get_search_state.length()), &root_search,
				                   &err))
				{
					continue;
				}

				static bool on_champ_select = true; //false when in champ select
				static int use_backup_id = 0;
				static bool is_picked = false;

				if (root_search["searchState"].asString() != "Found") // not found, not in champ select
				{
					on_champ_select = true;
					use_backup_id = 0;
					is_picked = false;
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					continue;
				}

				session.SetUrl(std::format("https://127.0.0.1:{}/lol-champ-select/v1/session", lcu::league.port));
				std::string getChampSelect = session.Get().text;
				if (getChampSelect.find("RPC_ERROR") != std::string::npos)
				// game found but champ select error means queue pop
				{
					on_champ_select = true;
					use_backup_id = 0;
					is_picked = false;
					if (s.game_tab.auto_accept_enabled)
					{
						session.SetUrl(std::format("https://127.0.0.1:{}/lol-matchmaking/v1/ready-check/accept",
						                           lcu::league.port));
						session.SetBody("");
						session.Post();
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}
				// in champ select
				if (!reader->parse(getChampSelect.c_str(),
				                   getChampSelect.c_str() + static_cast<int>(getChampSelect.length()), &root_champ_select,
				                   &err))
				{
					continue;
				}

				if (on_champ_select)
				{
					on_champ_select = false;

					if (!s.game_tab.instant_message.empty() || s.game_tab.instant_mute || s.game_tab.side_notification)
					{
						std::thread instant_message_thread(&game_tab::instant_message, s.game_tab.instant_mute,
						                                   s.game_tab.side_notification);
						instant_message_thread.detach();
					}
				}

				if ((s.game_tab.instalock_enabled || s.game_tab.auto_ban_id) && !is_picked)
				{
					session.SetUrl(std::format("https://127.0.0.1:{}/lol-login/v1/session", lcu::league.port));
					std::string get_session = session.Get().text;
					if (!reader->parse(get_session.c_str(), get_session.c_str() + static_cast<int>(get_session.length()),
					                   &root_session, &err))
					{
						continue;
					}

					const int cell_id = root_champ_select["localPlayerCellId"].asInt();
					for (Json::Value::ArrayIndex j = 0; j < root_champ_select["actions"].size(); j++)
					{
						auto actions = root_champ_select["actions"][j];
						if (!actions.isArray())
						{
							continue;
						}
						for (auto& action : actions)
						{
							// search for own actions
							if (action["actorCellId"].asInt() == cell_id)
							{
								if (std::string action_type = action["type"].asString(); action_type == "pick" && s.game_tab.instalock_id && s.
									game_tab.instalock_enabled)
								{
									if (action["completed"].asBool() == false)
									{
										if (!is_picked)
										{
											std::this_thread::sleep_for(
												std::chrono::milliseconds(s.game_tab.instalock_delay));

											int current_pick = s.game_tab.instalock_id;
											if (use_backup_id)
												current_pick = use_backup_id;

											session.SetUrl(std::format(
												"https://127.0.0.1:{}/lol-champ-select/v1/session/actions/{}",
												lcu::league.port, action["id"].asString()));
											session.SetBody(
												R"({"completed":true,"championId":)" + std::to_string(current_pick) +
												"}");
											session.Patch();
										}
									}
									else
									{
										is_picked = true;
									}
								}
								else if (action_type == "ban" && s.game_tab.auto_ban_id && s.game_tab.auto_ban_enabled)
								{
									if (action["completed"].asBool() == false)
									{
										std::this_thread::sleep_for(std::chrono::milliseconds(s.game_tab.auto_ban_delay));

										session.SetUrl(std::format(
											"https://127.0.0.1:{}/lol-champ-select/v1/session/actions/{}",
											lcu::league.port, action["id"].asString()));
										session.SetBody(
											R"({"completed":true,"championId":)" + std::to_string(s.game_tab.auto_ban_id) +
											"}");
										session.Patch();
									}
								}
							}

							else if ((s.game_tab.dodge_on_ban || s.game_tab.backup_id) && s.game_tab.instalock_enabled && s.
								game_tab.instalock_id)
							{
								if (is_picked)
									break;

								if (action["actorCellId"].asInt() == cell_id)
									continue;

								if (action["type"].asString() == "ban" && action["completed"].asBool() == true)
								{
									if (action["championId"].asInt() == s.game_tab.instalock_id)
									{
										if (s.game_tab.dodge_on_ban)
										{
											session.SetUrl(
												std::format("https://127.0.0.1:{}", lcu::league.port) +
												R"(/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])");
											session.SetBody("");
											session.Post();
										}
										else if (s.game_tab.backup_id)
										{
											use_backup_id = s.game_tab.backup_id;
										}
									}
								}
								else if (action["type"].asString() == "pick" && action["completed"].asBool() ==
									true)
								{
									if (s.game_tab.backup_id && action["championId"].asInt() == s.game_tab.instalock_id)
									{
										use_backup_id = s.game_tab.backup_id;
									}
								}
							}
						}
					}
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}
		}
	}

	static std::string multi_search(const std::string& website)
	{
		std::string names;
		std::string champ_select = lcu::request("GET", "https://127.0.0.1/lol-champ-select/v1/session");
		if (!champ_select.empty() && champ_select.find("RPC_ERROR") == std::string::npos)
		{
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;
			Json::Value root_region;
			Json::Value root_c_select;
			Json::Value root_summoner;
			Json::Value root_participants;

			std::wstring summ_names;

			if (reader->parse(champ_select.c_str(), champ_select.c_str() + static_cast<int>(champ_select.length()),
			                  &root_c_select, &err))
			{
				if (auto team_arr = root_c_select["myTeam"]; team_arr.isArray())
				{
					bool is_ranked = false;
					for (auto& i : team_arr)
					{
						if (i["nameVisibilityType"].asString() == "HIDDEN")
						{
							is_ranked = true;
							break;
						}

						if (std::string summ_id = i["summonerId"].asString(); summ_id != "0")
						{
							std::string summoner = lcu::request(
								"GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + summ_id);
							if (reader->parse(summoner.c_str(), summoner.c_str() + static_cast<int>(summoner.length()),
							                  &root_summoner, &err))
							{
								summ_names += utils::string_to_wstring(root_summoner["internalName"].asString()) + L",";
							}
						}
					}

					//	Ranked Lobby Reveal
					if (is_ranked)
					{
						summ_names = L"";

						lcu::set_current_client_riot_info();
						std::string participants = Get(cpr::Url{
							                               std::format(
								                               "https://127.0.0.1:{}/chat/v5/participants/champ-select",
								                               lcu::riot.port)
						                               },
						                               cpr::Header{utils::string_to_header(lcu::riot.header)},
						                               cpr::VerifySsl{false}).text;
						if (reader->parse(participants.c_str(),
						                  participants.c_str() + static_cast<int>(participants.length()),
						                  &root_participants, &err))
						{
							if (auto participants_arr = root_participants["participants"]; participants_arr.isArray())
							{
								for (auto& i : participants_arr)
								{
									summ_names += utils::string_to_wstring(i["name"].asString()) + L",";
								}
							}
						}
					}

					std::wstring region;
					if (website == "U.GG")
					{
						std::string get_authorization = lcu::request("GET", "/lol-rso-auth/v1/authorization");
						if (reader->parse(get_authorization.c_str(),
						                  get_authorization.c_str() + static_cast<int>(get_authorization.length()),
						                  &root_region, &err))
						{
							region = utils::string_to_wstring(root_region["currentPlatformId"].asString());
						}
					}
					else
					{
						std::string get_region = lcu::request("GET", "/riotclient/get_region_locale");
						if (reader->parse(get_region.c_str(), get_region.c_str() + static_cast<int>(get_region.length()),
						                  &root_region, &err))
						{
							region = utils::string_to_wstring(root_region["webRegion"].asString());
						}
					}

					if (!region.empty())
					{
						if (summ_names.empty())
							return "Failed to get summoner names";

						if (summ_names.at(summ_names.size() - 1) == L',')
							summ_names.pop_back();

						std::wstring url;
						if (website == "OP.GG")
						{
							url = L"https://" + region + L".op.gg/multi/query=" + summ_names;
						}
						else if (website == "U.GG")
						{
							url = L"https://u.gg/multisearch?summoners=" + summ_names + L"&region=" + utils::to_lower(
								region);
						}
						else if (website == "PORO.GG")
						{
							url = L"https://poro.gg/multi?region=" + utils::to_upper(region) + L"&q=" + summ_names;
						}
						else if (website == "Porofessor.gg")
						{
							url = L"https://porofessor.gg/pregame/" + region + L"/" + summ_names;
						}

						ShellExecuteW(nullptr, nullptr, url.c_str(), nullptr, nullptr, SW_SHOW);
						return utils::wstring_to_string(url);
					}
					return "Failed to get region";
				}
			}
		}

		return "Champion select not found";
	}

	static std::string change_runes_opgg()
	{
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root_current_page;

		std::string current_champion = lcu::request("GET", "/lol-champ-select/v1/current-champion");
		if (current_champion == "0" || current_champion.empty() || current_champion.find("RPC_ERROR") != std::string::npos)
		{
			return "Champion not picked";
		}

		std::string current_champion_name;
		for (const auto& [key, name, skins] : champ_skins)
		{
			if (std::stoi(current_champion) == key)
			{
				current_champion_name = name;
				break;
			}
		}

		std::string get_current_page = lcu::request("GET", "/lol-perks/v1/currentpage");
		if (!reader->parse(get_current_page.c_str(), get_current_page.c_str() + static_cast<int>(get_current_page.length()),
		                   &root_current_page, &err))
		{
			return "Failed to get current rune page";
		}
		std::string current_page_id = root_current_page["id"].asString();

		std::stringstream ss_opgg(
			Get(cpr::Url{"https://www.op.gg/champions/" + utils::to_lower(current_champion_name)}).text);
		std::vector<std::string> runes;
		std::string primary_perk, secondary_perk;

		std::string buf;
		while (ss_opgg >> buf)
		{
			if (runes.size() == 9)
				break;

			if (buf.find("src=\"https://opgg-static.akamaized.net/images/lol/perk") != std::string::npos
				|| buf.find("src=\"https://opgg-static.akamaized.net/meta/images/lol/perk") != std::string::npos)
			{
				if (buf.find("grayscale") != std::string::npos)
					continue;

				if (buf.find("/perkStyle/") != std::string::npos)
				{
					buf = buf.substr(buf.find("/perkStyle/") + strlen("/perkStyle/"), 4);
					if (primary_perk.empty())
						primary_perk = buf;
					else if (secondary_perk.empty())
						secondary_perk = buf;
				}
				else if (buf.find("/perk/") != std::string::npos)
				{
					buf = buf.substr(buf.find("/perk/") + strlen("/perk/"), 4);
					runes.emplace_back(buf);
				}
				else if (buf.find("/perkShard/") != std::string::npos)
				{
					buf = buf.substr(buf.find("/perkShard/") + strlen("/perkShard/"), 4);
					runes.emplace_back(buf);
				}
			}
		}
		if (runes.size() != 9 || primary_perk.empty() || secondary_perk.empty())
		{
			return "Failed to fetch op.gg runes";
		}

		lcu::request("DELETE", "/lol-perks/v1/pages/" + current_page_id);

		Json::Value root_page;
		root_page["name"] = current_champion_name + " OP.GG";
		root_page["primaryStyleId"] = primary_perk;
		root_page["subStyleId"] = secondary_perk;
		root_page["selectedPerkIds"] = Json::Value(Json::arrayValue);
		for (const std::string& rune : runes)
			root_page["selectedPerkIds"].append(rune);
		root_page["current"] = true;

		return lcu::request("POST", "lol-perks/v1/pages", root_page.toStyledString());
	}

	static std::string champ_id_to_name(const int& id)
	{
		for (const auto& [key, name, skins] : champ_skins)
		{
			if (id == key)
			{
				return name;
			}
		}
		return "";
	}

	// TODO: rewrite and move these to config file
	static std::string get_old_jwt(const std::string& acc_id, int& old_timestamp)
	{
		char* p_roaming;
		size_t roaming_len;
		_dupenv_s(&p_roaming, &roaming_len, "APPDATA");
		std::string roaming = p_roaming;
		std::string file_path = roaming + "\\tempar.json";

		std::fstream file(file_path, std::ios_base::in);
		if (file.good())
		{
			std::string config;
			std::string temp;
			while (std::getline(file, temp))
			{
				config += temp + "\n";
			}

			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING jsoncpp_string;

			if (reader->parse(config.c_str(), config.c_str() + static_cast<int>(config.length()), &root, &jsoncpp_string))
			{
				if (auto t = root[acc_id]; !t.empty())
				{
					std::string old_jwt;
					if (auto t2 = root[acc_id]["time"]; !t2.empty())
						old_timestamp = t2.asUInt();
					if (auto t2 = root[acc_id]["jwt"]; !t2.empty())
						old_jwt = t2.asString();
					file.close();
					return old_jwt;
				}
			}
		}
		file.close();
		return {};
	}

	static bool check_jwt(const std::string& acc_id)
	{
		char* p_roaming;
		size_t roaming_len;
		_dupenv_s(&p_roaming, &roaming_len, "APPDATA");
		std::string roaming = p_roaming;
		std::string file_path = roaming + "\\tempar.json";
		unsigned timestamp = 0;

		std::fstream file(file_path, std::ios_base::in);
		if (file.good())
		{
			std::string config;
			std::string temp;
			while (std::getline(file, temp))
			{
				config += temp + "\n";
			}

			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;

			if (reader->parse(config.c_str(), config.c_str() + static_cast<int>(config.length()), &root, &err))
			{
				if (auto t = root[acc_id]; !t.empty())
				{
					std::string old_jwt; // CppEntityAssignedButNoRead
					if (auto t2 = root[acc_id]["time"]; !t2.empty())
						timestamp = t2.asUInt();
					if (auto t2 = root[acc_id]["jwt"]; !t2.empty())
						old_jwt = t2.asString();
				}
				else
					return true;
			}
		}
		file.close();

		if (timestamp + 60 * 60 * 24 > time(nullptr))
		{
			return false;
		}
		return true;
	}

#pragma warning ( push )
#pragma warning (disable : 4996)
	static void save_jwt(const std::string& acc_id, const std::string& jwt, unsigned timestamp)
	{
		char* p_roaming;
		size_t roaming_len;
		_dupenv_s(&p_roaming, &roaming_len, "APPDATA");
		std::string roaming = p_roaming;
		std::string file_path = roaming + "\\tempar.json";
		// if file doesn't exist, create new one with {} so it can be parsed
		if (!std::filesystem::exists(file_path))
		{
			std::ofstream file(file_path);
			file << "{}";
			file.close();
		}

		std::ifstream i_file(file_path);
		if (i_file.good())
		{
			Json::Value root;
			if (Json::Reader reader; reader.parse(i_file, root, false))
			{
				root[acc_id]["jwt"] = jwt;
				root[acc_id]["time"] = timestamp;

				if (!root.toStyledString().empty())
				{
					std::ofstream o_file(file_path);
					o_file << root.toStyledString() << std::endl;
					o_file.close();
				}
			}
		}
		i_file.close();
	}
#pragma warning( pop )
};
