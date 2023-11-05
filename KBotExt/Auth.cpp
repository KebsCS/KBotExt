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

	const std::string cmdLine = Utils::WstringToString(GetProcessCommandLine(pid));
	if (cmdLine.empty())
		return {};

	ClientInfo info;
	info.port = GetPort(cmdLine, riotClient);
	info.token = GetToken(cmdLine, riotClient);
	info.path = GetProcessPath(pid);
	info.version = GetFileVersion(info.path);

	return info;
}

int Auth::GetPort(const std::string& cmdLine, const bool riotClient)
{
	std::regex regexStr;
	regexStr = riotClient ? "--riotclient-app-port=(\\d*)" : "--app-port=(\\d*)";
	if (std::smatch m; std::regex_search(cmdLine, m, regexStr))
		return std::stoi(m[1].str());

	return 0;
}

std::string Auth::GetToken(const std::string& cmdLine, const bool riotClient)
{
	std::regex regexStr;
	regexStr = riotClient ? "--riotclient-auth-token=([\\w-]*)" : "--remoting-auth-token=([\\w-]*)";
	if (std::smatch m; std::regex_search(cmdLine, m, regexStr))
	{
		std::string token = "riot:" + m[1].str();
		char* tokenArray = token.data();
		return base64.Encode(reinterpret_cast<unsigned char*>(tokenArray), static_cast<unsigned>(token.size()));
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
		"User-Agent: Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) LeagueOfLegendsClient/" + info.version +
		" (CEF 91) Safari/537.36" + "\r\n" +
		"X-Riot-Source: rcp-fe-lol-social" + "\r\n" +
		R"(sec-ch-ua: "Chromium";v="91")" + "\r\n" +
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
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) RiotClient/" + info.version +
		" (CEF 74) Safari/537.36" + "\r\n" +
		"sec-ch-ua: Chromium" + "\r\n" +
		"Referer: https://127.0.0.1:" + std::to_string(info.port) + "/index.html" + "\r\n" +
		"Accept-Encoding: gzip, deflate, br" + "\r\n" +
		"Accept-Language: en-US,en;q=0.9\r\n\r\n";
}

DWORD Auth::GetProcessId(const std::wstring& processName)
{
	static HMODULE kernel32 = GetModuleHandleA("kernel32");
	static auto pCreateToolhelp32Snapshot = (decltype(&CreateToolhelp32Snapshot))GetProcAddress(kernel32, "CreateToolhelp32Snapshot");
	static auto pProcess32FirstW = (decltype(&Process32FirstW))GetProcAddress(kernel32, "Process32FirstW");
	static auto pProcess32NextW = (decltype(&Process32NextW))GetProcAddress(kernel32, "Process32NextW");

	const HANDLE snapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof(PROCESSENTRY32W);
		if (pProcess32FirstW(snapshot, &entry))
		{
			do
			{
				if (std::wstring(entry.szExeFile) == processName)
				{
					CloseHandle(snapshot);
					return entry.th32ProcessID;
				}
			} while (pProcess32NextW(snapshot, &entry));
		}
	}
	CloseHandle(snapshot);
	return 0;
}

std::vector<DWORD> Auth::GetAllProcessIds(const std::wstring& processName)
{
	static HMODULE kernel32 = GetModuleHandleA("kernel32");
	static auto pCreateToolhelp32Snapshot = (decltype(&CreateToolhelp32Snapshot))GetProcAddress(kernel32, "CreateToolhelp32Snapshot");
	static auto pProcess32FirstW = (decltype(&Process32FirstW))GetProcAddress(kernel32, "Process32FirstW");
	static auto pProcess32NextW = (decltype(&Process32NextW))GetProcAddress(kernel32, "Process32NextW");

	std::vector<DWORD> pids;
	const HANDLE snapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof(PROCESSENTRY32W);
		if (pProcess32FirstW(snapshot, &entry))
		{
			do
			{
				if (std::wstring(entry.szExeFile) == processName)
				{
					pids.emplace_back(entry.th32ProcessID);
				}
			} while (pProcess32NextW(snapshot, &entry));
		}
	}
	CloseHandle(snapshot);
	return pids;
}

std::wstring Auth::GetProcessCommandLine(const DWORD& processId)
{
	using tNtQueryInformationProcess = NTSTATUS(__stdcall*)
		(
			HANDLE ProcessHandle,
			ULONG ProcessInformationClass,
			PVOID ProcessInformation,
			ULONG ProcessInformationLength,
			PULONG ReturnLength
			);

	static HMODULE kernel32 = GetModuleHandleA("kernel32");

	static auto pOpenProcess = (decltype(&OpenProcess))GetProcAddress(kernel32, "OpenProcess");
	std::wstring result;
	const HANDLE processHandle = pOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processId);

	static auto pGetNativeSystemInfo = (decltype(&GetNativeSystemInfo))GetProcAddress(kernel32, "GetNativeSystemInfo");
	SYSTEM_INFO si;
	pGetNativeSystemInfo(&si);

	static auto pIsWow64Process = (decltype(&IsWow64Process))GetProcAddress(kernel32, "IsWow64Process");
	static auto pGetCurrentProcess = (decltype(&GetCurrentProcess))GetProcAddress(kernel32, "GetCurrentProcess");
	BOOL wow;
	pIsWow64Process(pGetCurrentProcess(), &wow);

	const DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	const DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	const DWORD pebSize = ProcessParametersOffset + 8; // size until ProcessParameters
	const auto peb = static_cast<PBYTE>(malloc(pebSize));
	ZeroMemory(peb, pebSize);

	const DWORD processParametersSize = CommandLineOffset + 16;
	const auto processParameters = static_cast<PBYTE>(malloc(processParametersSize));
	ZeroMemory(processParameters, processParametersSize);

	if (wow)
	{
		using PROCESS_BASIC_INFORMATION_WOW64 = struct _PROCESS_BASIC_INFORMATION_WOW64
		{
			PVOID Reserved1[2];
			PVOID64 PebBaseAddress;
			PVOID Reserved2[4];
			ULONG_PTR UniqueProcessId[2];
			PVOID Reserved3[2];
		};

		using UNICODE_STRING_WOW64 = struct _UNICODE_STRING_WOW64
		{
			USHORT Length;
			USHORT MaximumLength;
			PVOID64 Buffer;
		};

		using tNtWow64ReadVirtualMemory64 = NTSTATUS(NTAPI*)(
			IN HANDLE ProcessHandle,
			IN PVOID64 BaseAddress,
			OUT PVOID Buffer,
			IN ULONG64 Size,
			OUT PULONG64 NumberOfBytesRead);

		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		if (const auto NtQueryInformationProcess =
			reinterpret_cast<tNtQueryInformationProcess>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64"));
			NtQueryInformationProcess(processHandle, 0, &pbi, sizeof(pbi), nullptr) != 0)
		{
			MessageBoxA(nullptr, "NtQueryInformationProcess failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		const auto NtWow64ReadVirtualMemory64 =
			reinterpret_cast<tNtWow64ReadVirtualMemory64>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64"));

		if (NtWow64ReadVirtualMemory64(processHandle, pbi.PebBaseAddress, peb, pebSize, nullptr) != 0)
		{
			MessageBoxA(nullptr, "PEB NtWow64ReadVirtualMemory64 failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		if (const auto parameters = *reinterpret_cast<PVOID64*>(peb + ProcessParametersOffset); NtWow64ReadVirtualMemory64(
			processHandle, parameters, processParameters, processParametersSize, nullptr) != 0)
		{
			MessageBoxA(nullptr, "processParameters NtWow64ReadVirtualMemory64 failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		const UNICODE_STRING_WOW64* pCommandLine = reinterpret_cast<UNICODE_STRING_WOW64*>(processParameters + CommandLineOffset);
		const auto commandLineCopy = static_cast<PWSTR>(malloc(pCommandLine->MaximumLength));
		if (NtWow64ReadVirtualMemory64(processHandle, pCommandLine->Buffer, commandLineCopy, pCommandLine->MaximumLength, nullptr) != 0)
		{
			MessageBoxA(nullptr, "pCommandLine NtWow64ReadVirtualMemory64 failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		result = std::wstring(commandLineCopy);
		CloseHandle(processHandle);
	}
	else
	{
		using PROCESS_BASIC_INFORMATION = struct _PROCESS_BASIC_INFORMATION
		{
			LONG ExitStatus;
			PVOID PebBaseAddress;
			ULONG_PTR AffinityMask;
			LONG BasePriority;
			HANDLE UniqueProcessId;
			HANDLE InheritedFromUniqueProcessId;
		};

		typedef struct _UNICODE_STRING
		{
			USHORT Length;
			USHORT MaximumLength;
			PWSTR Buffer;
		} UNICODE_STRING, * PUNICODE_STRING [[maybe_unused]];
		/*[[maybe_unused]]*/ using PCUNICODE_STRING = const UNICODE_STRING*;

		PROCESS_BASIC_INFORMATION pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		if (const auto NtQueryInformationProcess =
			reinterpret_cast<tNtQueryInformationProcess>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess"));
			NtQueryInformationProcess(processHandle, 0, &pbi, sizeof(pbi), nullptr) != 0)
		{
			MessageBoxA(nullptr, "NtQueryInformationProcess failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		static auto pReadProcessMemory = (decltype(&ReadProcessMemory))GetProcAddress(kernel32, "ReadProcessMemory");
		if (!pReadProcessMemory(processHandle, pbi.PebBaseAddress, peb, pebSize, nullptr))
		{
			MessageBoxA(nullptr, "PEB ReadProcessMemory failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		if (const PBYTE* parameters = static_cast<PBYTE*>(*reinterpret_cast<LPVOID*>(peb + ProcessParametersOffset)); !pReadProcessMemory(
			processHandle, parameters, processParameters, processParametersSize, nullptr))
		{
			MessageBoxA(nullptr, "processParameters ReadProcessMemory failed", nullptr, 0);
			CloseHandle(processHandle);
			return {};
		}

		const UNICODE_STRING* pCommandLine = reinterpret_cast<UNICODE_STRING*>(processParameters + CommandLineOffset);
		const auto commandLineCopy = static_cast<PWSTR>(malloc(pCommandLine->MaximumLength));
		if (!pReadProcessMemory(processHandle, pCommandLine->Buffer, commandLineCopy, pCommandLine->MaximumLength, nullptr))
		{
			MessageBoxA(nullptr, "pCommandLine ReadProcessMemory failed", nullptr, 0);
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
	static HMODULE kernel32 = GetModuleHandleA("kernel32");
	static auto pOpenProcess = (decltype(&OpenProcess))GetProcAddress(kernel32, "OpenProcess");

	if (const HANDLE processHandle = pOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processId))
	{
		static auto pK32GetModuleFileNameExW = (decltype(&K32GetModuleFileNameExW))GetProcAddress(kernel32, "K32GetModuleFileNameExW");
		if (WCHAR result[MAX_PATH]; pK32GetModuleFileNameExW(processHandle, nullptr, result, MAX_PATH))
		{
			CloseHandle(processHandle);
			return { result };
		}
		CloseHandle(processHandle);
	}
	return L"";
}

std::string Auth::GetFileVersion(const std::wstring& file)
{
	if (const DWORD versionSize = GetFileVersionInfoSizeW(file.c_str(), nullptr))
	{
		std::vector<unsigned char> versionInfo(versionSize);

		if (GetFileVersionInfoW(file.c_str(), 0, versionSize, versionInfo.data()))
		{
			VS_FIXEDFILEINFO* lpFfi = nullptr;
			UINT size = sizeof(VS_FIXEDFILEINFO);
			if (VerQueryValueW(versionInfo.data(), L"\\", reinterpret_cast<LPVOID*>(&lpFfi), &size))
			{
				const DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
				const DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;
				std::string result = std::format("{}.{}.{}.{}",
					HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS),
					HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));
				return result;
			}
		}
	}

	return "";
}