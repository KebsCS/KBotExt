#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <regex>
#include <format>
#pragma comment(lib, "Version.lib")

#include "Auth.h"
#include "Utils.h"

client_info auth::get_client_info(const DWORD& pid, const bool riot_client)
{
	if (!pid)
		return {};

	const std::string cmd_line = utils::wstring_to_string(get_process_command_line(pid));
	if (cmd_line.empty())
		return {};

	client_info info;
	info.port = get_port(cmd_line, riot_client);
	info.token = get_token(cmd_line, riot_client);
	info.path = get_process_path(pid);
	info.version = get_file_version(info.path);

	return info;
}

int auth::get_port(const std::string& cmd_line, const bool riot_client)
{
	std::regex regex_str;
	regex_str = riot_client ? ("--riotclient-app-port=(\\d*)") : ("--app-port=(\\d*)");
	if (std::smatch m; std::regex_search(cmd_line, m, regex_str))
		return std::stoi(m[1].str());

	return 0;
}

std::string auth::get_token(const std::string& cmd_line, bool riot_client)
{
	std::regex regex_str;
	regex_str = riot_client ? ("--riotclient-auth-token=([\\w-]*)") : ("--remoting-auth-token=([\\w-]*)");
	std::smatch m;
	if (std::regex_search(cmd_line, m, regex_str))
	{
		std::string token = "riot:" + m[1].str();
		char* token_array = token.data();
		return base64_.encode(reinterpret_cast<unsigned char*>(token_array), token.size());
	}

	return "";
}

std::string auth::make_league_header(const client_info& info)
{
	return "Host: 127.0.0.1:" + std::to_string(info.port) + "\r\n" +
		"Connection: keep-alive" + "\r\n" +
		"Authorization: Basic " + info.token + "\r\n" +
		"Accept: application/json" + "\r\n" +
		"Content-Type: application/json" + "\r\n" +
		"Origin: https://127.0.0.1:" + std::to_string(info.port) + "\r\n" +
		"User-Agent: Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) LeagueOfLegendsClient/"
		+ info.version + " (CEF 91) Safari/537.36" + "\r\n" +
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

std::string auth::make_riot_header(const client_info& info)
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
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) RiotClient/" + info.
		version + " (CEF 74) Safari/537.36" + "\r\n" +
		"sec-ch-ua: Chromium" + "\r\n" +
		"Referer: https://127.0.0.1:" + std::to_string(info.port) + "/index.html" + "\r\n" +
		"Accept-Encoding: gzip, deflate, br" + "\r\n" +
		"Accept-Language: en-US,en;q=0.9\r\n\r\n";
}

DWORD auth::get_process_id(const std::wstring& process_name)
{
	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof(PROCESSENTRY32W);
		if (Process32FirstW(snapshot, &entry))
		{
			do
			{
				if (std::wstring(entry.szExeFile) == process_name)
				{
					CloseHandle(snapshot);
					return entry.th32ProcessID;
				}
			}
			while (Process32NextW(snapshot, &entry));
		}
	}
	CloseHandle(snapshot);
	return 0;
}

std::vector<DWORD> auth::get_all_process_ids(const std::wstring& process_name)
{
	std::vector<DWORD> process_ids;
	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof(PROCESSENTRY32W);
		if (Process32FirstW(snapshot, &entry))
		{
			do
			{
				if (std::wstring(entry.szExeFile) == process_name)
				{
					process_ids.emplace_back(entry.th32ProcessID);
				}
			}
			while (Process32NextW(snapshot, &entry));
		}
	}
	CloseHandle(snapshot);
	return process_ids;
}

std::wstring auth::get_process_command_line(const DWORD& process_id)
{
	using t_nt_query_information_process = NTSTATUS(__stdcall*)
	(
		HANDLE process_handle,
		ULONG process_information_class,
		PVOID process_information,
		ULONG process_information_length,
		PULONG return_length
	);

	std::wstring result;
	const HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, process_id);

	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	BOOL wow;
	IsWow64Process(GetCurrentProcess(), &wow);

	const DWORD process_parameters_offset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	const DWORD command_line_offset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	const DWORD peb_size = process_parameters_offset + 8; // size until ProcessParameters
	const auto peb = static_cast<PBYTE>(malloc(peb_size));
	if (peb == nullptr) {
	    throw std::bad_alloc();
	}
	ZeroMemory(peb, peb_size);

	const DWORD process_parameters_size = command_line_offset + 16;
	const auto process_parameters = static_cast<PBYTE>(malloc(process_parameters_size));
	if (process_parameters == nullptr) {
	    free(peb);
	    throw std::bad_alloc();
	}
	ZeroMemory(process_parameters, process_parameters_size);

	if (wow)
	{
		using process_basic_information_wow64 = struct process_basic_information_wow64
		{
			PVOID reserved1[2];
			PVOID64 peb_base_address;
			PVOID reserved2[4];
			ULONG_PTR unique_process_id[2];
			PVOID reserved3[2];
		};

		using unicode_string_wow64 = struct unicode_string_wow64
		{
			USHORT length;
			USHORT maximum_length;
			PVOID64 buffer;
		};

		using t_nt_wow64_read_virtual_memory64 = NTSTATUS(NTAPI*)(
			IN HANDLE handle,
			IN PVOID64 base_address,
			OUT PVOID buffer,
			IN ULONG64 size,
			OUT PULONG64 number_of_bytes_read);

		process_basic_information_wow64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		if (const auto nt_query_information_process =
			reinterpret_cast<t_nt_query_information_process>(GetProcAddress(GetModuleHandleA("ntdll.dll"),
			                                                                "NtWow64QueryInformationProcess64")); nt_query_information_process(
			process_handle, 0, &pbi, sizeof(pbi), nullptr) != 0)
		{
		    MessageBoxA(nullptr, "NtWow64QueryInformationProcess64 failed", nullptr, 0);
		    CloseHandle(process_handle);
		    return {};
		}

		const auto nt_wow64_read_virtual_memory64 =
			reinterpret_cast<t_nt_wow64_read_virtual_memory64>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64"));

		if (nt_wow64_read_virtual_memory64(process_handle, pbi.peb_base_address, peb, peb_size, nullptr) != 0)
		{
			MessageBoxA(nullptr, "PEB NtWow64ReadVirtualMemory64 failed", nullptr, 0);
			CloseHandle(process_handle);
			return {};
		}

		if (const auto parameters = *reinterpret_cast<PVOID64*>(peb + process_parameters_offset); nt_wow64_read_virtual_memory64(
				process_handle, parameters, process_parameters, process_parameters_size, nullptr) !=
			0)
		{
			MessageBoxA(nullptr, "processParameters NtWow64ReadVirtualMemory64 failed", nullptr, 0);
			CloseHandle(process_handle);
			return {};
		}

		const auto p_command_line = reinterpret_cast<unicode_string_wow64*>(process_parameters + command_line_offset);
		const auto command_line_copy = static_cast<PWSTR>(malloc(p_command_line->maximum_length));
		if (command_line_copy == nullptr)
		{
		    MessageBoxA(nullptr, "Memory allocation failed", nullptr, 0);
		    CloseHandle(process_handle);
		    return {};
		}

		if (nt_wow64_read_virtual_memory64(process_handle, p_command_line->buffer, command_line_copy,
		                                   p_command_line->maximum_length, nullptr) != 0)
		{
		    MessageBoxA(nullptr, "pCommandLine NtWow64ReadVirtualMemory64 failed", nullptr, 0);
		    CloseHandle(process_handle);
		    return {};
		}

		result = std::wstring(command_line_copy);
		CloseHandle(process_handle);
	}
	else
	{
		using process_basic_information = struct process_basic_information
		{
			LONG exit_status;
			PVOID peb_base_address;
			ULONG_PTR affinity_mask;
			LONG base_priority;
			HANDLE unique_process_id;
			HANDLE inherited_from_unique_process_id;
		};

		using unicode_string = struct unicode_string
		{
			USHORT length;
			USHORT maximum_length;
			PWSTR buffer;
		};

		process_basic_information pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		if (const auto nt_query_information_process =
				reinterpret_cast<t_nt_query_information_process>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess"));
		    nt_query_information_process(process_handle, 0, &pbi, sizeof(pbi), nullptr) != 0)
		{
		    MessageBoxA(nullptr, "NtQueryInformationProcess failed", nullptr, 0);
		    CloseHandle(process_handle);
		    return {};
		}

		if (!ReadProcessMemory(process_handle, pbi.peb_base_address, peb, peb_size, nullptr))
		{
			MessageBoxA(nullptr, "PEB ReadProcessMemory failed", nullptr, 0);
			CloseHandle(process_handle);
			return {};
		}

		if (const auto parameters = static_cast<PBYTE*>(*reinterpret_cast<LPVOID*>(peb + process_parameters_offset)); !ReadProcessMemory(
			process_handle, parameters, process_parameters, process_parameters_size, nullptr))
		{
			MessageBoxA(nullptr, "processParameters ReadProcessMemory failed", nullptr, 0);
			CloseHandle(process_handle);
			return {};
		}

		const auto p_command_line = reinterpret_cast<unicode_string*>(process_parameters + command_line_offset);
		const auto command_line_copy = static_cast<PWSTR>(malloc(p_command_line->maximum_length));
		if (!command_line_copy)
		{
		    // Handle memory allocation failure
		    MessageBoxA(nullptr, "Memory allocation for command_line_copy failed", nullptr, 0);
		    CloseHandle(process_handle);
		    return {};
		}

		if (!ReadProcessMemory(process_handle, p_command_line->buffer, command_line_copy, p_command_line->maximum_length, nullptr))
		{
			MessageBoxA(nullptr, "pCommandLine ReadProcessMemory failed", nullptr, 0);
		    CloseHandle(process_handle);
		    return {};
		}

		result = std::wstring(command_line_copy);
		CloseHandle(process_handle);
	}

	return result;
}

std::wstring auth::get_process_path(const DWORD& process_id)
{
	if (const HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, process_id))
	{
		if (WCHAR result[MAX_PATH]; GetModuleFileNameExW(process_handle, nullptr, result, MAX_PATH))
		{
			CloseHandle(process_handle);
			return std::basic_string(result);
		}
		CloseHandle(process_handle);
	}
	return L"";
}

std::string auth::get_file_version(const std::wstring& file)
{
	if (const DWORD version_size = GetFileVersionInfoSizeW(file.c_str(), nullptr))
	{
		std::vector<unsigned char> version_info(version_size);

		if (GetFileVersionInfoW(file.c_str(), 0, version_size, &version_info[0]))
		{
			VS_FIXEDFILEINFO* lp_ffi;
			UINT size = sizeof(VS_FIXEDFILEINFO);
			if (VerQueryValueW(&version_info[0], L"\\", reinterpret_cast<LPVOID*>(&lp_ffi), &size))
			{
				const DWORD dw_file_version_ms = lp_ffi->dwFileVersionMS;
				const DWORD dw_file_version_ls = lp_ffi->dwFileVersionLS;
				std::string result = std::format("{}.{}.{}.{}",
				                                 HIWORD(dw_file_version_ms), LOWORD(dw_file_version_ms),
				                                 HIWORD(dw_file_version_ls), LOWORD(dw_file_version_ls));
				return result;
			}
		}
	}

	return "";
}
