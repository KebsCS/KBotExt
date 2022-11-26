#pragma once

#include "Definitions.h"
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

		if (once)
		{
			once = false;
			std::copy(S.customTab.method.begin(), S.customTab.method.end(), method);
			std::copy(S.customTab.urlText.begin(), S.customTab.urlText.end(), urlText);
			std::copy(S.customTab.requestText.begin(), S.customTab.requestText.end(), requestText);
			std::copy(S.customTab.port.begin(), S.customTab.port.end(), inputPort);
			std::copy(S.customTab.header.begin(), S.customTab.header.end(), inputHeader);
		}

		if (onOpen)
		{
			customHeader = LCU::league.header;
			customPort = LCU::league.port;
		}

		ImGui::Text("Method:");
		ImGui::SetNextItemWidth(600.f);
		ImGui::InputText("##inputMethod", method, IM_ARRAYSIZE(method));

		ImGui::Text("URL:");
		ImGui::InputTextMultiline("##inputUrl", urlText, IM_ARRAYSIZE(urlText), ImVec2(600, 20));

		ImGui::Text("Body:");
		ImGui::InputTextMultiline("##inputBody", (requestText), IM_ARRAYSIZE(requestText), ImVec2(600, 100), ImGuiInputTextFlags_AllowTabInput);

		S.customTab.method = method;
		S.customTab.urlText = urlText;
		S.customTab.requestText = requestText;

		if (ImGui::CollapsingHeader("Custom Port/Header"))
		{
			isCustomOpen = true;
			ImGui::Text("Port:");
			ImGui::InputText("##inputPort", inputPort, 64, ImGuiInputTextFlags_CharsDecimal);

			ImGui::SameLine();

			if (ImGui::Button("Set Riot Client Info"))
			{
				LCU::SetCurrentClientRiotInfo();
				std::strcpy(inputPort, std::to_string(LCU::riot.port).c_str());
				std::strcpy(inputHeader, LCU::riot.header.c_str());
			}

			ImGui::Text("Header:");
			ImGui::InputTextMultiline("##inputHeader", (inputHeader), IM_ARRAYSIZE(inputHeader), ImVec2(600, 100), ImGuiInputTextFlags_AllowTabInput);

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

		static std::string result;
		if (ImGui::Button("Send custom request##customTab"))
		{
			std::string sURL = std::string(urlText);

			if (sURL.find("https://127.0.0.1") == std::string::npos && !isCustomOpen)
			{
				if (sURL.find("https://") == std::string::npos && sURL.find("http://") == std::string::npos)
				{
					while (sURL[0] == ' ' || sURL[0] == '\n')
						sURL.erase(sURL.begin());
					if (sURL[0] != '/')
						sURL.insert(0, "/");
					sURL.insert(0, "https://127.0.0.1");
				}
			}
			result = HTTP::Request(method, sURL, requestText, customHeader, "", "", customPort);
		}
		ImGui::Text("Result:");
		ImGui::SameLine();
		if (ImGui::Button("Copy to clipboard##customTab"))
		{
			Utils::CopyToClipboard(result);
		}

		static Json::StreamWriterBuilder wBuilder;
		static std::string sResultJson;
		static char* cResultJson;

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
			cResultJson = &sResultJson[0];
			ImGui::InputTextMultiline("##customResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 300));
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
			std::copy(S.invokeTab.destination.begin(), S.invokeTab.destination.end(), destination);
			std::copy(S.invokeTab.method.begin(), S.invokeTab.method.end(), method);
			std::copy(S.invokeTab.args.begin(), S.invokeTab.args.end(), args);
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
			std::string req = std::format("https://127.0.0.1/lol-login/v1/session/invoke?destination={0}&method={1}&args=[{2}]",
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
};