#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Auth.h"

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

			static bool fill = false;
			if (ImGui::Button("Practice Tool 5v5"))
			{
				custom = R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
				fill = true;
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

			ImGui::Columns(1);

			ImGui::Separator();

			static int inputGameID = 0;
			ImGui::InputInt("##inputGameID:", &inputGameID, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##gameID"))
			{
				gameID = inputGameID;
			}

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
			// fill practice tool with bots
			if (fill)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
				std::vector<int>champIDs = { 22, 18, 33,12,10,21,62,89,44,51,96,54,81,98,30,122,11,13,69 };
				for (int i = 0; i < 4; i++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					std::string addBlue = R"({"botDifficulty":"MEDIUM","championId":)" + std::to_string(RandomInt(0, champIDs.size() - 1)) + R"(,"teamId":"100"})";
					result = http->Request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", addBlue, auth->leagueHeader, "", "", auth->leaguePort);
				}
				for (int i = 0; i < 5; i++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					std::string addRed = R"({"botDifficulty":"MEDIUM","championId":)" + std::to_string(RandomInt(0, champIDs.size() - 1)) + R"(,"teamId":"200"})";
					result = http->Request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", addRed, auth->leagueHeader, "", "", auth->leaguePort);
				}
				fill = false;
			}

			ImGui::Separator();

			ImGui::Columns(2, 0, false);
			if (ImGui::Button("Start queue"))
			{
				result = http->Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search", "", auth->leagueHeader, "", "", auth->leaguePort);
			}
			ImGui::SameLine();
			if (ImGui::Button("Dodge"))
			{
				result = http->Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])", "", auth->leagueHeader, "", "", auth->leaguePort);
			}
			ImGui::NextColumn();

			ImGui::Columns(1);

			// TODO

			ImGui::Checkbox("Auto accept", &bAutoAccept);

			ImGui::Text("Instant message:");
			ImGui::InputText("##inputInstantMessage", instantMessage, IM_ARRAYSIZE(instantMessage));
			ImGui::Checkbox("Instalock", &bInstalock);
			if (ImGui::CollapsingHeader("Instalock champ"))
			{
				for (auto min : champsMinimal)
				{
					if (!min.owned)
						continue;

					char bufchamp[128];
					sprintf_s(bufchamp, "##Select %s", min.alias.c_str());
					ImGui::Text("%s", min.alias.c_str());
					ImGui::SameLine();
					ImGui::RadioButton(bufchamp, &instalockID, min.id);
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
};