#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"

class profile_tab
{
public:
	static void render()
	{
		if (ImGui::BeginTabItem("Profile"))
		{
			static char status_text[1024 * 16];
			ImGui::Text("Status:");
			const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);

			ImGui::InputTextMultiline("##inputStatus", (status_text), IM_ARRAYSIZE(status_text), ImVec2(
				                          s.window.width - 230.f,
				                          (label_size.y + ImGui::GetStyle().FramePadding.y) * 6.f),
			                          ImGuiInputTextFlags_AllowTabInput);
			if (ImGui::Button("Submit status"))
			{
				std::string body = R"({"statusMessage":")" + std::string(status_text) + "\"}";

				size_t n_pos = 0;
				while (n_pos != std::string::npos)
				{
					n_pos = body.find('\n', n_pos);
					if (n_pos != std::string::npos)
					{
						body.erase(body.begin() + n_pos);
						body.insert(n_pos, "\\n");
					}
				}
				std::string result = lcu::request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
				if (result.find("errorCode") != std::string::npos)
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
			}

			ImGui::SameLine();
			static int availability = 0;
			static int last_availability = 0;
			ImGui::RadioButton("Online", &availability, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Mobile", &availability, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Away", &availability, 2);
			ImGui::SameLine();
			ImGui::RadioButton("Offline", &availability, 3);

			if (availability != last_availability)
			{
				last_availability = availability;
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
				default: ;
				}
				body += "\"}";
				lcu::request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
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
			ImGui::RadioButton("Diamond", &rank, 5);
			ImGui::SameLine();
			ImGui::RadioButton("Master", &rank, 6);
			ImGui::SameLine();
			ImGui::RadioButton("GrandMaster", &rank, 7);
			ImGui::SameLine();
			ImGui::RadioButton("Challenger", &rank, 8);

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
			ImGui::RadioButton("()", &queue, 6);

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
					body += "";
					break;
				default: ;
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
				default: ;
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
				default: ;
				}

				body += R"("}})";

				lcu::request("PUT", "https://127.0.0.1/lol-chat/v1/me", body);
			}

			ImGui::SameLine();
			if (ImGui::Button("Empty##emptyRank"))
			{
				lcu::request("PUT", "https://127.0.0.1/lol-chat/v1/me",
				             R"({"lol":{"rankedLeagueQueue":"","rankedLeagueTier":"","rankedLeagueDivision":""}})");
			}
			ImGui::SameLine();
			ImGui::help_marker("Only in the friend's list, not on profile");

			ImGui::NextColumn();

			if (ImGui::Button("Invisible banner"))
			{
				std::string player_p = get_player_preferences();
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(player_p.c_str(), player_p.c_str() + static_cast<int>(player_p.length()), &root, &err))
				{
					root["bannerAccent"] = "2";
					lcu::request("POST", "/lol-challenges/v1/update-player-preferences/", root.toStyledString());
				}
			}

			ImGui::SameLine();
			ImGui::help_marker("Works if last season's rank is unranked");

			ImGui::Columns(1);

			ImGui::Separator();

			static int send_change_badges = -99;
			ImGui::Text("Challenge badges:");
			ImGui::SameLine();

			if (ImGui::Button("Empty"))
			{
				send_change_badges = -1;
			}

			ImGui::SameLine();

			if (ImGui::Button("Copy 1st to all 3"))
			{
				send_change_badges = -2;
			}

			ImGui::SameLine();
			ImGui::Text("Glitched:");
			ImGui::SameLine();

			if (ImGui::Button("0##glitchedBadges"))
			{
				send_change_badges = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("1##glitchedBadges"))
			{
				send_change_badges = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("2##glitchedBadges"))
			{
				send_change_badges = 2;
			}
			ImGui::SameLine();
			if (ImGui::Button("3##glitchedBadges"))
			{
				send_change_badges = 3;
			}
			ImGui::SameLine();
			if (ImGui::Button("4##glitchedBadges"))
			{
				send_change_badges = 4;
			}
			ImGui::SameLine();
			if (ImGui::Button("5##glitchedBadges"))
			{
				send_change_badges = 5;
			}

			if (send_change_badges != -99)
			{
				std::string player_p = get_player_preferences();
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(player_p.c_str(), player_p.c_str() + static_cast<int>(player_p.length()), &root, &err))
				{
					Json::Value json_array;
					if (send_change_badges != -1)
					{
						for (size_t i = 0; i < 3; i++)
						{
							if (send_change_badges == -2)
							{
								if (root["challengeIds"].isArray() && !root["challengeIds"].empty())
								{
									json_array.append(root["challengeIds"][0]);
								}
							}
							else
							{
								json_array.append(send_change_badges);
							}
						}
					}
					root["challengeIds"] = json_array;
					lcu::request("POST", "/lol-challenges/v1/update-player-preferences/", root.toStyledString());
				}
				send_change_badges = -99;
			}

			ImGui::Separator();
			static int masteryLvl;
			ImGui::Text("Mastery:");
			ImGui::InputInt("##inputMasteryLvl:", &masteryLvl, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitMasteryLvl"))
			{
				std::string result = lcu::request("PUT", "https://127.0.0.1/lol-chat/v1/me",
				                                  R"({"lol":{"masteryScore":")" + std::to_string(
					                                  masteryLvl) + "\"}}");
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
				}
			}
			ImGui::SameLine();
			ImGui::help_marker("Shown on splash art when hovered over in friend's list");

			ImGui::Separator();

			static int icon_id;
			ImGui::Text("Icon:");
			ImGui::InputInt("##inputIcon:", &icon_id, 1, 100);
			ImGui::SameLine();
			if (ImGui::Button("Submit##submitIcon"))
			{
				std::string body = R"({"profileIconId":)" + std::to_string(icon_id) + "}";
				std::string result = lcu::request("PUT", "https://127.0.0.1/lol-summoner/v1/current-summoner/icon",
				                                  body);
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Submit 2##submitIcon2"))
			{
				std::string body = R"({"icon":)" + std::to_string(icon_id) + "}";
				std::string result = lcu::request("PUT", "https://127.0.0.1/lol-chat/v1/me/", body);
				if (result.find("errorCode") != std::string::npos)
				{
					MessageBoxA(nullptr, result.c_str(), nullptr, 0);
				}
			}

			if (ImGui::CollapsingHeader("Backgrounds"))
			{
				if (champ_skins.empty())
				{
					ImGui::Text("Skin data is still being fetched");
				}
				else
				{
					for (const auto& [key, name, skins] : champ_skins)
					{
						if (ImGui::TreeNode(name.c_str()))
						{
							for (const auto& [fst, snd] : skins)
							{
								if (ImGui::Button(snd.c_str()))
								{
									std::string body = R"({"key":"backgroundSkinId","value":)" + fst + "}";
									std::string result = lcu::request(
										"POST", "https://127.0.0.1/lol-summoner/v1/current-summoner/summoner-profile/",
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

	static std::string get_player_preferences()
	{
		std::string challenges_data = lcu::request("GET", "/lol-challenges/v1/summary-player-data/local-player");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(challenges_data.c_str(), challenges_data.c_str() + static_cast<int>(challenges_data.length()),
		                  &root, &err))
		{
			std::string title_id = root["title"]["itemId"].asString();
			std::string banner_id = root["bannerId"].asString();

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

			if (title_id != "-1")
			{
				if (result.size() != 1)
					result += ",";
				result += R"("title":")" + title_id + "\"";
			}

			if (banner_id != "")
			{
				if (result.size() != 1)
					result += ",";
				result += R"("bannerAccent":")" + banner_id + "\"";
			}
			result += "}";

			return result;
		}
		return "";
	}
};
