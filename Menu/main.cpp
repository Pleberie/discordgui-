#define _CRT_SECURE_NO_WARNINGS
using namespace std;
#define WEBSITE (string)"xerus.vip"
#define WEBSITE2 (string)"verus.vip"
#include "imgui\imgui.h"
#include "imgui_impl_dx9.h"
#include <d3d9.h>
#include <d3dx9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <stdio.h>
#include <urlmon.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <io.h>
#include <cstdlib>
#include <wininet.h>
#include <locale>
#include <sstream>

#include "background.hpp"
#include "font.hpp"
#include "csgo_logo.hpp"
#include "warface_logo.hpp"
#include "tool.h"
#include "md5.h"
#include <vector>

#pragma comment(lib, "wininet")
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "d3dx9.lib")

tool* tools;

#define MAX_STR_LEN 64
using namespace std;
int AppW = 800;
int AppH = 600;

bool next_window = false;

static bool WARFACE;
static bool CSGO;
string sub;

string hwid;
string key;
string key1;
string key2;
string user;
string times;
string pswd;
string mail;

string WARFACE_VERSION = "null";
string CSGO_VERSION = "null";

string WARFACE_STATUS = "null";
string CSGO_STATUS = "null";

string WARFACE_DAYS = "null";
string CSGO_DAYS = "null";

string LOADER_INFO = "null";

IDirect3DTexture9* background_IMAGE = nullptr;
IDirect3DTexture9* csgo_IMAGE = nullptr;
IDirect3DTexture9* warface_IMAGE = nullptr;
char request[512];

void sduidsgfsdc()
{
	if (rename("hq78HVhczc7k.dll", "csgo.dll") != 0)
		MessageBoxA(NULL, "îøèáî÷êà âûøëà", "îï õàõàõàõ", IDRETRY); //Íå àöåïòíóë ëèöåíçèþ
}

bool FileExists(const char* fname)
{
	return _access(fname, 0) != -1;
}

void ToClipboard(const char* text)
{
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		char* clip_data = (char*)(GlobalAlloc(GMEM_FIXED, MAX_PATH));
		lstrcpy(clip_data, text);
		SetClipboardData(CF_TEXT, (HANDLE)(clip_data));
		LCID* lcid = (DWORD*)(GlobalAlloc(GMEM_FIXED, sizeof(DWORD)));
		*lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT);
		SetClipboardData(CF_LOCALE, (HANDLE)(lcid));
		CloseClipboard();
	}
}

bool IsProcessRun(const char* const processName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &pe);

	while (1) {
		if (strcmp(pe.szExeFile, processName) == 0) return true;
		if (!Process32Next(hSnapshot, &pe)) return false;
	}
}

inline bool DebugObjectCheck()
{
	// Much easier in ASM but C/C++ looks so much better
	typedef NTSTATUS(WINAPI* pNtQueryInformationProcess)
		(HANDLE, UINT, PVOID, ULONG, PULONG);

	HANDLE hDebugObject = NULL;
	NTSTATUS Status;

	// Get NtQueryInformationProcess
	pNtQueryInformationProcess NtQIP = (pNtQueryInformationProcess)
		GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")),
			"NtQueryInformationProcess");

	Status = NtQIP(GetCurrentProcess(),
		0x1e, // ProcessDebugObjectHandle
		&hDebugObject, 4, NULL);

	if (Status != 0x00000000)
		return false;

	if (hDebugObject)
		return true;
	else
		return false;

	OutputDebugString("#########################");
	if (GetLastError() == 0)
		return true;
	else
		return false;
}

// Data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp;
POINTS m_Pos;
ImGuiIO& io = ImGui::GetIO();

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);

	//switch (msg)
	//{
	//case WM_SIZE:
	//	if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
	//	{
	//		ImGui_ImplDX9_InvalidateDeviceObjects();
	//		g_d3dpp.BackBufferWidth = LOWORD(lParam);
	//		g_d3dpp.BackBufferHeight = HIWORD(lParam);
	//		HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	//		if (hr == D3DERR_INVALIDCALL)
	//			IM_ASSERT(0);
	//		ImGui_ImplDX9_CreateDeviceObjects();
	//	}
	//	return 0;

	//case WM_SYSCOMMAND:
	//	if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
	//		return 0;
	//	break;

	//case WM_DESTROY:
	//	PostQuitMessage(0);
	//	return 0;
	//}
	//return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline void PushPopSS()
{
	__asm
	{
		push ss
		pop ss
		mov eax, 9 // This line executes but is stepped over
		xor edx, edx // This is where the debugger will step to
	}
}

void Colors()
{
	ImColor mainColor = ImColor(33, 143, 204, 255);
	ImColor bodyColor = ImColor(54, 57, 63, 255);
	ImColor bodyColor1 = ImColor(63, 70, 96, 255);
	ImColor bodyColor12 = ImColor(79, 79, 79, 255);
	ImColor fontColor = ImColor(255, 255, 255, 255);
	ImVec4 mainColorHovered = ImVec4(mainColor.Value.x + 0.1f, mainColor.Value.y + 0.1f, mainColor.Value.z + 0.1f, mainColor.Value.w);
	ImVec4 mainColorActive = ImVec4(mainColor.Value.x + 0.2f, mainColor.Value.y + 0.2f, mainColor.Value.z + 0.2f, mainColor.Value.w);
	ImVec4 menubarColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w - 0.8f);
	ImVec4 frameBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .1f);
	ImVec4 tooltipBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .05f);

	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(0, 50);
	//style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 8.f;
	//style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.GrabRounding = 6.f;
	style.ChildWindowRounding = 0.0f;
	style.FramePadding = ImVec2(4, 3);
	style.FrameRounding = 6.f;
	style.ItemSpacing = ImVec2(8, 3);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 3.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 0.0f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
	style.CurveTessellationTol = 1.25f;

	if (!next_window) {
		style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
		style.Colors[ImGuiCol_TextDisabled] = ImColor(255, 255, 255, 255);
	}
	else
		style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(bodyColor);
	style.Colors[ImGuiCol_Button] = ImColor(47, 49, 54, 255);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(64, 68, 75, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(64, 68, 75, 255);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);
	int centrx = (screenW / 2) - 150;
	int centry = (screenH / 2) - 126;
	// Create application window
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, _T("ImGui Example"), NULL };
	RegisterClassEx(&wc);
	HWND hwnd = CreateWindow(_T("ImGui Example"), _T("xerus.vip"), WS_POPUP, centrx, centry, 860, 550, NULL, NULL, wc.hInstance, NULL);
	// Initialize Direct3D
	LPDIRECT3D9 pD3D;
	if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		UnregisterClass(_T("ImGui Example"), wc.hInstance);
		return 0;
	}
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
															//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

															// Create the D3DDevice
	if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
	{
		pD3D->Release();
		UnregisterClass(_T("ImGui Example"), wc.hInstance);
		return 0;
	}

	// Setup ImGui binding
	ImGui_ImplDX9_Init(hwnd, g_pd3dDevice);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryTTF(MainFont, sizeof(MainFont), 18);
	io.Fonts->AddFontFromMemoryTTF(tabs, sizeof(tabs), 18);
	ImFont* TitleFont = io.Fonts->AddFontFromMemoryTTF(MainFont, sizeof(MainFont), 23);
	ImFont* tabFont = io.Fonts->AddFontFromMemoryTTF(tabs, sizeof(tabs), 18);

	if (background_IMAGE == nullptr)
		D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &background, sizeof(background), 1000, 1000, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &background_IMAGE);

	HW_PROFILE_INFO hwProfileInfo;

	string hwid;
	if (GetCurrentHwProfile(&hwProfileInfo))
		hwid = hwProfileInfo.szHwProfileGuid;

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		ImGui_ImplDX9_NewFrame();
		Colors();

		bool agree;
		static bool open = true;
		static int cur_tab;
		static int cur_cheat_tab = -1;

		if (cur_tab != 1) cur_cheat_tab = -1;

		if (!open)
			ExitProcess(0);
		DWORD dwFlag1 = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
		DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove;

		bool _visible;
		_visible = true;
		ImGui::GetIO().MouseDrawCursor = _visible;
		static bool visible_state = _visible;
		static float alpha = 1.f;
		static bool imageRender = true;
		static bool whiteBG = false;
		static int tab = 0;
		static bool box_esp = false;
		static std::string tabs[] = { "general", "music", "chat", "scripting", "configs" };


		ImGui::Begin("", &visible_state, next_window ? dwFlag : dwFlag1);
		{
			ImGuiStyle& style = ImGui::GetStyle();
			ImGui::SetWindowSize(ImVec2(860, 600));
			ImGui::SetCursorPos(ImVec2(0, 0));

			if (!next_window) {
				ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + (ImGui::GetWindowWidth() / 3.5) + 15, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()), ImColor(47, 49, 54, 255), style.WindowRounding);

				ImGui::SetCursorPos(ImVec2(15, 15));
				ImGui::BeginChild("Select a tab", ImVec2(860 - 30, 550 - 30));

				ImGui::PushFont(TitleFont);
				ImGui::PopFont();
				ImGui::Dummy(ImVec2(45, 0));
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 255, 255, 255), "Discord Hack");

				ImGui::Columns(2, nullptr, false);

				ImGui::SetCursorPosY(100);

				if (ImGui::BeginChild("##MainTabs", ImVec2((250 + style.WindowPadding.x * 2) + style.WindowPadding.x * 2, 300 - style.WindowPadding.y * 2), true, ImGuiWindowFlags_NoScrollbar)) {
					ImGui::PushFont(tabFont);

					for (int i = 0; i < sizeof(tabs); i++)
					{
						if (ImGui::Button(tabs[i].c_str(), ImVec2{ (235 + style.WindowPadding.x * 2), 35 + style.WindowPadding.x / 2 })) {
							tab = i;
						}
					}


					ImGui::PopFont();
				} ImGui::EndChild();

				ImGui::SameLine();
				ImGui::Dummy(ImVec2(40, 0));
				ImGui::SameLine();

				ImGui::BeginChild("##Tabs"); {
					switch (tab) {
					case 0:
						imageRender = true;
						ImGui::Text("update Logs");
						//i tink dis works
						break;
					case 1:
						ImGui::Text("Test");
						imageRender = true;
						ImGui::Checkbox("Box", &box_esp);
						break;
					case 2:
						ImGui::Text("Test 2");
						imageRender = false;
						
						break;
					case 3:
						ImGui::Text("Test 3");
						imageRender = false;
						
						break;
					case 4:
						ImGui::Text("Test 4");
						imageRender = false;
						
						break;
					case 5:
						ImGui::Text("Test 5");
						imageRender = false;
						
						break;
					default:
						ImGui::Text("Test 6");
						imageRender = false;
						
						break;
					}
					ImGui::EndChild();
				}
			}
		}	ImGui::End();

		// Rendering
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		//D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
		//	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			g_pd3dDevice->EndScene();
		}
		g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
	}

	ImGui_ImplDX9_Shutdown();
	if (g_pd3dDevice) g_pd3dDevice->Release();
	if (pD3D) pD3D->Release();
	UnregisterClass(_T("ImGui Example"), wc.hInstance);

	return 0;
}
