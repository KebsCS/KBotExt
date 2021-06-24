#pragma once

#include <utility>

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Auth.h"
#include "Utils.h"
#include "Misc.h"

inline bool bAutoAccept = false;
inline int instalockID;
inline bool bInstalock = false;
inline char instantMessage[50];

class GameTab
{
public:

	static void Render()
	{
		if (ImGui::BeginTabItem("Game"))
		{
			static std::string result;
			static std::string custom;
			static int gameID = 0;

			ImGui::Text("Games:");
			ImGui::Columns(4, 0, false);

			if (ImGui::Button("Blind pick"))
				gameID = BlindPick;

			if (ImGui::Button("Draft pick"))
				gameID = DraftPick;

			if (ImGui::Button("Solo/Duo"))
				gameID = SoloDuo;

			if (ImGui::Button("Flex"))
				gameID = Flex;

			ImGui::NextColumn();

			if (ImGui::Button("ARAM"))
				gameID = ARAM;

			if (ImGui::Button("ARURF"))
				gameID = ARURF;

			if (ImGui::Button("Nexus Blitz"))
				gameID = NexusBlitz;

			/*if (ImGui::Button("URF"))
				gameID = 318;*/

			ImGui::NextColumn();

			if (ImGui::Button("TFT Normal"))
				gameID = TFTNormal;

			if (ImGui::Button("TFT Ranked"))
				gameID = TFTRanked;

			if (ImGui::Button("TFT Hyper Roll"))
				gameID = TFTHyperRoll;

			if (ImGui::Button("TFT Tutorial"))
				gameID = TFTTutorial;

			ImGui::NextColumn();

			if (ImGui::Button("Practice Tool"))
			{
				custom = R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":1},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
			}

			if (ImGui::Button("Practice Tool 5v5"))
			{
				custom = R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
			}

			if (ImGui::Button("Clash"))
				gameID = Clash;

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(4, 0, false);

			if (ImGui::Button("Tutorial 1"))
				gameID = Tutorial1;

			if (ImGui::Button("Tutorial 2"))
				gameID = Tutorial2;

			if (ImGui::Button("Tutorial 3"))
				gameID = Tutorial3;

			ImGui::NextColumn();

			if (ImGui::Button("Intro Bots"))
				gameID = IntroBots;

			if (ImGui::Button("Beginner Bots"))
				gameID = BeginnerBots;

			if (ImGui::Button("Intermediate Bots"))
				gameID = IntermediateBots;

			ImGui::NextColumn();

			if (ImGui::Button("Custom Blind"))
				custom = R"({"customGameLobby":{"configuration":{"gameMode":"CLASSIC","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";

			if (ImGui::Button("Custom ARAM"))
				custom = R"({"customGameLobby":{"configuration":{"gameMode":"ARAM","gameMutator":"","gameServerRegion":"","mapId":12,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";

			//"id" 1- blind 2- draft -4 all random 6- tournament draft

			ImGui::NextColumn();

			static std::vector<std::pair<int, std::string>>botChamps;
			static int indexBots = 0; // Here we store our selection data as an index.
			const char* labelBots = "Bot";
			if (!botChamps.empty())
				labelBots = botChamps[indexBots].second.c_str();
			if (ImGui::BeginCombo("##comboBots", labelBots, 0))
			{
				if (botChamps.empty())
				{
					std::string getBots = http->Request("GET", "https://127.0.0.1/lol-lobby/v2/lobby/custom/available-bots", "", auth->leagueHeader, "", "", auth->leaguePort);
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (reader->parse(getBots.c_str(), getBots.c_str() + static_cast<int>(getBots.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
							{
								std::pair<int, std::string>temp = { root[i]["id"].asInt(),root[i]["name"].asString() };
								botChamps.emplace_back(temp);
							}
							std::sort(botChamps.begin(), botChamps.end(), [](std::pair<int, std::string> a, std::pair<int, std::string >b) {return a.second < b.second; });
						}
					}
				}

				for (int n = 0; n < botChamps.size(); n++)
				{
					const bool is_selected = (indexBots == n);
					if (ImGui::Selectable(botChamps[n].second.c_str(), is_selected))
						indexBots = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			std::vector<std::string>difficulties = { "NONE","EASY","MEDIUM","HARD","UBER","TUTORIAL","INTRO" };
			static int indexDifficulty = 0; // Here we store our selection data as an index.
			const char* labelDifficulty = difficulties[indexDifficulty].c_str();

			if (ImGui::BeginCombo("##comboDifficulty", labelDifficulty, 0))
			{
				for (int n = 0; n < difficulties.size(); n++)
				{
					const bool is_selected = (indexDifficulty == n);
					if (ImGui::Selectable(difficulties[n].c_str(), is_selected))
						indexDifficulty = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			static int botTeam = 0;

			if (ImGui::Button("Add bot##addBot"))
			{
				std::string team = botTeam ? R"(,"teamId":"200"})" : R"(,"teamId":"100"})";
				std::string body = R"({"botDifficulty":")" + difficulties[indexDifficulty] + R"(","championId":)" + std::to_string(botChamps[indexBots].first) + team;
				result = http->Request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", body, auth->leagueHeader, "", "", auth->leaguePort);
			}
			ImGui::SameLine();
			ImGui::RadioButton("Blue", &botTeam, 0); ImGui::SameLine();
			ImGui::RadioButton("Red", &botTeam, 1);

			ImGui::Columns(1);

			//ImGui::Separator();
			//static int inputGameID = 0;
			//ImGui::InputInt("##inputGameID:", &inputGameID, 1, 100);
			//ImGui::SameLine();
			//if (ImGui::Button("Submit##gameID"))
			//{
			//	gameID = inputGameID;
			//}

			// if pressed any button, gameID or custom changed
			if (gameID != 0 || !custom.empty())
			{
				std::string body;
				if (custom.empty())
				{
					body = R"({"queueId":)" + std::to_string(gameID) + "}";
				}
				else
				{
					body = custom;
					custom = "";
				}
				result = http->Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", body, auth->leagueHeader, "", "", auth->leaguePort);

				/*	{
						for (int i = 0; i < 10001; i++)
						{
							std::string res = R"({"customGameLobby":{"configuration":{"gameMode":"CLASSIC","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":)" + std::to_string(i) + R"(},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
							std::string xdd= http.Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", (res), auth->leagueHeader, "", "", clientPort);
							if (xdd.find("errorCode") == std::string::npos)
								std::cout << i << std::endl;
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
						}
					}*/

				gameID = 0;
			}

			ImGui::Separator();

			ImGui::Columns(3, 0, false);
			if (ImGui::Button("Start queue"))
			{
				result = http->Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search", "", auth->leagueHeader, "", "", auth->leaguePort);
			}
			ImGui::NextColumn();
			if (ImGui::Button("Dodge"))
			{
				result = http->Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])", "", auth->leagueHeader, "", "", auth->leaguePort);
			}
			ImGui::SameLine();
			Misc::HelpMarker("Dodges lobby instantly, you still lose LP, but you don't have to restart the client");
			ImGui::NextColumn();
			if (ImGui::Button("Multi OP.GG"))
			{
				std::string names;
				std::string champSelect = http->Request("GET", "https://127.0.0.1/lol-champ-select/v1/session", "", auth->leagueHeader, "", "", auth->leaguePort);
				if (!champSelect.empty() && champSelect.find("RPC_ERROR") == std::string::npos)
				{
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value rootLocale;
					Json::Value rootCSelect;
					Json::Value rootSummoner;

					std::string regionLocale = http->Request("GET", "https://127.0.0.1/riotclient/get_region_locale", "", auth->leagueHeader, "", "", auth->leaguePort);
					if (reader->parse(regionLocale.c_str(), regionLocale.c_str() + static_cast<int>(regionLocale.length()), &rootLocale, &err))
					{
						std::wstring region = utils->StringToWstring(rootLocale["webRegion"].asString());
						if (reader->parse(champSelect.c_str(), champSelect.c_str() + static_cast<int>(champSelect.length()), &rootCSelect, &err))
						{
							std::wstring url = L"https://" + region + L".op.gg/multi/query=";
							auto teamArr = rootCSelect["myTeam"];
							if (teamArr.isArray())
							{
								for (Json::Value::ArrayIndex i = 0; i < teamArr.size(); ++i)
								{
									std::string summId = teamArr[i]["summonerId"].asString();
									if (summId != "0")
									{
										std::string summoner = http->Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + summId, "", auth->leagueHeader, "", "", auth->leaguePort);
										if (reader->parse(summoner.c_str(), summoner.c_str() + static_cast<int>(summoner.length()), &rootSummoner, &err))
										{
											std::wstring summName = utils->StringToWstring(rootSummoner["internalName"].asString());
											url += summName + L",";
										}
									}
								}
								ShellExecuteW(0, 0, url.c_str(), 0, 0, SW_SHOW);
								result = utils->WstringToString(url);
							}
						}
					}
				}
				else
					result = "Champion select not found";
			}

			ImGui::Columns(1);

			// TODO

			ImGui::Checkbox("Auto accept", &bAutoAccept);

			ImGui::Text("Instant message:");
			ImGui::InputText("##inputInstantMessage", instantMessage, IM_ARRAYSIZE(instantMessage));
			ImGui::Checkbox("Instalock", &bInstalock);
			if (ImGui::CollapsingHeader("Instalock champ"))
			{
				std::vector<std::pair<int, std::string>>instalockChamps = GetInstalockChamps();
				for (auto champ : instalockChamps)
				{
					char bufchamp[128];
					sprintf_s(bufchamp, "##Select %s", champ.second.c_str());
					ImGui::Text("%s", champ.second.c_str());
					ImGui::SameLine();
					ImGui::RadioButton(bufchamp, &instalockID, champ.first);
				}
			}

			//if (ImGui::CollapsingHeader("Auto ban champ"))
			//{
			//	//todo list of champions from communitydragon
			//	//or /lol-champ-select/v1/bannable-champion-ids
			//	for (auto champ : champSkins)
			//	{
			//		char bufchamp[128];
			//		sprintf_s(bufchamp, "##Select %s", champ.name.c_str());
			//		ImGui::Text("%s", champ.name.c_str());
			//		//ImGui::RadioButton(bufchamp, &instalockID, chamnp);
			//
			// Add next to champ name, that goes into moveable list, and it bans from top to bottom if other champs are banned.
			//	}
			//}
			//Ban champion
			//PATCH https://127.0.0.1/lol-champ-select/v1/session/actions/8
			//{"completed":true,"championId":131}

			ImGui::Separator();

			if (ImGui::Button("Boost"))
			{
				std::string wallet = http->Request("GET", "https://127.0.0.1/lol-inventory/v1/wallet/RP", "", auth->leagueHeader, "", "", auth->leaguePort);

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (!reader->parse(wallet.c_str(), wallet.c_str() + static_cast<int>(wallet.length()), &root, &err))
					result = wallet;
				else
				{
					unsigned RP = root["RP"].asUInt();
					if (RP < 95)
					{
						result = http->Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","activateBattleBoostV1",""])", "", auth->leagueHeader, "", "", auth->leaguePort);
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text("ARAM/ARURF Boost, use only if you don't have enough RP for boost");

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
				ImGui::InputTextMultiline("##gameResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 300));
			}

			ImGui::EndTabItem();
		}
	}

	static std::vector<std::pair<int, std::string>> GetInstalockChamps()
	{
		std::vector<std::pair<int, std::string>>temp;

		std::string result = http->Request("GET", "https://127.0.0.1/lol-champions/v1/owned-champions-minimal", "", auth->leagueHeader, "", "", auth->leaguePort);
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			if (root.isArray())
			{
				for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
				{
					if (root[i]["freeToPlay"].asBool() == true || root[i]["ownership"]["owned"].asBool() == true)
					{
						std::pair<int, std::string > champ = { root[i]["id"].asInt() , root[i]["alias"].asString() };
						temp.emplace_back(champ);
					}
				}
			}
		}
		std::sort(temp.begin(), temp.end(), [](std::pair<int, std::string > a, std::pair<int, std::string >b) {return a.second < b.second; });
		return temp;
	}
};