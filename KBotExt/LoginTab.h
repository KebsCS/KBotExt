#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "Utils.h"
#include "LCU.h"
#include "Misc.h"
#include "Config.h"

#pragma warning(disable : 4996)

class login_tab
{
	static void login_on_client_open(const std::string& username, const std::string& password)
	{
		while (true)
		{
			if (FindWindowA("RCLIENT", "Riot Client") && lcu::riot.port != 0)
			{
				// waits to be sure that client is fully loaded
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				login(username, password);
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	static std::string login(std::string username, std::string password)
	{
		// If riot client not open
		if (lcu::riot.port == 0)
		{
			if (std::filesystem::exists(s.league_path))
			{
				misc::launch_client("");
				std::thread t(login_on_client_open, username, password);
				t.detach();
				return "Launching client...";
			}
			return "Invalid client path, change it in Settings tab";
		}

		cpr::Session session;
		session.SetVerifySsl(false);
		session.SetHeader(utils::string_to_header(lcu::riot.header));
		session.SetUrl(std::format("https://127.0.0.1:{}/rso-auth/v2/authorizations", lcu::riot.port));
		session.SetBody(R"({"clientId":"riot-client","trustLevels":["always_trusted"]})");
		session.Post();
		// refresh session

		std::string login_body = R"({"username":")" + username + R"(","password":")" + password +
			R"(","persistLogin":false})";
		session.SetUrl(std::format("https://127.0.0.1:{}/rso-auth/v1/session/credentials", lcu::riot.port));
		session.SetBody(login_body);
		std::string result = session.Put().text;

		const Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			if (!root["type"].empty())
				return root["type"].asString();
			if (!root["message"].empty())
				return root["message"].asString();
		}
		return result;
	}

public:
	static void render()
	{
		if (ImGui::BeginTabItem("Login"))
		{
			static std::string result;
			static bool once = true;
			static char league_args[1024 * 16];
			static std::string s_args;

			if (once)
			{
				once = false;
				std::ranges::copy(s.login_tab.league_args, league_args);
			}

			ImGui::Columns(2, nullptr, false);

			static std::vector<std::pair<std::string, std::string>> langs = {
				{"English (US)", "en_US"}, {"Japanese", "ja_JP"}, {"Korean", "ko_KR"}, {"Chinese (China)", "zh_CN"},
				{"German", "de_DE"}, {"Spanish (Spain)", "es_ES"}, {"Polish", "pl_PL"}, {"Russian", "ru_RU"},
				{"French", "fr_FR"}, {"Turkish", "tr_TR"}, {"Portuguese", "pt_BR"}, {"Czech", "cs_CZ"},
				{"Greek", "el_GR"},
				{"Romanian", "ro_RO"}, {"Hungarian", "hu_HU"}, {"English (UK)", "en_GB"}, {"Italian", "it_IT"},
				{"Spanish (Mexico)", "es_MX"}, {"Spanish (Argentina)", "es_AR"}, {"English (Australia)", "en_AU"},
				{"Malay", "ms_MY"}, {"English (Philippines)", "en_PH"}, {"English (Singapore)", "en_SG"},
				{"Thai", "th_TH"},
				{"Vietnamese", "vi_VN"}, {"Indonesian", "id_ID"}, {"Tagalog", "tl_PH"}, {"Chinese (Malaysia)", "zh_MY"},
				{"Chinese (Taiwan)", "zh_TW"}
			};

			auto find_lang = std::ranges::find_if(langs, [](const std::pair<std::string, std::string>& k) {
				return k.second == s.login_tab.language;
			});

			static std::pair selected_lang = {find_lang[0].first, find_lang[0].second};

			if (ImGui::Button("Launch client"))
			{
				if (!std::filesystem::exists(s.league_path))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					misc::launch_client(s_args);
					result = s.league_path + "LeagueClient.exe " + s_args;
				}
			}
			ImGui::SameLine();

			if (ImGui::BeginCombo("##language", selected_lang.first.c_str()))
			{
				for (const auto& [fst, snd] : langs)
				{
					if (ImGui::Selectable(fst.c_str(), fst == selected_lang.first))
					{
						selected_lang = {fst, snd};
						s.login_tab.language = snd;
						config::save();

						std::string locale_arg = std::format("--locale={} ", selected_lang.second);
						if (size_t locale_pos = s_args.find("--locale="); locale_pos != std::string::npos)
						{
							s_args.replace(locale_pos, locale_arg.size(), locale_arg);
						}
						else
							s_args += locale_arg;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(s.league_path))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					misc::launch_legacy_client();
				}
			}
			ImGui::Columns(1);

			std::ranges::copy(s_args, league_args);
			ImGui::Text(" Args: ");
			ImGui::SameLine();
			ImGui::InputText("##inputLeagueArgs", league_args, IM_ARRAYSIZE(league_args));

			s_args = league_args;
			s.login_tab.league_args = s_args;

			ImGui::Separator();

			static char username[128];
			ImGui::Text("Username:");
			ImGui::InputText("##inputUsername", username, IM_ARRAYSIZE(username));

			static char password[128];
			ImGui::Text("Password:");
			ImGui::InputText("##inputPassword", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

			if (ImGui::Button("Login") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
			{
				if (!std::string(username).empty() && !std::string(password).empty())
					result = login(username, password);
			}

			ImGui::SameLine();
			if (ImGui::Button("Save") && !std::string(username).empty() && !std::string(password).empty())
			{
				if (!std::filesystem::exists(s.settings_file))
				{
					std::ofstream file(s.settings_file);
					file << "{}";
					file.close();
				}

				std::ifstream i_file(s.settings_file);

				if (i_file.good())
				{
					Json::Value root;
					if (Json::Reader reader; reader.parse(i_file, root, false))
					{
						if (!root["accounts"].isArray())
							root["accounts"] = Json::Value(Json::arrayValue);
						Json::Value acc_array = root["accounts"];

						acc_array.append(std::format("{0}:{1}", username, password));
						root["accounts"] = acc_array;

						std::ofstream o_file(s.settings_file);
						o_file << root.toStyledString() << std::endl;
						o_file.close();
					}
				}
				i_file.close();
			}

			ImGui::SameLine();
			ImGui::help_marker(
				"This part is only, if you want to save your login and pass to config file and login with 1 click. You don't have to do that, you can just log in the usual way in client and launch the tool anytime you want");

			ImGui::SameLine();
			ImGui::Columns(2, nullptr, false);
			ImGui::NextColumn();

			static std::string ban_check = "";

			if (ImGui::Button("Check ban reason"))
			{
				Json::Value auth_data;
				auth_data["acr_values"] = "";
				auth_data["claims"] = "";
				auth_data["client_id"] = "riot-client";
				auth_data["nonce"] = utils::random_string(22);
				auth_data["code_challenge"] = "";
				auth_data["code_challenge_method"] = "";
				auth_data["redirect_uri"] = "http://localhost/redirect";
				auth_data["response_type"] = "token id_token";
				auth_data["scope"] =
					"openid offline_access lol ban profile email phone birthdate summoner link lol_region";

				cpr::Header auth_header = {
					{"Content-Type", "application/json"},
					{"Accept-Encoding", "deflate"},
					{"User-Agent", "RiotClient/69.0.3.228.1352 rso-auth (Windows;10;;Home, x64)"},
					{"Pragma", "no-cache"},
					{"Accept-Language", "en-GB,en,*"},
					{"Accept", "application/json, text/plain, */*"}
				};

				cpr::Session session;
				session.SetHeader(auth_header);

				std::string valo_api = Get(cpr::Url{"https://valorant-api.com/v1/version"}).text;

				std::regex regex_str("\"riotClientBuild\":\"(.*?)\"");
				if (std::smatch m; std::regex_search(valo_api, m, regex_str))
				{
					session.UpdateHeader(cpr::Header{
						{"User-Agent", "RiotClient/" + m[1].str() + " rso-auth (Windows;10;;Home, x64)"}
					});
				}

				session.SetBody(auth_data.toStyledString());
				session.SetUrl("https://auth.riotgames.com/api/v1/authorization");
				session.Post();

				Json::Value auth_data2;
				auth_data2["language"] = "en_GB";
				auth_data2["password"] = password;
				auth_data2["region"] = Json::nullValue;
				auth_data2["remember"] = false;
				auth_data2["type"] = "auth";
				auth_data2["username"] = username;

				session.SetBody(auth_data2.toStyledString());
				session.SetUrl("https://auth.riotgames.com/api/v1/authorization");
				std::string r = session.Put().text;

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root_auth;
				ban_check = r;
				if (r.find("\"error\"") == std::string::npos && reader->parse(
					r.c_str(), r.c_str() + static_cast<int>(r.length()), &root_auth, &err))
				{
					std::string uri = root_auth["response"]["parameters"]["uri"].asString();
					size_t start_index = uri.find("#access_token=") + strlen("#access_token=");
					size_t end_index = uri.find("&scope");
					std::string bearer = uri.substr(start_index, end_index - start_index);
					session.UpdateHeader(cpr::Header{{"Authorization", "Bearer " + bearer}});

					session.SetUrl("https://auth.riotgames.com/userinfo");
					r = session.Get().text;
					Json::Value root_info;

					if (reader->parse(r.c_str(), r.c_str() + static_cast<int>(r.length()), &root_info, &err))
					{
						ban_check = root_info.toStyledString();
						std::cout << root_info.toStyledString();
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Get email"))
			{
				cpr::Session session;
				cpr::Header auth_header = {
					{"Content-Type", "application/json"},
					{"Accept-Encoding", "deflate"},
					{"Upgrade-Insecure-Requests", "1"},
					{"sec-ch-ua", R"("Not/A)Brand";v="99", "Google Chrome";v="115", "Chromium";v="115")"},
					{"sec-ch-ua-platform", "\"Windows\""},
					{"sec-ch-ua-mobile", "?0"},
					{
						"User-Agent",
						"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"
					},
					{"Sec-Fetch-Site", "cross-site"},
					{"Sec-Fetch-Mode", "navigate"},
					{"Sec-Fetch-Dest", "document"},
					{"Accept-Language", "en-US,en;q=0.9"},
					{
						"Accept",
						"text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7"
					},
					{"Referer", "https://riotgames.zendesk.com/"},
				};
				session.SetHeader(auth_header);

				session.SetUrl(
					"https://auth.riotgames.com/authorize?redirect_uri=https://login.playersupport.riotgames.com/login_callback&client_id=player-support-zendesk&ui_locales=en-us%20en-us&response_type=code&scope=openid%20email");
				session.Get();

				Json::Value auth_data2;
				auth_data2["language"] = "en_GB";
				auth_data2["password"] = password;
				auth_data2["region"] = Json::nullValue;
				auth_data2["remember"] = false;
				auth_data2["type"] = "auth";
				auth_data2["username"] = username;

				session.SetBody(auth_data2.toStyledString());
				session.SetUrl("https://auth.riotgames.com/api/v1/authorization");
				std::string r = session.Put().text;

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root_auth;
				ban_check = r;
				if (r.find("\"error\"") == std::string::npos && reader->parse(
					r.c_str(), r.c_str() + static_cast<int>(r.length()), &root_auth, &err))
				{
					session.SetUrl(root_auth["response"]["parameters"]["uri"].asString());
					session.Get().text;

					session.SetUrl("https://support-leagueoflegends.riotgames.com/hc/en-us/requests");
					std::string support = session.Get().text;
					std::regex regex_str("\"email\":(.*?),");
					if (std::smatch m; std::regex_search(support, m, regex_str))
					{
						ban_check = m.str();
					}
				}
			}

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);

			std::ifstream i_file(s.settings_file);

			if (i_file.good())
			{
				Json::Value root;
				if (Json::Reader reader; reader.parse(i_file, root, false))
				{
					if (auto acc_array = root["accounts"]; acc_array.isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < acc_array.size(); ++i)
						{
							std::string acc = acc_array[i].asString();
							std::string acc_username = acc.substr(0, acc.find(":"));
							std::string acc_password = acc.substr(acc.find(":") + 1);
							if (ImGui::Button(acc_username.c_str()))
							{
								result = login(acc_username, acc_password);
							}

							ImGui::SameLine();
							std::string delete_button = "Delete##" + acc;
							if (ImGui::Button(delete_button.c_str()))
							{
								std::ofstream o_file(s.settings_file);
								acc_array.removeIndex(i, nullptr);
								root["accounts"] = acc_array;
								o_file << root.toStyledString() << std::endl;
								o_file.close();
							}
						}
					}
				}
			}
			i_file.close();

			ImGui::NextColumn();

			ImGui::TextWrapped(ban_check.c_str());

			ImGui::Columns(1);

			ImGui::TextWrapped(result.c_str());

			ImGui::EndTabItem();
		}
	}
};
