#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"

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
			static int iLastSort = -1;
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
				std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &root, &err))
				{
					std::string summId = root["summonerId"].asString();

					std::string getChampions = LCU::Request("GET",
					                                        std::format("https://127.0.0.1/lol-champions/v1/inventories/{}/champions-minimal",
					                                                    summId));

					if (reader->parse(getChampions.c_str(), getChampions.c_str() + static_cast<int>(getChampions.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto& champObj : root)
							{
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
								auto& ownershipObj = champObj["ownership"];
								champ.freeToPlayReward = ownershipObj["freeToPlayReward"].asBool();
								champ.owned = ownershipObj["owned"].asInt();
								if (champ.owned)
									iChampsOwned++;
								if (champ.freeToPlay && !champ.owned)
									champ.purchased = "0";
								else
									champ.purchased = champObj["purchased"].asString();
								champ.rankedPlayEnabled = champObj["rankedPlayEnabled"].asBool();
								//auto rolesObj = champObj.GetObject("roles"); // TODO
								champ.squarePortraitPath = champObj["squarePortraitPath"].asString();
								champ.stingerSfxPath = champObj["stingerSfxPath"].asString();
								champ.title = champObj["title"].asString();

								champsMinimal.emplace_back(champ);
							}
						}
					}

					std::string getCollections = LCU::Request("GET",
					                                          std::format("https://127.0.0.1/lol-collections/v1/inventories/{}/champion-mastery",
					                                                      summId));

					if (reader->parse(getCollections.c_str(), getCollections.c_str() + static_cast<int>(getCollections.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto& champObj : root)
							{
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
								champ.lastPlayTime = std::to_string(champObj["lastPlayTime"].asUInt64());
								champ.playerId = std::to_string(champObj["playerId"].asInt64());
								champ.tokensEarned = champObj["tokensEarned"].asInt();

								champsMastery.emplace_back(champ);
							}
						}
					}
				}

				champsAll.clear();

				for (const auto& minimal : champsMinimal)
				{
					if (!minimal.owned)
						continue;
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
					std::ranges::sort(champsAll, [](const ChampAll& lhs, const ChampAll& rhs) {
						return lhs.min.name < rhs.min.name;
					});
					break;
				// purchase date
				case 1:
					std::ranges::sort(champsAll, [](const ChampAll& lhs, const ChampAll& rhs) {
						return std::stoll(lhs.min.purchased) < std::stoll(rhs.min.purchased);
					});
					break;
				// mastery points
				case 2:
					std::ranges::sort(champsAll, [](const ChampAll& lhs, const ChampAll& rhs) {
						return lhs.mas.championPoints > rhs.mas.championPoints;
					});
					break;
				// id
				case 3:
					std::ranges::sort(champsAll, [](const ChampAll& lhs, const ChampAll& rhs) {
						return lhs.min.id < rhs.min.id;
					});
					break;
				default: ;
				}
			}

			ImGui::SameLine();
			static char allNamesSeparator[64] = ",";
			if (ImGui::Button("Copy names to clipboard##champsTab"))
			{
				std::string allNames;
				for (const auto& [min, mas] : champsAll)
				{
					if (!min.owned)
						continue;
					allNames += min.name + allNamesSeparator;
				}
				Utils::CopyToClipboard(allNames);
			}
			ImGui::SameLine();

			const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);
			ImGui::InputTextMultiline("##separatorChampsTab", allNamesSeparator, IM_ARRAYSIZE(allNamesSeparator),
			                          ImVec2(0, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f), ImGuiInputTextFlags_AllowTabInput);

			ImGui::Separator();
			ImGui::Text("Champions owned: %d", iChampsOwned);
			for (const auto& [min, mas] : champsAll)
			{
				if (!min.owned)
					continue;

				int64_t t = std::stoll(min.purchased);
				t /= 1000;
				char buffer[50];
				strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));

				std::string inputId = "champInput";
				inputId.append(std::to_string(min.id));
				char input[768];

				float textHeight = (label_size.y + ImGui::GetStyle().FramePadding.y) * 3.f;
				std::string text = std::format(R"(name: {}
purchased: {}
id: {})", min.name, buffer, min.id);

				if (!mas.lastPlayTime.empty())
				{
					textHeight = (label_size.y + ImGui::GetStyle().FramePadding.y) * 12.f;

					t = std::stoll(mas.lastPlayTime);
					t /= 1000;
					strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));

					text += std::format(R"(
championLevel: {}
championPoints: {}
championPointsSinceLastLevel: {}
championPointsUntilNextLevel: {}
chestGranted: {}
formattedChampionPoints: {}
formattedMasteryGoal: {}
highestGrade: {}
lastPlayTime: {}
playerId: {}
tokensEarned: {})", mas.championLevel, mas.championPoints, mas.championPointsSinceLastLevel,
					                    mas.championPointsUntilNextLevel, mas.chestGranted, mas.formattedChampionPoints, mas.formattedMasteryGoal,
					                    mas.highestGrade, buffer, mas.playerId, mas.tokensEarned);
				}

				strcpy(input, text.c_str());
				ImGui::PushID(inputId.c_str());
				ImGui::InputTextMultiline("", input, IM_ARRAYSIZE(input), ImVec2(ImGui::GetWindowSize().x, textHeight),
				                          ImGuiInputTextFlags_ReadOnly);
				ImGui::PopID();
			}

			ImGui::EndTabItem();
		}
		else
			bOnOpen = true;
	}
};
