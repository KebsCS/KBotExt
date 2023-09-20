﻿#include "DirectX.h"

#include "GameTab.h"
#include "InfoTab.h"
#include "LoginTab.h"
#include "ProfileTab.h"
#include "MiscTab.h"
#include "CustomTab.h"
#include "SkinsTab.h"
#include "ChampsTab.h"
#include "SettingsTab.h"

bool Direct3D11Render::DirectXInit(const HWND hWnd)
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

	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	constexpr D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
	if (constexpr UINT createDeviceFlags = 0; D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
	                                                                        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
	                                                                        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	if (!CreateRenderTarget())
		return false;

	Renderimgui(hWnd);

	Misc::CheckVersion();

	gamePatch = Misc::GetCurrentPatch();

	std::thread t{Misc::GetAllChampionSkins};
	t.detach();

	std::thread AutoAcceptThread(&GameTab::AutoAccept);
	AutoAcceptThread.detach();

	return true;
}

void Direct3D11Render::StartFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Direct3D11Render::EndFrame()
{
	// Rendering
	auto clear_color = ImVec4(0, 0, 0, 255.f);
	ImGui::EndFrame();
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pd3dRenderTargetView, nullptr);
	g_pd3dDeviceContext->ClearRenderTargetView(g_pd3dRenderTargetView, reinterpret_cast<float*>(&clear_color));
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}

int Direct3D11Render::Render() const
{
	static char buf[255];
	static std::string connectedTo = "";
	static std::string currentInfo = "";
	if (gamePatch == "0.0.0")
	{
		currentInfo = "Failed to connect, most likely blocked by antivirus or firewall";
	}
	else
	{
		if (LCU::IsProcessGood())
			connectedTo = "| Connected to: " + LCU::leagueProcesses[LCU::indexLeagueProcesses].second;

		if (champSkins.empty())
			currentInfo = "Fetching skin data...";
		else
			currentInfo = "";
	}

	sprintf_s(buf, ("KBotExt by kebs - %s %s \t %s ###AnimatedTitle"), gamePatch.c_str(), connectedTo.c_str(), currentInfo.c_str());

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(685, 462), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(S.Window.width - 15), static_cast<float>(S.Window.height - 38)));
	constexpr ImGuiWindowFlags flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin(buf, nullptr, flags); // , ImGuiWindowFlags_AlwaysAutoResize);
	if (constexpr ImGuiTabBarFlags tab_bar_flags = 0; ImGui::BeginTabBar("TabBar", tab_bar_flags))
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

void Direct3D11Render::Shutdown()
{
	CleanupRenderTarget();
	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
		g_pSwapChain = nullptr;
	}
	if (g_pd3dDeviceContext)
	{
		g_pd3dDeviceContext->Release();
		g_pd3dDeviceContext = nullptr;
	}
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = nullptr;
	}
}

bool Direct3D11Render::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	if (S_OK != g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))
		return false;
	if (S_OK != g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pd3dRenderTargetView))
		return false;
	pBackBuffer->Release();
	return true;
}

void Direct3D11Render::CleanupRenderTarget()
{
	if (g_pd3dRenderTargetView)
	{
		g_pd3dRenderTargetView->Release();
		g_pd3dRenderTargetView = nullptr;
	}
}

void Direct3D11Render::Renderimgui(const HWND hWnd)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	MenuInit();

	// Setup Platform/Renderer back-ends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

void Direct3D11Render::InitializeFonts()
{
	const ImGuiIO& io = ImGui::GetIO();
	(void)io;

	static constexpr ImWchar ranges[] = {0x1, 0x1FFFF, 0};
	static ImFontConfig cfg;
	cfg.OversampleH = cfg.OversampleV = 1;

	io.Fonts->AddFontDefault(&cfg);

	cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
	cfg.MergeMode = true;

	using tSHGetFolderPathW = HRESULT(WINAPI*)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
	const auto SHGetFolderPathW = reinterpret_cast<tSHGetFolderPathW>(GetProcAddress(LoadLibraryW(L"shell32.dll"), "SHGetFolderPathW"));

	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, 0x0024/*CSIDL_WINDOWS*/, NULL, 0, szPath)))
	{
		std::filesystem::path fontsPath(szPath);
		fontsPath = fontsPath / "Fonts";

		if (is_directory(fontsPath))
		{
			for (const std::vector<std::string> fonts = {
				     "seguiemj.ttf", // emojis
				     "segoeuib.ttf", // cyrillic
				     "malgunbd.ttf", // korean
				     "YuGothB.ttc", // japanese
				     "simsun.ttc", // simplified chinese
				     "msjh.ttc", // traditional chinese
				     "seguisym.ttf", // symbols
			     }; const auto& f : fonts)
			{
				if (const std::filesystem::path path = fontsPath / f; exists(path))
				{
					io.Fonts->AddFontFromFileTTF(path.string().c_str(), 13.0f, &cfg, ranges);
				}
			}
		}
	}
}

void Direct3D11Render::MenuInit()
{
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.FontGlobalScale = S.fontScale;

	InitializeFonts();

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
