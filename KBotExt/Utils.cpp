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
#include "Definitions.h"

int Utils::RandomInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(min, max);

	return distr(gen);
}

std::string Utils::ToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) {return static_cast<char>(std::tolower(c)); });
	return str;
}

std::wstring Utils::ToLower(std::wstring wstr)
{
	std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towlower);
	return wstr;
}

std::string Utils::ToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) {return static_cast<char>(std::toupper(c)); });
	return str;
}

std::wstring Utils::ToUpper(std::wstring wstr)
{
	std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towupper);
	return wstr;
}

bool Utils::StringContains(std::string strA, std::string strB, bool ignoreCase)
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

bool Utils::StringContains(std::wstring wstrA, std::wstring wstrB, bool ignoreCase)
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

std::string Utils::WstringToString(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wstr);
}

std::wstring Utils::StringToWstring(std::string str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	try
	{
		return converter.from_bytes(str);
	}
	catch (std::range_error)
	{
		std::wostringstream s;
		s << str.c_str();
		return s.str();
	}
}

std::vector<std::string> Utils::StringSplit(std::string str, std::string separator)
{
	size_t pos = 0;
	std::string token;
	std::vector<std::string> vec;
	while ((pos = str.find(separator)) != std::string::npos)
	{
		token = str.substr(0, pos);
		vec.emplace_back(token);
		str.erase(0, pos + separator.length());
	}
	vec.emplace_back(str);
	return vec;
}

std::string Utils::RandomString(size_t size)
{
	std::string str = "";

	for (size_t i = 0; i < size; i++)
		str += RandomInt(0, 1) ? RandomInt(48, 57) : RandomInt(97, 122);

	return str;
}

std::wstring Utils::RandomWString(size_t size, std::pair<unsigned, unsigned>range)
{
	std::wstring str = L"";

	if (range.first == 0 && range.second == 0)
	{
		for (size_t i = 0; i < size; i++)
			str += RandomInt(0, 1) ? RandomInt(48, 57) : RandomInt(97, 122);
	}
	else
	{
		for (size_t i = 0; i < size; i++)
			str += (wchar_t)RandomInt(range.first, range.second);
	}

	return str;
}

void Utils::CopyToClipboard(std::string text)
{
	if (!::OpenClipboard(NULL))
		return;
	const int wbufLen = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
	HGLOBAL wbufHandle = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbufLen * sizeof(WCHAR));
	if (wbufHandle == NULL)
	{
		::CloseClipboard();
		return;
	}
	WCHAR* wbufGlobal = (WCHAR*)::GlobalLock(wbufHandle);
	::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wbufGlobal, wbufLen);
	::GlobalUnlock(wbufHandle);
	::EmptyClipboard();
	if (::SetClipboardData(CF_UNICODETEXT, wbufHandle) == NULL)
		::GlobalFree(wbufHandle);
	::CloseClipboard();
}

bool Utils::DownloadFile(std::string fileName, std::string directory, std::string url)
{
	// Create folder if it doesn't exists
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directory(directory);
	if (fileName[0] == '\\')
		fileName.erase(0);
	std::string file = directory + "\\" + fileName;

	// Don't download if file already exists
	if (std::filesystem::exists(file))
		return true;

	std::string fullPath = std::filesystem::current_path().string() + "\\" + file;
	std::string toDownload = url + fileName;

	// Download file
	HRESULT result = URLDownloadToFileA(NULL, toDownload.c_str(), fullPath.c_str(), 0, NULL);

	if (result != S_OK)
		return false;
	return true;
}

bool Utils::ContainsOnlyASCII(std::string buff)
{
	for (size_t i = 0; i < buff.size(); ++i)
	{
		if (buff[i] == 0)
			return true;
		if ((unsigned char)buff[i] > 127)
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
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

bool Utils::RenameExe()
{
	char szExeFileName[MAX_PATH];
	GetModuleFileNameA(NULL, szExeFileName, MAX_PATH);
	std::string path = std::string(szExeFileName);
	std::string exe = path.substr(path.find_last_of("\\") + 1, path.size());
	std::string newname;
	newname = Utils::RandomString(RandomInt(5, 10));
	newname += ".exe";
	if (!rename(exe.c_str(), newname.c_str()))
		return true;
	else return false;
}

bool Utils::HideFile(std::string file)
{
	int attr = GetFileAttributesA(file.c_str());
	if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)
	{
		SetFileAttributesA(file.c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
		return true;
	}
	return false;
}

bool Utils::RunAsUser(LPCWSTR lpApplicationName, LPWSTR lpCommandLine)
{
	typedef BOOL(WINAPI* tOpenProcessToken)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
	static tOpenProcessToken OpenProcessToken = (tOpenProcessToken)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "OpenProcessToken");

	HANDLE hProcessToken = 0;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
	{
		// fails if current process isn't elevated
		CloseHandle(hProcessToken);
		return false;
	}

	typedef BOOL(WINAPI* tLookupPrivilegeValueW)(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);
	static tLookupPrivilegeValueW LookupPrivilegeValueW = (tLookupPrivilegeValueW)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "LookupPrivilegeValueW");

	TOKEN_PRIVILEGES tkp = { 0 };
	tkp.PrivilegeCount = 1;
	if (!LookupPrivilegeValueW(NULL, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid))
	{
		CloseHandle(hProcessToken);
		return false;
	}

	typedef BOOL(WINAPI* tAdjustTokenPrivileges)(HANDLE TokenHandle, BOOL DisableAllPrivileges,
		PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
	static tAdjustTokenPrivileges AdjustTokenPrivileges = (tAdjustTokenPrivileges)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "AdjustTokenPrivileges");

	DWORD returnLength = 0;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hProcessToken, FALSE, &tkp, 0, NULL, &returnLength))
	{
		CloseHandle(hProcessToken);
		return false;
	}

	typedef HWND(WINAPI* tGetShellWindow)();
	static tGetShellWindow GetShellWindow = (tGetShellWindow)GetProcAddress(LoadLibraryW(L"user32.dll"), "GetShellWindow");

	HWND hwnd = GetShellWindow();
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

	HANDLE hShellProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!hShellProcess)
	{
		CloseHandle(hProcessToken);
		return false;
	}

	HANDLE hShellProcessToken = 0;
	if (!OpenProcessToken(hShellProcess, TOKEN_DUPLICATE, &hShellProcessToken))
	{
		CloseHandle(hProcessToken);
		CloseHandle(hShellProcess);
		CloseHandle(hShellProcessToken);
		return false;
	}

	typedef BOOL(WINAPI* tDuplicateTokenEx)(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES pTokenAttributes,
		SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken);
	static tDuplicateTokenEx DuplicateTokenEx = (tDuplicateTokenEx)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "DuplicateTokenEx");

	HANDLE hPrimaryToken = 0;
	if (!DuplicateTokenEx(hShellProcessToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hPrimaryToken))
	{
		CloseHandle(hProcessToken);
		CloseHandle(hShellProcess);
		CloseHandle(hShellProcessToken);
		CloseHandle(hPrimaryToken);
		return false;
	}

	typedef BOOL(WINAPI* tCreateProcessWithTokenW)(HANDLE hToken, DWORD dwLogonFlags, LPCWSTR lpApplicationName,
		LPWSTR lpCommandLine, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
		LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	static tCreateProcessWithTokenW CreateProcessWithTokenW = (tCreateProcessWithTokenW)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "CreateProcessWithTokenW");

	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOWNORMAL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	if (!CreateProcessWithTokenW(hPrimaryToken, 0, lpApplicationName, lpCommandLine, 0/*DETACHED_PROCESS - error 87*/, NULL, NULL, &si, &pi))
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