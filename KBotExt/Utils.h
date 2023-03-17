#pragma once

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

	static std::wstring StringToWstring(std::string str);
	static std::string WstringToString(std::wstring wstr);

	static std::vector<std::string> StringSplit(std::string str, std::string separator);

	static std::string RandomString(size_t size);
	static std::wstring RandomWString(size_t size, std::pair<unsigned, unsigned>range = { 0,0 });

	static void CopyToClipboard(std::string text);

	static bool DownloadFile(std::string fileName, std::string directory, std::string url);

	static bool ContainsOnlyASCII(std::string buff);

	static std::string Utf8Encode(const std::wstring& wstr);

	static std::string Exec(const char* cmd);

	static bool RenameExe();

	// adds the "Hidden" attribute to a file
	static bool HideFile(std::string file);

	static bool RunAsUser(LPCWSTR lpApplicationName, LPWSTR lpCommandLine);
};