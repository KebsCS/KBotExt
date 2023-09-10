#pragma once

#include <cpr/cpr.h>

class utils
{
public:
	utils() = default;
	~utils() = default;

	static int random_int(int min, int max);

	static std::string to_lower(std::string str);
	static std::wstring to_lower(std::wstring wstr);

	static std::string to_upper(std::string str);
	static std::wstring to_upper(std::wstring wstr);

	static bool string_contains(std::string str_a, std::string str_b, bool ignore_case = false);
	static bool string_contains(std::wstring str_a, std::wstring str_b, bool ignore_case = false);

	static std::wstring string_to_wstring(const std::string& str);
	static std::string wstring_to_string(const std::wstring& wstr);

	static std::vector<std::string> string_split(std::string str, const std::string& separator, int max = -1);

	static std::string random_string(size_t size);
	static std::wstring random_w_string(size_t size, std::pair<unsigned, unsigned> range = {0, 0});

	static void copy_to_clipboard(const std::string& text);

	static bool download_file(std::string file_name, const std::string& directory, const std::string& url);

	static bool contains_only_ascii(const std::string& buff);

	static std::string utf8_encode(const std::wstring& wstr);

	static std::string exec(const char* cmd);

	static bool rename_exe();

	static bool hide_file(const std::string& file);

	static bool run_as_user(LPCWSTR lp_application_name, LPWSTR lp_command_line);

	static cpr::Header string_to_header(const std::string& str);
};
