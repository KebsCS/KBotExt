#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"

#pragma warning (disable : 4996)

class champs_tab
{
public:
	static void render()
	{
		static bool b_on_open = true;
		static unsigned i_champs_owned = 0;
		static bool b_sort_on_open = false;

		if (ImGui::BeginTabItem("Champs"))
		{
			ImGui::Text("Sort by: ");
			ImGui::SameLine();

			static int i_sort = 0;
			static int i_last_sort = -1;
			ImGui::RadioButton("Alphabetically", &i_sort, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Purchase date", &i_sort, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Mastery points", &i_sort, 2);
			ImGui::SameLine();
			ImGui::RadioButton("ID", &i_sort, 3);

			if (b_on_open)
			{
				b_on_open = false;
				b_sort_on_open = true;
				i_champs_owned = 0;
				champs_minimal.clear();
				champs_mastery.clear();

				static Json::Value root;
				static Json::CharReaderBuilder builder;
				static const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				static JSONCPP_STRING err;
				std::string get_session = lcu::request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(get_session.c_str(), get_session.c_str() + static_cast<int>(get_session.length()), &root,
				                  &err))
				{
					std::string summoner_id = root["summonerId"].asString();

					std::string get_champions = lcu::request("GET",
					                                         std::format(
						                                         "https://127.0.0.1/lol-champions/v1/inventories/{}/champions-minimal",
						                                         summoner_id));

					if (reader->parse(get_champions.c_str(),
					                  get_champions.c_str() + static_cast<int>(get_champions.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto champ_obj : root)
							{
								champ_minimal champ;

								champ.active = champ_obj["active"].asBool();
								champ.alias = champ_obj["alias"].asString();
								champ.ban_vo_path = champ_obj["banVoPath"].asString();
								champ.base_load_screen_path = champ_obj["baseLoadScreenPath"].asString();
								champ.bot_enabled = champ_obj["botEnabled"].asBool();
								champ.choose_vo_path = champ_obj["chooseVoPath"].asString();
								champ.free_to_play = champ_obj["freeToPlay"].asBool();
								champ.id = champ_obj["id"].asInt();
								champ.name = champ_obj["name"].asString();
								auto ownership_obj = champ_obj["ownership"];
								champ.free_to_play_reward = ownership_obj["freeToPlayReward"].asBool();
								champ.owned = ownership_obj["owned"].asInt();
								if (champ.owned)
i_champs_owned++;
								if (champ.free_to_play && !champ.owned)
champ.purchased = "0";
								else
champ.purchased = champ_obj["purchased"].asString();
								champ.ranked_play_enabled = champ_obj["rankedPlayEnabled"].asBool();
								// auto rolesObj = champObj.GetObject("roles"); // TODO
								champ.square_portrait_path = champ_obj["squarePortraitPath"].asString();
								champ.stinger_sfx_path = champ_obj["stingerSfxPath"].asString();
								champ.title = champ_obj["title"].asString();

								champs_minimal.emplace_back(champ);
							}
						}
					}

					std::string get_collections = lcu::request("GET",
					                                           std::format(
						                                           "https://127.0.0.1/lol-collections/v1/inventories/{}/champion-mastery",
						                                           summoner_id));

					if (reader->parse(get_collections.c_str(),
					                  get_collections.c_str() + static_cast<int>(get_collections.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto champ_obj : root)
							{
								champ_mastery champ;
								champ.champion_id = champ_obj["championId"].asInt();
								champ.champion_level = champ_obj["championLevel"].asInt();
								champ.champion_points = champ_obj["championPoints"].asInt();
								champ.champion_points_since_last_level = champ_obj["championPointsSinceLastLevel"].asInt();
								champ.champion_points_until_next_level = champ_obj["championPointsUntilNextLevel"].asInt();
								champ.chest_granted = champ_obj["chestGranted"].asInt();
								champ.formatted_champion_points = champ_obj["formattedChampionPoints"].asString();
								champ.formatted_mastery_goal = champ_obj["formattedMasteryGoal"].asString();
								champ.highest_grade = champ_obj["highestGrade"].asString();
								champ.last_play_time = std::to_string(champ_obj["lastPlayTime"].asUInt64());
								champ.player_id = std::to_string(champ_obj["playerId"].asInt64());
								champ.tokens_earned = champ_obj["tokensEarned"].asInt();

								champs_mastery.emplace_back(champ);
							}
						}
					}
				}

				champs_all.clear();

				for (const auto& minimal : champs_minimal)
				{
					if (!minimal.owned)
						continue;
					champ_all champ;
					champ.min = minimal;
					for (const auto& mastery : champs_mastery)
					{
						if (minimal.id == mastery.champion_id)
						{
							champ.mas = mastery;
						}
					}
					champs_all.emplace_back(champ);
				}
			}

			if ((i_last_sort != i_sort) || b_sort_on_open)
			{
				b_sort_on_open = false;
				i_last_sort = i_sort;
				switch (i_sort)
				{
				case 0:
					std::ranges::sort(champs_all, [](const champ_all& lhs, const champ_all& rhs) {
						return lhs.min.name < rhs.min.name;
					});
					break;
				case 1:
					std::ranges::sort(champs_all, [](const champ_all& lhs, const champ_all& rhs) {
						return std::stoll(lhs.min.purchased) < std::stoll(rhs.min.purchased);
					});
					break;
				case 2:
					std::ranges::sort(champs_all, [](const champ_all& lhs, const champ_all& rhs) {
						return lhs.mas.champion_points > rhs.mas.champion_points;
					});
					break;
				case 3:
					std::ranges::sort(champs_all, [](const champ_all& lhs, const champ_all& rhs) {
						return lhs.min.id < rhs.min.id;
					});
					break;
				default: ;
				}
			}

			ImGui::SameLine();
			static char all_names_separator[64] = ",";
			if (ImGui::Button("Copy names to clipboard##champsTab"))
			{
				std::string all_names;
				for (const auto& [min, mas] : champs_all)
				{
					if (!min.owned)
						continue;
					all_names += min.name + all_names_separator;
				}
				utils::copy_to_clipboard(all_names);
			}
			ImGui::SameLine();

			const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);
			ImGui::InputTextMultiline("##separatorChampsTab", all_names_separator, IM_ARRAYSIZE(all_names_separator),
			                          ImVec2(0, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f),
			                          ImGuiInputTextFlags_AllowTabInput);

			ImGui::Separator();
			ImGui::Text("Champions owned: %d", i_champs_owned);
			for (const auto& [min, mas] : champs_all)
			{
			    if (!min.owned) continue;

			    int64_t purchase_timestamp = std::stoll(min.purchased) / 1000;
			    std::tm purchase_time_tm;
			    localtime_s(&purchase_time_tm, &purchase_timestamp);
			    char purchase_time_str[100];
			    std::strftime(purchase_time_str, sizeof(purchase_time_str), "%Y-%m-%d %H:%M:%S", &purchase_time_tm);
			    std::string general_info = std::format(R"(name: {} purchased: {} id: {})", min.name, purchase_time_str, min.id);

			    if (!mas.last_play_time.empty())
			    {
			        int64_t last_play_timestamp = std::stoll(mas.last_play_time) / 1000;
			        std::tm last_play_time_tm;
			        localtime_s(&last_play_time_tm, &last_play_timestamp);
			        char last_play_time_str[100];
			        std::strftime(last_play_time_str, sizeof(last_play_time_str), "%Y-%m-%d %H:%M:%S", &last_play_time_tm);

			        std::string additional_info = std::format(
R"(championLevel: {}
championPoints: {}
championPointsSinceLastLevel: {}
championPointsUntilNextLevel: {}
chestGranted: {}
formattedChampionPoints: {}
formattedMasteryGoal: {}
highestGrade: {}
lastPlayTime: {}
playerId: {}
tokensEarned: {})",
			            mas.champion_level, mas.champion_points, mas.champion_points_since_last_level, mas.champion_points_until_next_level,
			            mas.chest_granted, mas.formatted_champion_points, mas.formatted_mastery_goal, mas.highest_grade, last_play_time_str,
			            mas.player_id, mas.tokens_earned);

			        general_info += "\n" + additional_info;
			    }

			    std::string input_id = "champInput" + std::to_string(min.id);

			    char input[768];
			    strcpy(input, general_info.c_str());

			    float text_height = (label_size.y + ImGui::GetStyle().FramePadding.y) * (mas.last_play_time.empty() ? 3.f : 12.f);

			    ImGui::PushID(input_id.c_str());
			    ImGui::InputTextMultiline("", input, IM_ARRAYSIZE(input), ImVec2(ImGui::GetWindowSize().x, text_height), ImGuiInputTextFlags_ReadOnly);
			    ImGui::PopID();
			}
			ImGui::EndTabItem();
		}
		else
			b_on_open = true;
	}
};
