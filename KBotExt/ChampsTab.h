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
		static bool champsOpen = true;
		if (ImGui::BeginTabItem("Champs"))
		{
			static std::string req;
			static std::string req2;
			static bool added = false;
			static int iChampsOwned = 0;

			static Json::Value root;
			static Json::CharReaderBuilder builder;
			static const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			static JSONCPP_STRING err;

			if (champsOpen)
			{
				std::string session = http->Request("GET", "https://127.0.0.1/lol-login/v1/session", "", auth->leagueHeader, "", "", auth->leaguePort);

				if (reader->parse(session.c_str(), session.c_str() + static_cast<int>(session.length()), &root, &err))
				{
					int summonerId = root["summonerId"].asInt();

					req = http->Request("GET", "https://127.0.0.1/lol-collections/v1/inventories/" + std::to_string(summonerId) + "/champion-mastery", "", auth->leagueHeader, "", "", auth->leaguePort);

					req2 = http->Request("GET", "https://127.0.0.1/lol-champions/v1/inventories/" + std::to_string(summonerId) + "/champions-minimal", "", auth->leagueHeader, "", "", auth->leaguePort);

					added = false;
					iChampsOwned = 0;
					champsMinimal.clear();
					champsMastery.clear();
				}
			}
			if (req.find("errorCode") != std::string::npos || req2.find("errorCode") != std::string::npos || req == "[]")
			{
				ImGui::TextWrapped("%s", req.c_str());

				ImGui::TextWrapped("%s", req2.c_str());
			}
			else
			{
				if (!added)
				{
					if (reader->parse(req2.c_str(), req2.c_str() + static_cast<int>(req2.length()), &root, &err))
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
							champ.purchased = std::to_string(champObj["purchased"].asInt64()).c_str();
							champ.rankedPlayEnabled = champObj["rankedPlayEnabled"].asBool();
							//auto rolesObj = champObj.GetObject("roles"); //todo
							champ.squarePortraitPath = champObj["squarePortraitPath"].asString();
							champ.stingerSfxPath = champObj["stingerSfxPath"].asString();
							champ.title = champObj["title"].asString();

							champsMinimal.emplace_back(champ);
						}

						if (reader->parse(req.c_str(), req.c_str() + static_cast<int>(req.length()), &root, &err))
						{
							for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i)
							{
								auto champObj = root[i];

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
								champ.lastPlayTime = std::to_string(champObj["lastPlayTime"].asInt64()).c_str();
								champ.playerId = std::to_string(champObj["playerId"].asInt64()).c_str();
								champ.tokensEarned = champObj["tokensEarned"].asInt();

								champsMastery.emplace_back(champ);
							}
							//sort alphabetcally
							std::sort(champsMinimal.begin(), champsMinimal.end(), [](const ChampMinimal& lhs, const ChampMinimal& rhs) {
								return lhs.name < rhs.name;
								});
							added = true;
						}
					}
				}

				ImGui::Text("Champions owned: %d", iChampsOwned);
				for (auto min : champsMinimal)
				{
					if (!min.owned)
						continue;

					ImGui::Separator();
					ImGui::Text("name: %s", min.name.c_str());
					int64_t t = std::stoll(min.purchased);
					t /= 1000;
					char buffer[50];
					strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));
					ImGui::Text("purchased: %s", buffer);
					ImGui::Text("id: %d", min.id);
					for (auto man : champsMastery)
					{
						if (min.id == man.championId)
						{
							ImGui::Text("championLevel: %d", man.championLevel);
							ImGui::Text("championPoints: %d", man.championPoints);
							ImGui::Text("championPointsSinceLastLevel: %d", man.championPointsSinceLastLevel);
							ImGui::Text("championPointsUntilNextLevel: %d", man.championPointsUntilNextLevel);
							ImGui::Text("chestGranted: %d", man.chestGranted);
							ImGui::Text("formattedChampionPoints: %s", man.formattedChampionPoints.c_str());
							ImGui::Text("formattedMasteryGoal: %s", man.formattedMasteryGoal.c_str());
							ImGui::Text("highestGrade: %s", man.highestGrade.c_str());
							int64_t t = std::stoll(man.lastPlayTime);
							t /= 1000;
							char buffer[50];
							strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));
							ImGui::Text("lastPlayTime: %s", buffer);
							ImGui::Text("playerId: %s", man.playerId.c_str());
							ImGui::Text("tokensEarned: %d", man.tokensEarned);

							break;
						}
					}
				}
			}

			//ImGui::TextWrapped(req.c_str());
			champsOpen = false;
			ImGui::EndTabItem();
		}
		else
			champsOpen = true;
	}
};