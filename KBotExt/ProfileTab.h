#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"

class ProfileTab
{
public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Profile"))
		{
			static char statusText[1024 * 16];
			ImGui::Text("Status:");
			ImGui::InputTextMultiline("##inputStatus", (statusText), IM_ARRAYSIZE(statusText), ImVec2(400, 100), ImGuiInputTextFlags_AllowTabInput);
			if (ImGui::Button("Submit status"))
			{
				std::string body = "{\"statusMessage\":\"" + std::string(statusText) + "\"}";

				size_t nPos = 0;
				while (nPos != std::string::npos)
				{
					nPos = body.find("\n", nPos);
					if (nPos != std::string::npos)
					{
						body.erase(body.begin() + nPos);
						body.insert(nPos, "\\n");
					}
				}
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
				if (result.find("errorCode") != std::string::npos)
					MessageBoxA(0, result.c_str(), 0, 0);
			}

			ImGui::SameLine();
			static int availability = 0;
			static int lastAvailability = 0;
			ImGui::RadioButton("Online", &availability, 0); ImGui::SameLine();
			ImGui::RadioButton("Mobile", &availability, 1); ImGui::SameLine();
			ImGui::RadioButton("Away", &availability, 2); ImGui::SameLine();
			ImGui::RadioButton("Offline", &availability, 3);

			if (availability != lastAvailability)
			{
				lastAvailability = availability;
				std::string body = R"({"availability":")";
				switch (availability)
				{
				case 0:
					body += "online";
					break;
				case 1:
					body += "mobile";
					break;
				case 2:
					body += "away";
					break;
				case 3:
					body += "offline";
					break;
				}
				body += "\"}";
				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
			}

			ImGui::Separator();

			ImGui::Text("Rank:");
			static int rank = 0;
			ImGui::RadioButton("Iron", &rank, 0); ImGui::SameLine();
			ImGui::RadioButton("Bronze", &rank, 1); ImGui::SameLine();
			ImGui::RadioButton("Silver", &rank, 2); ImGui::SameLine();
			ImGui::RadioButton("Gold", &rank, 3); ImGui::SameLine();
			ImGui::RadioButton("Platinum", &rank, 4); ImGui::SameLine();
			ImGui::RadioButton("Diamond", &rank, 5); ImGui::SameLine();
			ImGui::RadioButton("Master", &rank, 6); ImGui::SameLine();
			ImGui::RadioButton("GrandMaster", &rank, 7); ImGui::SameLine();
			ImGui::RadioButton("Challenger", &rank, 8);

			static int tier = 0;
			ImGui::RadioButton("I", &tier, 0); ImGui::SameLine();
			ImGui::RadioButton("II", &tier, 1); ImGui::SameLine();
			ImGui::RadioButton("III", &tier, 2); ImGui::SameLine();
			ImGui::RadioButton("IV", &tier, 3); ImGui::SameLine();
			ImGui::RadioButton("None", &tier, 4);

			static int queue = 0;
			ImGui::RadioButton("Solo/Duo", &queue, 0); ImGui::SameLine();
			ImGui::RadioButton("Flex 5v5", &queue, 1); ImGui::SameLine();
			ImGui::RadioButton("Flex 3v3", &queue, 2); ImGui::SameLine();
			ImGui::RadioButton("TFT", &queue, 3); ImGui::SameLine();
			ImGui::RadioButton("Hyper Roll", &queue, 4); ImGui::SameLine();
			ImGui::RadioButton("Double Up", &queue, 5); ImGui::SameLine();
			ImGui::RadioButton("()", &queue, 6);

			ImGui::Columns(2, 0, false);

			if (ImGui::Button("Submit##submitRank"))
			{
				std::string body = R"({"lol":{"rankedLeagueQueue":")";
				switch (queue)
				{
				case 0:
					body += "RANKED_SOLO_5x5";
					break;
				case 1:
					body += "RANKED_FLEX_SR";
					break;
				case 2:
					body += "RANKED_FLEX_TT";
					break;
				case 3:
					body += "RANKED_TFT";
					break;
				case 4:
					body += "RANKED_TFT_TURBO";
					break;
				case 5:
					body += "RANKED_TFT_DOUBLE_UP";
					break;
				case 6:
					body += "";
					break;
				}

				body += R"(","rankedLeagueTier":")";

				switch (rank)
				{
				case 0:
					body += "IRON";
					break;
				case 1:
					body += "BRONZE";
					break;
				case 2:
					body += "SILVER";
					break;
				case 3:
					body += "GOLD";
					break;
				case 4:
					body += "PLATINUM";
					break;
				case 5:
					body += "DIAMOND";
					break;
				case 6:
					body += "MASTER";
					break;
				case 7:
					body += "GRANDMASTER";
					break;
				case 8:
					body += "CHALLENGER";
					break;
				}

				body += R"(","rankedLeagueDivision":")";

				switch (tier)
				{
				case 0:
					body += "I";
					break;
				case 1:
					body += "II";
					break;
				case 2:
					body += "III";
					break;
				case 3:
					body += "IV";
					break;
				case 4:
					body += "";
					break;
				}

				body += R"("}})";

				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
			}

			ImGui::SameLine();
			if (ImGui::Button("Empty##emptyRank"))
			{
				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", R"({"lol":{"rankedLeagueQueue":"","rankedLeagueTier":"","rankedLeagueDivision":""}})");
			}

			ImGui::NextColumn();

			if (ImGui::Button("Empty challenge badges"))
			{
				LCU::Request("POST", "https://127.0.0.1/lol-challenges/v1/update-player-preferences/", R"({"challengeIds": []})");
			}

			ImGui::Columns(1);

			ImGui::Separator();
			static int masteryLvl;
			ImGui::Text("Mastery:");
			ImGui::InputInt("##inputMasteryLvl:", &masteryLvl, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitMasteryLvl"))
			{
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", "{\"lol\":{\"masteryScore\":\"" + std::to_string(masteryLvl) + "\"}}");
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(0, result.c_str(), 0, 0);
				}
			}

			ImGui::Separator();

			static int gamemodeID;
			ImGui::Text("Lobby Gamemode ID:");
			ImGui::SameLine;
			ImGui::HelpMarker("DraftPick = 400\nSoloDuo = 420\nBlindPick = 430\nFlex = 440\nARAM = 450\nClash = 700\nIntroBots = 830\nBeginnerBots = 840\nIntermediateBots = 850\nARURF = 900\nTFTNormal = 1090\nTFTRanked = 1100\nTFTTutorial = 1110\nTFTHyperRoll = 1130\nNexusBlitz = 1300\nHA = 860\nTutorial1 = 2000\nTutorial2 = 2010\nTutorial3 = 2020");
			ImGui::InputInt("##inputGamemode:", &gamemodeID, 1, 100);

			static char lobbyPlayers[2048 * 16];
			ImGui::Text("Lobby Player Summoner IDs:");
			ImGui::SameLine;
			ImGui::HelpMarker("E.g. [123, 456, ...]");
			ImGui::InputText("##inputPlayers:", lobbyPlayers, 1000);

			if (ImGui::Button("Submit##submitLobby"))
			{
				std::string body = R"({"lol": {"pty": "{\"queueId\":)" + std::to_string(gamemodeID) + R"(, \"summoners\":)" + lobbyPlayers + "}\"}}";
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
			}

			ImGui::Separator();

			static int iconID;
			ImGui::Text("Icon:");
			ImGui::InputInt("##inputIcon:", &iconID, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitIcon"))
			{
				std::string body = R"({"profileIconId":)" + std::to_string(iconID) + "}";
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-summoner/v1/current-summoner/icon", body);
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(0, result.c_str(), 0, 0);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Submit 2##submitIcon2"))
			{
				std::string body = R"({"icon":)" + std::to_string(iconID) + "}";
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me/", body);
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(0, result.c_str(), 0, 0);
				}
			}

			static int backgroundID;
			ImGui::Text("Background:");

			ImGui::InputInt("##inputBackground", &backgroundID, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitBackground"))
			{
				std::string body = R"({"key":"backgroundSkinId","value":)" + std::to_string(backgroundID) + "}";
				std::string result = LCU::Request("POST", "https://127.0.0.1/lol-summoner/v1/current-summoner/summoner-profile/", body);
			}

			if (ImGui::CollapsingHeader("Backgrounds"))
			{
				if (champSkins.empty())
				{
					ImGui::Text("Skin data still downloading");
				}
				else
				{
					for (const auto& c : champSkins)
					{
						if (ImGui::TreeNode(c.name.c_str()))
						{
							for (const auto& s : c.skins)
							{
								if (ImGui::Button(s.second.c_str()))
								{
									std::string body = R"({"key":"backgroundSkinId","value":)" + s.first + "}";
									std::string result = LCU::Request("POST", "https://127.0.0.1/lol-summoner/v1/current-summoner/summoner-profile/", body);
								}
							}
							ImGui::TreePop();
						}
					}
				}
			}

			ImGui::EndTabItem();
		}
	}
};