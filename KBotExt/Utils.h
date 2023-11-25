#pragma once

#include <cpr/cpr.h>

class Utils
{
public:
	Utils() = default;
	~Utils() = default;

	static int RandomInt(int min, int max);

	static std::string ToLower(std::string str);
	static std::wstring ToLower(std::wstring wstr);

	static std::string ToUpper(std::string str);
	static std::wstring ToUpper(std::wstring wstr);

	//check if strA contains strB
	static bool StringContains(std::string strA, std::string strB, bool ignoreCase = false);
	static bool StringContains(std::wstring strA, std::wstring strB, bool ignoreCase = false);

	static std::wstring StringToWstring(const std::string& str);
	static std::string WstringToString(const std::wstring& wstr);

	static std::vector<std::string> StringSplit(std::string str, const std::string& separator, int max = -1);

	static std::string RandomString(size_t size);
	static std::wstring RandomWString(size_t size, std::pair<unsigned, unsigned> range = { 0, 0 });

	static void CopyToClipboard(const std::string& text);

	static bool DownloadFile(std::string fileName, const std::string& directory, const std::string& url);

	static bool ContainsOnlyASCII(const std::string& buff);

	static std::string Utf8Encode(const std::wstring& wstr);

	static std::string Exec(const char* cmd);

	static std::string RenameExe();

	static void OpenUrl(const char* url, const char* args = nullptr, int flags = SW_SHOWNORMAL);
	static void OpenUrl(const wchar_t* url, const wchar_t* args = nullptr, int flags = SW_SHOWNORMAL);

	// adds the "Hidden" attribute to a file
	static bool HideFile(const std::string& file);

	static bool RunAsUser(LPCWSTR lpApplicationName, LPWSTR lpCommandLine);

	static cpr::Header StringToHeader(const std::string& str);
};
