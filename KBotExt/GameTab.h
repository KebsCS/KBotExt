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

			if (ImGui::Button("TFT Double Up"))
				gameID = TFTDoubleUp;

			ImGui::NextColumn();

			if (ImGui::Button("TFT Tutorial"))
				gameID = TFTTutorial;

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
				if (botChamps.empty())
				{
					MessageBoxA(0, "Pick the bot's champion first", "Adding bots failed", MB_OK);
				}
				else
				{
					std::string team = botTeam ? R"(,"teamId":"200"})" : R"(,"teamId":"100"})";
					std::string body = R"({"botDifficulty":")" + difficulties[indexDifficulty] + R"(","championId":)" + std::to_string(botChamps[indexBots].first) + team;
					result = LCU::Request("POST", "https://127.0.0.1/lol-lobby/v1/lobby/custom/bots", body);
				}
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

			ImGui::Columns(2, 0, false);
			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 7));
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
					"{\"firstPreference\":\"" + firstPosition[S.gameTab.indexFirstRole]
					+ "\",\"secondPreference\":\"" + secondPosition[S.gameTab.indexSecondRole] + "\"}");
			}
			ImGui::SameLine();
			ImGui::HelpMarker("If you are already in a lobby you can use this button to pick the roles, or start a new lobby with the buttons above");

			ImGui::NextColumn();

			if (ImGui::Button("Change runes"))
			{
				result = ChangeRunesOpgg();
			}

			ImGui::SameLine();
			ImGui::Columns(2, 0, false);

			ImGui::Checkbox("Blue/Red Side notification", &S.gameTab.sideNotification);

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
				if (MessageBoxA(0, "Are you sure?", "Refunding last purchase", MB_OKCANCEL) == IDOK)
				{
					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value rootPurchaseHistory;

					std::string storeHeader = LCU::GetStoreHeader();
					std::string storeUrl = LCU::Request("GET", "/lol-store/v1/getStoreUrl");
					storeUrl.erase(std::remove(storeUrl.begin(), storeUrl.end(), '"'), storeUrl.end());

					std::string purchaseHistory = HTTP::Request("GET", storeUrl + "/storefront/v3/history/purchase", "", storeHeader);
					if (reader->parse(purchaseHistory.c_str(), purchaseHistory.c_str() + static_cast<int>(purchaseHistory.length()), &rootPurchaseHistory, &err))
					{
						std::string accountId = rootPurchaseHistory["player"]["accountId"].asString();
						std::string transactionId = rootPurchaseHistory["transactions"][0]["transactionId"].asString();
						result = HTTP::Request("POST", storeUrl + "/storefront/v3/refund",
							"{\"accountId\":" + accountId + ",\"transactionId\":\"" + transactionId + "\",\"inventoryType\":\"CHAMPION\",\"language\":\"EN_US\"}",
							storeHeader);
					}
					else
					{
						result = purchaseHistory;
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

			static bool isStillBeingFetched = true;
			if (!champSkins.empty())
				isStillBeingFetched = false;

			static std::string chosenInstalock = "Instalock champ \t\tChosen: " + Misc::ChampIdToName(S.gameTab.instalockId) + "###AnimatedInstalock";
			static int lastInstalockId = 0;
			if ((lastInstalockId != S.gameTab.instalockId) && !isStillBeingFetched)
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

			ImGui::Columns(3, 0, false);

			ImGui::Checkbox("Auto ban", &S.gameTab.autoBanEnabled);
			ImGui::NextColumn();

			ImGui::SliderInt("Delay##sliderautoBanDelay", &S.gameTab.autoBanDelay, 0, 10000, "%d ms");

			ImGui::NextColumn();

			ImGui::Checkbox("Instant Mute", &S.gameTab.instantMute);

			ImGui::Columns(1);

			static std::string chosenAutoban = "Auto ban\t\t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.autoBanId) + "###AnimatedAutoban";
			static int lastAutoban = 0;
			if ((lastAutoban != S.gameTab.autoBanId) && !isStillBeingFetched)
			{
				lastAutoban = S.gameTab.autoBanId;
				chosenAutoban = "Auto ban\t\t\t\tChosen: " + Misc::ChampIdToName(S.gameTab.autoBanId) + "###AnimatedAutoban";
			}
			if (ImGui::CollapsingHeader(chosenAutoban.c_str()))
			{
				if (champSkins.empty())
				{
					ImGui::Text("Champion data is still being fetched");
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
					if (root[i]["freeToPlay"].asBool() == true || root[i]["ownership"]["owned"].asBool() == true ||
						(root[i]["ownership"].isMember("xboxGPReward") && root[i]["ownership"]["xboxGPReward"].asBool() == true))
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
			std::string getChat = HTTP::Request("GET", "https://127.0.0.1/chat/v5/participants/champ-select", "",
				LCU::riot.header, "", "", LCU::riot.port);
			if (!reader->parse(getChat.c_str(), getChat.c_str() + static_cast<int>(getChat.length()), &root, &err))
			{
				continue;
			}

			const auto participantsArr = root["participants"];
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
									std::format("{{\"summonerId\":{0},\"puuid\":\"{1}\",\"obfuscatedSummonerId\":{2},\"obfuscatedPuuid\":\"{3}\"}}",
										player["summonerId"].asString(), player["puuid"].asString(), player["obfuscatedSummonerId"].asString(),
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
							if (rootCSelect["myTeam"].isArray() && rootCSelect["myTeam"].size() > 0)
							{
								std::string notif = "You are on the ";
								if (rootCSelect["myTeam"][0]["team"].asInt() == 1)
									notif += "Blue Side";
								else
									notif += "Red Side";
								LCU::Request("POST", std::format("/lol-chat/v1/conversations/{}/messages", cid),
									"{\"body\":\"" + notif + "\",\"type\":\"celebration\"}");
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
					std::string error = LCU::Request("POST", request, body);
					if (error.find("errorCode") != std::string::npos)
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

				std::string getSearchState = LCU::Request("GET", "https://127.0.0.1/lol-lobby/v2/lobby/matchmaking/search-state");
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

				std::string getChampSelect = LCU::Request("GET", "https://127.0.0.1/lol-champ-select/v1/session");
				if (getChampSelect.find("RPC_ERROR") != std::string::npos) // game found but champ select error means queue pop
				{
					onChampSelect = true;
					useBackupId = 0;
					isPicked = false;
					if (S.gameTab.autoAcceptEnabled)
					{
						LCU::Request("POST", "https://127.0.0.1/lol-matchmaking/v1/ready-check/accept", "");
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}
				else // in champ select
				{
					if (!reader->parse(getChampSelect.c_str(), getChampSelect.c_str() + static_cast<int>(getChampSelect.length()), &rootChampSelect, &err))
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
						std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
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
											if (!isPicked)
											{
												std::this_thread::sleep_for(std::chrono::milliseconds(S.gameTab.instalockDelay));

												int currentPick = S.gameTab.instalockId;
												if (useBackupId)
													currentPick = useBackupId;

												LCU::Request("PATCH", "https://127.0.0.1/lol-champ-select/v1/session/actions/" + actions[i]["id"].asString(),
													R"({"completed":true,"championId":)" + std::to_string(currentPick) + "}");
											}
										}
										else
										{
											isPicked = true;
										}
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
								}
								// action that isn't our player, if dodge on ban enabled or backup pick
								else if ((S.gameTab.dodgeOnBan || S.gameTab.backupId) && S.gameTab.instalockEnabled && S.gameTab.instalockId)
								{
									if (isPicked)
										break;

									if (actions[i]["actorCellId"].asInt() == cellId)
										continue;

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
									else if (actions[i]["type"].asString() == "pick" && actions[i]["completed"].asBool() == true)
									{
										if (S.gameTab.backupId && actions[i]["championId"].asInt() == S.gameTab.instalockId)
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

			std::wstring summNames = L"";
			bool isRanked = false;

			if (reader->parse(champSelect.c_str(), champSelect.c_str() + static_cast<int>(champSelect.length()), &rootCSelect, &err))
			{
				auto teamArr = rootCSelect["myTeam"];
				if (teamArr.isArray())
				{
					for (Json::Value::ArrayIndex i = 0; i < teamArr.size(); ++i)
					{
						if (teamArr[i]["nameVisibilityType"].asString() == "HIDDEN")
						{
							isRanked = true;
							break;
						}

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

					//	Ranked Lobby Reveal
					if (isRanked)
					{
						summNames = L"";

						LCU::SetCurrentClientRiotInfo();
						std::string participants = HTTP::Request("GET", "https://127.0.0.1/chat/v5/participants/champ-select", "",
							LCU::riot.header, "", "", LCU::riot.port);
						if (reader->parse(participants.c_str(), participants.c_str() + static_cast<int>(participants.length()), &rootPartcipants, &err))
						{
							auto participantsArr = rootPartcipants["participants"];
							if (participantsArr.isArray())
							{
								for (Json::Value::ArrayIndex i = 0; i < participantsArr.size(); ++i)
								{
									summNames += Utils::StringToWstring(participantsArr[i]["name"].asString()) + L",";
								}
							}
						}
					}

					std::wstring region;
					if (website == "U.GG") // platformId (euw1, eun1, na1)
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

						ShellExecuteW(0, 0, url.c_str(), 0, 0, SW_SHOW);
						return Utils::WstringToString(url);
					}
					else
						return "Failed to get region";
				}
			}
		}

		return "Champion select not found";
	}

	static std::string ChangeRunesOpgg()
	{
		/*	std::string champSelect = LCU::Request("GET", "/lol-champ-select/v1/session");
			if (champSelect.empty() || champSelect.find("RPC_ERROR") != std::string::npos)
			{
				return "Champion select not found";
			}*/

		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value rootCurrentPage;
		//Json::Value rootCSelect;
		//
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
		for (const auto& c : champSkins)
		{
			if (std::stoi(currentChampion) == c.key)
			{
				currentChampionName = c.name;
				break;
			}
		}

		std::string getCurrentPage = LCU::Request("GET", "/lol-perks/v1/currentpage");
		if (!reader->parse(getCurrentPage.c_str(), getCurrentPage.c_str() + static_cast<int>(getCurrentPage.length()), &rootCurrentPage, &err))
		{
			return "Failed to get current rune page";
		}
		std::string currentPageId = rootCurrentPage["id"].asString();

		std::stringstream ssOpgg(HTTP::Request("GET", "https://www.op.gg/champions/" + Utils::ToLower(currentChampionName)));
		std::vector<std::string>runes;
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
};