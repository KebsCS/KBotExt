#pragma once

class InvokeTab
{
public:

	static void Render()
	{
		if (ImGui::BeginTabItem("Invoke"))
		{
			static char destination[1024 * 16];
			ImGui::Text("Destination:");
			ImGui::InputTextMultiline("##inputDestination", destination, IM_ARRAYSIZE(destination), ImVec2(600, 20));

			static char method[1024 * 16];
			ImGui::Text("Method:");
			ImGui::InputTextMultiline("##inputMethod", method, IM_ARRAYSIZE(method), ImVec2(600, 20));

			static char args[1024 * 16];
			ImGui::Text("Args:");
			ImGui::InputTextMultiline("##inputArgs", args, IM_ARRAYSIZE(args), ImVec2(600, 50));

			static std::string result;
			if (ImGui::Button("Submit##submitInvoke"))
			{
				std::string req = "https://127.0.0.1/lol-login/v1/session/invoke?destination=" + std::string(destination) + "&method=" + std::string(method) + "&args=[" + std::string(args) + "]";
				result = http->Request("POST", req, "", auth->leagueHeader, "", "", auth->leaguePort);
			}

			ImGui::Text("Result:");
			ImGui::SameLine();

			if (ImGui::Button("Copy to clipboard##invokeTab"))
			{
				utils->CopyToClipboard(result);
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
				ImGui::InputTextMultiline("##gameResult", cResultJson, sResultJson.size() + 1, ImVec2(600, 200));
			}

			ImGui::EndTabItem();
		}
	}
};