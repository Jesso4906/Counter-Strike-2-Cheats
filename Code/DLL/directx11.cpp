#include "directx11.h"
#include "memoryTools.h"

Present presentGateway;
Present present;
BYTE overwrittenBytes[5];

bool HookPresent()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGISwapChain* swapChain;
	ID3D11Device* device;

	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, nullptr) == S_OK)
	{
		void** p_vtable = *(void***)(swapChain);
		swapChain->Release();
		device->Release();
		present = (Present)((uintptr_t)p_vtable[8] + 5); // adding 5 because the first instruction is a jump to steam's overlay; creating a trampoline with it causes recursinon
	}
	
	if (present == 0) 
	{
		// if using the device method failed, try searching for the pattern instead
		present = (Present)(FindArrayOfBytes((uintptr_t)GetModuleHandle(L"dxgi.dll"), (BYTE*)"\x48\x89\x74\x24\x20\x55\x57\x41\x56\x48", 10, 0x00)); // this sig is for after the jmp to steam's overlay

		if (present == 0) { return false; }
	}

	memcpy(overwrittenBytes, (void*)present, 5);

	presentGateway = (Present)TrampolineHook((void*)present, (void*)DetourPresent, 5, true); // DetourPresent has the same signature as Present so it can just be jumped to

	if (presentGateway == nullptr) { return false; }

	return true;
}

void UnhookPresent()
{
	SetBytes((void*)present, overwrittenBytes, 5);
}

WNDPROC oWndProc = NULL;
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) { return true; }

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = true;
HWND window = NULL;
ID3D11Device* p_device = NULL;
ID3D11DeviceContext* p_context = NULL;
ID3D11RenderTargetView* mainRenderTargetView = NULL;

long __stdcall DetourPresent(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags) 
{
	if (init) 
	{
		if (SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device)))
		{
			p_device->GetImmediateContext(&p_context);

			// Get HWND to the current window of the target/game
			DXGI_SWAP_CHAIN_DESC sd;
			p_swap_chain->GetDesc(&sd);
			window = sd.OutputWindow;

			// Location in memory where imgui is rendered to
			ID3D11Texture2D* pBackBuffer;
			p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			// create a render target pointing to the back buffer
			p_device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			// This does not destroy the back buffer! It only releases the pBackBuffer object which we only needed to create the RTV.
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

			// Init ImGui 
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(p_device, p_context);

			init = false;
		}
		else 
		{
			return presentGateway(p_swap_chain, sync_interval, flags);
		}
	}
	
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	Draw();

	ImGui::EndFrame();

	ImGui::Render();

	p_context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// execute overwritten bytes and jmp back to original Present
	return presentGateway(p_swap_chain, sync_interval, flags);
}