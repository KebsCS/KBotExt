#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <regex>
#include <format>
#pragma comment(lib, "Version.lib")

#include "Auth.h"
#include "Utils.h"

ClientInfo Auth::GetClientInfo(const DWORD& pid, bool riotClient)
{
	if (!pid)
		return {};

	std::string cmdLine = Utils::WstringToString(GetProcessCommandLine(pid));
	if (cmdLine.empty())
		return {};

	ClientInfo info;
	info.port = GetPort(cmdLine, riotClient);
	info.token = GetToken(cmdLine, riotClient);
	info.path = GetProcessPath(pid);
	info.version = GetFileVersion(info.path);

	return info;
}

int Auth::GetPort(const std::string& cmdLine, bool riotClient)
{
	std::regex regexStr;
	regexStr = riotClient ? ("--riotclient-app-port=(\\d*)") : ("--app-port=(\\d*)");
	std::smatch m;
	if (std::regex_search(cmdLine, m, regexStr))
		return std::stoi(m[1].str());

	return 0;
}

std::string Auth::GetToken(const std::string& cmdLine, bool riotClient)
{
	std::regex regexStr;
	regexStr = riotClient ? ("--riotclient-auth-token=([\\w-]*)") : ("--remoting-auth-token=([\\w-]*)");
	std::smatch m;
	if (std::regex_search(cmdLine, m, regexStr))
	{
		std::string token = "riot:" + m[1].str();
		char* tokenArray = &token[0];
		return base64.Encode(reinterpret_cast<unsigned char*>(tokenArray), static_cast<unsigned>(token.size())).c_str();
	}

	return "";
}

std::string Auth::MakeLeagueHeader(const ClientInfo& info)
{
	return "Host: 127.0.0.1:" + std::to_string(info.port) + "\r\n" +
		"Connection: keep-alive" + "\r\n" +
		"Authorization: Basic " + info.token + "\r\n" +
		"Accept: application/json" + "\r\n" +
		"Content-Type: application/json" + "\r\n" +
		"Origin: https://127.0.0.1:" + std::to_string(info.port) + "\r\n" +
		"User-Agent: Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) LeagueOfLegendsClient/" + info.version + " (CEF 91) Safari/537.36" + "\r\n" +
		"X-Riot-Source: rcp-fe-lol-social" + "\r\n" +
		"sec-ch-ua: \"Chromium\";v=\"91\"" + "\r\n" +
		"sec-ch-ua-mobile: ?0" + "\r\n" +
		"Sec-Fetch-Site: same-origin" + "\r\n" +
		"Sec-Fetch-Mode: cors" + "\r\n" +
		"Sec-Fetch-Dest: empty" + "\r\n" +
		"Referer: https://127.0.0.1:" + std::to_string(info.port) + "/index.html" + "\r\n" +
		"Accept-Encoding: gzip, deflate, br" + "\r\n" +
		"Accept-Language: en-US,en;q=0.9\r\n\r\n";
}

std::string Auth::MakeRiotHeader(const ClientInfo& info)
{
	return "Host: 127.0.0.1:" + std::to_string(info.port) + "\r\n" +
		"Connection: keep-alive" + "\r\n" +
		"Authorization: Basic " + info.token + "\r\n" +
		"Accept: application/json" + "\r\n" +
		"Access-Control-Allow-Credentials: true" + "\r\n" +
		"Access-Control-Allow-Origin: 127.0.0.1" + "\r\n" +
		"Content-Type: application/json" + "\r\n" +
		"Origin: https://127.0.0.1:" + std::to_string(info.port) + "\r\n" +
		"Sec-Fetch-Dest: empty" + "\r\n" +
		"Sec-Fetch-Mode: cors" + "\r\n" +
		"Sec-Fetch-Site: same-origin" + "\r\n" +
		"Sec-Fetch-User: ?F" + "\r\n" +
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) RiotClient/" + info.version + " (CEF 74) Safari/537.36" + "\r\n" +
		"sec-ch-ua: Chromium" + "\r\n" +
		"Referer: https://127.0.0.1:" + std::to_string(info.port) + "/index.html" + "\r\n" +
		"Accept-Encoding: gzip, deflate, br" + "\r\n" +
		"Accept-Language: en-US,en;q=0.9\r\n\r\n";
}

DWORD Auth::GetProcessId(const std::wstring& processName)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
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
					CloseHandle(snapshot);
					return entry.th32ProcessID;
				}
			} while (Process32NextW(snapshot, &entry));
		}
	}
	CloseHandle(snapshot);
	return 0;
}

std::vector<DWORD> Auth::GetAllProcessIds(const std::wstring& processName)
{
	std::vector<DWORD>pids;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
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
					pids.emplace_back(entry.th32ProcessID);
				}
			} while (Process32NextW(snapshot, &entry));
		}
	}
	CloseHandle(snapshot);
	return pids;
}

std::wstring Auth::GetProcessCommandLine(const DWORD& processId)
{
	typedef NTSTATUS(__stdcall* tNtQueryInformationProcess)
		(
			HANDLE ProcessHandle,
			ULONG ProcessInformationClass,
			PVOID ProcessInformation,
			ULONG ProcessInformationLength,
			PULONG ReturnLength
			);

	std::wstring result;
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processId);

	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	BOOL wow;
	IsWow64Process(GetCurrentProcess(), &wow);

	DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	DWORD pebSize = ProcessParametersOffset + 8; // size until ProcessParameters
	PBYTE peb = (PBYTE)malloc(pebSize);
	ZeroMemory(peb, pebSize);

	DWORD processParametersSize = CommandLineOffset + 16;
	PBYTE processParameters = (PBYTE)malloc(processParametersSize);
	ZeroMemory(processParameters, processParametersSize);

	if (wow)
	{
		typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
			PVOID Reserved1[2];
			PVOID64 PebBaseAddress;
			PVOID Reserved2[4];
			ULONG_PTR UniqueProcessId[2];
			PVOID Reserved3[2];
		} PROCESS_BASIC_INFORMATION_WOW64;

		typedef struct _UNICODE_STRING_WOW64 {
			USHORT Length;
			USHORT MaximumLength;
			PVOID64 Buffer;
		} UNICODE_STRING_WOW64;

		typedef NTSTATUS(NTAPI* tNtWow64ReadVirtualMemory64)(
			IN HANDLE ProcessHandle,
			IN PVOID64 BaseAddress,
			OUT PVOID Buffer,
			IN ULONG64 Size,
			OUT PULONG64 NumberOfBytesRead);

		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		tNtQueryInformationProcess NtQueryInformationProcess =
			(tNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
		if (NtQueryInformationProcess(processHandle, 0, &pbi, sizeof(pbi), 0) != 0)
		{
			MessageBoxA(0, "NtQueryInformationProcess failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		tNtWow64ReadVirtualMemory64 NtWow64ReadVirtualMemory64 =
			(tNtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");

		if (NtWow64ReadVirtualMemory64(processHandle, pbi.PebBaseAddress, peb, pebSize, NULL) != 0)
		{
			MessageBoxA(0, "PEB NtWow64ReadVirtualMemory64 failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		PVOID64 parameters = (PVOID64) * ((PVOID64*)(peb + ProcessParametersOffset));
		if (NtWow64ReadVirtualMemory64(processHandle, parameters, processParameters, processParametersSize, NULL) != 0)
		{
			MessageBoxA(0, "processParameters NtWow64ReadVirtualMemory64 failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(processParameters + CommandLineOffset);
		PWSTR commandLineCopy = (PWSTR)malloc(pCommandLine->MaximumLength);
		if (NtWow64ReadVirtualMemory64(processHandle, pCommandLine->Buffer, commandLineCopy, pCommandLine->MaximumLength, NULL) != 0)
		{
			MessageBoxA(0, "pCommandLine NtWow64ReadVirtualMemory64 failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		result = std::wstring(commandLineCopy);
		CloseHandle(processHandle);
	}
	else
	{
		typedef struct _PROCESS_BASIC_INFORMATION {
			LONG ExitStatus;
			PVOID PebBaseAddress;
			ULONG_PTR AffinityMask;
			LONG BasePriority;
			HANDLE UniqueProcessId;
			HANDLE InheritedFromUniqueProcessId;
		} PROCESS_BASIC_INFORMATION;

		typedef struct _UNICODE_STRING
		{
			USHORT Length;
			USHORT MaximumLength;
			PWSTR Buffer;
		} UNICODE_STRING, * PUNICODE_STRING;
		typedef const UNICODE_STRING* PCUNICODE_STRING;

		PROCESS_BASIC_INFORMATION pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		tNtQueryInformationProcess NtQueryInformationProcess =
			(tNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
		if (NtQueryInformationProcess(processHandle, 0, &pbi, sizeof(pbi), 0) != 0)
		{
			MessageBoxA(0, "NtQueryInformationProcess failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		if (!ReadProcessMemory(processHandle, pbi.PebBaseAddress, peb, pebSize, NULL))
		{
			MessageBoxA(0, "PEB ReadProcessMemory failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset);
		if (!ReadProcessMemory(processHandle, parameters, processParameters, processParametersSize, NULL))
		{
			MessageBoxA(0, "processParameters ReadProcessMemory failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(processParameters + CommandLineOffset);
		PWSTR commandLineCopy = (PWSTR)malloc(pCommandLine->MaximumLength);
		if (!ReadProcessMemory(processHandle, pCommandLine->Buffer, commandLineCopy, pCommandLine->MaximumLength, NULL))
		{
			MessageBoxA(0, "pCommandLine ReadProcessMemory failed", 0, 0);
			CloseHandle(processHandle);
			return {};
		}

		result = std::wstring(commandLineCopy);
		CloseHandle(processHandle);
	}

	return result;
}

std::wstring Auth::GetProcessPath(const DWORD& processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processId);
	if (processHandle)
	{
		WCHAR result[MAX_PATH];
		if (GetModuleFileNameExW(processHandle, NULL, result, MAX_PATH))
		{
			CloseHandle(processHandle);
			return std::wstring(result);
		}
		CloseHandle(processHandle);
	}
	return L"";
}

std::string Auth::GetFileVersion(const std::wstring& file)
{
	DWORD versionSize = GetFileVersionInfoSizeW(file.c_str(), 0);
	if (versionSize)
	{
		std::vector<unsigned char> versionInfo(versionSize);

		if (GetFileVersionInfoW(file.c_str(), 0, versionSize, &versionInfo[0]))
		{
			VS_FIXEDFILEINFO* lpFfi;
			UINT size = sizeof(VS_FIXEDFILEINFO);
			if (VerQueryValueW(&versionInfo[0], L"\\", (LPVOID*)&lpFfi, &size))
			{
				DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
				DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;
				std::string result = std::format("{}.{}.{}.{}",
					HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS),
					HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));
				return result;
			}
		}
	}

	return "";
}