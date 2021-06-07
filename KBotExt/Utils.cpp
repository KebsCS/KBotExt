#include <locale>
#include <codecvt>
#include <sstream>
#include <filesystem>
#include <array>
//URLDownloadToFileA
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#include "Utils.h"
#include "Definitions.h"

std::string Utils::ToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) std::tolower);
	return str;
}

std::wstring Utils::ToLower(std::wstring str)
{
	std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) std::tolower);
	return str;
}

std::string Utils::ToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) std::toupper);
	return str;
}

bool Utils::StringContains(std::string strA, std::string strB, bool ignore_case)
{
	if (strA.empty() || strB.empty())
		return true;

	if (ignore_case)
	{
		strA = ToLower(strA);
		strB = ToLower(strB);
	}

	if (strA.find(strB) != std::string::npos)
		return true;

	return false;
}

bool Utils::StringContains(std::wstring strA, std::wstring strB, bool ignore_case)
{
	if (strA.empty() || strB.empty())
		return true;

	if (ignore_case)
	{
		strA = ToLower(strA);
		strB = ToLower(strB);
	}

	if (strA.find(strB) != std::wstring::npos)
		return true;

	return false;
}

std::string Utils::WstringToString(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	try
	{
		return converter.to_bytes(wstr);
	}
	catch (std::range_error)
	{
		/*std::stringstream s;
		s << wstr.c_str();
		return s.str();*/
		return "range_error";
	}
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

std::string Utils::RandomString(int size)
{
	std::string str = "";

	for (int i = 0; i < size; i++)
		str += RandomInt(0, 1) ? RandomInt(48, 57) : RandomInt(97, 122);

	return str;
}

std::string Utils::FormatString(const char* c, const char* args...)
{
	char buff[200];
	sprintf_s(buff, c, args);

	return std::string(buff);
}

void Utils::CopyToClipboard(std::string text)
{
	if (!::OpenClipboard(NULL))
		return;
	const int wbuf_length = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
	HGLOBAL wbuf_handle = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(WCHAR));
	if (wbuf_handle == NULL)
	{
		::CloseClipboard();
		return;
	}
	WCHAR* wbuf_global = (WCHAR*)::GlobalLock(wbuf_handle);
	::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wbuf_global, wbuf_length);
	::GlobalUnlock(wbuf_handle);
	::EmptyClipboard();
	if (::SetClipboardData(CF_UNICODETEXT, wbuf_handle) == NULL)
		::GlobalFree(wbuf_handle);
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
	for (int i = 0; i < buff.size(); ++i)
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
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
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
	newname = utils->RandomString(RandomInt(5, 10));
	newname += ".exe";
	if (!rename(exe.c_str(), newname.c_str()))
		return true;
	else return false;
}

bool Utils::HideFile(std::string path)
{
	int attr = GetFileAttributesA(path.c_str());
	if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)
	{
		SetFileAttributesA(path.c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
		return true;
	}
	return false;
}

Utils* utils = new Utils();