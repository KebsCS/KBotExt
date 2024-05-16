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
			const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);

			ImGui::InputTextMultiline("##inputStatus", statusText, IM_ARRAYSIZE(statusText), ImVec2(S.Window.width - 230.f,
				(label_size.y + ImGui::GetStyle().FramePadding.y) * 6.f), ImGuiInputTextFlags_AllowTabInput);
			if (ImGui::Button("Submit status"))
			{
				std::string body = R"({"statusMessage":")" + std::string(statusText) + "\"}";

				size_t nPos = 0;
				while (nPos != std::string::npos)
				{
					nPos = body.find('\n', nPos);
					if (nPos != std::string::npos)
					{
						body.erase(body.begin() + nPos);
						body.insert(nPos, "\\n");
					}
				}
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
				if (result.find("errorCode") != std::string::npos)
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
			}

			ImGui::SameLine();
			static int availability = 0;
			static int lastAvailability = 0;
			ImGui::RadioButton("Online", &availability, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Mobile", &availability, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Away", &availability, 2);
			ImGui::SameLine();
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
				default:;
				}
				body += "\"}";
				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
			}

			ImGui::SameLine();
			ImGui::Text("\t   ");
			ImGui::SameLine();
			if (ImGui::Button("Disconnect from chat"))
			{
				LCU::SetCurrentClientRiotInfo();
				cpr::Post(cpr::Url{ std::format("https://127.0.0.1:{}/chat/v1/suspend", LCU::riot.port) },
					cpr::Body{ "{\"config\":\"disable\"}" },
					cpr::Header{ Utils::StringToHeader(LCU::riot.header) }, cpr::VerifySsl{ false });
			}

			ImGui::SameLine();
			if (ImGui::Button("Reconnect to chat"))
			{
				LCU::SetCurrentClientRiotInfo();
				cpr::Post(cpr::Url{ std::format("https://127.0.0.1:{}/chat/v1/resume", LCU::riot.port) },
					cpr::Header{ Utils::StringToHeader(LCU::riot.header) }, cpr::VerifySsl{ false });
			}

			ImGui::Separator();

			ImGui::Text("Rank:");
			static int rank = 0;
			ImGui::RadioButton("Iron", &rank, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Bronze", &rank, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Silver", &rank, 2);
			ImGui::SameLine();
			ImGui::RadioButton("Gold", &rank, 3);
			ImGui::SameLine();
			ImGui::RadioButton("Platinum", &rank, 4);
			ImGui::SameLine();
			ImGui::RadioButton("Emerald", &rank, 5);
			ImGui::SameLine();
			ImGui::RadioButton("Diamond", &rank, 6);
			ImGui::SameLine();
			ImGui::RadioButton("Master", &rank, 7);
			ImGui::SameLine();
			ImGui::RadioButton("GM", &rank, 8);
			ImGui::SameLine();
			ImGui::RadioButton("Challenger", &rank, 9);

			static int tier = 0;
			ImGui::RadioButton("I", &tier, 0);
			ImGui::SameLine();
			ImGui::RadioButton("II", &tier, 1);
			ImGui::SameLine();
			ImGui::RadioButton("III", &tier, 2);
			ImGui::SameLine();
			ImGui::RadioButton("IV", &tier, 3);
			ImGui::SameLine();
			ImGui::RadioButton("None", &tier, 4);

			static int queue = 0;
			ImGui::RadioButton("Solo/Duo", &queue, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Flex 5v5", &queue, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Flex 3v3", &queue, 2);
			ImGui::SameLine();
			ImGui::RadioButton("TFT", &queue, 3);
			ImGui::SameLine();
			ImGui::RadioButton("Hyper Roll", &queue, 4);
			ImGui::SameLine();
			ImGui::RadioButton("Double Up", &queue, 5);
			ImGui::SameLine();
			ImGui::RadioButton("Arena", &queue, 6);
			ImGui::SameLine();
			ImGui::RadioButton("()", &queue, 7);

			ImGui::Columns(2, nullptr, false);

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
					body += "CHERRY";
					break;
				case 7:
					body += "";
					break;
				default:;
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
					body += "EMERALD";
					break;
				case 6:
					body += "DIAMOND";
					break;
				case 7:
					body += "MASTER";
					break;
				case 8:
					body += "GRANDMASTER";
					break;
				case 9:
					body += "CHALLENGER";
					break;
				default:;
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
				default:;
				}

				body += R"("}})";

				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
			}

			ImGui::SameLine();
			if (ImGui::Button("Empty##emptyRank"))
			{
				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me",
					R"({"lol":{"rankedLeagueQueue":"","rankedLeagueTier":"","rankedLeagueDivision":""}})");
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Only in the friend's list, not on profile");

			ImGui::NextColumn();

			if (ImGui::Button("Invisible banner"))
			{
				std::string playerP = GetPlayerPreferences();
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(playerP.c_str(), playerP.c_str() + static_cast<int>(playerP.length()), &root, &err))
				{
					root["bannerAccent"] = "2";
					LCU::Request("POST", "/lol-challenges/v1/update-player-preferences/", root.toStyledString());
				}
			}

			ImGui::SameLine();
			ImGui::HelpMarker("Works if last season's rank is unranked");

			ImGui::Columns(1);

			ImGui::Separator();

			static int sendChangeBadges = -99;
			ImGui::Text("Challenge badges:");
			ImGui::SameLine();

			if (ImGui::Button("Empty"))
			{
				sendChangeBadges = -1;
			}

			ImGui::SameLine();

			if (ImGui::Button("Copy 1st to all 3"))
			{
				sendChangeBadges = -2;
			}

			ImGui::SameLine();
			ImGui::Text("Glitched:");
			ImGui::SameLine();

			if (ImGui::Button("0##glitchedBadges"))
			{
				sendChangeBadges = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("1##glitchedBadges"))
			{
				sendChangeBadges = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("2##glitchedBadges"))
			{
				sendChangeBadges = 2;
			}
			ImGui::SameLine();
			if (ImGui::Button("3##glitchedBadges"))
			{
				sendChangeBadges = 3;
			}
			ImGui::SameLine();
			if (ImGui::Button("4##glitchedBadges"))
			{
				sendChangeBadges = 4;
			}
			ImGui::SameLine();
			if (ImGui::Button("5##glitchedBadges"))
			{
				sendChangeBadges = 5;
			}

			if (sendChangeBadges != -99)
			{
				std::string playerP = GetPlayerPreferences();
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(playerP.c_str(), playerP.c_str() + static_cast<int>(playerP.length()), &root, &err))
				{
					Json::Value jsonArray;
					if (sendChangeBadges != -1)
					{
						for (size_t i = 0; i < 3; i++)
						{
							if (sendChangeBadges == -2)
							{
								if (root["challengeIds"].isArray() && !root["challengeIds"].empty())
								{
									jsonArray.append(root["challengeIds"][0]);
								}
							}
							else
							{
								jsonArray.append(sendChangeBadges);
							}
						}
					}
					root["challengeIds"] = jsonArray;
					LCU::Request("POST", "/lol-challenges/v1/update-player-preferences/", root.toStyledString());
				}
				sendChangeBadges = -99;
			}

			ImGui::Separator();

			ImGui::Text("Challenge Points Text: ");

			ImGui::SameLine();
			static char bufChallengePoints[100];
			static size_t lastSize = 0;

			ImGui::SetNextItemWidth(static_cast<float>(S.Window.width / 3));
			ImGui::InputText("##inputChallengePoints", bufChallengePoints, IM_ARRAYSIZE(bufChallengePoints));

			if (lastSize != strlen(bufChallengePoints))
			{
				lastSize = strlen(bufChallengePoints);
				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me",
					R"({"lol":{"challengePoints":")" + std::string(bufChallengePoints) + R"("}})");
			}

			ImGui::SameLine();

			if (ImGui::Button("Set Challenges Rank"))
			{
				std::string body = R"({"lol":{"challengeCrystalLevel":")";
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
					body += "EMERALD";
					break;
				case 6:
					body += "DIAMOND";
					break;
				case 7:
					body += "MASTER";
					break;
				case 8:
					body += "GRANDMASTER";
					break;
				case 9:
					body += "CHALLENGER";
					break;
				default:;
				}

				LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me", body + R"("}})");
			}

			ImGui::SameLine();

			ImGui::HelpMarker("Select the rank using radio buttons above, then press this button.");

			ImGui::Separator();
			static int masteryLvl;
			ImGui::Text("Mastery:");
			ImGui::InputInt("##inputMasteryLvl:", &masteryLvl, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitMasteryLvl"))
			{
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me",
					R"({"lol":{"masteryScore":")" + std::to_string(masteryLvl) + "\"}}");
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
				}
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Shown on splash art when hovered over in friend's list");

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
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Submit 2##submitIcon2"))
			{
				std::string body = R"({"icon":)" + std::to_string(iconID) + "}";
				std::string result = LCU::Request("PUT", "https://127.0.0.1/lol-chat/v1/me/", body);
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
				}
			}

			/*static int backgroundID;
			ImGui::Text("Background:");

			ImGui::InputInt("##inputBackground", &backgroundID, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitBackground"))
			{
				std::string body = R"({"key":"backgroundSkinId","value":)" + std::to_string(backgroundID) + "}";
				std::string result = LCU::Request("POST", "https://127.0.0.1/lol-summoner/v1/current-summoner/summoner-profile/", body);
			}*/

			if (ImGui::CollapsingHeader("Backgrounds"))
			{
				if (champSkins.empty())
				{
					ImGui::Text("Skin data is still being fetched");
				}
				else
				{
					for (const auto& [key, name, skins] : champSkins)
					{
						if (ImGui::TreeNode(name.c_str()))
						{
							for (const auto& [fst, snd] : skins)
							{
								if (ImGui::Button(snd.c_str()))
								{
									std::string body = R"({"key":"backgroundSkinId","value":)" + fst + "}";
									std::string result = LCU::Request("POST", "https://127.0.0.1/lol-summoner/v1/current-summoner/summoner-profile/",
										body);
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

	static std::string GetPlayerPreferences()
	{
		std::string challengesData = LCU::Request("GET", "/lol-challenges/v1/summary-player-data/local-player");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(challengesData.c_str(), challengesData.c_str() + static_cast<int>(challengesData.length()), &root, &err))
		{
			std::string titleId = root["title"]["itemId"].asString();
			std::string bannerId = root["bannerId"].asString();

			std::string result = "{";
			for (Json::Value::ArrayIndex i = 0; i < root["topChallenges"].size(); i++)
			{
				if (i == 0)
					result += "\"challengeIds\":[";

				result += root["topChallenges"][i]["id"].asString();

				if (i != root["topChallenges"].size() - 1)
					result += ",";
				else
					result += "]";
			}

			if (titleId != "-1")
			{
				if (result.size() != 1)
					result += ",";
				result += R"("title":")" + titleId + "\"";
			}

			if (!bannerId.empty())
			{
				if (result.size() != 1)
					result += ",";
				result += R"("bannerAccent":")" + bannerId + "\"";
			}
			result += "}";

			return result;
		}
		return "";
	}
};
