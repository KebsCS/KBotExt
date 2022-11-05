#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "LCU.h"
#include "Utils.h"
#include "Misc.h"

class GameTab
{
public:

	static void Render()
	{
		if (ImGui::BeginTabItem("Game"))
		{
			static std::string result;
			static std::string custom;

			static std::vector<std::string>firstPosition = { "UNSELECTED", "TOP", "JUNGLE", "MIDDLE", "BOTTOM", "UTILITY", "FILL" };
			static std::vector<std::string>secondPosition = { "UNSELECTED", "TOP", "JUNGLE", "MIDDLE", "BOTTOM", "UTILITY", "FILL" };

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
			static size_t indexBots = 0; // Here we store our selection data as an index.
			const char* labelBots = "Bot";
			if (!botChamps.empty())
				labelBots = botChamps[indexBots].second.c_str();
			if (ImGui::BeginCombo("##comboBots", labelBots, 0))
			{
				if (botChamps.empty())
				{
					std::string getBots = LCU::Request("GET", "https://127.0.0.1/lol-lobby/v2/lobby/custom/available-bots");
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

				for (size_t n = 0; n < botChamps.size(); n++)
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
			static size_t indexDifficulty = 0; // Here we store our selection data as an index.
			const char* labelDifficulty = difficulties[indexDifficulty].c_str();

			if (ImGui::BeginCombo("##comboDifficulty", labelDifficulty, 0))
			{
				for (size_t n = 0; n < difficulties.size(); n++)
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
				result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", body);
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
				if (gameID == DraftPick || gameID == SoloDuo || gameID == Flex)
				{
					result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", body);
					LCU::Request("PUT", "/lol-lobby/v1/lobby/members/localMember/position-preferences",
						"{\"firstPreference\":\"" + firstPosition[S.gameTab.indexFirstRole]
						+ "\",\"secondPreference\":\"" + secondPosition[S.gameTab.indexSecondRole] + "\"}");
				}
				else
				{
					result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", body);
				}

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

			const char* labelFirstPosition = firstPosition[S.gameTab.indexFirstRole].c_str();
			if (ImGui::BeginCombo("##comboFirstPosition", labelFirstPosition, 0))
			{
				for (size_t n = 0; n < firstPosition.size(); n++)
				{
					const bool isSelected = (S.gameTab.indexFirstRole == n);
					if (ImGui::Selectable(firstPosition[n].c_str(), isSelected))
						S.gameTab.indexFirstRole = n;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::Text("Primary");

			ImGui::NextColumn();

			const char* second_labelPosition = secondPosition[S.gameTab.indexSecondRole].c_str();
			if (ImGui::BeginCombo("##comboSecondPosition", second_labelPosition, 0))
			{
				for (size_t n = 0; n < secondPosition.size(); n++)
				{
					const bool isSelected = (S.gameTab.indexSecondRole == n);
					if (ImGui::Selectable(secondPosition[n].c_str(), isSelected))
						S.gameTab.indexSecondRole = n;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::Text("Secondary");

			ImGui::NextColumn();

			if (ImGui::Button("Pick roles"))
			{
				result = LCU::Request("PUT", "/lol-lobby/v1/lobby/members/localMember/position-preferences",
					"{\"firstPreference\":\"" + firstPosition[S.gameTab.indexFirstRole]
					+ "\",\"secondPreference\":\"" + secondPosition[S.gameTab.indexSecondRole] + "\"}");
			}
			ImGui::SameLine();
			ImGui::HelpMarker("If you are already in a lobby you can use this button to pick the roles, or start a new lobby with the buttons above");

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(3, 0, false);
			if (ImGui::Button("Start queue"))
			{
				result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search");
			}
			ImGui::NextColumn();

			// if you press this during queue search you wont be able to start the queue again
			// unless you reenter the lobby :)
			if (ImGui::Button("Dodge"))
			{
				result = LCU::Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])", "");
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Dodges lobby instantly, you still lose LP, but you don't have to restart the client");
			ImGui::NextColumn();

			static std::vector<std::string>itemsMultiSearch = {
				"OP.GG", "U.GG", "PORO.GG", "Porofessor.gg"
			};
			const char* selectedMultiSearch = itemsMultiSearch[S.gameTab.indexMultiSearch].c_str();

			if (ImGui::Button("Multi-Search"))
			{
				std::string names;
				std::string champSelect = LCU::Request("GET", "https://127.0.0.1/lol-champ-select/v1/session");
				if (!champSelect.empty() && champSelect.find("RPC_ERROR") == std::string::npos)
				{
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value rootRegion;
					Json::Value rootCSelect;
					Json::Value rootSummoner;

					if (reader->parse(champSelect.c_str(), champSelect.c_str() + static_cast<int>(champSelect.length()), &rootCSelect, &err))
					{
						auto teamArr = rootCSelect["myTeam"];
						if (teamArr.isArray())
						{
							std::wstring summNames = L"";
							for (Json::Value::ArrayIndex i = 0; i < teamArr.size(); ++i)
							{
								std::string summId = teamArr[i]["summonerId"].asString();
								if (summId != "0")
								{
									std::string summoner = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + summId);
									if (reader->parse(summoner.c_str(), summoner.c_str() + static_cast<int>(summoner.length()), &rootSummoner, &err))
									{
										summNames += Utils::StringToWstring(rootSummoner["internalName"].asString()) + L",";
									}
								}
							}

							std::wstring region;
							if (itemsMultiSearch[S.gameTab.indexMultiSearch] == "U.GG") // platformId (euw1, eun1, na1)
							{
								std::string getAuthorization = LCU::Request("GET", "/lol-rso-auth/v1/authorization");
								if (reader->parse(getAuthorization.c_str(), getAuthorization.c_str() + static_cast<int>(getAuthorization.length()), &rootRegion, &err))
								{
									region = Utils::StringToWstring(rootRegion["currentPlatformId"].asString());
								}
							}
							else // region code (euw, eune na)
							{
								std::string getRegion = LCU::Request("GET", "/riotclient/get_region_locale");
								if (reader->parse(getRegion.c_str(), getRegion.c_str() + static_cast<int>(getRegion.length()), &rootRegion, &err))
								{
									region = Utils::StringToWstring(rootRegion["webRegion"].asString());
								}
							}

							if (!region.empty())
							{
								if (summNames.at(summNames.size() - 1) == L',')
									summNames.pop_back();

								std::wstring url;
								if (itemsMultiSearch[S.gameTab.indexMultiSearch] == "OP.GG")
								{
									url = L"https://" + region + L".op.gg/multi/query=" + summNames;
								}
								else if (itemsMultiSearch[S.gameTab.indexMultiSearch] == "U.GG")
								{
									url = L"https://u.gg/multisearch?summoners=" + summNames + L"&region=" + Utils::ToLower(region);
								}
								else if (itemsMultiSearch[S.gameTab.indexMultiSearch] == "PORO.GG")
								{
									url = L"https://poro.gg/multi?region=" + Utils::ToUpper(region) + L"&q=" + summNames;
								}
								else if (itemsMultiSearch[S.gameTab.indexMultiSearch] == "Porofessor.gg")
								{
									url = L"https://porofessor.gg/pregame/" + region + L"/" + summNames;
								}

								ShellExecuteW(0, 0, url.c_str(), 0, 0, SW_SHOW);
								result = Utils::WstringToString(url);
							}
							else
								result = "Failed to get region";
						}
					}
				}
				else
					result = "Champion select not found";
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 6));
			if (ImGui::BeginCombo("##comboMultiSearch", selectedMultiSearch, 0))
			{
				for (size_t n = 0; n < itemsMultiSearch.size(); n++)
				{
					const bool is_selected = (S.gameTab.indexMultiSearch == n);
					if (ImGui::Selectable(itemsMultiSearch[n].c_str(), is_selected))
						S.gameTab.indexMultiSearch = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(3, 0, false);
			ImGui::Checkbox("Auto accept", &S.gameTab.autoAcceptEnabled);

			ImGui::NextColumn();
			if (ImGui::Button("Invite everyone to lobby"))
			{
				std::string getFriends = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/friends");

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(getFriends.c_str(), getFriends.c_str() + static_cast<int>(getFriends.length()), &root, &err))
				{
					if (root.isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i)
						{
							std::string friendSummId = root[i]["summonerId"].asString();
							std::string inviteBody = "[{\"toSummonerId\":" + friendSummId + "}]";
							LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/invitations", inviteBody);
						}
						result = "Invited friends to lobby";
					}
				}
			}

			ImGui::NextColumn();

			if (ImGui::Button("Refund last purchase"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value rootSession;
				Json::Value rootPurchaseHistory;

				std::string storeUrl = LCU::Request("GET", "https://127.0.0.1/lol-store/v1/getStoreUrl");
				storeUrl = storeUrl.erase(0, 1).erase(storeUrl.size() - 1);
				std::string session = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(session.c_str(), session.c_str() + static_cast<int>(session.length()), &rootSession, &err))
				{
					std::string accountId = rootSession["accountId"].asString();
					std::string idToken = rootSession["idToken"].asString();
					std::string authorizationHeader = "Authorization: Bearer " + idToken + "\r\n" +
						"Accept: application/json" + "\r\n" +
						"Content-Type: application/json" + "\r\n";
					std::string purchaseHistory = HTTP::Request("GET", storeUrl + "/storefront/v3/history/purchase", "", authorizationHeader, "", "");
					if (reader->parse(purchaseHistory.c_str(), purchaseHistory.c_str() + static_cast<int>(purchaseHistory.length()), &rootPurchaseHistory, &err))
					{
						std::string transactionId = rootPurchaseHistory["transactions"][0]["transactionId"].asString();
						result = HTTP::Request("POST", storeUrl + "/storefront/v3/refund", "{\"accountId\":" + accountId + ",\"transactionId\":\"" + transactionId + "\",\"inventoryType\":\"CHAMPION\",\"language\":\"EN_US\"}", authorizationHeader, "", "");
					}
				}
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Buy a champion, pick it during a game and click this button before the game ends, no refund token will be used to refund it");

			ImGui::Columns(1);

			ImGui::Columns(2, 0, false);

			ImGui::Text("Instant message: ");
			ImGui::SameLine();
			static char bufInstantMessage[500];
			std::copy(S.gameTab.instantMessage.begin(), S.gameTab.instantMessage.end(), bufInstantMessage);
			//ImGui::SetNextItemWidth(S.Window.width / 3);
			ImGui::InputText("##inputInstantMessage", bufInstantMessage, IM_ARRAYSIZE(bufInstantMessage));
			S.gameTab.instantMessage = bufInstantMessage;

			ImGui::NextColumn();
			ImGui::SliderInt("Delay##sliderInstantMessageDelay", &S.gameTab.instantMessageDelay, 0, 10000, "%d ms");

			ImGui::Columns(1);

			ImGui::Columns(2, 0, false);

			ImGui::SliderInt("Time(s)##sliderInstantMessageTimes", &S.gameTab.instantMessageTimes, 1, 10, "%d");

			ImGui::NextColumn();

			ImGui::SliderInt("Delay between msgs##sliderInstantMessageDelayTimes", &S.gameTab.instantMessageDelayTimes, 0, 10000, "%d ms");

			ImGui::Columns(1);

			ImGui::Columns(3, 0, false);

			ImGui::Checkbox("Instalock", &S.gameTab.instalockEnabled);
			ImGui::NextColumn();

			ImGui::SliderInt("Delay##sliderInstalockDelay", &S.gameTab.instalockDelay, 0, 10000, "%d ms");

			ImGui::NextColumn();

			ImGui::Checkbox("Dodge on champion ban", &S.gameTab.dodgeOnBan);

			ImGui::SameLine();
			ImGui::HelpMarker("Ignores backup pick");

			ImGui::Columns(1);

			static bool isStillDownloading = true;
			if (!champSkins.empty())
				isStillDownloading = false;

			static std::string chosenInstalock = "Instalock champ \t\tChosen: " + Misc::ChampIdToName(S.gameTab.instalockId) + "###AnimatedInstalock";
			static int lastInstalockId = 0;
			if ((lastInstalockId != S.gameTab.instalockId) && !isStillDownloading)
			{
				lastInstalockId = S.gameTab.instalockId;
				chosenInstalock = "Instalock champ \t\tChosen: " + Misc::ChampIdToName(S.gameTab.instalockId) + "###AnimatedInstalock";
			}
			if (ImGui::CollapsingHeader(chosenInstalock.c_str()))
			{
				std::vector<std::pair<int, std::string>>instalockChamps = GetInstalockChamps();
				for (const auto& champ : instalockChamps)
				{
					char bufchamp[128];
					sprintf_s(bufchamp, "##Select %s", champ.second.c_str());
					ImGui::Text("%s", champ.second.c_str());
					ImGui::SameLine();
					ImGui::RadioButton(bufchamp, &S.gameTab.instalockId, champ.first);
				}
			}

			static std::string chosenBackup = "Backup pick \t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.backupId) + "###AnimatedBackup";
			static int lastBackupId = 0;
			if ((lastBackupId != S.gameTab.backupId) && !isStillDownloading)
			{
				lastBackupId = S.gameTab.backupId;
				chosenBackup = "Backup pick \t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.backupId) + "###AnimatedBackup";
			}
			if (ImGui::CollapsingHeader(chosenBackup.c_str()))
			{
				ImGui::Text("None");
				ImGui::SameLine();
				ImGui::RadioButton("##noneBackupPick", &S.gameTab.backupId, 0);
				std::vector<std::pair<int, std::string>>instalockChamps = GetInstalockChamps();
				for (const auto& champ : instalockChamps)
				{
					char bufchamp[128];
					sprintf_s(bufchamp, "##Select %s", champ.second.c_str());
					ImGui::Text("%s", champ.second.c_str());
					ImGui::SameLine();
					ImGui::RadioButton(bufchamp, &S.gameTab.backupId, champ.first);
				}
			}

			ImGui::Columns(2, 0, false);

			ImGui::Checkbox("Auto ban", &S.gameTab.autoBanEnabled);
			ImGui::NextColumn();

			ImGui::SliderInt("Delay##sliderautoBanDelay", &S.gameTab.autoBanDelay, 0, 10000, "%d ms");

			ImGui::Columns(1);

			static std::string chosenAutoban = "Auto ban\t\t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.autoBanId) + "###AnimatedAutoban";
			static int lastAutoban = 0;
			if ((lastAutoban != S.gameTab.autoBanId) && !isStillDownloading)
			{
				lastAutoban = S.gameTab.autoBanId;
				chosenAutoban = "Auto ban\t\t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.autoBanId) + "###AnimatedAutoban";
			}
			if (ImGui::CollapsingHeader(chosenAutoban.c_str()))
			{
				if (champSkins.empty())
				{
					ImGui::Text("Champion data still downloading");
				}
				else
				{
					ImGui::Text("None");
					ImGui::SameLine();
					ImGui::RadioButton("##nonechamp", &S.gameTab.autoBanId, 0);
					for (const auto& c : champSkins)
					{
						char bufchamp[128];
						sprintf_s(bufchamp, "##Select %s", c.name.c_str());
						ImGui::Text("%s", c.name.c_str());
						ImGui::SameLine();
						ImGui::RadioButton(bufchamp, &S.gameTab.autoBanId, c.key);
					}
				}
			}

			//ImGui::Separator();

			// Patched :(
		/*	if (ImGui::Button("Free ARAM/ARURF boost"))
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
					else
					{
						MessageBoxA(0, "You have enough RP", "It's not possible to grant you a free skin boost", 0);
					}
				}
			}
			ImGui::SameLine();
			Misc::HelpMarker("Works only when you don't have enough RP for boost");*/

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

		std::string result = LCU::Request("GET", "https://127.0.0.1/lol-champions/v1/owned-champions-minimal");
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

	static void InstantMessage()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instantMessageDelay));
		Json::Value root;
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		std::string getChat = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/conversations");
		if (reader->parse(getChat.c_str(), getChat.c_str() + static_cast<int>(getChat.length()), &root, &err))
		{
			if (root.isArray())
			{
				for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
				{
					if (root[i]["type"].asString() != "championSelect")
						continue;
					std::string lobbyID = root[i]["id"].asString();
					std::string request = "https://127.0.0.1/lol-chat/v1/conversations/" + lobbyID + "/messages";
					std::string error = "errorCode";
					while (error.find("errorCode") != std::string::npos)
					{
						error = LCU::Request("POST", request, R"({"type":"chat", "body":")" + std::string(S.gameTab.instantMessage) + R"("})");
						if (S.gameTab.instantMessageTimes > 1)
						{
							for (int time = 0; time < S.gameTab.instantMessageTimes - 1; time++)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instantMessageDelayTimes));
								error = LCU::Request("POST", request, R"({"type":"chat", "body":")" + std::string(S.gameTab.instantMessage) + R"("})");
							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					break;
				}
			}
		}
	}

	static void AutoAccept()
	{
		while (true)
		{
			if (!FindWindowA("RiotWindowClass", "League of Legends (TM) Client"))
			{
				if (::FindWindowA("RCLIENT", "League of Legends"))
				{
					if (S.gameTab.autoAcceptEnabled || S.gameTab.autoBanId || (S.gameTab.dodgeOnBan && S.gameTab.instalockEnabled) || (S.gameTab.instalockEnabled && S.gameTab.instalockId))
					{
						Json::Value rootSearch;
						Json::Value rootChampSelect;
						Json::Value rootSession;
						Json::CharReaderBuilder builder;
						const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
						JSONCPP_STRING err;
						std::string getSearchState = LCU::Request("GET", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search-state");
						if (reader->parse(getSearchState.c_str(), getSearchState.c_str() + static_cast<int>(getSearchState.length()), &rootSearch, &err))
						{
							static bool foundCell = false;
							static bool sendMessage = true;
							static int useBackupId = 0;
							std::string searchState = rootSearch["searchState"].asString();
							if (searchState == "Found")
							{
								std::string getChampSelect = LCU::Request("GET", "https://127.0.0.1/lol-champ-select/v1/session");
								if (getChampSelect.find("RPC_ERROR") != std::string::npos)
								{
									foundCell = false;
									sendMessage = true;
									useBackupId = 0;
									if (S.gameTab.autoAcceptEnabled)
									{
										LCU::Request("POST", "https://127.0.0.1/lol-matchmaking/v1/ready-check/accept", "");
									}
									std::this_thread::sleep_for(std::chrono::milliseconds(100));
								}
								else
								{
									if (reader->parse(getChampSelect.c_str(), getChampSelect.c_str() + static_cast<int>(getChampSelect.length()), &rootChampSelect, &err))
									{
										//if (S.gameTab.dodgeOnBan && S.gameTab.instalockId && S.gameTab.instalockEnabled)
										//{
										//	// empty on draft? look within actions
										//	auto myTeamBans = rootChampSelect["bans"]["myTeamBans"];
										//	if (myTeamBans.isArray())
										//	{
										//		for (Json::Value::ArrayIndex i = 0; i < myTeamBans.size(); i++)
										//		{
										//			if (myTeamBans[i].asInt() == S.gameTab.instalockId)
										//			{
										//				http->Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])", "", auth->leagueHeader, "", "", auth->leaguePort);
										//			}
										//		}
										//	}
										//	auto theirTeamBans = rootChampSelect["bans"]["theirTeamBans"];
										//	if (theirTeamBans.isArray())
										//	{
										//		for (Json::Value::ArrayIndex i = 0; i < theirTeamBans.size(); i++)
										//		{
										//			if (theirTeamBans[i].asInt() == S.gameTab.instalockId)
										//			{
										//				http->Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])", "", auth->leagueHeader, "", "", auth->leaguePort);
										//			}
										//		}
										//	}
										//}
										if (sendMessage && !S.gameTab.instantMessage.empty())
										{
											sendMessage = false;
											std::thread instantMessageThread(&GameTab::InstantMessage);
											instantMessageThread.detach();
										}
										if (S.gameTab.instalockEnabled || S.gameTab.autoBanId)
										{
											// get own summid
											std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
											if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &rootSession, &err))
											{
												static int cellId = 0;

												if (!foundCell)
												{
													auto myTeam = rootChampSelect["myTeam"];
													std::string summId = rootSession["summonerId"].asString();
													if (myTeam.isArray())
													{
														// get own cellId
														for (Json::Value::ArrayIndex i = 0; i < myTeam.size(); i++)
														{
															if (myTeam[i]["summonerId"].asString() == summId)
															{
																cellId = myTeam[i]["cellId"].asInt();
																foundCell = true;
																break;
															}
														}
													}
												}
												else
												{
													for (Json::Value::ArrayIndex j = 0; j < rootChampSelect["actions"].size(); j++)
													{
														auto actions = rootChampSelect["actions"][j];
														if (actions.isArray())
														{
															for (Json::Value::ArrayIndex i = 0; i < actions.size(); i++)
															{
																// search for own actions
																if (actions[i]["actorCellId"].asInt() == cellId)
																{
																	std::string actionType = actions[i]["type"].asString();
																	if (actionType == "pick" && S.gameTab.instalockId && S.gameTab.instalockEnabled)
																	{
																		// if havent picked yet
																		if (actions[i]["completed"].asBool() == false)
																		{
																			std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instalockDelay));

																			int currentPick = S.gameTab.instalockId;
																			if (useBackupId)
																				currentPick = useBackupId;

																			LCU::Request("PATCH", "https://127.0.0.1/lol-champ-select/v1/session/actions/" + actions[i]["id"].asString(),
																				R"({"completed":true,"championId":)" + std::to_string(currentPick) + "}");
																		}
																		//else
																		//{
																		//	// we picked already, theres nothing to do so sleep
																		//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
																		//	break;
																		//}
																	}
																	else if (actionType == "ban" && S.gameTab.autoBanId && S.gameTab.autoBanEnabled)
																	{
																		if (actions[i]["completed"].asBool() == false)
																		{
																			std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.autoBanDelay));

																			LCU::Request("PATCH", "https://127.0.0.1/lol-champ-select/v1/session/actions/" + actions[i]["id"].asString(),
																				R"({"completed":true,"championId":)" + std::to_string(S.gameTab.autoBanId) + "}");
																		}
																	}
																	//else break;
																}
																// if dodge on ban enabled or backup pick
																if ((S.gameTab.dodgeOnBan || S.gameTab.backupId) && S.gameTab.instalockEnabled && S.gameTab.instalockId)
																{
																	if (actions[i]["type"].asString() == "ban" && actions[i]["completed"].asBool() == true)
																	{
																		if (actions[i]["championId"].asInt() == S.gameTab.instalockId)
																		{
																			if (S.gameTab.dodgeOnBan)
																			{
																				LCU::Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])", "");
																			}
																			else if (S.gameTab.backupId)
																			{
																				useBackupId = S.gameTab.backupId;
																			}
																		}
																	}
																}
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
							else
							{
								foundCell = false;
								sendMessage = true;
								useBackupId = 0;
								std::this_thread::sleep_for(std::chrono::milliseconds(1000));
							}
						}
					}
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
};