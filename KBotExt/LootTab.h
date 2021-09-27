#pragma once

#include <regex>

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Auth.h"

class LootTab
{
public:
	static void Render()
	{
		static bool lootOpen = true;
		static bool lootFound = false;

		if (ImGui::BeginTabItem("Loot"))
		{
			static std::string result;
			static Json::Value root;

			if (lootOpen)
			{
				lootOpen = false;
				std::string getLoot = http->Request("GET", "https://127.0.0.1/lol-loot/v1/player-loot-map", "", auth->leagueHeader, "", "", auth->leaguePort);

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;

				if (!reader->parse(getLoot.c_str(), getLoot.c_str() + static_cast<int>(getLoot.length()), &root, &err))
					result = getLoot;
				else
				{
					lootFound = true;
				}
			}

			//GET /lol-loot/v1/player-display-categories
			//["CHEST","CHAMPION","SKIN","COMPANION","ETERNALS","EMOTE","WARDSKIN","SUMMONERICON"]

			//craft keys
			//lol-loot/v1/recipes/MATERIAL_key_fragment_forge/craft?repeat=1
			//["MATERIAL_key_fragment"]

			//disenchant skins
			//lol-loot/v1/recipes/SKIN_RENTAL_disenchant/craft?repeat=1
			//["CHAMPION_SKIN_RENTAL_36002"]

			//disenchant eternals
			//lol-loot/v1/recipes/STATSTONE_SHARD_DISENCHANT/craft?repeat=1
			//["STATSTONE_SHARD_66600058"]

			//disenchant wards
			//lol-loot/v1/recipes/WARDSKIN_RENTAL_disenchant/craft?repeat=1
			//["WARD_SKIN_RENTAL_199"]

			//disenchant champ shards
			//lol-loot/v1/recipes/CHAMPION_RENTAL_disenchant/craft?repeat=1
			//["CHAMPION_RENTAL_22"]

			// todo remove this and merge with misc tab
			// league implemented crafting multiple at the same time

			if (ImGui::Button("Craft Key"))
				result = http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/MATERIAL_key_fragment_forge/craft?repeat=1", "[\"MATERIAL_key_fragment\"]", auth->leagueHeader, "", "", auth->leaguePort);

			if (ImGui::Button("Open Chest"))
				result = http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/CHEST_generic_OPEN/craft?repeat=1", R"(["CHEST_generic","MATERIAL_key"])", auth->leagueHeader, "", "", auth->leaguePort);

			if (ImGui::Button("Open Mastery Chest"))
				result = http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/CHEST_champion_mastery_OPEN/craft?repeat=1", R"(["CHEST_champion_mastery","MATERIAL_key"])", auth->leagueHeader, "", "", auth->leaguePort);

			static std::vector<std::pair<std::string, std::string>>items = {
				{"Champion shards","CHAMPION_RENTAL"}, {"Champion pernaments","CHAMPION"},
				{"Skin shards","CHAMPION_SKIN_RENTAL"}, {"Skin pernaments", "CHAMPION_SKIN"},
				{"Eternals","STATSTONE_SHARD"},{"Wards","WARDSKIN_RENTAL"}
			};
			static size_t item_current_idx = 0;
			const char* combo_label = items[item_current_idx].first.c_str();

			if (ImGui::Button("Disenchant all: "))
			{
				if (lootFound)
				{
					if (MessageBoxA(0, "Are you sure?", 0, MB_OKCANCEL) == IDOK)
					{
						int i = 0;

						for (std::string name : root.getMemberNames())
						{
							std::regex regexStr("^" + items[item_current_idx].second + "_[\\d]+");

							if (std::regex_match(name, regexStr))
							{
								std::string disenchantCase = items[item_current_idx].second == "STATSTONE_SHARD" ? "DISENCHANT" : "disenchant";

								std::string disenchantName = items[item_current_idx].second;
								if (items[item_current_idx].second == "CHAMPION_SKIN_RENTAL")
									disenchantName = "SKIN_RENTAL";

								std::string disenchantUrl = std::format("https://127.0.0.1/lol-loot/v1/recipes/{0}_{1}/craft?repeat=1", disenchantName, disenchantCase);
								std::string disenchantBody = std::format(R"(["{}"])", name).c_str();
								http->Request("POST", disenchantUrl, disenchantBody, auth->leagueHeader, "", "", auth->leaguePort);
								i++;
							}
						}
						result = std::format("Disenchanted {0} {1}", std::to_string(i), items[item_current_idx].first);
					}
				}
				else
					result = "Loot not found";
			}

			ImGui::SameLine();

			if (ImGui::BeginCombo("##comboDisenchant", combo_label, 0))
			{
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

			static Json::StreamWriterBuilder wBuilder;
			static std::string sResultJson;
			static char* cResultJson;

			if (!result.empty())
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root2;
				if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root2, &err))
					sResultJson = result;
				else
				{
					sResultJson = Json::writeString(wBuilder, root2);
				}
				result = "";
			}

			if (!sResultJson.empty())
			{
				cResultJson = &sResultJson[0];
				ImGui::InputTextMultiline("##lootResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 200));
			}

			ImGui::EndTabItem();
		}
		else
		{
			lootOpen = true;
			lootFound = false;
		}
	}
};