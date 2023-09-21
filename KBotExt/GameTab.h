#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"
#include "Utils.h"
#include "Misc.h"

class GameTab
{
private:
	static inline bool onOpen = true;

public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Game"))
		{
			static std::string result;
			static std::string custom;

			static std::vector<std::pair<long, std::string>> gamemodes;

			if (onOpen)
			{
				if (gamemodes.empty())
				{
					std::string getQueues = LCU::Request("GET", "/lol-game-queues/v1/queues");
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (reader->parse(getQueues.c_str(), getQueues.c_str() + static_cast<int>(getQueues.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (Json::Value::ArrayIndex i = 0; i < root.size(); i++)
							{
								if (root[i]["queueAvailability"].asString() != "Available")
									continue;

								int64_t id = root[i]["id"].asInt64();
								std::string name = root[i]["name"].asString();
								name += " " + std::to_string(id);
								//std::cout << id << " " << name << std::endl;
								std::pair<long, std::string> temp = {id, name};
								gamemodes.emplace_back(temp);
							}

							std::ranges::sort(gamemodes, [](auto& left, auto& right) {
								return left.first < right.first;
							});
						}
					}
				}
			}

			static std::vector<std::string> firstPosition = {"UNSELECTED", "TOP", "JUNGLE", "MIDDLE", "BOTTOM", "UTILITY", "FILL"};
			static std::vector<std::string> secondPosition = {"UNSELECTED", "TOP", "JUNGLE", "MIDDLE", "BOTTOM", "UTILITY", "FILL"};

			static int gameID = 0;

			ImGui::Columns(4, nullptr, false);

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

			if (ImGui::Button("ARURF 1V1 (PBE)"))
				gameID = 901;

			/*if (ImGui::Button("URF"))
				gameID = 318;*/

			ImGui::NextColumn();

			if (ImGui::Button("TFT Normal"))
				gameID = TFTNormal;

			if (ImGui::Button("TFT Ranked"))
				gameID = TFTRanked;

			if (ImGui::Button("TFT Hyper Roll"))
				gameID = TFTHyperRoll;

			if (ImGui::Button("TFT Double Up"))
				gameID = TFTDoubleUp;

			ImGui::NextColumn();

			if (ImGui::Button("TFT Tutorial"))
				gameID = TFTTutorial;

			if (ImGui::Button("Practice Tool"))
			{
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":1},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
			}

			if (ImGui::Button("Practice Tool 5v5"))
			{
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"PRACTICETOOL","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
			}

			if (ImGui::Button("Clash"))
				gameID = Clash;

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(4, nullptr, false);

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
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"CLASSIC","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";

			if (ImGui::Button("Custom ARAM"))
				custom =
					R"({"customGameLobby":{"configuration":{"gameMode":"ARAM","gameMutator":"","gameServerRegion":"","mapId":12,"mutators":{"id":1},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";

			//"id" 1- blind 2- draft -4 all random 6- tournament draft

			static int indexGamemodes = -1;
			auto labelGamemodes = "All Gamemodes";
			if (indexGamemodes != -1)
				labelGamemodes = gamemodes[indexGamemodes].second.c_str();

			if (ImGui::BeginCombo("##combolGamemodes", labelGamemodes, 0))
			{
				for (size_t n = 0; n < gamemodes.size(); n++)
				{
					const bool is_selected = indexGamemodes == n;
					if (ImGui::Selectable(gamemodes[n].second.c_str(), is_selected))
						indexGamemodes = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();

			if (ImGui::Button("Create##gamemode"))
			{
				gameID = gamemodes[indexGamemodes].first;
			}

			ImGui::NextColumn();

			static std::vector<std::pair<int, std::string>> botChamps;
			static size_t indexBots = 0; // Here we store our selection data as an index.
			auto labelBots = "Bot";
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
							for (auto& i : root)
							{
								std::pair temp = {i["id"].asInt(), i["name"].asString()};
								botChamps.emplace_back(temp);
							}
							std::ranges::sort(botChamps, [](std::pair<int, std::string> a, std::pair<int, std::string> b) {
								return a.second < b.second;
							});
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
			std::vector<std::string> difficulties = {"NONE", "EASY", "MEDIUM", "HARD", "UBER", "TUTORIAL", "INTRO"};
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
				if (botChamps.empty())
				{
					MessageBoxA(nullptr, "Pick the bots champion first", "Adding bots failed", MB_OK);
				}
				else
				{
					std::string team = botTeam ? R"(,"teamId":"200"})" : R"(,"teamId":"100"})";
					std::string body = R"({"botDifficulty":")" + difficulties[indexDifficulty] + R"(","championId":)" + std::to_string(
						botChamps[indexBots].first) + team;
					result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", body);
				}
			}
			ImGui::SameLine();
			ImGui::RadioButton("Blue", &botTeam, 0);
			ImGui::SameLine();
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
					             R"({"firstPreference":")" + firstPosition[S.gameTab.indexFirstRole]
					             + R"(","secondPreference":")" + secondPosition[S.gameTab.indexSecondRole] + "\"}");
				}
				else
				{
					result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", body);
				}

				//for (int i = 0; i < 10001; i++)
				//{
				//	std::string res =
				//		R"({"customGameLobby":{"configuration":{"gameMode":"CLASSIC","gameMutator":"","gameServerRegion":"","mapId":11,"mutators":{"id":)"
				//		+ std::to_string(i) +
				//		R"(},"spectatorPolicy":"AllAllowed","teamSize":5},"lobbyName":"KBot","lobbyPassword":null},"isCustom":true})";
				//	if (std::string xdd = http.Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby", res, auth->leagueHeader, "", "", clientPort);
				//		xdd.find("errorCode") == std::string::npos)
				//		std::cout << i << std::endl;
				//	std::this_thread::sleep_for(std::chrono::milliseconds(10));
				//}

				gameID = 0;
			}

			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 7));
			if (const char* labelFirstPosition = firstPosition[S.gameTab.indexFirstRole].c_str(); ImGui::BeginCombo(
				"##comboFirstPosition", labelFirstPosition, 0))
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
			/*		ImGui::SameLine();
					ImGui::Text("Primary");*/

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 7));
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
			//ImGui::SameLine();
			//ImGui::Text("Secondary");

			ImGui::SameLine();

			if (ImGui::Button("Pick roles"))
			{
				result = LCU::Request("PUT", "/lol-lobby/v1/lobby/members/localMember/position-preferences",
				                      R"({"firstPreference":")" + firstPosition[S.gameTab.indexFirstRole]
				                      + R"(","secondPreference":")" + secondPosition[S.gameTab.indexSecondRole] + "\"}");
			}
			ImGui::SameLine();
			ImGui::HelpMarker("If you are already in a lobby you can use this button to pick the roles, or start a new lobby with the buttons above");

			ImGui::NextColumn();

			if (ImGui::Button("Change runes"))
			{
				result = ChangeRunesOpgg();
			}

			ImGui::SameLine();
			ImGui::Columns(2, nullptr, false);

			ImGui::Checkbox("Blue/Red Side notification", &S.gameTab.sideNotification);

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(3, nullptr, false);
			if (ImGui::Button("Start queue"))
			{
				result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search");
			}
			ImGui::NextColumn();

			// if you press this during queue search you wont be able to start the queue again
			// unless you reenter the lobby :)
			if (ImGui::Button("Dodge"))
			{
				result = LCU::Request(
					"POST",
					R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])",
					"");
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Dodges lobby instantly, you still lose LP, but you don't have to restart the client");
			ImGui::NextColumn();

			static std::vector<std::string> itemsMultiSearch = {
				"OP.GG", "U.GG", "PORO.GG", "Porofessor.gg"
			};
			const char* selectedMultiSearch = itemsMultiSearch[S.gameTab.indexMultiSearch].c_str();

			if (ImGui::Button("Multi-Search"))
			{
				result = MultiSearch(itemsMultiSearch[S.gameTab.indexMultiSearch]);
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

			ImGui::Columns(3, nullptr, false);
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
						for (auto& i : root)
						{
							std::string friendSummId = i["summonerId"].asString();
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
				if (MessageBoxA(nullptr, "Are you sure?", "Refunding last purchase", MB_OKCANCEL) == IDOK)
				{
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value rootPurchaseHistory;

					cpr::Header storeHeader = Utils::StringToHeader(LCU::GetStoreHeader());

					std::string storeUrl = LCU::Request("GET", "/lol-store/v1/getStoreUrl");
					std::erase(storeUrl, '"');

					std::string purchaseHistory = cpr::Get(cpr::Url{storeUrl + "/storefront/v3/history/purchase"}, cpr::Header{storeHeader}).text;
					if (reader->parse(purchaseHistory.c_str(), purchaseHistory.c_str() + static_cast<int>(purchaseHistory.length()),
					                  &rootPurchaseHistory, &err))
					{
						std::string accountId = rootPurchaseHistory["player"]["accountId"].asString();
						std::string transactionId = rootPurchaseHistory["transactions"][0]["transactionId"].asString();
						result = cpr::Post(cpr::Url{storeUrl + "/storefront/v3/refund"}, cpr::Header{storeHeader},
						              cpr::Body{
							              "{\"accountId\":" + accountId + R"(,"transactionId":")" + transactionId +
							              R"(","inventoryType":"CHAMPION","language":"en_US"})"
						              }).text;
					}
					else
					{
						result = purchaseHistory;
					}
				}
			}
			ImGui::SameLine();
			ImGui::HelpMarker(
				"Buy a champion, pick it during a game and click this button before the game ends, no refund token will be used to refund it");

			ImGui::Columns(1);

			ImGui::Separator();

			//ImGui::Columns(2, 0, false);

			ImGui::Text("Instant message:");
			ImGui::SameLine();
			static char bufInstantMessage[500];
			std::ranges::copy(S.gameTab.instantMessage, bufInstantMessage);
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 6));
			ImGui::InputText("##inputInstantMessage", bufInstantMessage, IM_ARRAYSIZE(bufInstantMessage));
			S.gameTab.instantMessage = bufInstantMessage;

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 7));
			ImGui::SliderInt("Delay##sliderInstantMessageDelay", &S.gameTab.instantMessageDelay, 0, 10000, "%d ms");
			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 10));
			ImGui::SliderInt("Time(s)##sliderInstantMessageTimes", &S.gameTab.instantMessageTimes, 1, 10, "%d");

			ImGui::SameLine();

			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 7));
			ImGui::SliderInt("Delay between##sliderInstantMessageDelayTimes", &S.gameTab.instantMessageDelayTimes, 0, 4000, "%d ms");

			ImGui::Separator();

			static bool isStillBeingFetched = true;
			if (!champSkins.empty())
				isStillBeingFetched = false;

			static ImGui::ComboAutoSelectData instalockComboData;

			if (onOpen)
			{
				std::vector<std::pair<int, std::string>> instalockChamps = GetInstalockChamps();

				if (!instalockChamps.empty())
				{
					std::vector<std::string> instalockChampsNames;
					instalockChampsNames.reserve(instalockChamps.size());

					std::string selectedChamp = ChampIdToName(S.gameTab.instalockId);
					std::ranges::copy(selectedChamp, instalockComboData.input);

					for (size_t i = 0; i < instalockChamps.size(); i++)
					{
						instalockChampsNames.emplace_back(instalockChamps[i].second);
						if (instalockComboData.input == instalockChamps[i].second)
						{
							instalockComboData.index = i;
						}
					}
					instalockComboData.items = instalockChampsNames;
				}
			}

			ImGui::Checkbox("Instalock", &S.gameTab.instalockEnabled);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 6));
			if (ImGui::ComboAutoSelect("##comboInstalock", instalockComboData))
			{
				if (instalockComboData.index != -1)
				{
					for (const auto& [key, name, skins] : champSkins)
					{
						if (instalockComboData.input == name)
						{
							S.gameTab.instalockId = key;
						}
					}
				}
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 6));

			ImGui::SliderInt("Delay##sliderInstalockDelay", &S.gameTab.instalockDelay, 0, 10000, "%d ms");

			ImGui::SameLine();

			ImGui::Checkbox("Dodge on champion ban", &S.gameTab.dodgeOnBan);

			ImGui::SameLine();
			ImGui::HelpMarker("Ignores backup pick");

			static std::string chosenBackup = "Backup pick \t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.backupId) + "###AnimatedBackup";
			static int lastBackupId = 0;
			if ((lastBackupId != S.gameTab.backupId) && !isStillBeingFetched)
			{
				lastBackupId = S.gameTab.backupId;
				chosenBackup = "Backup pick \t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.backupId) + "###AnimatedBackup";
			}
			if (ImGui::CollapsingHeader(chosenBackup.c_str()))
			{
				ImGui::Text("None");
				ImGui::SameLine();
				ImGui::RadioButton("##noneBackupPick", &S.gameTab.backupId, 0);
				std::vector<std::pair<int, std::string>> instalockChamps = GetInstalockChamps();
				for (const auto& [fst, snd] : instalockChamps)
				{
					char bufchamp[128];
					sprintf_s(bufchamp, "##Select %s", snd.c_str());
					ImGui::Text("%s", snd.c_str());
					ImGui::SameLine();
					ImGui::RadioButton(bufchamp, &S.gameTab.backupId, fst);
				}
			}

			ImGui::Checkbox("Auto ban", &S.gameTab.autoBanEnabled);
			ImGui::SameLine();

			static ImGui::ComboAutoSelectData autobanComboData;
			if (onOpen)
			{
				std::vector<std::string> autobanChampsNames;
				if (!champSkins.empty())
				{
					autobanChampsNames.reserve(champSkins.size());

					std::string selectedChamp = ChampIdToName(S.gameTab.autoBanId);
					std::ranges::copy(selectedChamp, autobanComboData.input);

					for (size_t i = 0; i < champSkins.size(); i++)
					{
						autobanChampsNames.emplace_back(champSkins[i].name);

						if (autobanComboData.input == champSkins[i].name)
						{
							autobanComboData.index = i;
						}
					}
					autobanComboData.items = autobanChampsNames;
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 6));
			if (ImGui::ComboAutoSelect("##comboAutoban", autobanComboData))
			{
				if (autobanComboData.index != -1)
				{
					for (const auto& [key, name, skins] : champSkins)
					{
						if (autobanComboData.input == name)
						{
							S.gameTab.autoBanId = key;
						}
					}
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 6));
			ImGui::SliderInt("Delay##sliderautoBanDelay", &S.gameTab.autoBanDelay, 0, 10000, "%d ms");

			ImGui::SameLine();

			ImGui::Checkbox("Instant Mute", &S.gameTab.instantMute);

			/*
				Free ARAM Boost exploit
				Fun fact: I've reported this to Riot on 23rd August 2021 together with the refund exploit
				I've got a response that my report is a duplicate (it wasn't, I found the exploits)
				Almost 2 years later, both of them are still not fixed.
			*/

			ImGui::SeparatorText("Free ARAM Boost");

			static std::string storeToken; // EntityAssignedButNoRead
			static cpr::Header storeHeader;
			static std::string accountId;
			static std::string boosted;
			static int ownedRP;
			if (ImGui::Button("Is boost available for this account?"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				// get rp amount
				std::string getWallet = LCU::Request("GET", "https://127.0.0.1/lol-inventory/v1/wallet/RP");
				if (reader->parse(getWallet.c_str(), getWallet.c_str() + static_cast<int>(getWallet.length()), &root, &err))
				{
					ownedRP = root["RP"].asInt();
				}

				// get accountId
				std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &root, &err))
				{
					accountId = root["accountId"].asString();
				}

				if (!CheckJWT(accountId) && ownedRP < 95)
				{
					int timeleft = 0;
					std::string temp = GetOldJWT(accountId, timeleft);
					timeleft = timeleft + 60 * 60 * 24 - time(nullptr);
					int minutes = timeleft / 60 - 60 * (timeleft / (60 * 60));
					boosted = "Boost available, time left on this account: " + std::to_string(timeleft / (60 * 60)) + ":" + std::to_string(minutes);
				}
				else
				{
					// get owned champions
					std::vector<int> ownedChampions;
					std::string getChampions = LCU::Request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION");
					if (reader->parse(getChampions.c_str(), getChampions.c_str() + static_cast<int>(getChampions.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto obj : root)
							{
								if (obj["ownershipType"].asString() == "OWNED")
								{
									ownedChampions.emplace_back(obj["itemId"].asInt());
								}
							}
						}
					}

					std::vector<std::pair<int, int>> champsToBuy; // price, id
					std::string getCatalog = LCU::Request("GET", "https://127.0.0.1/lol-store/v1/catalog");
					if (reader->parse(getCatalog.c_str(), getCatalog.c_str() + static_cast<int>(getCatalog.length()), &root, &err))
					{
						if (root.isArray())
						{
							for (auto obj : root)
							{
								if (obj["inventoryType"].asString() == "CHAMPION")
								{
									if (obj["sale"].empty() == true)
									{
										for (Json::Value::ArrayIndex i = 0; i < obj["prices"].size(); i++)
										{
											if (auto price = obj["prices"][i]; price["currency"].asString() == "RP")
											{
												champsToBuy.emplace_back(price["cost"].asInt(), obj["itemId"].asInt());
											}
										}
									}
									else
									{
										for (Json::Value::ArrayIndex i = 0; i < obj["sale"]["prices"].size(); i++)
										{
											if (auto sale = obj["sale"]["prices"][i]; sale["currency"].asString() == "RP")
											{
												champsToBuy.emplace_back(sale["cost"].asInt(), obj["itemId"].asInt());
											}
										}
									}
								}
							}
						}
					}

					int idToBuy = 0;
					int priceToBuy = 0;

					for (auto [fst, snd] : champsToBuy)
					{
						bool found = false;
						for (int id : ownedChampions)
						{
							if (snd == id)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							if (ownedRP - fst > 0 && ownedRP - fst < 95)
							{
								priceToBuy = fst;
								idToBuy = snd;
								break;
							}
						}
					}
					if (idToBuy != 0 && priceToBuy != 0)
					{
						boosted = "Boost is available for this account";
					}
					else
					{
						boosted = "Boost IS NOT available for this account";
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Free ARAM/ARURF boost"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				// get rp ammount
				std::string getWallet = LCU::Request("GET", "https://127.0.0.1/lol-inventory/v1/wallet/RP");
				if (reader->parse(getWallet.c_str(), getWallet.c_str() + static_cast<int>(getWallet.length()), &root, &err))
				{
					ownedRP = root["RP"].asInt();
				}

				// get accountId
				std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &root, &err))
				{
					accountId = root["accountId"].asString();
				}

				bool bNeedNewJwt = true;
				if (!CheckJWT(accountId))
				{
					bNeedNewJwt = false;
					if (ownedRP >= 95)
						bNeedNewJwt = true;
				}

				// get owned champions
				std::vector<int> ownedChampions;
				std::string getChampions = LCU::Request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION");
				if (reader->parse(getChampions.c_str(), getChampions.c_str() + static_cast<int>(getChampions.length()), &root, &err))
				{
					if (root.isArray())
					{
						for (auto obj : root)
						{
							if (obj["ownershipType"].asString() == "OWNED")
							{
								ownedChampions.emplace_back(obj["itemId"].asInt());
							}
						}
					}
				}

				std::vector<std::pair<int, int>> champsToBuy; // price, id
				std::string getCatalog = LCU::Request("GET", "https://127.0.0.1/lol-store/v1/catalog");
				if (reader->parse(getCatalog.c_str(), getCatalog.c_str() + static_cast<int>(getCatalog.length()), &root, &err))
				{
					if (root.isArray())
					{
						for (auto obj : root)
						{
							if (obj["inventoryType"].asString() == "CHAMPION")
							{
								if (obj["sale"].empty() == true)
								{
									for (Json::Value::ArrayIndex i = 0; i < obj["prices"].size(); i++)
									{
										if (auto price = obj["prices"][i]; price["currency"].asString() == "RP")
										{
											champsToBuy.emplace_back(price["cost"].asInt(), obj["itemId"].asInt());
										}
									}
								}
								else
								{
									for (Json::Value::ArrayIndex i = 0; i < obj["sale"]["prices"].size(); i++)
									{
										if (auto sale = obj["sale"]["prices"][i]; sale["currency"].asString() == "RP")
										{
											champsToBuy.emplace_back(sale["cost"].asInt(), obj["itemId"].asInt());
										}
									}
								}
							}
						}
					}
				}

				int idToBuy = 0;
				int priceToBuy = 0;

				for (auto [fst, snd] : champsToBuy)
				{
					bool found = false;
					for (int id : ownedChampions)
					{
						if (snd == id)
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						if (ownedRP - fst > 0 && ownedRP - fst < 95)
						{
							priceToBuy = fst;
							idToBuy = snd;
							break;
						}
					}
				}
				if ((idToBuy != 0 && priceToBuy != 0) || !bNeedNewJwt)
				{
					std::string getStoreUrl = LCU::Request("GET", "https://127.0.0.1/lol-store/v1/getStoreUrl");
					std::erase(getStoreUrl, '"');

					Json::CharReaderBuilder builder2;
					const std::unique_ptr<Json::CharReader> reader2(builder2.newCharReader());
					JSONCPP_STRING err2;
					Json::Value root2;

					// get signedWalletJwt
					std::string signedWalletJwt = LCU::Request("GET", "https://127.0.0.1/lol-inventory/v1/signedWallet/RP");
					if (reader2->parse(signedWalletJwt.c_str(), signedWalletJwt.c_str() + static_cast<int>(signedWalletJwt.length()), &root2, &err2))
					{
						signedWalletJwt = root2["RP"].asString();
						if (bNeedNewJwt)
						{
							SaveJWT(accountId, signedWalletJwt, time(nullptr));
						}
						else
						{
							int timeleft = 0;
							signedWalletJwt = GetOldJWT(accountId, timeleft);
						}

						Json::CharReaderBuilder builder3;
						const std::unique_ptr<Json::CharReader> reader3(builder3.newCharReader());
						JSONCPP_STRING err3;
						Json::Value root3;

						// get Bearer token for store
						std::string authorizations = LCU::Request("GET", "https://127.0.0.1/lol-rso-auth/v1/authorization/access-token");
						if (reader3->parse(authorizations.c_str(), authorizations.c_str() + static_cast<int>(authorizations.length()), &root3, &err3))
						{
							storeToken = root3["token"].asString();
							storeHeader = Utils::StringToHeader(LCU::GetStoreHeader());
							if (bNeedNewJwt)
							{
								// buy a champion
								std::string purchaseBody = R"({"accountId":)" + accountId + R"(,"items":[{"inventoryType":"CHAMPION","itemId":)" +
									std::to_string(idToBuy)
									+ R"(,"ipCost":null,"rpCost":)" + std::to_string(priceToBuy) + R"(,"quantity":1}]})";
								std::string purchaseUrl = getStoreUrl + "/storefront/v3/purchase?language=en_US";
								std::string purchase = cpr::Post(cpr::Url{purchaseUrl}, cpr::Body{purchaseBody}, cpr::Header{storeHeader}).text;
								boosted = "Bought " + ChampIdToName(idToBuy) + " - dont play this champion, or you wont be able to refund RP";
							}

							std::this_thread::sleep_for(std::chrono::seconds(1));

							// boost with signedWalletJwt
							std::string boostUrl =
								R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","activateBattleBoostV1","{\"signedWalletJwt\":\")"
								+ signedWalletJwt + R"(\"}"])";
							LCU::Request("POST", boostUrl);
						}
					}
				}
				else
				{
					boosted = "Ineligible for boost on this account";
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Refund RP"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				// get accountId
				std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &root, &err))
				{
					accountId = root["accountId"].asString();
				}

				std::string getStoreUrl = LCU::Request("GET", "https://127.0.0.1/lol-store/v1/getStoreUrl");
				std::erase(getStoreUrl, '"');

				std::string authorizations = LCU::Request("GET", "https://127.0.0.1/lol-rso-auth/v1/authorization/access-token");
				if (reader->parse(authorizations.c_str(), authorizations.c_str() + static_cast<int>(authorizations.length()), &root, &err))
				{
					storeToken = root["token"].asString();
					storeHeader = Utils::StringToHeader(LCU::GetStoreHeader());
				}

				std::string historyUrl = getStoreUrl + "/storefront/v3/history/purchase?language=en_US";
				std::string getHistory = cpr::Get(cpr::Url{historyUrl}, cpr::Header{storeHeader}).text;
				if (reader->parse(getHistory.c_str(), getHistory.c_str() + static_cast<int>(getHistory.length()), &root, &err))
				{
					if (root["transactions"].isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < root["transactions"].size(); i++)
						{
							if (auto transaction = root["transactions"][i]; transaction["refundable"].asBool() == true)
							{
								if (transaction["inventoryType"].asString() == "CHAMPION")
								{
									if (transaction["currencyType"].asString() == "RP")
									{
										if (transaction["requiresToken"].asBool() == false)
										{
											std::string refundUrl = getStoreUrl + "/storefront/v3/refund";
											std::string refundBody = R"({"accountId":)" + accountId + R"(,"transactionId":")" + transaction[
												"transactionId"].asString() + R"(","inventoryType":"CHAMPION","language":"en_US"})";
											Post(cpr::Url{refundUrl}, cpr::Body{refundBody}, cpr::Header{storeHeader});
											boosted = "Refunded";
										}
									}
								}
							}
						}
					}
				}
			}

			ImGui::SameLine();
			ImGui::HelpMarker("Instructions:\n"
				"You have to wait at least 1 hour after finishing your last game, otherwise the RP you used for boost will get consumed\n"
				"The longer you wait, the better. On a single token you can boost unlimited amount of times in 24h\n"
				"The exploit stores your RP, buys a champion with RP so you're left with <95 and then boosts using the stored RP\n"
				"Don't play with the champion the boost bought, or you wont be able to get your RP back\n");

			ImGui::TextWrapped(boosted.c_str());

			//ImGui::Separator();

			// Patched :(
			//if (ImGui::Button("Free ARAM/ARURF boost"))
			//{
			//	std::string wallet = http->Request("GET", "https://127.0.0.1/lol-inventory/v1/wallet/RP", "", auth->leagueHeader, "", "", auth->leaguePort);
			//	Json::CharReaderBuilder builder;
			//	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			//	JSONCPP_STRING err;
			//	Json::Value root;
			//	if (!reader->parse(wallet.c_str(), wallet.c_str() + static_cast<int>(wallet.length()), &root, &err))
			//		result = wallet;
			//	else
			//	{
			//		if (unsigned RP = root["RP"].asUInt(); RP < 95)
			//		{
			//			result = http->Request("POST", R"(https://127.0.0.1/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","activateBattleBoostV1",""])", "", auth->leagueHeader, "", "", auth->leaguePort);
			//		}
			//		else
			//		{
			//			MessageBoxA(0, "You have enough RP", "It's not possible to grant you a free skin boost", 0);
			//		}
			//	}
			//}
			//ImGui::SameLine();
			//Misc::HelpMarker("Works only when you don't have enough RP for boost");

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
				cResultJson = sResultJson.data();
				ImGui::InputTextMultiline("##gameResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 300));
			}

			if (onOpen)
				onOpen = false;

			ImGui::EndTabItem();
		}
		else
		{
			onOpen = true;
		}
	}

	static std::vector<std::pair<int, std::string>> GetInstalockChamps()
	{
		std::vector<std::pair<int, std::string>> temp;

		std::string result = LCU::Request("GET", "https://127.0.0.1/lol-champions/v1/owned-champions-minimal");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			if (root.isArray())
			{
				for (auto& i : root)
				{
					if (i["freeToPlay"].asBool() == true || i["ownership"]["owned"].asBool() == true ||
						(i["ownership"].isMember("xboxGPReward") && i["ownership"]["xboxGPReward"].asBool() == true))
					{
						std::string loadScreenPath = i["baseLoadScreenPath"].asString();
						size_t nameStart = loadScreenPath.find("ASSETS/Characters/") + strlen("ASSETS/Characters/");
						std::string champName = loadScreenPath.substr(nameStart, loadScreenPath.find('/', nameStart) - nameStart);

						std::pair champ = {i["id"].asInt(), champName};
						temp.emplace_back(champ);
					}
				}
			}
		}
		std::ranges::sort(temp, [](std::pair<int, std::string> a, std::pair<int, std::string> b) { return a.second < b.second; });
		return temp;
	}

	static void InstantMessage(const bool instantMute = false, const bool sideNotification = false)
	{
		auto start = std::chrono::system_clock::now();
		while (true)
		{
			auto now = std::chrono::system_clock::now();
			std::chrono::duration<double> diff = now - start;
			if (diff.count() > 10) // took 10 seconds and still didn't connect to chat
			{
				return;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;

			LCU::SetCurrentClientRiotInfo();
			std::string getChat = cpr::Get(cpr::Url{std::format("https://127.0.0.1:{}/chat/v5/participants/champ-select", LCU::riot.port)},
			                          cpr::Header{Utils::StringToHeader(LCU::riot.header)}, cpr::VerifySsl{false}).text;
			if (!reader->parse(getChat.c_str(), getChat.c_str() + static_cast<int>(getChat.length()), &root, &err))
			{
				continue;
			}

			const auto& participantsArr = root["participants"];
			if (!participantsArr.isArray())
			{
				continue;
			}
			if (participantsArr.size() <= 1)
			{
				continue;
			}

			const std::string cid = participantsArr[0]["cid"].asString();

			if (instantMute || sideNotification)
			{
				std::string champSelect = LCU::Request("GET", "/lol-champ-select/v1/session");
				Json::Value rootCSelect;
				if (!champSelect.empty() && champSelect.find("RPC_ERROR") == std::string::npos)
				{
					if (reader->parse(champSelect.c_str(), champSelect.c_str() + static_cast<int>(champSelect.length()), &rootCSelect, &err))
					{
						if (instantMute)
						{
							int localPlayerCellId = rootCSelect["localPlayerCellId"].asInt();
							for (Json::Value::ArrayIndex i = 0; i < rootCSelect["myTeam"].size(); i++)
							{
								Json::Value player = rootCSelect["myTeam"][i];
								if (player["cellId"].asInt() == localPlayerCellId)
									continue;

								LCU::Request("POST", "/lol-champ-select/v1/toggle-player-muted",
								             std::format(R"({{"summonerId":{0},"puuid":"{1}","obfuscatedSummonerId":{2},"obfuscatedPuuid":"{3}"}})",
								                         player["summonerId"].asString(), player["puuid"].asString(),
								                         player["obfuscatedSummonerId"].asString(),
								                         player["obfuscatedPuuid"].asString()));

								/*	LCU::Request("POST", "/telemetry/v1/events/general_metrics_number",
										R"({"eventName":"champ_select_toggle_player_muted_clicked","value":"0","spec":"high","isLowSpecModeOn":"false"})");

									LCU::Request("POST", std::format("/lol-chat/v1/conversations/{}/messages", cid),
										std::format("{{\"body\":\"{} is muted.\",\"type\":\"celebration\"}}", "player"));
								*/
							}
						}

						if (sideNotification)
						{
							if (rootCSelect["myTeam"].isArray() && !rootCSelect["myTeam"].empty())
							{
								std::string notification = "You are on the ";
								if (rootCSelect["myTeam"][0]["team"].asInt() == 1)
									notification += "Blue Side";
								else
									notification += "Red Side";
								LCU::Request("POST", std::format("/lol-chat/v1/conversations/{}/messages", cid),
								             R"({"body":")" + notification + R"(","type":"celebration"})");
							}
						}
					}
				}
			}

			if (S.gameTab.instantMessage.empty())
				return;

			const std::string request = "https://127.0.0.1/lol-chat/v1/conversations/" + cid + "/messages";
			const std::string body = R"({"type":"chat", "body":")" + std::string(S.gameTab.instantMessage) + R"("})";

			std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instantMessageDelay));

			now = std::chrono::system_clock::now();
			int numOfSent = 0;
			while (true)
			{
				for (; numOfSent < S.gameTab.instantMessageTimes; numOfSent++)
				{
					if (std::string error = LCU::Request("POST", request, body); error.find("errorCode") != std::string::npos)
					{
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instantMessageDelayTimes));
				}

				if (numOfSent >= S.gameTab.instantMessageTimes)
				{
					return;
				}

				diff = now - start;
				if (diff.count() > 10) // took 10 seconds and still not all messages sent
				{
					return;
				}
			}
		}
	}

	static void AutoAccept()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (FindWindowA("RiotWindowClass", "League of Legends (TM) Client"))
			{
				// game is running, auto accept is not needed
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}

			if (!FindWindowA("RCLIENT", "League of Legends"))
			{
				continue;
			}

			if (S.gameTab.autoAcceptEnabled || (S.gameTab.autoBanEnabled && S.gameTab.autoBanId) ||
				(S.gameTab.dodgeOnBan && S.gameTab.instalockEnabled) ||
				(S.gameTab.instalockEnabled && S.gameTab.instalockId) ||
				!S.gameTab.instantMessage.empty())
			{
				Json::Value rootSearch;
				Json::Value rootChampSelect;
				Json::Value rootSession;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;

				cpr::Session session;
				session.SetVerifySsl(false);
				session.SetHeader(Utils::StringToHeader(LCU::league.header));
				session.SetUrl(std::format("https://127.0.0.1:{}/lol-lobby/v2/lobby/matchmaking/search-state", LCU::league.port));

				std::string getSearchState = session.Get().text;
				if (!reader->parse(getSearchState.c_str(), getSearchState.c_str() + static_cast<int>(getSearchState.length()), &rootSearch, &err))
				{
					continue;
				}

				static bool onChampSelect = true; //false when in champ select
				static int useBackupId = 0;
				static bool isPicked = false;

				if (rootSearch["searchState"].asString() != "Found") // not found, not in champ select
				{
					onChampSelect = true;
					useBackupId = 0;
					isPicked = false;
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					continue;
				}

				session.SetUrl(std::format("https://127.0.0.1:{}/lol-champ-select/v1/session", LCU::league.port));
				std::string getChampSelect = session.Get().text;
				if (getChampSelect.find("RPC_ERROR") != std::string::npos) // game found but champ select error means queue pop
				{
					onChampSelect = true;
					useBackupId = 0;
					isPicked = false;
					if (S.gameTab.autoAcceptEnabled)
					{
						session.SetUrl(std::format("https://127.0.0.1:{}/lol-matchmaking/v1/ready-check/accept", LCU::league.port));
						session.SetBody("");
						session.Post();
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
				else // in champ select
				{
					if (!reader->parse(getChampSelect.c_str(), getChampSelect.c_str() + static_cast<int>(getChampSelect.length()), &rootChampSelect,
					                   &err))
					{
						continue;
					}

					if (onChampSelect)
					{
						onChampSelect = false;

						if (!S.gameTab.instantMessage.empty() || S.gameTab.instantMute || S.gameTab.sideNotification)
						{
							std::thread instantMessageThread(&GameTab::InstantMessage, S.gameTab.instantMute, S.gameTab.sideNotification);
							instantMessageThread.detach();
						}
					}

					if ((S.gameTab.instalockEnabled || S.gameTab.autoBanId) && !isPicked)
					{
						// get own summid
						session.SetUrl(std::format("https://127.0.0.1:{}/lol-login/v1/session", LCU::league.port));
						std::string getSession = session.Get().text;
						if (!reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &rootSession, &err))
						{
							continue;
						}

						const int cellId = rootChampSelect["localPlayerCellId"].asInt();
						for (Json::Value::ArrayIndex j = 0; j < rootChampSelect["actions"].size(); j++)
						{
							auto actions = rootChampSelect["actions"][j];
							if (!actions.isArray())
							{
								continue;
							}
							for (auto& action : actions)
							{
								// search for own actions
								if (action["actorCellId"].asInt() == cellId)
								{
									if (std::string actionType = action["type"].asString(); actionType == "pick" && S.gameTab.instalockId && S.gameTab
										.instalockEnabled)
									{
										// if haven't picked yet
										if (action["completed"].asBool() == false)
										{
											if (!isPicked)
											{
												std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instalockDelay));

												int currentPick = S.gameTab.instalockId;
												if (useBackupId)
													currentPick = useBackupId;

												session.SetUrl(std::format("https://127.0.0.1:{}/lol-champ-select/v1/session/actions/{}",
												                           LCU::league.port,
												                           action["id"].asString()));
												session.SetBody(R"({"completed":true,"championId":)" + std::to_string(currentPick) + "}");
												session.Patch();
											}
										}
										else
										{
											isPicked = true;
										}
									}
									else if (actionType == "ban" && S.gameTab.autoBanId && S.gameTab.autoBanEnabled)
									{
										if (action["completed"].asBool() == false)
										{
											std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.autoBanDelay));

											session.SetUrl(std::format("https://127.0.0.1:{}/lol-champ-select/v1/session/actions/{}",
											                           LCU::league.port,
											                           action["id"].asString()));
											session.SetBody(R"({"completed":true,"championId":)" + std::to_string(S.gameTab.autoBanId) + "}");
											session.Patch();
										}
									}
								}
								// action that isn't our player, if dodge on ban enabled or backup pick
								else if ((S.gameTab.dodgeOnBan || S.gameTab.backupId) && S.gameTab.instalockEnabled && S.gameTab.instalockId)
								{
									if (isPicked)
										break;

									if (action["actorCellId"].asInt() == cellId)
										continue;

									if (action["type"].asString() == "ban" && action["completed"].asBool() == true)
									{
										if (action["championId"].asInt() == S.gameTab.instalockId)
										{
											if (S.gameTab.dodgeOnBan)
											{
												session.SetUrl(
													std::format("https://127.0.0.1:{}", LCU::league.port) +
													R"(/lol-login/v1/session/invoke?destination=lcdsServiceProxy&method=call&args=["","teambuilder-draft","quitV2",""])");
												session.SetBody("");
												session.Post();
											}
											else if (S.gameTab.backupId)
											{
												useBackupId = S.gameTab.backupId;
											}
										}
									}
									else if (action["type"].asString() == "pick" && action["completed"].asBool() == true)
									{
										if (S.gameTab.backupId && action["championId"].asInt() == S.gameTab.instalockId)
										{
											useBackupId = S.gameTab.backupId;
										}
									}
								}
							}
						}
					}
					else // instalock or autoban not enabled, we do nothing in champ select
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
				}
			}
		}
	}

	static std::string MultiSearch(const std::string& website)
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
			Json::Value rootPartcipants;

			std::wstring summNames;
			bool isRanked = false;

			if (reader->parse(champSelect.c_str(), champSelect.c_str() + static_cast<int>(champSelect.length()), &rootCSelect, &err))
			{
				auto teamArr = rootCSelect["myTeam"];
				if (teamArr.isArray())
				{
					for (auto& i : teamArr)
					{
						if (i["nameVisibilityType"].asString() == "HIDDEN")
						{
							isRanked = true;
							break;
						}

						std::string summId = i["summonerId"].asString();
						if (summId != "0")
						{
							std::string summoner = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + summId);
							if (reader->parse(summoner.c_str(), summoner.c_str() + static_cast<int>(summoner.length()), &rootSummoner, &err))
							{
								summNames += Utils::StringToWstring(rootSummoner["internalName"].asString()) + L",";
							}
						}
					}

					//	Ranked Lobby Reveal
					if (isRanked)
					{
						summNames = L"";

						LCU::SetCurrentClientRiotInfo();
						std::string participants = cpr::Get(
							cpr::Url{std::format("https://127.0.0.1:{}/chat/v5/participants/champ-select", LCU::riot.port)},
							cpr::Header{Utils::StringToHeader(LCU::riot.header)}, cpr::VerifySsl{false}).text;
						if (reader->parse(participants.c_str(), participants.c_str() + static_cast<int>(participants.length()), &rootPartcipants,
						                  &err))
						{
							auto participantsArr = rootPartcipants["participants"];
							if (participantsArr.isArray())
							{
								for (auto& i : participantsArr)
								{
									summNames += Utils::StringToWstring(i["name"].asString()) + L",";
								}
							}
						}
					}

					std::wstring region;
					if (website == "U.GG") // platformId (euw1, eun1, na1)
					{
						std::string getAuthorization = LCU::Request("GET", "/lol-rso-auth/v1/authorization");
						if (reader->parse(getAuthorization.c_str(), getAuthorization.c_str() + static_cast<int>(getAuthorization.length()),
						                  &rootRegion, &err))
						{
							region = Utils::StringToWstring(rootRegion["currentPlatformId"].asString());
						}
					}
					else // region code (euw, eune na)
					{
						std::string getRegion = LCU::Request("GET", "/riotclient/region-locale");
						if (reader->parse(getRegion.c_str(), getRegion.c_str() + static_cast<int>(getRegion.length()), &rootRegion, &err))
						{
							region = Utils::StringToWstring(rootRegion["webRegion"].asString());
						}
					}

					if (!region.empty())
					{
						if (summNames.empty())
							return "Failed to get summoner names";

						if (summNames.at(summNames.size() - 1) == L',')
							summNames.pop_back();

						std::wstring url;
						if (website == "OP.GG")
						{
							url = L"https://" + region + L".op.gg/multi/query=" + summNames;
						}
						else if (website == "U.GG")
						{
							url = L"https://u.gg/multisearch?summoners=" + summNames + L"&region=" + Utils::ToLower(region);
						}
						else if (website == "PORO.GG")
						{
							url = L"https://poro.gg/multi?region=" + Utils::ToUpper(region) + L"&q=" + summNames;
						}
						else if (website == "Porofessor.gg")
						{
							url = L"https://porofessor.gg/pregame/" + region + L"/" + summNames;
						}

						ShellExecuteW(nullptr, nullptr, url.c_str(), nullptr, nullptr, SW_SHOW);
						return Utils::WstringToString(url);
					}
					return "Failed to get region";
				}
			}
		}

		return "Champion select not found";
	}

	static std::string ChangeRunesOpgg()
	{
		//std::string champSelect = LCU::Request("GET", "/lol-champ-select/v1/session");
		//if (champSelect.empty() || champSelect.find("RPC_ERROR") != std::string::npos)
		//{
		//	return "Champion select not found";
		//}

		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value rootCurrentPage;
		//Json::Value rootCSelect;

		//if (!reader->parse(champSelect.c_str(), champSelect.c_str() + static_cast<int>(champSelect.length()), &rootCSelect, &err))
		//{
		//	return "Failed to get champion select";
		//}

		std::string currentChampion = LCU::Request("GET", "/lol-champ-select/v1/current-champion");
		if (currentChampion == "0" || currentChampion.empty() || currentChampion.find("RPC_ERROR") != std::string::npos)
		{
			return "Champion not picked";
		}

		std::string currentChampionName;
		for (const auto& [key, name, skins] : champSkins)
		{
			if (std::stoi(currentChampion) == key)
			{
				currentChampionName = name;
				break;
			}
		}

		std::string getCurrentPage = LCU::Request("GET", "/lol-perks/v1/currentpage");
		if (!reader->parse(getCurrentPage.c_str(), getCurrentPage.c_str() + static_cast<int>(getCurrentPage.length()), &rootCurrentPage, &err))
		{
			return "Failed to get current rune page";
		}
		std::string currentPageId = rootCurrentPage["id"].asString();

		std::stringstream ssOpgg(cpr::Get(cpr::Url{"https://www.op.gg/champions/" + Utils::ToLower(currentChampionName)}).text);
		std::vector<std::string> runes;
		std::string primaryPerk, secondaryPerk;

		std::string buf;
		while (ssOpgg >> buf)
		{
			if (runes.size() == 9)
				break;

			if (buf.find("src=\"https://opgg-static.akamaized.net/images/lol/perk") != std::string::npos
				|| buf.find("src=\"https://opgg-static.akamaized.net/meta/images/lol/perk") != std::string::npos)
			{
				if (buf.find("grayscale") != std::string::npos)
					continue;

				if (buf.find("/perkStyle/") != std::string::npos)
				{
					buf = buf.substr(buf.find("/perkStyle/") + strlen("/perkStyle/"), 4);
					if (primaryPerk.empty())
						primaryPerk = buf;
					else if (secondaryPerk.empty())
						secondaryPerk = buf;
				}
				else if (buf.find("/perk/") != std::string::npos)
				{
					buf = buf.substr(buf.find("/perk/") + strlen("/perk/"), 4);
					runes.emplace_back(buf);
				}
				else if (buf.find("/perkShard/") != std::string::npos)
				{
					buf = buf.substr(buf.find("/perkShard/") + strlen("/perkShard/"), 4);
					runes.emplace_back(buf);
				}
			}
		}
		if (runes.size() != 9 || primaryPerk.empty() || secondaryPerk.empty())
		{
			return "Failed to fetch op.gg runes";
		}

		LCU::Request("DELETE", "/lol-perks/v1/pages/" + currentPageId);

		Json::Value rootPage;
		rootPage["name"] = currentChampionName + " OP.GG";
		rootPage["primaryStyleId"] = primaryPerk;
		rootPage["subStyleId"] = secondaryPerk;
		rootPage["selectedPerkIds"] = Json::Value(Json::arrayValue);
		for (const std::string& rune : runes)
			rootPage["selectedPerkIds"].append(rune);
		rootPage["current"] = true;

		return LCU::Request("POST", "lol-perks/v1/pages", rootPage.toStyledString());
	}

	static std::string ChampIdToName(const int& id)
	{
		for (const auto& [key, name, skins] : champSkins)
		{
			if (id == key)
			{
				return name;
			}
		}
		return "";
	}

	// TODO: rewrite and move these to config file
	static std::string GetOldJWT(std::string accId, int& oldtimestamp)
	{
		char* pRoaming;
		size_t roamingLen;
		[[maybe_unused]] errno_t err = _dupenv_s(&pRoaming, &roamingLen, "APPDATA");
		std::string roaming = pRoaming;
		std::string filePath = roaming + "\\tempar.json";

		std::string oldJWT;

		std::fstream file(filePath, std::ios_base::in);
		if (file.good())
		{
			std::string config;
			std::string temp;
			while (std::getline(file, temp))
			{
				config += temp + "\n";
			}

			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING local_err;

			if (reader->parse(config.c_str(), config.c_str() + static_cast<int>(config.length()), &root, &local_err))
			{
				if (auto t = root[accId]; !t.empty())
				{
					if (auto t2 = root[accId]["time"]; !t2.empty())
						oldtimestamp = t2.asUInt();
					if (auto t2 = root[accId]["jwt"]; !t2.empty())
						oldJWT = t2.asString();
					file.close();
					return oldJWT;
				}
			}
		}
		file.close();
	}

	// true if need new jwt
	static bool CheckJWT(std::string accId)
	{
		char* pRoaming;
		size_t roamingLen;
		[[maybe_unused]] errno_t err = _dupenv_s(&pRoaming, &roamingLen, "APPDATA");
		std::string roaming = pRoaming;
		std::string filePath = roaming + "\\tempar.json";
		unsigned timestamp = 0;

		std::fstream file(filePath, std::ios_base::in);
		if (file.good())
		{
			std::string config;
			std::string temp;
			while (std::getline(file, temp))
			{
				config += temp + "\n";
			}

			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING local_err;

			if (reader->parse(config.c_str(), config.c_str() + static_cast<int>(config.length()), &root, &local_err))
			{
				if (auto t = root[accId]; !t.empty())
				{
					std::string oldJWT; // CppEntityAssignedButNoRead
					if (auto t2 = root[accId]["time"]; !t2.empty())
						timestamp = t2.asUInt();
					if (auto t2 = root[accId]["jwt"]; !t2.empty())
						oldJWT = t2.asString();
				}
				else
					return true;
			}
		}
		file.close();

		// if old timestamp is still valid
		if (timestamp + 60 * 60 * 24 > time(nullptr))
		{
			return false;
		}
		return true;
	}

#pragma warning ( push )
#pragma warning (disable : 4996)
	static void SaveJWT(std::string accId, std::string jwt, unsigned timestamp)
	{
		char* pRoaming;
		size_t roamingLen;
		[[maybe_unused]] errno_t err = _dupenv_s(&pRoaming, &roamingLen, "APPDATA");
		std::string roaming = pRoaming;
		std::string filePath = roaming + "\\tempar.json";
		// if file doesn't exist, create new one with {} so it can be parsed
		if (!std::filesystem::exists(filePath))
		{
			std::ofstream file(filePath);
			file << "{}";
			file.close();
		}

		Json::Reader reader;
		Json::Value root;

		std::ifstream iFile(filePath);
		if (iFile.good())
		{
			if (reader.parse(iFile, root, false))
			{
				root[accId]["jwt"] = jwt;
				root[accId]["time"] = timestamp;

				if (!root.toStyledString().empty())
				{
					std::ofstream oFile(filePath);
					oFile << root.toStyledString() << std::endl;
					oFile.close();
				}
			}
		}
		iFile.close();
	}
#pragma warning( pop )
};
