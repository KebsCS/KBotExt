#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Auth.h"

#pragma warning (disable : 4996)

class ChampsTab
{
public:
	static void Render()
	{
		static bool bOnOpen = true;
		static unsigned iChampsOwned = 0;
		static bool bSortOnOpen = false;

		if (ImGui::BeginTabItem("Champs"))
		{
			ImGui::Text("Sort by: ");
			ImGui::SameLine();

			static int iSort = 0;
			static short iLastSort = -1;
			ImGui::RadioButton("Alphabetically", &iSort, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Purchase date", &iSort, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Mastery points", &iSort, 2);
			ImGui::SameLine();
			ImGui::RadioButton("ID", &iSort, 3);

			if (bOnOpen)
			{
				bOnOpen = false;
				bSortOnOpen = true;
				iChampsOwned = 0;
				champsMinimal.clear();
				champsMastery.clear();

				static Json::Value root;
				static Json::CharReaderBuilder builder;
				static const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				static JSONCPP_STRING err;
				std::string getSession = http->Request("GET", "https://127.0.0.1/lol-login/v1/session", "", auth->leagueHeader, "", "", auth->leaguePort);
				if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &root, &err))
				{
					std::string summId = root["summonerId"].asString();

					std::string getChampions = http->Request("GET",
						std::format("https://127.0.0.1/lol-champions/v1/inventories/{}/champions-minimal", summId), "", auth->leagueHeader, "", "", auth->leaguePort);

					if (reader->parse(getChampions.c_str(), getChampions.c_str() + static_cast<int>(getChampions.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i)
							{
								auto champObj = root[i];
								ChampMinimal champ;

								champ.active = champObj["active"].asBool();
								champ.alias = champObj["alias"].asString();
								champ.banVoPath = champObj["banVoPath"].asString();
								champ.baseLoadScreenPath = champObj["baseLoadScreenPath"].asString();
								champ.botEnabled = champObj["botEnabled"].asBool();
								champ.chooseVoPath = champObj["chooseVoPath"].asString();
								champ.freeToPlay = champObj["freeToPlay"].asBool();
								champ.id = champObj["id"].asInt();
								champ.name = champObj["name"].asString();
								auto ownershipObj = champObj["ownership"];
								champ.freeToPlayReward = ownershipObj["freeToPlayReward"].asBool();
								champ.owned = ownershipObj["owned"].asInt();
								if (champ.owned)
									iChampsOwned++;
								if (champ.freeToPlay && !champ.owned)
									champ.purchased = "0";
								else
									champ.purchased = champObj["purchased"].asString();
								champ.rankedPlayEnabled = champObj["rankedPlayEnabled"].asBool();
								//auto rolesObj = champObj.GetObject("roles"); //todo
								champ.squarePortraitPath = champObj["squarePortraitPath"].asString();
								champ.stingerSfxPath = champObj["stingerSfxPath"].asString();
								champ.title = champObj["title"].asString();

								champsMinimal.emplace_back(champ);
							}
						}
					}

					std::string getCollections = http->Request("GET",
						std::format("https://127.0.0.1/lol-collections/v1/inventories/{}/champion-mastery", summId), "", auth->leagueHeader, "", "", auth->leaguePort);

					if (reader->parse(getCollections.c_str(), getCollections.c_str() + static_cast<int>(getCollections.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (Json::Value::ArrayIndex j = 0; j < root.size(); ++j)
							{
								auto champObj = root[j];

								ChampMastery champ;
								champ.championId = champObj["championId"].asInt();
								champ.championLevel = champObj["championLevel"].asInt();
								champ.championPoints = champObj["championPoints"].asInt();
								champ.championPointsSinceLastLevel = champObj["championPointsSinceLastLevel"].asInt();
								champ.championPointsUntilNextLevel = champObj["championPointsUntilNextLevel"].asInt();
								champ.chestGranted = champObj["chestGranted"].asInt();
								champ.formattedChampionPoints = champObj["formattedChampionPoints"].asString();
								champ.formattedMasteryGoal = champObj["formattedMasteryGoal"].asString();
								champ.highestGrade = champObj["highestGrade"].asString();
								champ.lastPlayTime = std::to_string(champObj["lastPlayTime"].asUInt64()).c_str();
								champ.playerId = std::to_string(champObj["playerId"].asInt64()).c_str();
								champ.tokensEarned = champObj["tokensEarned"].asInt();

								champsMastery.emplace_back(champ);
							}
						}
					}
				}

				champsAll.clear();

				for (const auto& minimal : champsMinimal)
				{
					ChampAll champ;
					champ.min = minimal;
					for (const auto& mastery : champsMastery)
					{
						if (minimal.id == mastery.championId)
						{
							champ.mas = mastery;
						}
					}
					champsAll.emplace_back(champ);
				}
			}

			if ((iLastSort != iSort) || bSortOnOpen)
			{
				bSortOnOpen = false;
				iLastSort = iSort;
				switch (iSort)
				{
					// alphabetically
				case 0:
					std::sort(champsAll.begin(), champsAll.end(), [](const ChampAll& lhs, const ChampAll& rhs) {
						return lhs.min.name < rhs.min.name;
						});
					break;
					// purchase date
				case 1:
					std::sort(champsAll.begin(), champsAll.end(), [](const ChampAll& lhs, const ChampAll& rhs) {
						return std::stoll(lhs.min.purchased) < std::stoll(rhs.min.purchased);
						});
					break;
					// mastery points
				case 2:
					std::sort(champsAll.begin(), champsAll.end(), [](const ChampAll& lhs, const ChampAll& rhs) {
						return lhs.mas.championPoints > rhs.mas.championPoints;
						});
					break;
					// id
				case 3:
					std::sort(champsAll.begin(), champsAll.end(), [](const ChampAll& lhs, const ChampAll& rhs) {
						return lhs.min.id < rhs.min.id;
						});
					break;
				}
			}

			ImGui::Separator();
			ImGui::Text("Champions owned: %d", iChampsOwned);
			for (const auto& champ : champsAll)
			{
				if (!champ.min.owned)
					continue;

				ImGui::Separator();
				ImGui::Text("name: %s", champ.min.name.c_str());
				int64_t t = std::stoll(champ.min.purchased);
				t /= 1000;
				char buffer[50];
				strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));
				ImGui::Text("purchased: %s", buffer);
				ImGui::Text("id: %d", champ.min.id);

				if (champ.mas.lastPlayTime.empty())
					continue;

				ImGui::Text("championLevel: %d", champ.mas.championLevel);
				ImGui::Text("championPoints: %d", champ.mas.championPoints);
				ImGui::Text("championPointsSinceLastLevel: %d", champ.mas.championPointsSinceLastLevel);
				ImGui::Text("championPointsUntilNextLevel: %d", champ.mas.championPointsUntilNextLevel);
				ImGui::Text("chestGranted: %d", champ.mas.chestGranted);
				ImGui::Text("formattedChampionPoints: %s", champ.mas.formattedChampionPoints.c_str());
				ImGui::Text("formattedMasteryGoal: %s", champ.mas.formattedMasteryGoal.c_str());
				ImGui::Text("highestGrade: %s", champ.mas.highestGrade.c_str());
				t = std::stoll(champ.mas.lastPlayTime);
				t /= 1000;
				strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));
				ImGui::Text("lastPlayTime: %s", buffer);
				ImGui::Text("playerId: %s", champ.mas.playerId.c_str());
				ImGui::Text("tokensEarned: %d", champ.mas.tokensEarned);
			}

			ImGui::EndTabItem();
		}
		else
			bOnOpen = true;
	}
};