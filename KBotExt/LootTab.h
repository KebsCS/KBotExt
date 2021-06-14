#pragma once

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

			if (ImGui::Button("Craft Key"))
				result = http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/MATERIAL_key_fragment_forge/craft?repeat=1", "[\"MATERIAL_key_fragment\"]", auth->leagueHeader, "", "", auth->leaguePort);

			if (ImGui::Button("Open Chest"))
				result = http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/CHEST_generic_OPEN/craft?repeat=1", R"(["CHEST_generic","MATERIAL_key"])", auth->leagueHeader, "", "", auth->leaguePort);

			if (ImGui::Button("Open Mastery Chest"))
				result = http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/CHEST_champion_mastery_OPEN/craft?repeat=1", R"(["CHEST_champion_mastery","MATERIAL_key"])", auth->leagueHeader, "", "", auth->leaguePort);

			if (ImGui::Button("Disenchant all champion shards"))
			{
				if (lootFound)
				{
					if (MessageBoxA(0, "Are you sure?", 0, MB_OKCANCEL) == IDOK)
					{
						int i = 0;

						for (std::string name : root.getMemberNames())
						{
							if (name.find("CHAMPION_RENTAL") != std::string::npos)
							{
								std::string body = "[\"" + name + "\"]";
								http->Request("POST", "https://127.0.0.1/lol-loot/v1/recipes/CHAMPION_RENTAL_disenchant/craft?repeat=1", body, auth->leagueHeader, "", "", auth->leaguePort);
								i++;
							}
						}
						result = "Disenchanted " + std::to_string(i) + " champion shards";
					}
				}
				else
					result = "Loot not found";
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