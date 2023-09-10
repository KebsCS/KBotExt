#pragma once

#include <fstream>
#include <thread>
#include <filesystem>
#include <tlhelp32.h>
#include "LCU.h"
#include "Config.h"

#include "imgui_internal.h"

#ifdef _MSC_VER
#include <Shlwapi.h>
#define ISTRSTR StrStrIA
#pragma comment(lib, "Shlwapi.lib")
#else
#define ISTRSTR strcasestr
#endif

class misc
{
public:
	static inline std::string program_version = "1.5.3";
	static inline std::string latest_version = "";

	static bool launch_client(const std::string& args)
	{
		const std::string path = std::format("{}LeagueClient.exe", s.league_path).c_str();
		if (s.no_admin)
		{
			if (utils::run_as_user(utils::string_to_wstring(path).c_str(), utils::string_to_wstring(args).data()))
				return true;
		}
		ShellExecuteA(nullptr, "open", path.c_str(), args.c_str(), nullptr, SW_SHOWNORMAL);
		return false;
	}

	static void launch_legacy_client()
	{
		if (!std::filesystem::exists(std::format("{}LoL Companion", s.league_path)))
		{
			std::filesystem::create_directory(std::format("{}LoL Companion", s.league_path));
		}
		if (!std::filesystem::exists(std::format("{}LoL Companion/system.yaml", s.league_path)))
		{
			std::ifstream infile(std::format("{}system.yaml", s.league_path));
			std::ofstream outfile(std::format("{}LoL Companion/system.yaml", s.league_path));
			std::string content;

			std::string temp;
			while (std::getline(infile, temp))
				content += temp + "\n";

			infile.close();
			size_t pos = content.find("riotclient:");
			content = content.substr(0, pos + 11);

			outfile << content;
			outfile.close();
		}

		if (FindWindowA("RCLIENT", "League of Legends"))
		{
			lcu::request("POST", "https://127.0.0.1/process-control/v1/process/quit");
			std::this_thread::sleep_for(std::chrono::milliseconds(4500));
		}

		ShellExecuteA(nullptr, "open", std::format("{}LeagueClient.exe", s.league_path).c_str(),
		              std::format("--system-yaml-override=\"{}LoL Companion/system.yaml\"", s.league_path).c_str(),
		              nullptr, SW_SHOWNORMAL);
	}

	static void check_version()
	{
		const std::string get_latest = Get(cpr::Url{"https://api.github.com/repos/KebsCS/KBotExt/releases/latest"}).text;

		const Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(get_latest.c_str(), get_latest.c_str() + static_cast<int>(get_latest.length()), &root, &err))
		{
			std::string latest_tag = root["tag_name"].asString();
			latest_version = latest_tag;

			const std::vector<std::string> latest_name_split = utils::string_split(latest_tag, ".");
			const std::vector<std::string> program_version_split = utils::string_split(program_version, ".");

			for (size_t i = 0; i < 2; i++)
			{
				if (latest_name_split[i] != program_version_split[i])
				{
					if (MessageBoxA(nullptr, "Open download website?", "New major version available",
					                MB_YESNO | MB_SETFOREGROUND) == IDYES)
					{
						ShellExecuteW(nullptr, nullptr, L"https://github.com/KebsCS/KBotExt/releases/latest", nullptr,
						              nullptr, SW_SHOW);
					}
				}
			}
			if (latest_tag != program_version
				&& std::ranges::find(s.ignored_versions, latest_tag) == s.ignored_versions.end())
			{
				if (const auto status = MessageBoxA(
					nullptr, "Open download website?\nCancel to ignore this version forever",
					"New minor update available", MB_YESNOCANCEL | MB_SETFOREGROUND); status == IDYES)
				{
					ShellExecuteW(nullptr, nullptr, L"https://github.com/KebsCS/KBotExt/releases/latest", nullptr,
					              nullptr, SW_SHOW);
				}
				else if (status == IDCANCEL)
				{
					s.ignored_versions.emplace_back(latest_tag);
					config::save();
				}
			}
		}
	}

	static std::string get_current_patch()
	{
		const std::string result = Get(cpr::Url{"http://ddragon.leagueoflegends.com/api/versions.json"}).text;
		const Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			return root[0].asString();
		}
		return "0.0.0";
	}

	static void get_all_champion_skins()
	{
		std::string get_skins = Get(cpr::Url{
			"https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/v1/skins.json"
		}).text;
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (!reader->parse(get_skins.c_str(), get_skins.c_str() + static_cast<int>(get_skins.length()), &root, &err))
			return;

		std::map<std::string, champ> champs;
		for (const std::string& id : root.getMemberNames())
		{
			const Json::Value current_skin = root[id];

			std::string load_screen_path = current_skin["loadScreenPath"].asString();
			size_t name_start = load_screen_path.find("ASSETS/Characters/") + strlen("ASSETS/Characters/");
			std::string champ_name = load_screen_path.substr(name_start, load_screen_path.find("/", name_start) - name_start);

			std::string name = current_skin["name"].asString();

			std::pair<std::string, std::string> skin;
			if (current_skin["isBase"].asBool() == true)
			{
				champs[champ_name].name = champ_name;

				std::string splash_path = current_skin["splashPath"].asString();
				size_t key_start = splash_path.find("champion-splashes/") + strlen("champion-splashes/");
				std::string champ_key = splash_path.substr(key_start, splash_path.find("/", key_start) - key_start);

				champs[champ_name].key = std::stoi(champ_key);
				skin.first = id;
				skin.second = "default";
				champs[champ_name].skins.insert(champs[champ_name].skins.begin(), skin);
			}
			else
			{
				if (current_skin["questSkinInfo"])
				{
					const Json::Value skin_tiers = current_skin["questSkinInfo"]["tiers"];
					for (Json::Value::ArrayIndex i = 0; i < skin_tiers.size(); i++)
					{
						skin.first = skin_tiers[i]["id"].asString();
						skin.second = skin_tiers[i]["name"].asString();
						champs[champ_name].skins.emplace_back(skin);
					}
				}
				else
				{
					skin.first = id;
					skin.second = name;
					champs[champ_name].skins.emplace_back(skin);
				}
			}
		}

		std::vector<champ> temp;
		for (const auto& champion_value : champs | std::views::values)
		{
			temp.emplace_back(champion_value);
		}
		champ_skins = temp;
	}

	static void task_kill_league()
	{
		for (const std::vector<std::wstring> league_procs = {
			     L"RiotClientCrashHandler.exe",
			     L"RiotClientServices.exe",
			     L"RiotClientUx.exe",
			     L"RiotClientUxRender.exe",

			     L"LeagueCrashHandler.exe",
			     L"LeagueClient.exe",
			     L"LeagueClientUx.exe",
			     L"LeagueClientUxRender.exe"
		     }; const auto& proc : league_procs)
		{
			terminate_process_by_name(proc);
		}
	}

	static std::string champ_id_to_name(const int id)
	{
		if (!id)
		{
			return "None";
		}
		if (champ_skins.empty())
		{
			return "No data"; // "Champion data is still being fetched";
		}
		{
			for (const auto& [key, name, skins] : champ_skins)
			{
				if (key == id)
					return name;
			}
		}
		return "";
	}

	static std::string clear_logs()
	{
		std::string result;

		task_kill_league();

		std::this_thread::sleep_for(std::chrono::seconds(2));

		std::error_code error_code;

		const auto league_path = std::filesystem::path(s.league_path);

		const auto riot_client_path = std::filesystem::path(
			s.league_path.substr(0, s.league_path.find_last_of("/\\", s.league_path.size() - 2))) / "Riot Client";

		char* p_local;
		size_t local_len;
		_dupenv_s(&p_local, &local_len, "LOCALAPPDATA");
		const auto local_app_data = std::filesystem::path(p_local);

		for (const std::vector league_files = {
			     league_path / "Logs",
			     league_path / "Config",
			     league_path / "debug.log",
			     riot_client_path / "UX" / "natives_blob.bin",
			     riot_client_path / "UX" / "snapshot_blob.bin",
			     riot_client_path / "UX" / "v8_context_snapshot.bin",
			     riot_client_path / "UX" / "icudtl.dat",
			     local_app_data / "Riot Games"
		     }; const auto& file : league_files)
		{
			if (exists(file))
			{
				SetFileAttributesA(file.string().c_str(),
				                   GetFileAttributesA(file.string().c_str()) & ~FILE_ATTRIBUTE_READONLY & ~
				                   FILE_ATTRIBUTE_HIDDEN);
				remove_all(file, error_code);
				result += file.string() + " - " + error_code.message() + "\n";
			}
		}

		int counter = 0;
		for (const auto& file : std::filesystem::directory_iterator(std::filesystem::temp_directory_path()))
		{
			remove_all(file, error_code);
			counter++;
		}
		result += "Deleted " + std::to_string(counter) + " files in temp directory\n";
		return result;
	}

	static bool terminate_process_by_name(const std::wstring& processName)
	{
		const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		bool result = false;
		if (snapshot != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32W entry;
			entry.dwSize = sizeof(PROCESSENTRY32W);
			if (Process32FirstW(snapshot, &entry))
			{
				do
				{
					if (std::wstring(entry.szExeFile) == processName)
					{
						const HANDLE process = OpenProcess(PROCESS_TERMINATE, false, entry.th32ProcessID);
						const bool terminate = TerminateProcess(process, 0);
						CloseHandle(process);
						result = terminate;
					}
				}
				while (Process32NextW(snapshot, &entry));
			}
		}
		CloseHandle(snapshot);
		return result;
	}
};

inline int fuzzy_search(const char* needle, void* data)
{
	const auto& items = *static_cast<std::vector<std::string>*>(data);
	for (int i = 0; i < static_cast<int>(items.size()); i++)
	{
		const auto haystack = items[i].c_str();
		// empty
		if (!needle[0])
		{
			if (!haystack[0])
				return i;
			continue;
		}
		if (strstr(haystack, needle))
			return i;
		if (ISTRSTR(haystack, needle))
			return i;
	}
	return -1;
}

static bool items_getter(void* data, const int n, const char** out_str)
{
	if (const auto& items = *static_cast<std::vector<std::string>*>(data); n >= 0 && n < static_cast<int>(items.size()))
	{
		*out_str = items[n].c_str();
		return true;
	}
	return false;
}

namespace ImGui
{
	static void help_marker(const char* desc)
	{
		TextDisabled("(?)");
		if (IsItemHovered())
		{
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			TextUnformatted(desc);
			PopTextWrapPos();
			EndTooltip();
		}
	}

#pragma warning( push )
#pragma warning( disable : 4505 ) //  warning C4505: 'ImGui::ArrowButtonDisabled': unreferenced function with internal linkage has been removed

	static void arrow_button_disabled(const char* id, const ImGuiDir dir)
	{
		PushStyleVar(ImGuiStyleVar_Alpha, GetStyle().Alpha * 0.5f);
		ArrowButton(id, dir);
		PopStyleVar();
	}
#pragma warning( pop )

	static void add_under_line(const ImColor col)
	{
		ImVec2 min = GetItemRectMin();
		const ImVec2 max = GetItemRectMax();
		min.y = max.y;
		GetWindowDrawList()->AddLine(min, max, col, 1.0f);
	}

	static void text_url(const char* name, const char* url, const uint8_t same_line_before = 1, const uint8_t same_line_after = 1)
	{
		if (1 == same_line_before) { SameLine(0.0f, GetStyle().ItemInnerSpacing.x); }
		PushStyleColor(ImGuiCol_Text, GetStyle().Colors[ImGuiCol_ButtonHovered]);
		Text(name);
		PopStyleColor();
		if (IsItemHovered())
		{
			if (IsMouseClicked(0))
			{
				ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
			}
			add_under_line(GetStyle().Colors[ImGuiCol_ButtonHovered]);
			SetTooltip("  Open in browser\n%s", url);
		}
		else
		{
			add_under_line(GetStyle().Colors[ImGuiCol_Button]);
		}
		if (1 == same_line_after) { SameLine(0.0f, GetStyle().ItemInnerSpacing.x); }
	}

#pragma warning( push )
#pragma warning( disable : 4996 )
	struct combo_auto_select_data
	{
		std::vector<std::string> items;
		int index = -1;
		char input[128] = {};

		combo_auto_select_data()
		= default;

		explicit combo_auto_select_data(const std::vector<std::string>& hints, const int selected_index = -1)
			: items(hints)
		{
			if (selected_index > -1 && selected_index < static_cast<int>(items.size()))
			{
				strncpy(input, items[selected_index].c_str(), sizeof(input) - 1);
				index = selected_index;
			}
		}
	};

	inline bool combo_auto_select_complex(const char* label, char* input, const int input_len, int* current_item,
	                                      std::vector<std::string> data, const int items_count, const ImGuiComboFlags flags)
	{
		// Always consume the SetNextWindowSizeConstraint() call in our early return paths
		const ImGuiContext& g = *GImGui;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		// Call the getter to obtain the preview string which is a parameter to BeginCombo()

		const ImGuiID popup_id = window->GetID(label);
		bool popup_is_already_opened = IsPopupOpen(popup_id, 0); //ImGuiPopupFlags_AnyPopupLevel);
		const char* s_active_idx_value1 = nullptr;
		items_getter(&data, *current_item, &s_active_idx_value1);
		const bool popup_needs_to_be_opened = (input[0] != 0) && (s_active_idx_value1 && strcmp(input, s_active_idx_value1));
		bool popup_just_opened = false;

		IM_ASSERT(
			(flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton |
				ImGuiComboFlags_NoPreview)); // Can't use both flags together

		const ImGuiStyle& style = g.Style;

		const float arrow_size = flags & ImGuiComboFlags_NoArrowButton ? 0.0f : GetFrameHeight();
		const ImVec2 label_size = CalcTextSize(label, nullptr, true);
		const float expected_w = CalcItemWidth();
		const float w = flags & ImGuiComboFlags_NoPreview ? arrow_size : expected_w;
		const ImRect frame_bb(window->DC.CursorPos,
		                      ImVec2(window->DC.CursorPos.x + w,
		                             window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2.0f));
		const ImRect total_bb(frame_bb.Min,
		                      ImVec2(
			                      (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + frame_bb.Max.
			                      x, frame_bb.Max.y));
		const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, popup_id, &frame_bb))
			return false;

		bool hovered, held;
		const bool pressed = ButtonBehavior(frame_bb, popup_id, &hovered, &held);

		if (!popup_is_already_opened)
		{
			const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
			RenderNavHighlight(frame_bb, popup_id);
			if (!(flags & ImGuiComboFlags_NoPreview))
				window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col,
				                                style.FrameRounding,
				                                (flags & ImGuiComboFlags_NoArrowButton)
					                                ? ImDrawFlags_RoundCornersAll
					                                : ImDrawFlags_RoundCornersLeft);
		}
		if (!(flags & ImGuiComboFlags_NoArrowButton))
		{
			const ImU32 bg_col = GetColorU32((popup_is_already_opened || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			const ImU32 text_col = GetColorU32(ImGuiCol_Text);
			window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding,
			                                (w <= arrow_size)
				                                ? ImDrawFlags_RoundCornersAll
				                                : ImDrawFlags_RoundCornersRight);
			if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
				RenderArrow(window->DrawList,
				            ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col,
				            ImGuiDir_Down, 1.0f);
		}

		if (!popup_is_already_opened)
		{
			RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
			if (input != nullptr && !(flags & ImGuiComboFlags_NoPreview))
			{
				RenderTextClipped(
					ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y),
					ImVec2(value_x2, frame_bb.Max.y),
					input,
					nullptr,
					nullptr,
					ImVec2(0.0f, 0.0f)
				);
			}

			if ((pressed || g.NavActivateId == popup_id || popup_needs_to_be_opened) && !popup_is_already_opened)
			{
				if (window->DC.NavLayerCurrent == 0)
					window->NavLastIds[0] = popup_id;
				OpenPopupEx(popup_id);
				popup_is_already_opened = true;
				popup_just_opened = true;
			}
		}

		if (label_size.x > 0)
		{
			RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);
		}

		if (!popup_is_already_opened)
		{
			return false;
		}

		const float total_w_minus_arrow = w - arrow_size;
		struct im_gui_size_callback_wrapper
		{
			static void size_callback(ImGuiSizeCallbackData* data)
			{
				const auto total_w_minus_arrow = static_cast<float*>(data->UserData);
				data->DesiredSize = ImVec2(*total_w_minus_arrow, 200.f);
			}
		};
		SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(total_w_minus_arrow, 150.f),
		                             im_gui_size_callback_wrapper::size_callback, (void*)&total_w_minus_arrow);

		char name[16];
		ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size);
		// Recycle windows based on depth

		// Peek into expected window size so we can position it
		if (ImGuiWindow* popup_window = FindWindowByName(name))
		{
			if (popup_window->WasActive)
			{
				const ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
				if (flags & ImGuiComboFlags_PopupAlignLeft)
					popup_window->AutoPosLastDirection = ImGuiDir_Left;
				const ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
				ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected,
				                                         &popup_window->AutoPosLastDirection, r_outer, frame_bb,
				                                         ImGuiPopupPositionPolicy_ComboBox);

				pos.y -= label_size.y + style.FramePadding.y * 2.0f;

				SetNextWindowPos(pos);
			}
		}

		// Horizontally align ourselves with the framed text
		constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		//    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
		const bool ret = Begin(name, nullptr, window_flags);

		PushItemWidth(GetWindowWidth());
		SetCursorPos(ImVec2(0.f, window->DC.CurrLineTextBaseOffset));
		if (popup_just_opened)
		{
			SetKeyboardFocusHere(0);
		}

		const bool done = InputTextEx("##inputText", nullptr, input, input_len, ImVec2(0, 0),
		                              ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue, nullptr,
		                              nullptr);
		PopItemWidth();

		if (!ret)
		{
			EndChild();
			PopItemWidth();
			EndPopup();
			IM_ASSERT(0); // This should never happen as we tested for IsPopupOpen() above
			return false;
		}

		constexpr ImGuiWindowFlags window_flags2 = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus;
		//0; //ImGuiWindowFlags_HorizontalScrollbar
		BeginChild("ChildL", ImVec2(GetContentRegionAvail().x, GetContentRegionAvail().y), false, window_flags2);

		bool selection_changed = false;
		if (input[0] != '\0')
		{
			const int new_idx = fuzzy_search(input, &data);
			const int idx = new_idx >= 0 ? new_idx : *current_item;
			selection_changed = *current_item != idx;
			*current_item = idx;
		}

		bool arrow_scroll = false;
		//int arrowScrollIdx = *current_item;

		if (IsKeyPressed(GetKeyIndex(ImGuiKey_UpArrow)))
		{
			if (*current_item > 0)
			{
				*current_item -= 1;
				arrow_scroll = true;
				SetWindowFocus();
			}
		}
		if (IsKeyPressed(GetKeyIndex(ImGuiKey_DownArrow)))
		{
			if (*current_item >= -1 && *current_item < items_count - 1)
			{
				*current_item += 1;
				arrow_scroll = true;
				SetWindowFocus();
			}
		}

		// select the first match
		if (IsKeyPressed(GetKeyIndex(ImGuiKey_Enter)))
		{
			arrow_scroll = true;
			*current_item = fuzzy_search(input, &data);
			if (*current_item < 0)
				*input = 0;
			CloseCurrentPopup();
		}

		if (IsKeyPressed(GetKeyIndex(ImGuiKey_Backspace)))
		{
			*current_item = fuzzy_search(input, &data);
			selection_changed = true;
		}

		if (done && !arrow_scroll)
		{
			CloseCurrentPopup();
		}

		bool done2 = false;

		for (int n = 0; n < items_count; n++)
		{
			const bool is_selected = n == *current_item;
			if (is_selected && (IsWindowAppearing() || selection_changed))
			{
				SetScrollHereY();
			}

			if (is_selected && arrow_scroll)
			{
				SetScrollHereY();
			}

			const char* select_value = nullptr;
			items_getter(&data, n, &select_value);

			// allow empty item
			char item_id[128];
			ImFormatString(item_id, sizeof(item_id), "%s##item_%02d", select_value, n);
			if (Selectable(item_id, is_selected))
			{
				selection_changed = *current_item != n;
				*current_item = n;
				strncpy(input, select_value, input_len);
				CloseCurrentPopup();
				done2 = true;
			}
		}

		if (arrow_scroll && *current_item > -1)
		{
			const char* s_active_idx_value2 = nullptr;
			items_getter(&data, *current_item, &s_active_idx_value2);
			strncpy(input, s_active_idx_value2, input_len);
			ImGuiWindow* wnd = FindWindowByName(name);
			const ImGuiID id = wnd->GetID("##inputText");
			ImGuiInputTextState* state = GetInputTextState(id);

			const char* buf_end = nullptr;
			state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, input, nullptr, &buf_end);
			state->CurLenA = buf_end - input;
			state->CursorClamp();
		}

		EndChild();
		EndPopup();

		const char* s_active_idx_value3 = nullptr;
		items_getter(&data, *current_item, &s_active_idx_value3);
		const bool ret1 = selection_changed && (s_active_idx_value3 && !strcmp(s_active_idx_value3, input));

		const bool widget_ret = done || done2 || ret1;

		return widget_ret;
	}
#pragma warning( pop )

	static bool combo_auto_select(const char* label, combo_auto_select_data& data, const ImGuiComboFlags flags = 0)
	{
		return combo_auto_select_complex(label, data.input, sizeof(data.input) - 1, &data.index, data.items,
		                                 data.items.size(), flags);
	}
}
