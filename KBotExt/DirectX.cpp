#include "DirectX.h"

#include "GameTab.h"
#include "InfoTab.h"
#include "LoginTab.h"
#include "ProfileTab.h"
#include "MiscTab.h"
#include "CustomTab.h"
#include "InvokeTab.h"
#include "SkinsTab.h"
#include "ChampsTab.h"
#include "SettingsTab.h"

// TODO: move this
void GetAllChampionSkins(std::string patch)
{
	std::vector < Champ > temp;

	std::string result = http->Request("GET", "http://ddragon.leagueoflegends.com/cdn/" + patch + "/data/en_US/champion.json");
	Json::CharReaderBuilder builder;
	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	JSONCPP_STRING err;
	Json::Value root;
	if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
	{
		for (const std::string& name : root["data"].getMemberNames())
		{
			std::string result2 = http->Request("GET", "http://ddragon.leagueoflegends.com/cdn/" + patch + "/data/en_US/champion/" + name + ".json");
			Json::Value root2;
			if (reader->parse(result2.c_str(), result2.c_str() + static_cast<int>(result2.length()), &root2, &err))
			{
				Champ champ;
				champ.name = name;
				champ.key = std::stoi(root2["data"][name]["key"].asString());
				auto skinArr = root2["data"][name]["skins"];
				for (Json::Value::ArrayIndex i = 0; i < skinArr.size(); ++i)
				{
					auto skinObj = skinArr[i];
					std::pair<std::string, std::string > skin;
					skin.first = skinObj["id"].asString();
					skin.second = skinObj["name"].asString();
					champ.skins.emplace_back(skin);
				}
				temp.emplace_back(champ);
			}
		}
	}
	champSkins = temp;
}

bool Direct3D9Render::DirectXInit(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	if (!CreateRenderTarget())
		return false;

	Renderimgui(hWnd);

	Misc::CheckVersion();

	gamePatch = Misc::GetCurrentPatch();

	std::thread t{ GetAllChampionSkins, gamePatch };
	t.detach();

	std::thread AutoAcceptThread(&GameTab::AutoAccept);
	AutoAcceptThread.detach();

	return true;
}

void Direct3D9Render::StartFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Direct3D9Render::EndFrame()
{
	// Rendering
	ImVec4 clear_color = ImVec4(0, 0, 0, 255.f);
	ImGui::EndFrame();
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pd3dRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_pd3dRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}

int Direct3D9Render::Render()
{
	char buf[90];
	sprintf_s(buf, ("KBotExt by kebs - %s \t %s ###AnimatedTitle"), gamePatch.c_str(), champSkins.empty() ? "Downloading skin data..." : "");

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(685, 462), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(S.Window.width - 15), static_cast<float>(S.Window.height - 38)));
	ImGuiWindowFlags flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin(buf, (bool*)0, flags);// , ImGuiWindowFlags_AlwaysAutoResize);
	ImGuiTabBarFlags tab_bar_flags = 0;// ImGuiTabBarFlags_Reorderable;
	if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
	{
		if (!closedClient)
		{
			GameTab::Render();

			ProfileTab::Render();

			InfoTab::Render();

			ChampsTab::Render();

			SkinsTab::Render();

			MiscTab::Render();

			CustomTab::Render();

			InvokeTab::Render();

			SettingsTab::Render();
		}
		else
		{
			LoginTab::Render();

			SettingsTab::Render();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();

	return 1;
}

void Direct3D9Render::Shutdown()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

bool Direct3D9Render::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	if (S_OK != g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))
		return false;
	if (S_OK != g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pd3dRenderTargetView))
		return false;
	pBackBuffer->Release();
	return true;
}

void Direct3D9Render::CleanupRenderTarget()
{
	if (g_pd3dRenderTargetView) { g_pd3dRenderTargetView->Release(); g_pd3dRenderTargetView = NULL; }
}

void Direct3D9Render::Renderimgui(HWND hWnd)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	MenuInit();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

void Direct3D9Render::MenuInit()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x2000, 0x206F, // General Punctuation
		0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
		0x4e00, 0x9FAF, // CJK Ideograms
		0x3131, 0x3163, // Korean alphabets
		0xAC00, 0xD7A3, // Korean characters
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0x2010, 0x205E, // Punctuations
		0x0E00, 0x0E7F, // Thai
		0x0102, 0x0103,
		0x0110, 0x0111,
		0x0128, 0x0129,
		0x0168, 0x0169,
		0x01A0, 0x01A1,
		0x01AF, 0x01B0,
		0x1EA0, 0x1EF9,
		0,
	};

	io.Fonts->AddFontDefault();

	for (const std::string& font : S.vFonts)
	{
		io.Fonts->AddFontFromFileTTF(font.c_str(), 13.0f, NULL, ranges);
	}
	if (S.selectedFont < io.Fonts->Fonts.size())
		io.FontDefault = io.Fonts->Fonts[S.selectedFont];

	ImGuiStyle& style = ImGui::GetStyle();

	//Main
	style.WindowPadding = ImVec2(4.f, 4.f);
	style.FramePadding = ImVec2(3.f, 3.f);
	style.ItemSpacing = ImVec2(5.f, 5.f);
	style.ItemInnerSpacing = ImVec2(5.f, 5.f);
	style.TouchExtraPadding = ImVec2(0.f, 0.f);
	style.ScrollbarSize = 15.f;
	style.GrabMinSize = 15.f;
	//Borders
	style.WindowBorderSize = 1.f;
	style.ChildBorderSize = 1.f;
	style.PopupBorderSize = 1.f;
	style.FrameBorderSize = 1.f;
	style.TabBorderSize = 1.f;
	//Rounding
	style.WindowRounding = 0.f;
	style.ChildRounding = 0.f;
	style.FrameRounding = 0.f;
	style.PopupRounding = 0.f;
	style.ScrollbarRounding = 0.f;
	style.GrabRounding = 0.f;
	style.LogSliderDeadzone = 5.f;
	style.TabRounding = 0.f;
	//Alignment
	style.WindowTitleAlign = ImVec2(0.f, 0.f);
	style.WindowMenuButtonPosition = 0;
	style.ColorButtonPosition = 1;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.5f, 0.5f);
	//AntiAliasing
	style.AntiAliasedLines = false;
	style.AntiAliasedLinesUseTex = false;
	style.AntiAliasedFill = false;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
	colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.01f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.01f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.63f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.63f);
	colors[ImGuiCol_Header] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.63f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.63f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.63f);
	colors[ImGuiCol_Tab] = ImVec4(0.39f, 0.39f, 0.39f, 0.39f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.63f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}