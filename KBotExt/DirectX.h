#pragma once

#include <string>

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <tchar.h>
#include <dinput.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <dcomp.h>
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d11.lib")

inline ID3D11Device* g_pd3d_device = nullptr;
inline ID3D11DeviceContext* g_pd3d_device_context = nullptr;
inline ID3D11RenderTargetView* g_pd3d_render_target_view = nullptr;
inline IDXGISwapChain* g_p_swap_chain = nullptr;

class direct_3d11_render
{
	std::string game_patch_;

public:
	bool closed_client = false;

	direct_3d11_render() = default;

	~direct_3d11_render() = default;

	static void start_frame();

	static void end_frame();

	// initializes directx, fonts, imgui and objects
	bool direct_x_init(HWND h_wnd);

	static bool create_render_target();

	static void cleanup_render_target();

	// main rendering loop
	int render() const;

	//releases directx and clears imgui
	static void shutdown();

	//initializes imgui
	void render_imgui(HWND h_wnd);

	static void initialize_fonts();

	//initializes imgui styles
	static void menu_init();
};
