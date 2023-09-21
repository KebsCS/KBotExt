#pragma once

#include "Includes.h"
#include "LCU.h"

class CustomTab
{
public:
	static void Custom()
	{
		static bool once = true;

		static char method[50];
		static char urlText[1024 * 16];
		static char requestText[1024 * 32];

		static char inputPort[64] = "";
		static char inputHeader[1024 * 16];

		static std::string customHeader = LCU::league.header;
		static int customPort = LCU::league.port;

		static bool isCustomOpen = false;

		static std::string ledgeUrl;
		static std::string storeUrl;
		static std::string localhostUrl = "https://127.0.0.1";

		if (once)
		{
			once = false;
			std::ranges::copy(S.customTab.method, method);
			std::ranges::copy(S.customTab.urlText, urlText);
			std::ranges::copy(S.customTab.requestText, requestText);
			std::ranges::copy(S.customTab.port, inputPort);
			std::ranges::copy(S.customTab.header, inputHeader);
		}

		if (onOpen)
		{
			customHeader = LCU::league.header;
			customPort = LCU::league.port;

			ledgeUrl = GetLedgeUrl();
			storeUrl = LCU::Request("GET", "/lol-store/v1/getStoreUrl");
			std::erase(storeUrl, '"');
		}

		ImGui::Text("Method:");
		const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);
		ImGui::InputTextEx("##inputMethod", nullptr, method, IM_ARRAYSIZE(method),
		                   ImVec2(S.Window.width - 130.f, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f), 0, nullptr, nullptr);

		ImGui::Text("URL:");
		ImGui::InputTextMultiline("##inputUrl", urlText, IM_ARRAYSIZE(urlText),
		                          ImVec2(S.Window.width - 130.f, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f));

		ImGui::Text("Body:");
		ImGui::InputTextMultiline("##inputBody", (requestText), IM_ARRAYSIZE(requestText), ImVec2(S.Window.width - 130.f,
		                                                                                          (label_size.y + ImGui::GetStyle().FramePadding.y) *
		                                                                                          6.f), ImGuiInputTextFlags_AllowTabInput);

		S.customTab.method = method;
		S.customTab.urlText = urlText;
		S.customTab.requestText = requestText;

		if (ImGui::CollapsingHeader("Custom Port/Header"))
		{
			isCustomOpen = true;
			ImGui::Text("Port:");
			ImGui::InputText("##inputPort", inputPort, 64, ImGuiInputTextFlags_CharsDecimal);

			ImGui::SameLine();

			if (ImGui::Button("LCU"))
			{
				if (strlen(urlText) == 0 || strcmp(urlText, localhostUrl.c_str()) == 0
					|| strcmp(urlText, storeUrl.c_str()) == 0 || strcmp(urlText, ledgeUrl.c_str()) == 0)
				{
					std::strcpy(urlText, localhostUrl.c_str());
				}
				std::strcpy(inputPort, std::to_string(LCU::league.port).c_str());
				std::strcpy(inputHeader, LCU::league.header.c_str());
			}
			ImGui::SameLine();

			if (ImGui::Button("Riot"))
			{
				LCU::SetCurrentClientRiotInfo();
				if (strlen(urlText) == 0 || strcmp(urlText, localhostUrl.c_str()) == 0
					|| strcmp(urlText, storeUrl.c_str()) == 0 || strcmp(urlText, ledgeUrl.c_str()) == 0)
				{
					std::strcpy(urlText, localhostUrl.c_str());
				}
				std::strcpy(inputPort, std::to_string(LCU::riot.port).c_str());
				std::strcpy(inputHeader, LCU::riot.header.c_str());
			}
			ImGui::SameLine();

			if (ImGui::Button("Store"))
			{
				std::string storeHeader = LCU::GetStoreHeader();
				if (storeHeader != "")
				{
					if (strlen(method) != 0)
					{
						std::strcpy(method, Utils::ToUpper(std::string(method)).c_str());
					}
					if (strlen(urlText) == 0 || strcmp(urlText, localhostUrl.c_str()) == 0
						|| strcmp(urlText, storeUrl.c_str()) == 0 || strcmp(urlText, ledgeUrl.c_str()) == 0)
					{
						storeUrl = LCU::Request("GET", "/lol-store/v1/getStoreUrl");
						std::erase(storeUrl, '"');
						std::strcpy(urlText, storeUrl.c_str());
					}
					std::strcpy(inputPort, "443");
					std::strcpy(inputHeader, storeHeader.c_str());
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Ledge"))
			{
				std::string ledgeHeader;

				ledgeUrl = GetLedgeUrl();
				if (!ledgeUrl.empty())
				{
					std::string ledgeHost;
					if (auto n = ledgeUrl.find("https://"); n != std::string::npos)
					{
						ledgeHost = ledgeUrl.substr(n + strlen("https://"));
					}
					ledgeHeader += "Host: " + ledgeHost + "\r\n";

					if (strlen(urlText) == 0 || strcmp(urlText, localhostUrl.c_str()) == 0
						|| strcmp(urlText, storeUrl.c_str()) == 0 || strcmp(urlText, ledgeUrl.c_str()) == 0)
					{
						std::strcpy(urlText, ledgeUrl.c_str());
					}
				}

				std::string sessionToken = LCU::Request("GET", "/lol-league-session/v1/league-session-token");
				std::erase(sessionToken, '\"');

				ledgeHeader += "Accept-Encoding: deflate, "/*gzip, */"zstd\r\n";
				ledgeHeader += "user-agent: LeagueOfLegendsClient/" + LCU::league.version + "\r\n";
				ledgeHeader += "Authorization: Bearer " + sessionToken + "\r\n";
				ledgeHeader += "Content-type: application/json\r\n";
				ledgeHeader += "Accept: application/json\r\n";

				if (strlen(method) != 0)
				{
					std::strcpy(method, Utils::ToUpper(std::string(method)).c_str());
				}

				std::strcpy(inputPort, "443");
				std::strcpy(inputHeader, ledgeHeader.c_str());
			}

			ImGui::Text("Header:");
			ImGui::InputTextMultiline("##inputHeader", (inputHeader), IM_ARRAYSIZE(inputHeader), ImVec2(S.Window.width - 130.f,
				                          (label_size.y + ImGui::GetStyle().FramePadding.y) * 6.f), ImGuiInputTextFlags_AllowTabInput);

			S.customTab.port = inputPort;
			S.customTab.header = inputHeader;

			if (!S.customTab.port.empty())
				customPort = std::stoi(S.customTab.port);
			else
				customPort = 443;

			customHeader = S.customTab.header;
		}
		else
		{
			isCustomOpen = false;
			customHeader = LCU::league.header;
			customPort = LCU::league.port;
		}

		ImGui::Columns(2, nullptr, false);

		static std::string result;
		if (ImGui::Button("Send custom request##customTab"))
		{
			auto sURL = std::string(urlText);

			if (sURL.find("https://127.0.0.1") == std::string::npos && !isCustomOpen)
			{
				if (sURL.find("https://") == std::string::npos && sURL.find("http://") == std::string::npos)
				{
					while (sURL[0] == ' ' || sURL[0] == '\n')
						sURL.erase(sURL.begin());
					if (sURL[0] != '/')
						sURL.insert(0, "/");
					sURL.insert(0, "https://127.0.0.1:" + std::to_string(LCU::league.port));
				}
			}
			else if (sURL.find("https://127.0.0.1:") == std::string::npos && !isCustomOpen)
			{
				sURL.insert(strlen("https://127.0.0.1"), ":" + std::to_string(LCU::league.port));
			}
			else if (sURL.find("https://") != std::string::npos || sURL.find("https://") != std::string::npos)
			{
				if (customPort != 443 && customPort != 80)
				{
					sURL.insert(sURL.find("/", strlen("https://")), ":" + std::to_string(customPort));
				}
			}

			cpr::Session customSession;
			customSession.SetVerifySsl(false);
			customSession.SetHeader(Utils::StringToHeader(customHeader));
			customSession.SetBody(requestText);
			customSession.SetUrl(sURL);

			cpr::Response r;

			const std::string upperMethod = Utils::ToUpper(method);
			if (upperMethod == "GET")
			{
				r = customSession.Get();
			}
			else if (upperMethod == "POST")
			{
				r = customSession.Post();
			}
			else if (upperMethod == "OPTIONS")
			{
				r = customSession.Options();
			}
			else if (upperMethod == "DELETE")
			{
				r = customSession.Delete();
			}
			else if (upperMethod == "PUT")
			{
				r = customSession.Put();
			}
			else if (upperMethod == "HEAD")
			{
				r = customSession.Head();
			}
			else if (upperMethod == "PATCH")
			{
				r = customSession.Patch();
			}

			result = r.text;
		}

		ImGui::SameLine();

		static Json::StreamWriterBuilder wBuilder;
		static std::string sResultJson;
		static char* cResultJson;

		if (ImGui::Button("Format JSON##customTab"))
		{
			if (!sResultJson.empty())
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(sResultJson.c_str(), sResultJson.c_str() + static_cast<int>(sResultJson.length()), &root, &err))
				{
					sResultJson = Json::writeString(wBuilder, root);
				}
			}
		}

		ImGui::NextColumn();

		ImGui::Text("Endpoints list:");
		ImGui::TextURL("LCU", "https://lcu.kebs.dev", 1, 0);
		ImGui::SameLine();
		ImGui::Text(" | ");
		ImGui::TextURL("Riot Client", "https://riotclient.kebs.dev", 1, 0);

		ImGui::Columns(1);

		if (!result.empty())
		{
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;
			Json::Value root;
			if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err) || isCustomOpen)
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
			ImGui::InputTextMultiline("##customResult", cResultJson, sResultJson.size() + 1, ImVec2(S.Window.width - 130.f,
				                          (label_size.y + ImGui::GetStyle().FramePadding.y) * 19.f));
		}
	}

	static void Invoke()
	{
		static bool once = true;

		static char destination[1024];
		static char method[1024];
		static char args[1024 * 32];

		if (once)
		{
			once = false;
			std::ranges::copy(S.invokeTab.destination, destination);
			std::ranges::copy(S.invokeTab.method, method);
			std::ranges::copy(S.invokeTab.args, args);
		}

		ImGui::Text("Destination:");
		ImGui::InputTextMultiline("##inputDestination", destination, IM_ARRAYSIZE(destination), ImVec2(600, 20));

		ImGui::Text("Method:");
		ImGui::InputTextMultiline("##inputMethod", method, IM_ARRAYSIZE(method), ImVec2(600, 20));

		ImGui::Text("Args:");
		ImGui::InputTextMultiline("##inputArgs", args, IM_ARRAYSIZE(args), ImVec2(600, 50));

		S.invokeTab.destination = destination;
		S.invokeTab.method = method;
		S.invokeTab.args = args;

		static std::string result;
		if (ImGui::Button("Submit##submitInvoke"))
		{
			const std::string req = std::format("https://127.0.0.1/lol-login/v1/session/invoke?destination={0}&method={1}&args=[{2}]",
			                                    destination, method, args);
			result = LCU::Request("POST", req, "");
		}

		ImGui::Text("Result:");
		ImGui::SameLine();

		if (ImGui::Button("Copy to clipboard##invokeTab"))
		{
			Utils::CopyToClipboard(result);
		}

		static Json::StreamWriterBuilder wBuilder;
		static std::string sResultJson;
		static char* cResultJson;

		if (!result.empty())
		{
			const Json::CharReaderBuilder builder;
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
			ImGui::InputTextMultiline("##gameResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 232));
		}
	}

private:
	static inline bool onOpen = true;

public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Custom"))
		{
			if (ImGui::BeginTabBar("TabBar"))
			{
				if (ImGui::BeginTabItem("HTTP/HTTPS"))
				{
					Custom();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("LCDS"))
				{
					Invoke();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
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

private:
	static std::string GetLedgeUrl()
	{
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value rootRegion;
		std::string region;
		std::string getRegion = LCU::Request("GET", "/riotclient/get_region_locale");
		if (reader->parse(getRegion.c_str(), getRegion.c_str() + static_cast<int>(getRegion.length()), &rootRegion, &err))
		{
			region = rootRegion["webRegion"].asString();
		}

		if (!region.empty())
		{
			std::ifstream systemYaml(S.leaguePath + "system.yaml");
			std::string line;
			while (std::getline(systemYaml, line))
			{
				if (line.find("league_edge_url: ") != std::string::npos
					&& line.find(region) != std::string::npos)
				{
					std::string league_edge_url = line;
					league_edge_url = league_edge_url.substr(league_edge_url.find("league_edge_url: ") + strlen("league_edge_url: "));
					systemYaml.close();
					return league_edge_url;
				}
			}
			systemYaml.close();
		}
		return "";
	}
};
