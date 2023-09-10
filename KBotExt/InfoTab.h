#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "Utils.h"
#include "LCU.h"
#include "Config.h"

class info_tab
{
public:
	static void render()
	{
		if (ImGui::BeginTabItem("Info"))
		{
			static bool once = true;
			static std::string result;
			static bool b_pressed = false;

			static std::string acc_id;
			static std::string summ_id;
			static std::string summ_name;

			static char player_name[50];
			if (once)
			{
				once = false;
				std::ranges::copy(s.info_tab.player_name, player_name);
			}

			ImGui::Text("Input player name:");
			ImGui::InputText("##inputPlayerName", player_name, IM_ARRAYSIZE(player_name));
			s.info_tab.player_name = player_name;
			ImGui::SameLine();

			if (ImGui::Button("Submit##playerName") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
			{
				result = lcu::request(
					"GET", "https://127.0.0.1/lol-summoner/v1/summoners?name=" + std::string(player_name));
				b_pressed = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("puuid##playerName"))
			{
				result = lcu::request(
					"GET", "https://127.0.0.1/lol-summoner/v1/summoners-by-puuid-cached/" + std::string(player_name));
				b_pressed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("summId##playerName"))
			{
				result = lcu::request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + std::string(player_name));
				b_pressed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Me##playerName"))
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;

				std::string my_summ_id;

				std::string get_session = lcu::request("GET", "https://127.0.0.1/lol-login/v1/session");
				if (reader->parse(get_session.c_str(), get_session.c_str() + static_cast<int>(get_session.length()), &root,
				                  &err))
				{
					my_summ_id = root["summonerId"].asString();
				}

				result = lcu::request("GET", "https://127.0.0.1/lol-summoner/v1/summoners/" + my_summ_id);
				b_pressed = true;
			}

			static Json::StreamWriterBuilder w_builder;
			static std::string s_result_json;
			static char* c_result_json;

			if (b_pressed)
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
					s_result_json = result;
				else
				{
					acc_id = std::to_string(root["accountId"].asUInt64());
					summ_id = std::to_string(root["summonerId"].asUInt64());
					summ_name = root["internalName"].asString();

					s_result_json = writeString(w_builder, root);
				}
			}

			if (!s_result_json.empty())
			{
				c_result_json = &s_result_json[0];
				ImGui::InputTextMultiline("##infoResult", c_result_json, s_result_json.size() + 1, ImVec2(600, 350));
			}

			if (ImGui::Button("Invite to lobby##infoTab"))
			{
				std::string invite = R"([{"toSummonerId":)" + acc_id + R"(}])";
				lcu::request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/invitations", invite);
				invite = R"([{"toSummonerId":)" + summ_id + R"(}])";
				lcu::request("POST", "https://127.0.0.1/lol-lobby/v2/lobby/invitations", invite);
			}
			ImGui::SameLine();
			if (ImGui::Button("Invite to friends##infoTab"))
			{
				std::string invite = R"({"name":")" + summ_name + R"("})";
				lcu::request("POST", "https://127.0.0.1/lol-chat/v1/friend-requests", invite);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add to block list##infoTab"))
			{
				std::string body = R"({ "summonerId":)" + summ_id + "}";
				lcu::request("POST", "https://127.0.0.1/lol-chat/v1/blocked-players", body);
			}

			if (ImGui::Button("Copy to clipboard##infoTab"))
			{
				utils::copy_to_clipboard(s_result_json);
			}

			ImGui::EndTabItem();
		}
	}
};
