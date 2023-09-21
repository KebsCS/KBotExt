#include <locale>
#include <codecvt>
#include <sstream>
#include <filesystem>
#include <array>
#include <cwctype>
#include <random>
//URLDownloadToFileA
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#include "Utils.h"

int Utils::RandomInt(const int min, const int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution dist(min, max);

	return dist(gen);
}

std::string Utils::ToLower(std::string str)
{
	std::ranges::transform(str, str.begin(),
	                       [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return str;
}

std::wstring Utils::ToLower(std::wstring wstr)
{
	std::ranges::transform(wstr, wstr.begin(), std::towlower);
	return wstr;
}

std::string Utils::ToUpper(std::string str)
{
	std::ranges::transform(str, str.begin(),
	                       [](const unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return str;
}

std::wstring Utils::ToUpper(std::wstring wstr)
{
	std::ranges::transform(wstr, wstr.begin(), std::towupper);
	return wstr;
}

bool Utils::StringContains(std::string strA, std::string strB, const bool ignoreCase)
{
	if (strA.empty() || strB.empty())
		return true;

	if (ignoreCase)
	{
		strA = ToLower(strA);
		strB = ToLower(strB);
	}

	if (strA.find(strB) != std::string::npos)
		return true;

	return false;
}

bool Utils::StringContains(std::wstring wstrA, std::wstring wstrB, const bool ignoreCase)
{
	if (wstrA.empty() || wstrB.empty())
		return true;

	if (ignoreCase)
	{
		wstrA = ToLower(wstrA);
		wstrB = ToLower(wstrB);
	}

	if (wstrA.find(wstrB) != std::wstring::npos)
		return true;

	return false;
}

std::string Utils::WstringToString(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

std::wstring Utils::StringToWstring(const std::string& str)
{
	try
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(str);
	}
	catch (std::range_error) // ThrowByValueCatchByrReference)
	{
		std::wostringstream s;
		s << str.c_str();
		return s.str();
	}
}

std::vector<std::string> Utils::StringSplit(std::string str, const std::string& separator, const int max)
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

std::string Utils::RandomString(const size_t size)
{
	std::string str;

	for (size_t i = 0; i < size; i++)
		str += std::to_string(RandomInt(0, 1) ? RandomInt(48, 57) : RandomInt(97, 122));

	return str;
}

std::wstring Utils::RandomWString(const size_t size, const std::pair<unsigned, unsigned> range)
{
	std::wstring str;

	if (range.first == 0 && range.second == 0)
	{
		for (size_t i = 0; i < size; i++)
			str += std::to_wstring(RandomInt(0, 1) ? RandomInt(48, 57) : RandomInt(97, 122));
	}
	else
	{
		for (size_t i = 0; i < size; i++)
			str += static_cast<wchar_t>(RandomInt(range.first, range.second));
	}

	return str;
}

void Utils::CopyToClipboard(const std::string& text)
{
	if (!OpenClipboard(nullptr))
		return;
	const int wbufLen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
	const HGLOBAL wbufHandle = GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(wbufLen) * sizeof(WCHAR));
	if (wbufHandle == nullptr)
	{
		CloseClipboard();
		return;
	}
	const auto wbufGlobal = static_cast<WCHAR*>(GlobalLock(wbufHandle));
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wbufGlobal, wbufLen);
	GlobalUnlock(wbufHandle);
	EmptyClipboard();
	if (SetClipboardData(CF_UNICODETEXT, wbufHandle) == nullptr)
		GlobalFree(wbufHandle);
	CloseClipboard();
}

bool Utils::DownloadFile(std::string fileName, const std::string& directory, const std::string& url)
{
	// Create folder if it doesn't exists
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directory(directory);
	if (fileName[0] == '\\')
		fileName.erase(0);
	const std::string file = directory + "\\" + fileName;

	// Don't download if file already exists
	if (std::filesystem::exists(file))
		return true;

	const std::string fullPath = std::filesystem::current_path().string() + "\\" + file;
	const std::string toDownload = url + fileName;

	// Download file

	if (const HRESULT result = URLDownloadToFileA(nullptr, toDownload.c_str(), fullPath.c_str(), 0, nullptr); result != S_OK)
		return false;
	return true;
}

bool Utils::ContainsOnlyASCII(const std::string& buff)
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

std::string Utils::Utf8Encode(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.to_bytes(wstr);
}

std::string Utils::Exec(const char* cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	const std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}
	return result;
}

bool Utils::RenameExe()
{
	char szExeFileName[MAX_PATH];
	GetModuleFileNameA(nullptr, szExeFileName, MAX_PATH);
	const auto path = std::string(szExeFileName);
	const std::string exe = path.substr(path.find_last_of('\\') + 1, path.size());
	std::string newname = RandomString(RandomInt(5, 10));
	newname += ".exe";
	if (!rename(exe.c_str(), newname.c_str()))
		return true;
	return false;
}

bool Utils::HideFile(const std::string& file)
{
	if (const int attr = GetFileAttributesA(file.c_str()); (attr & FILE_ATTRIBUTE_HIDDEN) == 0)
	{
		SetFileAttributesA(file.c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
		return true;
	}
	return false;
}

bool Utils::RunAsUser(const LPCWSTR lpApplicationName, const LPWSTR lpCommandLine)
{
	using tOpenProcessToken = BOOL(WINAPI*)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
	static auto OpenProcessToken = reinterpret_cast<tOpenProcessToken>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "OpenProcessToken"));

	HANDLE hProcessToken = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
	{
		// fails if current process isn't elevated
		CloseHandle(hProcessToken);
		return false;
	}

	using tLookupPrivilegeValueW = BOOL(WINAPI*)(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);
	static auto LookupPrivilegeValueW = reinterpret_cast<tLookupPrivilegeValueW>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "LookupPrivilegeValueW"));

	TOKEN_PRIVILEGES tkp = {0};
	tkp.PrivilegeCount = 1;
	if (!LookupPrivilegeValueW(nullptr, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid))
	{
		CloseHandle(hProcessToken);
		return false;
	}

	using tAdjustTokenPrivileges = BOOL(WINAPI*)(HANDLE TokenHandle, BOOL DisableAllPrivileges,
	                                             PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState,
	                                             PDWORD ReturnLength);
	static auto AdjustTokenPrivileges = reinterpret_cast<tAdjustTokenPrivileges>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "AdjustTokenPrivileges"));

	DWORD returnLength = 0;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hProcessToken, FALSE, &tkp, 0, nullptr, &returnLength))
	{
		CloseHandle(hProcessToken);
		return false;
	}

	using tGetShellWindow = HWND(WINAPI*)();
	static auto GetShellWindow = reinterpret_cast<tGetShellWindow>(GetProcAddress(LoadLibraryW(L"user32.dll"), "GetShellWindow"));

	const HWND hwnd = GetShellWindow();
	if (!hwnd)
	{
		CloseHandle(hProcessToken);
		return false;
	}

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (!pid)
	{
		CloseHandle(hProcessToken);
		return false;
	}

	const HANDLE hShellProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!hShellProcess)
	{
		CloseHandle(hProcessToken);
		return false;
	}

	HANDLE hShellProcessToken = nullptr;
	if (!OpenProcessToken(hShellProcess, TOKEN_DUPLICATE, &hShellProcessToken))
	{
		CloseHandle(hProcessToken);
		CloseHandle(hShellProcess);
		CloseHandle(hShellProcessToken);
		return false;
	}

	using tDuplicateTokenEx = BOOL(WINAPI*)(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES pTokenAttributes,
	                                        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken);
	static auto DuplicateTokenEx = reinterpret_cast<tDuplicateTokenEx>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "DuplicateTokenEx"));

	HANDLE hPrimaryToken = nullptr;
	if (!DuplicateTokenEx(hShellProcessToken, TOKEN_ALL_ACCESS, nullptr, SecurityImpersonation, TokenPrimary, &hPrimaryToken))
	{
		CloseHandle(hProcessToken);
		CloseHandle(hShellProcess);
		CloseHandle(hShellProcessToken);
		CloseHandle(hPrimaryToken);
		return false;
	}

	using tCreateProcessWithTokenW = BOOL(WINAPI*)(HANDLE hToken, DWORD dwLogonFlags, LPCWSTR lpApplicationName,
	                                               LPWSTR lpCommandLine, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
	                                               LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	static auto CreateProcessWithTokenW = reinterpret_cast<tCreateProcessWithTokenW>(GetProcAddress(
		LoadLibraryW(L"advapi32.dll"), "CreateProcessWithTokenW"));

	PROCESS_INFORMATION pi = {nullptr};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOWNORMAL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	if (!CreateProcessWithTokenW(hPrimaryToken, 0, lpApplicationName, lpCommandLine, 0/*DETACHED_PROCESS - error 87*/, nullptr, nullptr, &si, &pi))
	{
		//printf("%d",GetLastError());
		CloseHandle(hProcessToken);
		CloseHandle(hShellProcess);
		CloseHandle(hShellProcessToken);
		CloseHandle(hPrimaryToken);
		CloseHandle(pi.hProcess);
		return false;
	}

	CloseHandle(hProcessToken);
	CloseHandle(hShellProcess);
	CloseHandle(hShellProcessToken);
	CloseHandle(hPrimaryToken);
	CloseHandle(pi.hProcess);
	return true;
}

cpr::Header Utils::StringToHeader(const std::string& str)
{
	cpr::Header header;
	for (const auto& line : StringSplit(str, "\r\n"))
	{
		if (const auto index = line.find(": ", 0); index != std::string::npos)
		{
			header.insert({line.substr(0, index), line.substr(index + 2)});
		}
	}
	return header;
}
