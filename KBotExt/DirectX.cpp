#include "DirectX.h"

#include "GameTab.h"
#include "InfoTab.h"
#include "LoginTab.h"
#include "ProfileTab.h"
#include "MiscTab.h"
#include "CustomTab.h"
#include "SkinsTab.h"
#include "ChampsTab.h"
#include "SettingsTab.h"

bool direct_3d11_render::direct_x_init(const HWND h_wnd)
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
	sd.OutputWindow = h_wnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL feature_level;
	constexpr D3D_FEATURE_LEVEL feature_level_array[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
	if (constexpr UINT create_device_flags = 0; D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags,
	                                                                          feature_level_array,
	                                                                          2, D3D11_SDK_VERSION, &sd, &g_p_swap_chain, &g_pd3d_device,
	                                                                          &feature_level,
	                                                                          &g_pd3d_device_context) != S_OK)
		return false;

	if (!create_render_target())
		return false;

	render_imgui(h_wnd);

	misc::check_version();

	game_patch_ = misc::get_current_patch();

	std::thread t{misc::get_all_champion_skins};
	t.detach();

	std::thread auto_accept_thread(&game_tab::auto_accept);
	auto_accept_thread.detach();

	return true;
}

void direct_3d11_render::start_frame()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void direct_3d11_render::end_frame()
{
	// Rendering
	auto clear_color = ImVec4(0, 0, 0, 255.f);
	ImGui::EndFrame();
	ImGui::Render();
	g_pd3d_device_context->OMSetRenderTargets(1, &g_pd3d_render_target_view, nullptr);
	g_pd3d_device_context->ClearRenderTargetView(g_pd3d_render_target_view, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_p_swap_chain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}

int direct_3d11_render::render() const
{
	static char buf[255];
	static std::string connected_to;
	static std::string current_info;
	if (game_patch_ == "0.0.0")
	{
		current_info = "Failed to connect, most likely blocked by antivirus or firewall";
	}
	else
	{
		if (lcu::is_process_good())
			connected_to = "| Connected to: " + lcu::league_processes[lcu::index_league_processes].second;

		if (champ_skins.empty())
			current_info = "Fetching skin data...";
		else
			current_info = "";
	}

	sprintf_s(buf, "KBotExt by kebs - %s %s \t %s ###AnimatedTitle", game_patch_.c_str(), connected_to.c_str(),
	          current_info.c_str());

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(685, 462), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(s.window.width - 15), static_cast<float>(s.window.height - 38)));
	constexpr ImGuiWindowFlags flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin(buf, nullptr, flags); // , ImGuiWindowFlags_AlwaysAutoResize);
	if (constexpr ImGuiTabBarFlags tab_bar_flags = 0; ImGui::BeginTabBar("TabBar", tab_bar_flags))
	{
		if (!closed_client)
		{
			game_tab::render();

			profile_tab::render();

			info_tab::render();

			champs_tab::render();

			skins_tab::render();

			misc_tab::render();

			custom_tab::render();

			settings_tab::render();
		}
		else
		{
			login_tab::render();

			settings_tab::render();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();

	return 1;
}

void direct_3d11_render::shutdown()
{
	cleanup_render_target();
	if (g_p_swap_chain)
	{
		g_p_swap_chain->Release();
		g_p_swap_chain = nullptr;
	}
	if (g_pd3d_device_context)
	{
		g_pd3d_device_context->Release();
		g_pd3d_device_context = nullptr;
	}
	if (g_pd3d_device)
	{
		g_pd3d_device->Release();
		g_pd3d_device = nullptr;
	}
}

bool direct_3d11_render::create_render_target()
{
	ID3D11Texture2D* p_back_buffer;
	if (S_OK != g_p_swap_chain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer)))
		return false;
	if (S_OK != g_pd3d_device->CreateRenderTargetView(p_back_buffer, nullptr, &g_pd3d_render_target_view))
		return false;
	p_back_buffer->Release();
	return true;
}

void direct_3d11_render::cleanup_render_target()
{
	if (g_pd3d_render_target_view)
	{
		g_pd3d_render_target_view->Release();
		g_pd3d_render_target_view = nullptr;
	}
}

void direct_3d11_render::render_imgui(const HWND h_wnd)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	menu_init();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(h_wnd);
	ImGui_ImplDX11_Init(g_pd3d_device, g_pd3d_device_context);
}

void direct_3d11_render::initialize_fonts()
{
    const ImGuiIO& io = ImGui::GetIO();
    (void)io;

    static constexpr ImWchar ranges[] = {0x1, 0x1FFFF, 0};
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;

    io.Fonts->AddFontDefault(&cfg);

    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    cfg.MergeMode = true;

    using t_sh_get_folder_path_w = HRESULT(WINAPI*)(HWND hwnd, int csidl, HANDLE h_token, DWORD dw_flags, LPWSTR psz_path);
	const auto sh_get_folder_path_w = reinterpret_cast<t_sh_get_folder_path_w>(GetProcAddress(LoadLibraryW(L"shell32.dll"), "SHGetFolderPathW"));

            if (TCHAR sz_path[MAX_PATH]; SUCCEEDED(sh_get_folder_path_w(NULL, 0x0024/*CSIDL_WINDOWS*/, NULL, 0, sz_path)))
            {
                std::filesystem::path fonts_path(sz_path);
                fonts_path = fonts_path / "Fonts";

                if (is_directory(fonts_path))
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
                        if (const std::filesystem::path path = fonts_path / f; exists(path))
                        {
                            io.Fonts->AddFontFromFileTTF(path.string().c_str(), 13.0f, &cfg, ranges);
                        }
                    }
                }
            }
}

void direct_3d11_render::menu_init()
{
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.FontGlobalScale = s.font_scale;

	initialize_fonts();

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
