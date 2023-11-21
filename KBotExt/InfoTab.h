#pragma once

#include "Includes.h"
#include "Utils.h"
#include "LCU.h"
#include "Config.h"

class InfoTab
{
public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Info"))
		{
			static bool once = true;
			static std::string result;
			static bool bPressed = false;

			static std::string accID;
			static std::string summID;
			static std::string summName;
			static std::string gameName;
			static std::string tagLine;

			static char playerName[50];
			if (once)
			{
				once = false;
				std::ranges::copy(S.infoTab.playerName, playerName);
			}

			ImGui::Text("Input player name:");
			ImGui::InputText("##inputPlayerName", playerName, IM_ARRAYSIZE(playerName));
			S.infoTab.playerName = playerName;
			ImGui::SameLine();

			if (ImGui::Button("Submit##playerName") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
			{
				std::string tempName = std::string(playerName);
				tempName.erase(std::remove_if(tempName.begin(), tempName.end(),
					[](unsigned char x) { return std::isspace(x); }), tempName.end());

				size_t found;
				while ((found = tempName.find("#")) != std::string::npos)
				{
					tempName.replace(found, 1, "%23");
				}

				result = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners?name=" + tempName);

				// 2nd method
				/*auto riotId = Utils::StringSplit(tempName, "#");
				if (riotId.size() > 1)
				{
					result = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/alias/lookup?gameName=" + riotId[0] + "&tagLine=" + riotId[1]);

					Json::CharReaderBuilder builder;
					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					JSONCPP_STRING err;
					Json::Value root;
					if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
					{
						std::string puuid = root["puuid"].asString();
						if (puuid != "")
						{
							result = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners-by-puuid-cached/" + puuid);
						}
					}
				}*/

				bPressed = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("puuid##playerName"))
			{
				result = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners-by-puuid-cached/" + std::string(playerName));
				bPressed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("summId##playerName"))
			{
				result = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + std::string(playerName));
				bPressed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Me##playerName"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				std::string mySummId;

				std::string getSession = LCU::Request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(getSession.c_str(), getSession.c_str() + static_cast<int>(getSession.length()), &root, &err))
				{
					mySummId = root["summonerId"].asString();
				}

				result = LCU::Request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + mySummId);
				bPressed = true;
			}

			static Json::StreamWriterBuilder wBuilder;
			static std::string sResultJson;
			static char* cResultJson;

			if (bPressed)
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
					sResultJson = result;
				else
				{
					accID = std::to_string(root["accountId"].asUInt64());
					summID = std::to_string(root["summonerId"].asUInt64());
					summName = root["internalName"].asString();
					gameName = root["gameName"].asString();
					tagLine = root["tagLine"].asString();

					sResultJson = Json::writeString(wBuilder, root); // "CppRedundantQualifier"
				}

				//accID = std::to_string(root["accountId"].asUInt64()).c_str();
				//ImGui::Text("accountId: %s", accID.c_str());
				//ImGui::TextWrapped("displayName: %s", root["displayName"].asString().c_str());
				//summName = root["internalName"].asString().c_str();
				//ImGui::TextWrapped("internalName: %s", summName.c_str());
				//ImGui::Text("nameChangeFlag: %d", root["nameChangeFlag"].asBool());
				//ImGui::Text("percentCompleteForNextLevel: %d", root["percentCompleteForNextLevel"].asInt());
				//ImGui::Text("profileIconId: %d", root["profileIconId"].asInt());
				//ImGui::Text("puuid: %s", root["puuid"].asString().c_str());
				//summID = std::to_string(root["summonerId"].asUInt64()).c_str();;
				//ImGui::Text("summonerId: %d", summID);
				//ImGui::Text("summonerLevel: %d", root["summonerLevel"].asInt());
				//ImGui::Text("unnamed: %d", root["unnamed"].asBool());
				//ImGui::Text("xpSinceLastLevel: %d", root["xpSinceLastLevel"].asInt());
				//ImGui::Text("xpUntilNextLevel: %d", root["xpUntilNextLevel"].asInt());

				//auto& rerollPointsObj = root["rerollPoints"];
				//ImGui::Text("currentPoints: %d", rerollPointsObj["currentPoints"].asInt());
				//ImGui::Text("maxRolls: %d", rerollPointsObj["maxRolls"].asInt());
				//ImGui::Text("numberOfRolls: %d", rerollPointsObj["numberOfRolls"].asInt());
				//ImGui::Text("pointsCostToRoll: %d", rerollPointsObj["pointsCostToRoll"].asInt());
				//ImGui::Text("pointsToReroll: %d", rerollPointsObj["pointsToReroll"].asInt());
			}

			if (!sResultJson.empty())
			{
				cResultJson = sResultJson.data();
				ImGui::InputTextMultiline("##infoResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 350));
			}

			if (ImGui::Button("Invite to lobby##infoTab"))
			{
				std::string invite = R"([{"toSummonerId":)" + accID + R"(}])";
				LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/invitations", invite);
				invite = R"([{"toSummonerId":)" + summID + R"(}])";
				LCU::Request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/invitations", invite);
			}
			ImGui::SameLine();
			if (ImGui::Button("Invite to friends##infoTab"))
			{
				std::string invite = R"({"gameName":")" + gameName + R"(","tagLine":")" + tagLine + R"("})";
				LCU::Request("POST", "https://127.0.0.1/lol-chat/v2/friend-requests", invite);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add to block list##infoTab"))
			{
				std::string body = R"({ "summonerId":)" + summID + "}";
				LCU::Request("POST", "https://127.0.0.1/lol-chat/v1/blocked-players", body);
			}

			if (ImGui::Button("Copy to clipboard##infoTab"))
			{
				Utils::CopyToClipboard(sResultJson);
			}

			ImGui::EndTabItem();
		}
	}
};
