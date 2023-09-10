#include <locale>
#include <codecvt>
#include <sstream>
#include <filesystem>
#include <array>
#include <cwctype>
#include <random>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#include "Utils.h"
#include "Definitions.h"

int utils::random_int(const int min, const int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution dist(min, max);

	return dist(gen);
}

std::string utils::to_lower(std::string str)
{
	std::ranges::transform(str, str.begin(),
	                       [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return str;
}

std::wstring utils::to_lower(std::wstring wstr)
{
	std::ranges::transform(wstr, wstr.begin(), std::towlower);
	return wstr;
}

std::string utils::to_upper(std::string str)
{
	std::ranges::transform(str, str.begin(),
	                       [](const unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return str;
}

std::wstring utils::to_upper(std::wstring wstr)
{
	std::ranges::transform(wstr, wstr.begin(), std::towupper);
	return wstr;
}

bool utils::string_contains(std::string str_a, std::string str_b, const bool ignore_case)
{
	if (str_a.empty() || str_b.empty())
		return true;

	if (ignore_case)
	{
		str_a = to_lower(str_a);
		str_b = to_lower(str_b);
	}

	if (str_a.find(str_b) != std::string::npos)
		return true;

	return false;
}

bool utils::string_contains(std::wstring wstr_a, std::wstring wstr_b, const bool ignore_case)
{
	if (wstr_a.empty() || wstr_b.empty())
		return true;

	if (ignore_case)
	{
		wstr_a = to_lower(wstr_a);
		wstr_b = to_lower(wstr_b);
	}

	if (wstr_a.find(wstr_b) != std::wstring::npos)
		return true;

	return false;
}

std::string utils::wstring_to_string(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

std::wstring utils::string_to_wstring(const std::string& str)
{
	try
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(str);
	}
	catch (std::range_error)
	{
		std::wostringstream s;
		s << str.c_str();
		return s.str();
	}
}

std::vector<std::string> utils::string_split(std::string str, const std::string& separator, const int max)
{
	int count = 0;
	size_t pos;
	std::vector<std::string> vec;
	while ((pos = str.find(separator)) != std::string::npos)
	{
		if (max != -1 && count == max)
		{
			break;
		}
		count++;
		std::string token = str.substr(0, pos);
		vec.emplace_back(token);
		str.erase(0, pos + separator.length());
	}
	vec.emplace_back(str);
	return vec;
}

std::string utils::random_string(const size_t size)
{
	std::string str;

	for (size_t i = 0; i < size; i++)
		str += std::to_string(random_int(0, 1) ? random_int(48, 57) : random_int(97, 122));

	return str;
}

std::wstring utils::random_w_string(const size_t size, const std::pair<unsigned, unsigned> range)
{
	std::wstring str;

	if (range.first == 0 && range.second == 0)
	{
		for (size_t i = 0; i < size; i++)
			str += std::to_wstring(random_int(0, 1) ? random_int(48, 57) : random_int(97, 122));
	}
	else
	{
		for (size_t i = 0; i < size; i++)
			str += static_cast<wchar_t>(random_int(range.first, range.second));
	}

	return str;
}

void utils::copy_to_clipboard(const std::string& text)
{
	if (!OpenClipboard(nullptr))
		return;
	const int wbuf_len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
	const HGLOBAL wbuf_handle = GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(wbuf_len) * sizeof(WCHAR));
	if (wbuf_handle == nullptr)
	{
		CloseClipboard();
		return;
	}
	const auto wbuf_global = static_cast<WCHAR*>(GlobalLock(wbuf_handle));
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wbuf_global, wbuf_len);
	GlobalUnlock(wbuf_handle);
	EmptyClipboard();
	if (SetClipboardData(CF_UNICODETEXT, wbuf_handle) == nullptr)
		GlobalFree(wbuf_handle);
	CloseClipboard();
}

bool utils::download_file(std::string file_name, const std::string& directory, const std::string& url)
{
	// Create folder if it doesn't exists
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directory(directory);
	if (file_name[0] == '\\')
		file_name.erase(0);
	const std::string file = directory + "\\" + file_name;

	// Don't download if file already exists
	if (std::filesystem::exists(file))
		return true;

	const std::string full_path = std::filesystem::current_path().string() + "\\" + file;
	const std::string to_download = url + file_name;

	// Download file

	if (const HRESULT result = URLDownloadToFileA(nullptr, to_download.c_str(), full_path.c_str(), 0, nullptr); result != S_OK)
		return false;
	return true;
}

bool utils::contains_only_ascii(const std::string& buff)
{
	for (const char i : buff)
	{
		if (i == 0)
			return true;
		if (static_cast<unsigned char>(i) > 127)
			return false;
	}
	return true;
}

std::string utils::utf8_encode(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.to_bytes(wstr);
}

std::string utils::exec(const char* cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	const std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}
	return result;
}

bool utils::rename_exe()
{
	char sz_exe_file_name[MAX_PATH];
	GetModuleFileNameA(nullptr, sz_exe_file_name, MAX_PATH);
	const auto path = std::string(sz_exe_file_name);
	const std::string exe = path.substr(path.find_last_of("\\") + 1, path.size());
	std::string new_name = random_string(random_int(5, 10));
	new_name += ".exe";
	if (!rename(exe.c_str(), new_name.c_str()))
		return true;
	return false;
}

bool utils::hide_file(const std::string& file)
{
	if (const int attr = GetFileAttributesA(file.c_str()); (attr & FILE_ATTRIBUTE_HIDDEN) == 0)
	{
		SetFileAttributesA(file.c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
		return true;
	}
	return false;
}

bool utils::run_as_user(const LPCWSTR lp_application_name, const LPWSTR lp_command_line)
{
	using t_open_process_token = BOOL(WINAPI*)(HANDLE process_handle, DWORD desired_access, PHANDLE token_handle);
	static t_open_process_token open_process_token;
	open_process_token = reinterpret_cast<t_open_process_token>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "OpenProcessToken"));

	HANDLE h_process_token = nullptr;
	if (!open_process_token(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &h_process_token))
	{
		// fails if current process isn't elevated
		CloseHandle(h_process_token);
		return false;
	}

	using t_lookup_privilege_value_w = BOOL(WINAPI*)(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);
	static t_lookup_privilege_value_w lookup_privilege_value_w;
	lookup_privilege_value_w = reinterpret_cast<t_lookup_privilege_value_w>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "LookupPrivilegeValueW"));

	TOKEN_PRIVILEGES tkp = {0};
	tkp.PrivilegeCount = 1;
	if (!lookup_privilege_value_w(nullptr, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid))
	{
		CloseHandle(h_process_token);
		return false;
	}

	using t_adjust_token_privileges = BOOL(WINAPI*)(HANDLE token_handle, BOOL disable_all_privileges,
	                                                PTOKEN_PRIVILEGES new_state, DWORD buffer_length,
	                                                PTOKEN_PRIVILEGES previous_state, PDWORD return_length);
	static t_adjust_token_privileges adjust_token_privileges;
	adjust_token_privileges = reinterpret_cast<t_adjust_token_privileges>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "AdjustTokenPrivileges"));

	DWORD return_length = 0;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!adjust_token_privileges(h_process_token, FALSE, &tkp, 0, nullptr, &return_length))
	{
		CloseHandle(h_process_token);
		return false;
	}

	using t_get_shell_window = HWND(WINAPI*)();
	static t_get_shell_window get_shell_window;
	get_shell_window = reinterpret_cast<t_get_shell_window>(GetProcAddress(LoadLibraryW(L"user32.dll"), "GetShellWindow"));

	const HWND hwnd = get_shell_window();
	if (!hwnd)
	{
		CloseHandle(h_process_token);
		return false;
	}

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (!pid)
	{
		CloseHandle(h_process_token);
		return false;
	}

	const HANDLE h_shell_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!h_shell_process)
	{
		CloseHandle(h_process_token);
		return false;
	}

	HANDLE h_shell_process_token = nullptr;
	if (!open_process_token(h_shell_process, TOKEN_DUPLICATE, &h_shell_process_token))
	{
		CloseHandle(h_process_token);
		CloseHandle(h_shell_process);
		CloseHandle(h_shell_process_token);
		return false;
	}

	using t_duplicate_token_ex = BOOL(WINAPI*)(HANDLE h_existing_token, DWORD dw_desired_access,
	                                           LPSECURITY_ATTRIBUTES p_token_attributes,
	                                           SECURITY_IMPERSONATION_LEVEL impersonation_level, TOKEN_TYPE token_type,
	                                           PHANDLE ph_new_token);
	static t_duplicate_token_ex duplicate_token_ex;
	duplicate_token_ex = reinterpret_cast<t_duplicate_token_ex>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "DuplicateTokenEx"));

	HANDLE h_primary_token = nullptr;
	if (!duplicate_token_ex(h_shell_process_token, TOKEN_ALL_ACCESS, nullptr, SecurityImpersonation, TokenPrimary,
	                        &h_primary_token))
	{
		CloseHandle(h_process_token);
		CloseHandle(h_shell_process);
		CloseHandle(h_shell_process_token);
		CloseHandle(h_primary_token);
		return false;
	}

	using t_create_process_with_token_w = BOOL(WINAPI*)(HANDLE h_token, DWORD dw_logon_flags, LPCWSTR lp_application_name,
	                                                    LPWSTR lp_command_line, DWORD dw_creation_flags, LPVOID lp_environment,
	                                                    LPCWSTR lp_current_directory,
	                                                    LPSTARTUPINFOW lp_startup_info,
	                                                    LPPROCESS_INFORMATION lp_process_information);
	static t_create_process_with_token_w create_process_with_token_w;
	create_process_with_token_w = reinterpret_cast<t_create_process_with_token_w>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "CreateProcessWithTokenW"));

	PROCESS_INFORMATION pi = {nullptr};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOWNORMAL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	if (!create_process_with_token_w(h_primary_token, 0, lp_application_name, lp_command_line, 0/*DETACHED_PROCESS - error 87*/,
	                                 nullptr, nullptr, &si, &pi))
	{
		CloseHandle(h_process_token);
		CloseHandle(h_shell_process);
		CloseHandle(h_shell_process_token);
		CloseHandle(h_primary_token);
		CloseHandle(pi.hProcess);
		return false;
	}

	CloseHandle(h_process_token);
	CloseHandle(h_shell_process);
	CloseHandle(h_shell_process_token);
	CloseHandle(h_primary_token);
	CloseHandle(pi.hProcess);
	return true;
}

cpr::Header utils::string_to_header(const std::string& str)
{
	cpr::Header header;
	for (const auto& line : string_split(str, "\r\n"))
	{
		if (const auto index = line.find(': ', 0); index != std::string::npos)
		{
			header.insert({line.substr(0, index - 1), line.substr(index + 1)});
		}
	}
	return header;
}
