#pragma once

#include "Includes.h"
#include "LCU.h"

class custom_tab
{
public:
	static void custom()
	{
		static bool once = true;

		static char method[50];
		static char url_text[1024 * 16];
		static char request_text[1024 * 32];

		static char input_port[64] = "";
		static char input_header[1024 * 16];

		static std::string custom_header = lcu::league.header;
		static int custom_port = lcu::league.port;

		static bool is_custom_open = false;

		static std::string ledge_url;
		static std::string store_url;
		static std::string localhost_url = "https://127.0.0.1";

		if (once)
		{
			once = false;
			std::ranges::copy(s.custom_tab.method, method);
			std::ranges::copy(s.custom_tab.url_text, url_text);
			std::ranges::copy(s.custom_tab.request_text, request_text);
			std::ranges::copy(s.custom_tab.port, input_port);
			std::ranges::copy(s.custom_tab.header, input_header);
		}

		if (on_open_)
		{
			custom_header = lcu::league.header;
			custom_port = lcu::league.port;

			ledge_url = get_ledge_url();
			store_url = lcu::request("GET", "/lol-store/v1/getStoreUrl");
			std::erase(store_url, '"');
		}

		ImGui::Text("Method:");
		const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);
		ImGui::InputTextEx("##inputMethod", nullptr, method, IM_ARRAYSIZE(method),
		                   ImVec2(s.window.width - 130.f, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f), 0,
		                   nullptr, nullptr);

		ImGui::Text("URL:");
		ImGui::InputTextMultiline("##inputUrl", url_text, IM_ARRAYSIZE(url_text),
		                          ImVec2(s.window.width - 130.f,
		                                 label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f));

		ImGui::Text("Body:");
		ImGui::InputTextMultiline("##inputBody", (request_text), IM_ARRAYSIZE(request_text), ImVec2(
			                          s.window.width - 130.f,
			                          (label_size.y + ImGui::GetStyle().FramePadding.y) * 6.f),
		                          ImGuiInputTextFlags_AllowTabInput);

		s.custom_tab.method = method;
		s.custom_tab.url_text = url_text;
		s.custom_tab.request_text = request_text;

		if (ImGui::CollapsingHeader("Custom Port/Header"))
		{
			is_custom_open = true;
			ImGui::Text("Port:");
			ImGui::InputText("##inputPort", input_port, 64, ImGuiInputTextFlags_CharsDecimal);

			ImGui::SameLine();

			if (ImGui::Button("LCU"))
			{
				if (strlen(url_text) == 0 || strcmp(url_text, localhost_url.c_str()) == 0
					|| strcmp(url_text, store_url.c_str()) == 0 || strcmp(url_text, ledge_url.c_str()) == 0)
				{
					std::strcpy(url_text, localhost_url.c_str());
				}
				std::strcpy(input_port, std::to_string(lcu::league.port).c_str());
				std::strcpy(input_header, lcu::league.header.c_str());
			}
			ImGui::SameLine();

			if (ImGui::Button("Riot"))
			{
				lcu::set_current_client_riot_info();
				if (strlen(url_text) == 0 || strcmp(url_text, localhost_url.c_str()) == 0
					|| strcmp(url_text, store_url.c_str()) == 0 || strcmp(url_text, ledge_url.c_str()) == 0)
				{
					std::strcpy(url_text, localhost_url.c_str());
				}
				std::strcpy(input_port, std::to_string(lcu::riot.port).c_str());
				std::strcpy(input_header, lcu::riot.header.c_str());
			}
			ImGui::SameLine();

			if (ImGui::Button("Store"))
			{
				std::string store_header = lcu::get_store_header();
				if (!store_header.empty())
				{
					if (strlen(method) != 0)
					{
						std::strcpy(method, utils::to_upper(std::string(method)).c_str());
					}
					if (strlen(url_text) == 0 || strcmp(url_text, localhost_url.c_str()) == 0
						|| strcmp(url_text, store_url.c_str()) == 0 || strcmp(url_text, ledge_url.c_str()) == 0)
					{
						store_url = lcu::request("GET", "/lol-store/v1/getStoreUrl");
						std::erase(store_url, '"');
						std::strcpy(url_text, store_url.c_str());
					}
					std::strcpy(input_port, "443");
					std::strcpy(input_header, store_header.c_str());
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Ledge"))
			{
				std::string ledge_header;

				ledge_url = get_ledge_url();
				if (!ledge_url.empty())
				{
					std::string ledge_host;
					if (auto n = ledge_url.find("https://"); n != std::string::npos)
					{
						ledge_host = ledge_url.substr(n + strlen("https://"));
					}
					ledge_header += "Host: " + ledge_host + "\r\n";

					if (strlen(url_text) == 0 || strcmp(url_text, localhost_url.c_str()) == 0
						|| strcmp(url_text, store_url.c_str()) == 0 || strcmp(url_text, ledge_url.c_str()) == 0)
					{
						std::strcpy(url_text, ledge_url.c_str());
					}
				}

				std::string session_token = lcu::request("GET", "/lol-league-session/v1/league-session-token");
				std::erase(session_token, '\"');

				ledge_header += "Accept-Encoding: deflate, "/*gzip, */"zstd\r\n";
				ledge_header += "user-agent: LeagueOfLegendsClient/" + lcu::league.version + "\r\n";
				ledge_header += "Authorization: Bearer " + session_token + "\r\n";
				ledge_header += "Content-type: application/json\r\n";
				ledge_header += "Accept: application/json\r\n";

				if (strlen(method) != 0)
				{
					std::strcpy(method, utils::to_upper(std::string(method)).c_str());
				}

				std::strcpy(input_port, "443");
				std::strcpy(input_header, ledge_header.c_str());
			}

			ImGui::Text("Header:");
			ImGui::InputTextMultiline("##inputHeader", (input_header), IM_ARRAYSIZE(input_header), ImVec2(
				                          s.window.width - 130.f,
				                          (label_size.y + ImGui::GetStyle().FramePadding.y) * 6.f),
			                          ImGuiInputTextFlags_AllowTabInput);

			s.custom_tab.port = input_port;
			s.custom_tab.header = input_header;

			if (!s.custom_tab.port.empty())
				custom_port = std::stoi(s.custom_tab.port);
			else
				custom_port = 443;

			custom_header = s.custom_tab.header;
		}
		else
		{
			is_custom_open = false;
			custom_header = lcu::league.header;
			custom_port = lcu::league.port;
		}

		ImGui::Columns(2, nullptr, false);

		static std::string result;
		if (ImGui::Button("Send custom request##customTab"))
		{
			auto s_url = std::string(url_text);

			if (s_url.find("https://127.0.0.1") == std::string::npos && !is_custom_open)
			{
				if (s_url.find("https://") == std::string::npos && s_url.find("http://") == std::string::npos)
				{
					while (s_url[0] == ' ' || s_url[0] == '\n')
						s_url.erase(s_url.begin());
					if (s_url[0] != '/')
						s_url.insert(0, "/");
					s_url.insert(0, "https://127.0.0.1:" + std::to_string(lcu::league.port));
				}
			}
			else if (s_url.find("https://127.0.0.1:") == std::string::npos && !is_custom_open)
			{
				s_url.insert(strlen("https://127.0.0.1"), ":" + std::to_string(lcu::league.port));
			}
			else if (s_url.find("https://") != std::string::npos || s_url.find("https://") != std::string::npos)
			{
				if (custom_port != 443 && custom_port != 80)
				{
					s_url.insert(s_url.find('/', strlen("https://")), ":" + std::to_string(custom_port));
				}
			}

			cpr::Session custom_session;
			custom_session.SetVerifySsl(false);
			custom_session.SetHeader(utils::string_to_header(custom_header));
			custom_session.SetBody(request_text);
			custom_session.SetUrl(s_url);

			cpr::Response r;

			if (const std::string upper_method = utils::to_upper(method); upper_method == "GET")
			{
				r = custom_session.Get();
			}
			else if (upper_method == "POST")
			{
				r = custom_session.Post();
			}
			else if (upper_method == "OPTIONS")
			{
				r = custom_session.Options();
			}
			else if (upper_method == "DELETE")
			{
				r = custom_session.Delete();
			}
			else if (upper_method == "PUT")
			{
				r = custom_session.Put();
			}
			else if (upper_method == "HEAD")
			{
				r = custom_session.Head();
			}
			else if (upper_method == "PATCH")
			{
				r = custom_session.Patch();
			}

			result = r.text;
		}

		ImGui::SameLine();

		static Json::StreamWriterBuilder w_builder;
		static std::string s_result_json;
		static char* c_result_json;

		if (ImGui::Button("Format JSON##customTab"))
		{
			if (!s_result_json.empty())
			{
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				Json::Value root;
				if (reader->parse(s_result_json.c_str(), s_result_json.c_str() + static_cast<int>(s_result_json.length()),
				                  &root, &err))
				{
					s_result_json = writeString(w_builder, root);
				}
			}
		}

		ImGui::NextColumn();

		ImGui::Text("Endpoints list:");
		ImGui::text_url("LCU", "https://lcu.kebs.dev", 1, 0);
		ImGui::SameLine();
		ImGui::Text(" | ");
		ImGui::text_url("Riot Client", "https://riotclient.kebs.dev", 1, 0);

		ImGui::Columns(1);

		if (!result.empty())
		{
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;
			Json::Value root;
			if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err) ||
				is_custom_open)
				s_result_json = result;
			else
			{
				s_result_json = writeString(w_builder, root);
			}
			result = "";
		}

		if (!s_result_json.empty())
		{
			c_result_json = s_result_json.data();
			ImGui::InputTextMultiline("##customResult", c_result_json, s_result_json.size() + 1, ImVec2(
				                          s.window.width - 130.f,
				                          (label_size.y + ImGui::GetStyle().FramePadding.y) * 19.f));
		}
	}

	static void invoke()
	{
		static bool once = true;

		static char destination[1024];
		static char method[1024];
		static char args[1024 * 32];

		if (once)
		{
			once = false;
			std::ranges::copy(s.invoke_tab.destination, destination);
			std::ranges::copy(s.invoke_tab.method, method);
			std::ranges::copy(s.invoke_tab.args, args);
		}

		ImGui::Text("Destination:");
		ImGui::InputTextMultiline("##inputDestination", destination, IM_ARRAYSIZE(destination), ImVec2(600, 20));

		ImGui::Text("Method:");
		ImGui::InputTextMultiline("##inputMethod", method, IM_ARRAYSIZE(method), ImVec2(600, 20));

		ImGui::Text("Args:");
		ImGui::InputTextMultiline("##inputArgs", args, IM_ARRAYSIZE(args), ImVec2(600, 50));

		s.invoke_tab.destination = destination;
		s.invoke_tab.method = method;
		s.invoke_tab.args = args;

		static std::string result;
		if (ImGui::Button("Submit##submitInvoke"))
		{
			const std::string req = std::format(
				"https://127.0.0.1/lol-login/v1/session/invoke?destination={0}&method={1}&args=[{2}]",
				destination, method, args);
			result = lcu::request("POST", req, "");
		}

		ImGui::Text("Result:");
		ImGui::SameLine();

		if (ImGui::Button("Copy to clipboard##invokeTab"))
		{
			utils::copy_to_clipboard(result);
		}

		static Json::StreamWriterBuilder w_builder;
		static std::string s_result_json;
		static char* c_result_json;

		if (!result.empty())
		{
			const Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;
			Json::Value root;
			if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
				s_result_json = result;
			else
			{
				s_result_json = writeString(w_builder, root);
			}
			result = "";
		}

		if (!s_result_json.empty())
		{
			c_result_json = s_result_json.data();
			ImGui::InputTextMultiline("##gameResult", c_result_json, s_result_json.size() + 1, ImVec2(600, 232));
		}
	}

private:
	static inline bool on_open_ = true;

public:
	static void render()
	{
		if (ImGui::BeginTabItem("Custom"))
		{
			if (ImGui::BeginTabBar("TabBar"))
			{
				if (ImGui::BeginTabItem("HTTP/HTTPS"))
				{
					custom();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("LCDS"))
				{
					invoke();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			if (on_open_)
				on_open_ = false;

			ImGui::EndTabItem();
		}
		else
		{
			on_open_ = true;
		}
	}

private:
	static std::string get_ledge_url()
	{
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root_region;
		std::string region;
		std::string get_region = lcu::request("GET", "/riotclient/get_region_locale");
		if (reader->parse(get_region.c_str(), get_region.c_str() + static_cast<int>(get_region.length()), &root_region,
		                  &err))
		{
			region = root_region["webRegion"].asString();
		}

		if (!region.empty())
		{
			std::ifstream systemYaml(s.league_path + "system.yaml");
			std::string line;
			while (std::getline(systemYaml, line))
			{
				if (line.find("league_edge_url: ") != std::string::npos
					&& line.find(region) != std::string::npos)
				{
					std::string league_edge_url = line;
					league_edge_url = league_edge_url.substr(
						league_edge_url.find("league_edge_url: ") + strlen("league_edge_url: "));
					systemYaml.close();
					return league_edge_url;
				}
			}
			systemYaml.close();
		}
		return "";
	}
};
