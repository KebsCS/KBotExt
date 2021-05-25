#pragma once

#include <chrono>

class Utils
{
private:

public:

	Utils()
	{
	}

	~Utils() = default;

	//converts whole string to lowercase
	std::string ToLower(std::string str);
	std::wstring ToLower(std::wstring str);

	//converts whole string to uppercase
	std::string ToUpper(std::string str);

	//check if strA contains strB
	bool StringContains(std::string strA, std::string strB, bool ignore_case = false);
	bool StringContains(std::wstring strA, std::wstring strB, bool ignore_case = false);

	//string to wstring
	std::wstring StringToWstring(std::string str);

	std::string WstringToString(std::wstring wstr);

	std::string RandomString(int size);

	std::string FormatString(const char* c, const char* args...);

	void CopyToClipboard(std::string text);

	bool DownloadFile(std::string fileName, std::string directory = "Data", std::string url = "https://raw.githubusercontent.com/y3541599/test/main/");

	bool ContainsOnlyASCII(std::string buff);

	std::string Utf8Encode(const std::wstring& wstr);

	std::string Exec(const char* cmd);

	// renames program to random string
	bool RenameExe();
};

extern Utils* utils;
