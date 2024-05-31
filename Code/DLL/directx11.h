#pragma once
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include "imgui\imgui.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_impl_dx11.h"
#include "ImGui\imgui_internal.h"

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);

bool HookPresent();
void UnhookPresent();

extern WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // Forward declare message handler from imgui_impl_win32.cpp
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HWND window;
extern ID3D11Device* p_device;
extern ID3D11DeviceContext* p_context;
extern ID3D11RenderTargetView* mainRenderTargetView;

long __stdcall DetourPresent(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);

void Draw(); // defined in dllmain.cpp