#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "Utils.h"
#include "LCU.h"
#include "Misc.h"
#include "Config.h"

#pragma warning(disable : 4996)

class LoginTab
{
	static void LoginOnClientOpen(const std::string& username, const std::string& password)
	{
		while (true)
		{
			if (FindWindowA("RCLIENT", "Riot Client") && LCU::riot.port != 0)
			{
				// waits to be sure that client is fully loaded
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				Login(username, password);
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	static std::string Login(std::string username, std::string password)
	{
		// If riot client not open
		if (LCU::riot.port == 0)
		{
			if (std::filesystem::exists(S.leaguePath))
			{
				Misc::LaunchClient("");
				std::thread t(LoginOnClientOpen, username, password);
				t.detach();
				return "Launching client...";
			}
			return "Invalid client path, change it in Settings tab";
		}

		cpr::Session session;
		session.SetVerifySsl(false);
		session.SetHeader(Utils::StringToHeader(LCU::riot.header));
		session.SetUrl(std::format("https://127.0.0.1:{}/rso-auth/v2/authorizations", LCU::riot.port));
		session.SetBody(R"({"clientId":"riot-client","trustLevels":["always_trusted"]})");
		session.Post();
		// refresh session

		std::string loginBody = R"({"username":")" + username + R"(","password":")" + password + R"(","persistLogin":false})";
		session.SetUrl(std::format("https://127.0.0.1:{}/rso-auth/v1/session/credentials", LCU::riot.port));
		session.SetBody(loginBody);
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
	static void Render()
	{
		if (ImGui::BeginTabItem("Login"))
		{
			static std::string result;
			static bool once = true;
			static char leagueArgs[1024 * 16];
			static std::string sArgs;

			if (once)
			{
				once = false;
				std::ranges::copy(S.loginTab.leagueArgs, leagueArgs);
			}

			ImGui::Columns(2, nullptr, false);

			static std::vector<std::pair<std::string, std::string>> langs = {
				{"English (US)", "en_US"}, {"Japanese", "ja_JP"}, {"Korean", "ko_KR"}, {"Chinese (China)", "zh_CN"},
				{"German", "de_DE"}, {"Spanish (Spain)", "es_ES"}, {"Polish", "pl_PL"}, {"Russian", "ru_RU"},
				{"French", "fr_FR"}, {"Turkish", "tr_TR"}, {"Portuguese", "pt_BR"}, {"Czech", "cs_CZ"}, {"Greek", "el_GR"},
				{"Romanian", "ro_RO"}, {"Hungarian", "hu_HU"}, {"English (UK)", "en_GB"}, {"Italian", "it_IT"},
				{"Spanish (Mexico)", "es_MX"}, {"Spanish (Argentina)", "es_AR"}, {"English (Australia)", "en_AU"},
				{"Malay", "ms_MY"}, {"English (Philippines)", "en_PH"}, {"English (Singapore)", "en_SG"}, {"Thai", "th_TH"},
				{"Vietnamese", "vi_VN"}, {"Indonesian", "id_ID"}, {"Tagalog", "tl_PH"}, {"Chinese (Malaysia)", "zh_MY"}, {"Chinese (Taiwan)", "zh_TW"}
			};
			// find saved lang from cfg file
			auto findLang = std::ranges::find_if(langs, [](std::pair<std::string, std::string> k) {
				return k.second == S.loginTab.language;
			});

			static std::pair selectedLang = {findLang[0].first, findLang[0].second};

			if (ImGui::Button("Launch client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchClient(sArgs);
					result = S.leaguePath + "LeagueClient.exe " + sArgs;
				}
			}
			ImGui::SameLine();

			if (ImGui::BeginCombo("##language", selectedLang.first.c_str()))
			{
				for (const auto& [fst, snd] : langs)
				{
					if (ImGui::Selectable(fst.c_str(), fst == selectedLang.first))
					{
						selectedLang = {fst, snd};
						S.loginTab.language = snd;
						Config::Save();

						std::string localeArg = std::format("--locale={} ", selectedLang.second);
						size_t localePos = sArgs.find("--locale=");
						if (localePos != std::string::npos)
						{
							sArgs.replace(localePos, localeArg.size(), localeArg);
						}
						else
							sArgs += localeArg;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchLegacyClient();
				}
			}
			ImGui::Columns(1);

			std::ranges::copy(sArgs, leagueArgs);
			ImGui::Text(" Args: ");
			ImGui::SameLine();
			ImGui::InputText("##inputLeagueArgs", leagueArgs, IM_ARRAYSIZE(leagueArgs));

			sArgs = leagueArgs;
			S.loginTab.leagueArgs = sArgs;

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
					result = Login(username, password);
			}

			ImGui::SameLine();
			if (ImGui::Button("Save") && !std::string(username).empty() && !std::string(password).empty())
			{
				// if file doesn't exist, create new one with {} so it can be parsed
				if (!std::filesystem::exists(S.settingsFile))
				{
					std::ofstream file(S.settingsFile);
					file << "{}";
					file.close();
				}

				std::ifstream iFile(S.settingsFile);

				if (iFile.good())
				{
					Json::Value root;
					if (Json::Reader reader; reader.parse(iFile, root, false))
					{
						if (!root["accounts"].isArray())
							root["accounts"] = Json::Value(Json::arrayValue);
						Json::Value accArray = root["accounts"];

						accArray.append(std::format("{0}:{1}", username, password));
						root["accounts"] = accArray;

						std::ofstream oFile(S.settingsFile);
						oFile << root.toStyledString() << std::endl;
						oFile.close();
					}
				}
				iFile.close();
			}

			ImGui::SameLine();
			ImGui::HelpMarker(
				"This part is only, if you want to save your login and pass to config file and login with 1 click. You don't have to do that, you can just log in the usual way in client and launch the tool anytime you want");

			ImGui::SameLine();
			ImGui::Columns(2, nullptr, false);
			ImGui::NextColumn();

			static std::string banCheck;

			if (ImGui::Button("Check ban reason"))
			{
				Json::Value authData;
				authData["acr_values"] = "";
				authData["claims"] = "";
				authData["client_id"] = "riot-client";
				authData["nonce"] = Utils::RandomString(22);
				authData["code_challenge"] = "";
				authData["code_challenge_method"] = "";
				authData["redirect_uri"] = "http://localhost/redirect";
				authData["response_type"] = "token id_token";
				authData["scope"] = "openid offline_access lol ban profile email phone birthdate summoner link lol_region";

				cpr::Header authHeader = {
					{"Content-Type", "application/json"},
					{"Accept-Encoding", "deflate"},
					{"User-Agent", "RiotClient/69.0.3.228.1352 rso-auth (Windows;10;;Home, x64)"},
					{"Pragma", "no-cache"},
					{"Accept-Language", "en-GB,en,*"},
					{"Accept", "application/json, text/plain, */*"}
				};

				cpr::Session session;
				session.SetHeader(authHeader);

				std::string valoApi = cpr::Get(cpr::Url{"https://valorant-api.com/v1/version"}).text;

				std::regex regexStr("\"riotClientBuild\":\"(.*?)\"");
				if (std::smatch m; std::regex_search(valoApi, m, regexStr))
				{
					session.UpdateHeader(cpr::Header{{"User-Agent", "RiotClient/" + m[1].str() + " rso-auth (Windows;10;;Home, x64)"}});
				}

				session.SetBody(authData.toStyledString());
				session.SetUrl("https://auth.riotgames.com/api/v1/authorization");
				session.Post();

				Json::Value authData2;
				authData2["language"] = "en_GB";
				authData2["password"] = password;
				authData2["region"] = Json::nullValue;
				authData2["remember"] = false;
				authData2["type"] = "auth";
				authData2["username"] = username;

				session.SetBody(authData2.toStyledString());
				session.SetUrl("https://auth.riotgames.com/api/v1/authorization");
				std::string r = session.Put().text;

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value rootAuth;
				banCheck = r;
				if (r.find("\"error\"") == std::string::npos && reader->parse(r.c_str(), r.c_str() + static_cast<int>(r.length()), &rootAuth, &err))
				{
					std::string uri = rootAuth["response"]["parameters"]["uri"].asString();
					size_t startIndex = uri.find("#access_token=") + strlen("#access_token=");
					size_t endIndex = uri.find("&scope");
					std::string bearer = uri.substr(startIndex, endIndex - startIndex);
					session.UpdateHeader(cpr::Header{{"Authorization", "Bearer " + bearer}});

					session.SetUrl("https://auth.riotgames.com/userinfo");
					r = session.Get().text;
					Json::Value rootInfo;

					if (reader->parse(r.c_str(), r.c_str() + static_cast<int>(r.length()), &rootInfo, &err))
					{
						banCheck = rootInfo.toStyledString();
						std::cout << rootInfo.toStyledString();
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Get email"))
			{
				cpr::Session session;
				cpr::Header authHeader = {
					{"Content-Type", "application/json"},
					{"Accept-Encoding", "deflate"},
					{"Upgrade-Insecure-Requests", "1"},
					{"sec-ch-ua", R"("Not/A)Brand";v="99", "Google Chrome";v="115", "Chromium";v="115")"},
					{"sec-ch-ua-platform", "\"Windows\""},
					{"sec-ch-ua-mobile", "?0"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"},
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
				session.SetHeader(authHeader);

				session.SetUrl(
					"https://auth.riotgames.com/authorize?redirect_uri=https://login.playersupport.riotgames.com/login_callback&client_id=player-support-zendesk&ui_locales=en-us%20en-us&response_type=code&scope=openid%20email");
				session.Get();

				Json::Value authData2;
				authData2["language"] = "en_GB";
				authData2["password"] = password;
				authData2["region"] = Json::nullValue;
				authData2["remember"] = false;
				authData2["type"] = "auth";
				authData2["username"] = username;

				session.SetBody(authData2.toStyledString());
				session.SetUrl("https://auth.riotgames.com/api/v1/authorization");
				std::string r = session.Put().text;

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value rootAuth;
				banCheck = r;
				if (r.find("\"error\"") == std::string::npos && reader->parse(r.c_str(), r.c_str() + static_cast<int>(r.length()), &rootAuth, &err))
				{
					session.SetUrl(rootAuth["response"]["parameters"]["uri"].asString());
					session.Get().text; // UnusedValue

					session.SetUrl("https://support-leagueoflegends.riotgames.com/hc/en-us/requests");
					std::string support = session.Get().text;
					std::regex regexStr("\"email\":(.*?),");
					if (std::smatch m; std::regex_search(support, m, regexStr))
					{
						banCheck = m.str();
					}
				}
			}

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);

			std::ifstream iFile(S.settingsFile);

			if (iFile.good())
			{
				Json::Value root;
				if (Json::Reader reader; reader.parse(iFile, root, false))
				{
					if (auto accArray = root["accounts"]; accArray.isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < accArray.size(); ++i)
						{
							std::string acc = accArray[i].asString();
							std::string accUsername = acc.substr(0, acc.find(':'));
							std::string accPassword = acc.substr(acc.find(':') + 1);
							if (ImGui::Button(accUsername.c_str()))
							{
								result = Login(accUsername, accPassword);
							}

							ImGui::SameLine();
							std::string deleteButton = "Delete##" + acc;
							if (ImGui::Button(deleteButton.c_str()))
							{
								std::ofstream oFile(S.settingsFile);
								accArray.removeIndex(i, nullptr);
								root["accounts"] = accArray;
								oFile << root.toStyledString() << std::endl;
								oFile.close();
							}
						}
					}
				}
			}
			iFile.close();

			ImGui::NextColumn();

			ImGui::TextWrapped(banCheck.c_str()); // PotentiallyInsecureFormatSecurity

			ImGui::Columns(1);

			ImGui::TextWrapped(result.c_str()); // PotentiallyInsecureFormatSecurity

			ImGui::EndTabItem();
		}
	}
};
