#pragma once

#include <fstream>
#include <thread>
#include <filesystem>
#include <tlhelp32.h>
#include "HTTP.h"
#include "LCU.h"
#include "Config.h"

#include "imgui_internal.h"

#ifdef _MSC_VER
#include <Shlwapi.h>
#define istrstr StrStrIA
#pragma comment(lib, "Shlwapi.lib")
#else
#define istrstr strcasestr
#endif

class Misc
{
public:

	static inline std::string programVersion = "1.5.3";
	static inline std::string latestVersion = "";

	static bool LaunchClient(const std::string args)
	{
		const std::string path = std::format("{}LeagueClient.exe", S.leaguePath).c_str();
		if (S.noAdmin)
		{
			if (Utils::RunAsUser(Utils::StringToWstring(path).c_str(), &(Utils::StringToWstring(args))[0]))
				return true;
		}
		ShellExecuteA(NULL, "open", path.c_str(), args.c_str(), NULL, SW_SHOWNORMAL);
		return false;
	}

	static void LaunchLegacyClient()
	{
		if (!std::filesystem::exists(std::format("{}LoL Companion", S.leaguePath)))
		{
			std::filesystem::create_directory(std::format("{}LoL Companion", S.leaguePath));
		}
		if (!std::filesystem::exists(std::format("{}LoL Companion/system.yaml", S.leaguePath)))
		{
			std::ifstream infile(std::format("{}system.yaml", S.leaguePath));
			std::ofstream outfile(std::format("{}LoL Companion/system.yaml", S.leaguePath));
			std::string content = "";

			std::string temp;
			while (std::getline(infile, temp))
				content += temp + "\n";

			infile.close();
			size_t pos = content.find("riotclient:");
			content = content.substr(0, pos + 11);

			outfile << content;
			outfile.close();
		}

		if (::FindWindowA("RCLIENT", "League of Legends"))
		{
			LCU::Request("POST", "https://127.0.0.1/process-control/v1/process/quit");

			// wait for client to close (maybe theres a better method of doing that)
			std::this_thread::sleep_for(std::chrono::milliseconds(4500));
		}

		ShellExecuteA(NULL, "open", std::format("{}LeagueClient.exe", S.leaguePath).c_str(),
			std::format("--system-yaml-override=\"{}LoL Companion/system.yaml\"", S.leaguePath).c_str(), NULL, SW_SHOWNORMAL);
	}

	static void CheckVersion()
	{
		std::string getLatest = HTTP::Request("GET", "https://api.github.com/repos/KebsCS/KBotExt/releases/latest");

		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(getLatest.c_str(), getLatest.c_str() + static_cast<int>(getLatest.length()), &root, &err))
		{
			std::string latestTag = root["tag_name"].asString();
			Misc::latestVersion = latestTag;

			std::vector<std::string>latestNameSplit = Utils::StringSplit(latestTag, ".");
			std::vector<std::string>programVersionSplit = Utils::StringSplit(Misc::programVersion, ".");

			for (size_t i = 0; i < 2; i++)
			{
				if (latestNameSplit[i] != programVersionSplit[i])
				{
					if (MessageBoxA(0, "Open download website?", "New major version available", MB_YESNO | MB_SETFOREGROUND) == IDYES)
					{
						ShellExecuteW(0, 0, L"https://github.com/KebsCS/KBotExt/releases/latest", 0, 0, SW_SHOW);
					}
				}
			}
			if (latestTag != Misc::programVersion
				&& std::find(S.ignoredVersions.begin(), S.ignoredVersions.end(), latestTag) == S.ignoredVersions.end())
			{
				const auto status = MessageBoxA(0, "Open download website?\nCancel to ignore this version forever", "New minor update available", MB_YESNOCANCEL | MB_SETFOREGROUND);
				if (status == IDYES)
				{
					ShellExecuteW(0, 0, L"https://github.com/KebsCS/KBotExt/releases/latest", 0, 0, SW_SHOW);
				}
				else if (status == IDCANCEL)
				{
					S.ignoredVersions.emplace_back(latestTag);
					Config::Save();
				}
			}
		}
	}

	static std::string GetCurrentPatch()
	{
		std::string result = HTTP::Request("GET", "http://ddragon.leagueoflegends.com/api/versions.json");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			return root[0].asString();
		}
		return "0.0.0";
	}

	static void GetAllChampionSkins()
	{
		std::string getSkins = HTTP::Request("GET", "https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/v1/skins.json");
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (!reader->parse(getSkins.c_str(), getSkins.c_str() + static_cast<int>(getSkins.length()), &root, &err))
			return;

		std::map<std::string, Champ>champs;
		for (const std::string& id : root.getMemberNames())
		{
			const Json::Value currentSkin = root[id];

			std::string loadScreenPath = currentSkin["loadScreenPath"].asString();
			size_t nameStart = loadScreenPath.find("ASSETS/Characters/") + strlen("ASSETS/Characters/");
			std::string champName = loadScreenPath.substr(nameStart, loadScreenPath.find("/", nameStart) - nameStart);

			std::string name = currentSkin["name"].asString();

			std::pair<std::string, std::string> skin;
			if (currentSkin["isBase"].asBool() == true)
			{
				champs[champName].name = champName;

				std::string splashPath = currentSkin["splashPath"].asString();
				size_t keyStart = splashPath.find("champion-splashes/") + strlen("champion-splashes/");
				std::string champKey = splashPath.substr(keyStart, splashPath.find("/", keyStart) - keyStart);

				champs[champName].key = std::stoi(champKey);
				skin.first = id;
				skin.second = "default";
				champs[champName].skins.insert(champs[champName].skins.begin(), skin);
			}
			else
			{
				if (currentSkin["questSkinInfo"]) // K/DA ALL OUT Seraphine
				{
					const Json::Value skinTiers = currentSkin["questSkinInfo"]["tiers"];
					for (Json::Value::ArrayIndex i = 0; i < skinTiers.size(); i++)
					{
						skin.first = skinTiers[i]["id"].asString();
						skin.second = skinTiers[i]["name"].asString();
						champs[champName].skins.emplace_back(skin);
					}
				}
				else
				{
					skin.first = id;
					skin.second = name;
					champs[champName].skins.emplace_back(skin);
				}
			}
		}

		std::vector<Champ> temp;
		for (const auto& c : champs)
		{
			temp.emplace_back(c.second);
		}
		champSkins = temp;
	}

	static void TaskKillLeague()
	{
		std::vector<std::wstring>leagueProcs = {
			L"RiotClientCrashHandler.exe",
			L"RiotClientServices.exe",
			L"RiotClientUx.exe",
			L"RiotClientUxRender.exe",

			L"LeagueCrashHandler.exe",
			L"LeagueClient.exe",
			L"LeagueClientUx.exe",
			L"LeagueClientUxRender.exe"
		};

		for (const auto& proc : leagueProcs)
		{
			Misc::TerminateProcessByName(proc);
		}
	}

	static std::string ChampIdToName(int id)
	{
		if (!id)
		{
			return "None";
		}
		else if (champSkins.empty())
		{
			return "No data";// "Champion data is still being fetched";
		}
		{
			for (const auto& c : champSkins)
			{
				if (c.key == id)
					return c.name;
			}
		}
		return "";
	}

	// Terminate all league related processes,
	// remove read only and hidden property from files
	// and delete them
	static std::string ClearLogs()
	{
		std::string result = "";

		TaskKillLeague();

		std::this_thread::sleep_for(std::chrono::seconds(2));

		std::error_code errorCode;

		auto leaguePath = std::filesystem::path(S.leaguePath);

		auto riotClientPath = std::filesystem::path(
			S.leaguePath.substr(0, S.leaguePath.find_last_of("/\\", S.leaguePath.size() - 2))) / "Riot Client";

		char* pLocal;
		size_t localLen;
		_dupenv_s(&pLocal, &localLen, "LOCALAPPDATA");
		auto localAppData = std::filesystem::path(pLocal);

		std::vector<std::filesystem::path> leagueFiles = {
			leaguePath / "Logs",
			leaguePath / "Config",
			leaguePath / "debug.log",
			riotClientPath / "UX" / "natives_blob.bin",
			riotClientPath / "UX" / "snapshot_blob.bin",
			riotClientPath / "UX" / "v8_context_snapshot.bin",
			riotClientPath / "UX" / "icudtl.dat",
			localAppData / "Riot Games"
		};

		for (const auto& file : leagueFiles)
		{
			if (std::filesystem::exists(file))
			{
				SetFileAttributesA(file.string().c_str(), GetFileAttributesA(file.string().c_str()) & ~FILE_ATTRIBUTE_READONLY & ~FILE_ATTRIBUTE_HIDDEN);
				std::filesystem::remove_all(file, errorCode);
				result += file.string() + " - " + errorCode.message() + "\n";
			}
		}

		int counter = 0;
		for (const auto& file : std::filesystem::directory_iterator(std::filesystem::temp_directory_path()))
		{
			std::filesystem::remove_all(file, errorCode);
			counter++;
		}
		result += "Deleted " + std::to_string(counter) + " files in temp directory\n";
		return result;
	}

	// returns true on success
	static bool TerminateProcessByName(std::wstring processName)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
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
						HANDLE process = OpenProcess(PROCESS_TERMINATE, false, entry.th32ProcessID);
						bool terminate = TerminateProcess(process, 0);
						CloseHandle(process);
						result = terminate;
					}
				} while (Process32NextW(snapshot, &entry));
			}
		}
		CloseHandle(snapshot);
		return result;
	}
};

int FuzzySearch(const char* needle, void* data) {
	auto& items = *(std::vector<std::string> *)data;
	for (int i = 0; i < (int)items.size(); i++) {
		auto haystack = items[i].c_str();
		// empty
		if (!needle[0]) {
			if (!haystack[0])
				return i;
			continue;
		}
		// exact match
		if (strstr(haystack, needle))
			return i;
		// fuzzy match
		if (istrstr(haystack, needle))
			return i;
	}
	return -1;
}

static bool itemsGetter(void* data, int n, const char** out_str) {
	auto& items = *(std::vector<std::string> *)data;
	if (n >= 0 && n < (int)items.size()) {
		*out_str = items[n].c_str();
		return true;
	}
	return false;
}

namespace ImGui
{
	// Helper to display a little (?) mark which shows a tooltip when hovered.
	// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

#pragma warning( push )
#pragma warning( disable : 4505 ) //  warning C4505: 'ImGui::ArrowButtonDisabled': unreferenced function with internal linkage has been removed
	static void ArrowButtonDisabled(const char* id, ImGuiDir dir)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::ArrowButton(id, dir);
		ImGui::PopStyleVar();
	}
#pragma warning( pop )

	static void AddUnderLine(ImColor col)
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine(min, max, col, 1.0f);
	}

	static void TextURL(const char* name, const char* url, uint8_t sameLineBefore = 1, uint8_t sameLineAfter = 1)
	{
		if (1 == sameLineBefore) { ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); }
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::Text(name);
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(0))
			{
				ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
			}
			AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
			ImGui::SetTooltip("  Open in browser\n%s", url);
		}
		else
		{
			AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}
		if (1 == sameLineAfter) { ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); }
	}

#pragma warning( push )
#pragma warning( disable : 4996 )
	struct ComboAutoSelectData {
		std::vector<std::string> items;
		int index = -1;
		char input[128] = {};

		ComboAutoSelectData() {};

		ComboAutoSelectData(std::vector<std::string> hints, int selected_index = -1)
			: items(hints)
		{
			if (selected_index > -1 && selected_index < (int)items.size()) {
				strncpy(input, items[selected_index].c_str(), sizeof(input) - 1);
				index = selected_index;
			}
		}
	};

	bool ComboAutoSelectComplex(const char* label, char* input, int inputlen, int* current_item,
		std::vector<std::string> data, int items_count, ImGuiComboFlags flags)
	{
		// Always consume the SetNextWindowSizeConstraint() call in our early return paths
		ImGuiContext& g = *GImGui;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		// Call the getter to obtain the preview string which is a parameter to BeginCombo()

		const ImGuiID popupId = window->GetID(label);
		bool popupIsAlreadyOpened = IsPopupOpen(popupId, 0); //ImGuiPopupFlags_AnyPopupLevel);
		const char* sActiveidxValue1 = NULL;
		itemsGetter((void*)&data, *current_item, &sActiveidxValue1);
		bool popupNeedsToBeOpened = (input[0] != 0) && (sActiveidxValue1 && strcmp(input, sActiveidxValue1));
		bool popupJustOpened = false;

		IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

		const ImGuiStyle& style = g.Style;

		const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const float expected_w = CalcItemWidth();
		const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : expected_w;
		const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2.0f));
		const ImRect total_bb(frame_bb.Min, ImVec2((label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + frame_bb.Max.x, frame_bb.Max.y));
		const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, popupId, &frame_bb))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(frame_bb, popupId, &hovered, &held);

		if (!popupIsAlreadyOpened) {
			const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
			RenderNavHighlight(frame_bb, popupId);
			if (!(flags & ImGuiComboFlags_NoPreview))
				window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersLeft);
		}
		if (!(flags & ImGuiComboFlags_NoArrowButton)) {
			ImU32 bg_col = GetColorU32((popupIsAlreadyOpened || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			ImU32 text_col = GetColorU32(ImGuiCol_Text);
			window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
			if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
				RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
		}

		if (!popupIsAlreadyOpened) {
			RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
			if (input != NULL && !(flags & ImGuiComboFlags_NoPreview)) {
				RenderTextClipped(
					ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y),
					ImVec2(value_x2, frame_bb.Max.y),
					input,
					NULL,
					NULL,
					ImVec2(0.0f, 0.0f)
				);
			}

			if ((pressed || g.NavActivateId == popupId || popupNeedsToBeOpened) && !popupIsAlreadyOpened) {
				if (window->DC.NavLayerCurrent == 0)
					window->NavLastIds[0] = popupId;
				OpenPopupEx(popupId);
				popupIsAlreadyOpened = true;
				popupJustOpened = true;
			}
		}

		if (label_size.x > 0) {
			RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);
		}

		if (!popupIsAlreadyOpened) {
			return false;
		}

		const float totalWMinusArrow = w - arrow_size;
		struct ImGuiSizeCallbackWrapper {
			static void sizeCallback(ImGuiSizeCallbackData* data) {
				float* totalWMinusArrow = (float*)(data->UserData);
				data->DesiredSize = ImVec2(*totalWMinusArrow, 200.f);
			}
		};
		SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(totalWMinusArrow, 150.f), ImGuiSizeCallbackWrapper::sizeCallback, (void*)&totalWMinusArrow);

		char name[16];
		ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

		// Peek into expected window size so we can position it
		if (ImGuiWindow* popup_window = FindWindowByName(name)) {
			if (popup_window->WasActive) {
				ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
				if (flags & ImGuiComboFlags_PopupAlignLeft)
					popup_window->AutoPosLastDirection = ImGuiDir_Left;
				ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
				ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);

				pos.y -= label_size.y + style.FramePadding.y * 2.0f;

				SetNextWindowPos(pos);
			}
		}

		// Horizontally align ourselves with the framed text
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		//    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
		bool ret = Begin(name, NULL, window_flags);

		PushItemWidth(GetWindowWidth());
		SetCursorPos(ImVec2(0.f, window->DC.CurrLineTextBaseOffset));
		if (popupJustOpened) {
			SetKeyboardFocusHere(0);
		}

		bool done = InputTextEx("##inputText", NULL, input, inputlen, ImVec2(0, 0), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL);
		PopItemWidth();

		if (!ret) {
			EndChild();
			PopItemWidth();
			EndPopup();
			IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
			return false;
		}

		ImGuiWindowFlags window_flags2 = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus; //0; //ImGuiWindowFlags_HorizontalScrollbar
		BeginChild("ChildL", ImVec2(GetContentRegionAvail().x, GetContentRegionAvail().y), false, window_flags2);

		bool selectionChanged = false;
		if (input[0] != '\0') {
			int new_idx = FuzzySearch(input, (void*)&data);
			int idx = new_idx >= 0 ? new_idx : *current_item;
			selectionChanged = *current_item != idx;
			*current_item = idx;
		}

		bool arrowScroll = false;
		//int arrowScrollIdx = *current_item;

		if (IsKeyPressed(GetKeyIndex(ImGuiKey_UpArrow))) {
			if (*current_item > 0)
			{
				*current_item -= 1;
				arrowScroll = true;
				SetWindowFocus();
			}
		}
		if (IsKeyPressed(GetKeyIndex(ImGuiKey_DownArrow))) {
			if (*current_item >= -1 && *current_item < items_count - 1)
			{
				*current_item += 1;
				arrowScroll = true;
				SetWindowFocus();
			}
		}

		// select the first match
		if (IsKeyPressed(GetKeyIndex(ImGuiKey_Enter))) {
			arrowScroll = true;
			*current_item = FuzzySearch(input, (void*)&data);
			if (*current_item < 0)
				*input = 0;
			CloseCurrentPopup();
		}

		if (IsKeyPressed(GetKeyIndex(ImGuiKey_Backspace))) {
			*current_item = FuzzySearch(input, (void*)&data);
			selectionChanged = true;
		}

		if (done && !arrowScroll) {
			CloseCurrentPopup();
		}

		bool done2 = false;

		for (int n = 0; n < items_count; n++)
		{
			bool is_selected = n == *current_item;
			if (is_selected && (IsWindowAppearing() || selectionChanged)) {
				SetScrollHereY();
			}

			if (is_selected && arrowScroll) {
				SetScrollHereY();
			}

			const char* select_value = NULL;
			itemsGetter((void*)&data, n, &select_value);

			// allow empty item
			char item_id[128];
			ImFormatString(item_id, sizeof(item_id), "%s##item_%02d", select_value, n);
			if (Selectable(item_id, is_selected)) {
				selectionChanged = *current_item != n;
				*current_item = n;
				strncpy(input, select_value, inputlen);
				CloseCurrentPopup();
				done2 = true;
			}
		}

		if (arrowScroll && *current_item > -1) {
			const char* sActiveidxValue2 = NULL;
			itemsGetter((void*)&data, *current_item, &sActiveidxValue2);
			strncpy(input, sActiveidxValue2, inputlen);
			ImGuiWindow* wnd = FindWindowByName(name);
			const ImGuiID id = wnd->GetID("##inputText");
			ImGuiInputTextState* state = GetInputTextState(id);

			const char* buf_end = NULL;
			state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, input, NULL, &buf_end);
			state->CurLenA = (int)(buf_end - input);
			state->CursorClamp();
		}

		EndChild();
		EndPopup();

		const char* sActiveidxValue3 = NULL;
		itemsGetter((void*)&data, *current_item, &sActiveidxValue3);
		bool ret1 = (selectionChanged && (sActiveidxValue3 && !strcmp(sActiveidxValue3, input)));

		bool widgetRet = done || done2 || ret1;

		return widgetRet;
	}
#pragma warning( pop )

	static bool ComboAutoSelect(const char* label, ImGui::ComboAutoSelectData& data, ImGuiComboFlags flags = 0)
	{
		return ComboAutoSelectComplex(label, data.input, sizeof(data.input) - 1, &data.index, data.items, (int)data.items.size(), flags);
	}
}