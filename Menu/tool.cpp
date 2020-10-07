using namespace std;
#define _CRT_SECURE_NO_WARNINGS
#define SECURITY_WIN32
#include <winsock2.h>
#include <WinInet.h>
#include <stdlib.h>  
#include <gdiplus.h>
#include < Winternl.h>
#include <Iphlpapi.h>
#include <winnt.h>
#include <tlhelp32.h>
#include <time.h>
#include <vector>
#include <debugapi.h>
#include <shlwapi.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <ws2tcpip.h>
#include <windows.h>
#include <CommCtrl.h>
#include <mmsystem.h>
#include <VersionHelpers.h>
#include <ShlObj.h>
#include <Psapi.h>
#include <gdiplus.h>
#include <utility>
#include <fstream>
#include <DbgHelp.h>
#include "tool.h"
#include <random>
using namespace std;

tool::tool()
{
}


tool::~tool()
{
}
using namespace std;

typedef HMODULE(__stdcall* pLoadLibraryA)(LPCSTR);
typedef FARPROC(__stdcall* pGetProcAddress)(HMODULE, LPCSTR);

typedef INT(__stdcall* dllmain)(HMODULE, DWORD, LPVOID);


struct loaderdata
{
	LPVOID ImageBase;

	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseReloc;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;

	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;

};

DWORD FindProcessId(string processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processSnapshot);
	return 0;
}

DWORD __stdcall LibraryLoader(LPVOID Memory)
{

	loaderdata* LoaderParams = (loaderdata*)Memory;

	PIMAGE_BASE_RELOCATION pIBR = LoaderParams->BaseReloc;

	DWORD delta = (DWORD)((LPBYTE)LoaderParams->ImageBase - LoaderParams->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			int count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			PWORD list = (PWORD)(pIBR + 1);

			for (int i = 0; i < count; i++)
			{
				if (list[i])
				{
					PDWORD ptr = (PDWORD)((LPBYTE)LoaderParams->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	PIMAGE_IMPORT_DESCRIPTOR pIID = LoaderParams->ImportDirectory;

	// Resolve DLL imports
	while (pIID->Characteristics)
	{
		PIMAGE_THUNK_DATA OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)LoaderParams->ImageBase + pIID->OriginalFirstThunk);
		PIMAGE_THUNK_DATA FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)LoaderParams->ImageBase + pIID->FirstThunk);

		HMODULE hModule = LoaderParams->fnLoadLibraryA((LPCSTR)LoaderParams->ImageBase + pIID->Name);

		if (!hModule)
			return FALSE;

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				// Import by ordinal
				DWORD Function = (DWORD)LoaderParams->fnGetProcAddress(hModule,
					(LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			else
			{
				// Import by name
				PIMAGE_IMPORT_BY_NAME pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)LoaderParams->ImageBase + OrigFirstThunk->u1.AddressOfData);
				DWORD Function = (DWORD)LoaderParams->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);
				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			OrigFirstThunk++;
			FirstThunk++;
		}
		pIID++;
	}

	if (LoaderParams->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		dllmain EntryPoint = (dllmain)((LPBYTE)LoaderParams->ImageBase + LoaderParams->NtHeaders->OptionalHeader.AddressOfEntryPoint);

		return EntryPoint((HMODULE)LoaderParams->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
	}

	ZeroMemory(LoaderParams->ImageBase, 32);

	return TRUE;
}


DWORD __stdcall stub()
{
	return 0;
}


void tool::mmap(string process, string dllpath)
{
	// Target Dll
	LPCSTR Dll = "E://VD3B.dll";

	DWORD ProcessId = FindProcessId("csgo.exe");

	loaderdata LoaderParams;

	HANDLE hFile = CreateFileA(Dll, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, 0, NULL); // Open the DLL

	DWORD FileSize = GetFileSize(hFile, NULL);
	PVOID FileBuffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// Read the DLL
	ReadFile(hFile, FileBuffer, FileSize, NULL, NULL);

	// Target Dll's DOS Header
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;
	// Target Dll's NT Headers
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)FileBuffer + pDosHeader->e_lfanew);

	// Opening target process.
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	// Allocating memory for the DLL
	PVOID ExecutableImage = VirtualAllocEx(hProcess, NULL, pNtHeaders->OptionalHeader.SizeOfImage,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// Copy the headers to target process
	WriteProcessMemory(hProcess, ExecutableImage, FileBuffer,
		pNtHeaders->OptionalHeader.SizeOfHeaders, NULL);

	// Target Dll's Section Header
	PIMAGE_SECTION_HEADER pSectHeader = (PIMAGE_SECTION_HEADER)(pNtHeaders + 1);
	// Copying sections of the dll to the target process
	for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
	{
		WriteProcessMemory(hProcess, (PVOID)((LPBYTE)ExecutableImage + pSectHeader[i].VirtualAddress),
			(PVOID)((LPBYTE)FileBuffer + pSectHeader[i].PointerToRawData), pSectHeader[i].SizeOfRawData, NULL);
	}

	// Allocating memory for the loader code.
	PVOID LoaderMemory = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE); // Allocate memory for the loader code

	LoaderParams.ImageBase = ExecutableImage;
	LoaderParams.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)ExecutableImage + pDosHeader->e_lfanew);

	LoaderParams.BaseReloc = (PIMAGE_BASE_RELOCATION)((LPBYTE)ExecutableImage
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	LoaderParams.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)ExecutableImage
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	LoaderParams.fnLoadLibraryA = LoadLibraryA;
	LoaderParams.fnGetProcAddress = GetProcAddress;

	// Write the loader information to target process
	WriteProcessMemory(hProcess, LoaderMemory, &LoaderParams, sizeof(loaderdata),
		NULL);
	// Write the loader code to target process
	WriteProcessMemory(hProcess, (PVOID)((loaderdata*)LoaderMemory + 1), LibraryLoader,
		(DWORD)stub - (DWORD)LibraryLoader, NULL);
	// Create a remote thread to execute the loader code
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((loaderdata*)LoaderMemory + 1),
		LoaderMemory, 0, NULL);

	int msgboxID = MessageBoxA(
		NULL,
		(LPCSTR)"Successfully Injected!",
		(LPCSTR)"Loader",
		MB_OK
	);
	exit(-1);


	// Wait for the loader to finish executing
	WaitForSingleObject(hThread, INFINITE);

	std::cin.get();
	// free the allocated loader code
	VirtualFreeEx(hProcess, LoaderMemory, 0, MEM_RELEASE);



}

void tool::loadLibrary(string process, string dllpath) {

	DWORD dwProc = GetProcessIdByName(process);

	HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, dwProc);

	LPVOID allocMem = VirtualAllocEx(hProc, NULL, sizeof(dllpath), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(hProc, allocMem, dllpath.c_str(), sizeof(dllpath), NULL);

	CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, allocMem, 0, 0);

	CloseHandle(hProc);

}
void tool::set_console(int w, int h) {

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);

	MoveWindow(console, r.left, r.top, w, h, TRUE);
}
void tool::checkInternet() {

	if (!InternetCheckConnectionA((LPCSTR)"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0) && !InternetCheckConnectionA((LPCSTR)"http://www.facebook.com", FLAG_ICC_FORCE_CONNECTION, 0)) {

		int msgboxID = MessageBoxA(
			NULL,
			(LPCSTR)"There is a new version avaiable.",
			(LPCSTR)"Error",
			MB_OK
		);




	}
}

int tool::GetProcessIdByName(const std::string& p_name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 structprocsnapshot = { 0 };

	structprocsnapshot.dwSize = sizeof(PROCESSENTRY32);

	if (snapshot == INVALID_HANDLE_VALUE)return 0;
	if (Process32First(snapshot, &structprocsnapshot) == FALSE)return 0;

	while (Process32Next(snapshot, &structprocsnapshot))
	{
		if (!strcmp(structprocsnapshot.szExeFile, p_name.c_str()))
		{
			CloseHandle(snapshot);
			return structprocsnapshot.th32ProcessID;
		}
	}
	CloseHandle(snapshot);
	return 0;

}

string tool::httpRequest(string site, string param)
{
	HINTERNET hInternet = InternetOpenW(L"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	if (hInternet == NULL)
	{
		return "InternetOpenW failed(hInternet): " + GetLastError();
	}
	else
	{
		wstring widestr;
		for (int i = 0; i < site.length(); ++i)
		{
			widestr += wchar_t(site[i]);
		}
		const wchar_t* site_name = widestr.c_str();

		wstring widestr2;
		for (int i = 0; i < param.length(); ++i)
		{
			widestr2 += wchar_t(param[i]);
		}
		const wchar_t* site_param = widestr2.c_str();



		HINTERNET hConnect = InternetConnectW(hInternet, site_name, 80, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);

		if (hConnect == NULL)
		{
			return "InternetConnectW failed(hConnect == NULL): " + GetLastError();
		}
		else
		{
			const wchar_t* parrAcceptTypes[] = { L"text/*", NULL };

			HINTERNET hRequest = HttpOpenRequestW(hConnect, L"GET", site_param, NULL, NULL, parrAcceptTypes, 0, 0);

			if (hRequest == NULL)
			{
				return "HttpOpenRequestW failed(hRequest == NULL): " + GetLastError();
			}
			else
			{
				BOOL bRequestSent = HttpSendRequestW(hRequest, NULL, 0, NULL, 0);

				if (!bRequestSent)
				{
					return "!bRequestSent    HttpSendRequestW failed with error code " + GetLastError();
				}
				else
				{
					std::string strResponse;
					const int nBuffSize = 1024;
					char buff[nBuffSize];

					BOOL bKeepReading = true;
					DWORD dwBytesRead = -1;

					while (bKeepReading && dwBytesRead != 0)
					{
						bKeepReading = InternetReadFile(hRequest, buff, nBuffSize, &dwBytesRead);
						strResponse.append(buff, dwBytesRead);
					}
					return strResponse;
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}
}

bool tool::downloadFile(string url, string filepath) {

	DeleteUrlCacheEntry(url.c_str());

	HRESULT hr = URLDownloadToFile(
		NULL,
		url.c_str(),
		filepath.c_str(),
		0,
		NULL);

	if (SUCCEEDED(hr))
		return true;
	else
		return false;

}




void tool::checkPrivileges() {


	bool IsRunningAsAdmin = false;

	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	IsRunningAsAdmin = fRet;

	if (!IsRunningAsAdmin) {

		int msgboxID = MessageBoxA(
			NULL,
			(LPCSTR)"Please, run as admin.",
			(LPCSTR)"Error",
			MB_OK
		);
		exit(-1);


	}

}

void tool::setupConsole(string consoletitle, int w, int h) {

	SetConsoleTitleA(consoletitle.c_str());
	set_console(w, h);


}