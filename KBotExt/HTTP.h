#pragma once

#include<Windows.h>
#include<WinInet.h>
#pragma comment(lib,"WinInet.lib")
#include <string>

class HTTP
{
private:

	static inline std::string sHttps = "https://";
	static inline std::string sHttp = "http://";

	static std::string GetHost(std::string url)
	{
		bool Ishttps;
		url.find(sHttps) == std::string::npos ? Ishttps = false : Ishttps = true;
		size_t strpos, coutof;
		Ishttps ? strpos = url.find(sHttps) + strlen(sHttps.c_str()) : strpos = url.find(sHttp) + strlen(sHttp.c_str());
		url.find('/', strpos) == std::string::npos ? coutof = url.find('/', strpos) : coutof = url.find('/', strpos);
		std::string host;
		if (Ishttps)host = url.substr(strpos, coutof - strlen(sHttps.c_str()));
		else host = url.substr(strpos, coutof - strlen(sHttp.c_str()));
		strpos = host.find(':');
		if (strpos != std::string::npos)host = host.substr(0, strpos);
		return host;
	}

	static std::string GetURLPage(std::string url)
	{
		bool Ishttps;
		url.find(sHttps) == std::string::npos ? Ishttps = false : Ishttps = true;
		size_t strpos, coutof;
		Ishttps ? strpos = url.find(sHttps) + strlen(sHttps.c_str()) : strpos = url.find(sHttp) + strlen(sHttp.c_str());
		coutof = url.find('/', strpos) + 1;
		if (coutof == std::string::npos)return "";
		std::string urlpage = url.substr(coutof, strpos - coutof);
		return urlpage;
	}

public:

	static std::string Request(std::string method, std::string url, std::string requestData = "", std::string header = "",
		std::string cookies = "", std::string returnCookies = "", int port = -1)
	{
		return RequestWithCookies(method, url, requestData, header, cookies, returnCookies, port);
	}

	static std::string RequestWithCookies(std::string method, std::string url, std::string requestData, std::string header,
		std::string cookies, std::string& returnCookies, int port = -1)
	{
		if (header.find(("Host:")) == std::string::npos) {
			header.append(("Host: "));
			header.append(GetHost(url));
			header.append(("\r\n"));
		}

		//if (header.find(("User-Agent:")) == std::string::npos)header.append(("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 Edge/16.16299\r\n"));
		//if (header.find(("Accept:")) == std::string::npos)header.append(("Accept: */*\r\n"));
		//if (header.find(("Accept-Language:")) == std::string::npos)header.append(("Accept-Language: en-US\r\n"));
		//if (header.find(("Content-Type:")) == std::string::npos)header.append(("Content-Type: application/x-www-form-urlencoded\r\n"));
		//
		//if (header.find(("Referer:")) == std::string::npos) {
		//	header.append(("Referer: "));
		//	header.append(url);
		//	header.append(("\r\n"));
		//}

		if (!cookies.empty()) {
			if (header.find(("Cookies:")) == std::string::npos) {
				header.append(("Cookies: "));
				header.append(cookies);
				header.append(("\r\n"));
			}
			else {
				header.append(cookies);
				header.append(("\r\n"));
			}
		}
		HINTERNET internetOpen = InternetOpenA(header.c_str(), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, NULL);
		if (!internetOpen)
			return ("InternetOpenA failed. Last error: ") + std::to_string(GetLastError());

		bool isHttps = true;
		url.find(("https")) == std::string::npos ? isHttps = false : isHttps = true;
		INTERNET_PORT internetPort;
		if (port != -1)
		{
			internetPort = static_cast<INTERNET_PORT>(port);
		}
		else
		{
			isHttps ? internetPort = INTERNET_DEFAULT_HTTPS_PORT : internetPort = INTERNET_DEFAULT_HTTP_PORT;
		}
		HINTERNET internetConnect = InternetConnectA(internetOpen, GetHost(url).c_str(), internetPort, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if (!internetConnect)
		{
			std::string err = "InternetConnectA failed. Last error: " + std::to_string(GetLastError());
			InternetCloseHandle(internetOpen);
			return err;
		}

		DWORD RequestFlg = INTERNET_FLAG_RELOAD | INTERNET_COOKIE_THIRD_PARTY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE;
		if (cookies.empty())RequestFlg = RequestFlg | INTERNET_FLAG_NO_COOKIES;
		isHttps ? RequestFlg |= INTERNET_FLAG_SECURE : RequestFlg |= INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;

		HINTERNET openRequest = HttpOpenRequestA(internetConnect, method.c_str(), GetURLPage(url).c_str(), ("HTTP/1.1"), NULL, NULL, RequestFlg, NULL);
		if (!openRequest)
		{
			std::string err = "HttpOpenRequestA failed. Last error: " + std::to_string(GetLastError());
			InternetCloseHandle(internetConnect);
			InternetCloseHandle(internetOpen);
			return err;
		}

		// ignores ssl certificate errors
		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);
		if (InternetQueryOptionA(openRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &dwBuffLen))
		{
			dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_REVOCATION | SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_WRONG_USAGE | SECURITY_FLAG_IGNORE_WEAK_SIGNATURE;
			InternetSetOptionA(openRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
		}

		bool sendRequest = false;
		if (method == ("GET"))
		{
			sendRequest = HttpSendRequestA(openRequest, header.c_str(), header.length(), nullptr, NULL);
		}
		else
		{
			sendRequest = HttpSendRequestA(openRequest, header.c_str(), header.length(), (LPVOID)(requestData.c_str()), requestData.size());
		}
		if (!sendRequest)
		{
			if (GetLastError() == 12158)
				FixCachingProblem();

			std::string err = "HttpSendRequestA failed. Last error: " + std::to_string(GetLastError());
			InternetCloseHandle(internetConnect);
			InternetCloseHandle(internetOpen);
			InternetCloseHandle(openRequest);
			return err;
		}

		std::string ResultData;
		char* pResultData = nullptr;
		try { pResultData = new char[1025]; }
		catch (...)
		{
			std::string err = "Failed to allocate char array1. Last error: " + std::to_string(GetLastError());
			InternetCloseHandle(internetConnect);
			InternetCloseHandle(internetOpen);
			InternetCloseHandle(openRequest);
			return err;
		}
		UINT  ResultLen = 0;
		do
		{
			ZeroMemory(pResultData, 1025);
			InternetReadFile(openRequest, pResultData, 1024, reinterpret_cast<LPDWORD>(&ResultLen));
			ResultData.append(reinterpret_cast<char*>(pResultData), ResultLen);
		} while (ResultLen);
		char* pTmpQuery = nullptr;
		try { pTmpQuery = new char[4096]; }
		catch (...)
		{
			std::string err = "Failed to allocate char array2. Last error: " + std::to_string(GetLastError());
			InternetCloseHandle(internetConnect);
			InternetCloseHandle(internetOpen);
			InternetCloseHandle(openRequest);
			return err;
		}
		ZeroMemory(pTmpQuery, 4096 * sizeof(char));
		DWORD CookiesLength = 4095;
		HttpQueryInfoA(openRequest, HTTP_QUERY_SET_COOKIE, pTmpQuery, &CookiesLength, NULL);
		returnCookies.append(pTmpQuery);
		InternetCloseHandle(internetConnect);
		InternetCloseHandle(internetOpen);
		InternetCloseHandle(openRequest);
		delete[]pResultData;
		delete[]pTmpQuery;
		return ResultData;
	}

	static std::string GetLocalCookies(std::string host)
	{
		char* TmpCookies = nullptr;
		DWORD CookiesLen = 4097;
		try { TmpCookies = new char[CookiesLen]; }
		catch (...) { return ""; }
		ZeroMemory(TmpCookies, 4097);
		if (!InternetGetCookieA(host.c_str(), nullptr, TmpCookies, &CookiesLen))return "";
		std::string Cookies = TmpCookies;
		delete[]TmpCookies;
		return Cookies;
	}

private:

	// Fixes Wininet error 12158, #80 in GitHub issues
	static void FixCachingProblem()
	{
		typedef LSTATUS(WINAPI* tRegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
			REGSAM samDesired, PHKEY phkResult);
		static tRegOpenKeyExA RegOpenKeyExA = (tRegOpenKeyExA)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegOpenKeyExA");

		typedef LSTATUS(WINAPI* tRegQueryValueExA)(HKEY hKey, LPCSTR lpValueName,
			LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbDatan);
		static tRegQueryValueExA RegQueryValueExA = (tRegQueryValueExA)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegQueryValueExA");

		typedef LSTATUS(WINAPI* tRegSetValueExA)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
			DWORD dwType, const BYTE* lpData, DWORD cbData);
		static tRegSetValueExA RegSetValueExA = (tRegSetValueExA)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegSetValueExA");

		typedef LSTATUS(WINAPI* tRegCloseKey)(HKEY hKe);
		static tRegCloseKey RegCloseKey = (tRegCloseKey)GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCloseKey");

		bool bChanged = false;
		HKEY hkResult;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ | KEY_WRITE, &hkResult) == ERROR_SUCCESS)
		{
			DWORD value;
			DWORD newValue = 0;
			DWORD dwSize = sizeof(value);
			LSTATUS regQuery = RegQueryValueExA(hkResult, "DisableCachingOfSSLPages", 0, NULL, (LPBYTE)&value, &dwSize);
			if (regQuery == ERROR_SUCCESS)
			{
				if (value == 0x1)
				{
					RegSetValueExA(hkResult, "DisableCachingOfSSLPages", 0, REG_DWORD, (LPBYTE)&newValue, dwSize);
					bChanged = true;
				}
			}
			else if (regQuery == ERROR_FILE_NOT_FOUND) // if key doesnt exist, create it
			{
				RegSetValueExA(hkResult, "DisableCachingOfSSLPages", 0, REG_DWORD, (LPBYTE)&newValue, dwSize);
				bChanged = true;
			}
			RegCloseKey(hkResult);
		}

		if (bChanged == true)
		{
			MessageBoxA(NULL, "Restart the program\n\nIf this pop-up window keeps showing up: Open \"Internet Options\", "
				"Go to \"Advanced\" tab and disable \"Do not save encrypted pages to disk\". Press \"Apply\" and \"OK\"",
				"Updated faulty options", MB_OK);
			exit(EXIT_SUCCESS);
		}
	}
};
