#include "CFormatChange.h"

namespace hfjy
{
	namespace CFormatChange
	{
		inline char* wide_to_mtbytes(int c, const wchar_t* pWideText)
		{
			int size = WideCharToMultiByte(c, 0, (LPCWSTR)pWideText, -1, NULL, 0, NULL, NULL);
			if (size == 0) {
				return NULL;
			}
			char* pText = new char[size + 1];
			if (WideCharToMultiByte(c, 0, (LPCWSTR)pWideText, -1, pText, size, NULL, NULL) == 0) {
				delete[]pText;
				return NULL;
			}
			pText[size] = '\0';
			return pText;
		}

		inline wchar_t* mtbytes_to_wide(int c, const char* pText)
		{
			wchar_t* pWideText = NULL;
			int size = MultiByteToWideChar(c, 0, pText, -1, NULL, 0);
			if (size == 0) {
				return pWideText;
			}
			else {
				pWideText = new wchar_t[size + 1];
				if (MultiByteToWideChar(c, 0, pText, -1, (LPWSTR)pWideText, size) == 0) {
					delete[]pWideText;
					pWideText = NULL;
					return pWideText;
				}
				else {
					pWideText[size] = 0;
					return pWideText;
				}
			}
		}


		inline std::string local_to_utf8(const char* pText)
		{
			wchar_t* pWideText = mtbytes_to_wide(CP_ACP, pText);
			if (pWideText == NULL) {
				return "";
			}
			char* pUTF8 = wide_to_mtbytes(CP_UTF8, pWideText);
			if (pUTF8 == NULL) {
				delete[]pWideText;
				return "";
			}
			std::string r = pUTF8;
			delete[]pUTF8;
			delete[]pWideText;
			return r;
		}

		inline std::wstring utf8_to_wide(const char* pText)
		{
			wchar_t* pWide = mtbytes_to_wide(CP_UTF8, pText);
			std::wstring s = (wchar_t*)pWide;
			delete[]pWide;
			return s;
		}

		inline std::string wide_to_utf8(const wchar_t* pText)
		{
			char* pUTF8 = wide_to_mtbytes(CP_UTF8, pText);
			if (pUTF8 == NULL) {
				return "";
			}
			else {
				std::string r = pUTF8;
				delete[]pUTF8;
				return r;
			}
		}

		inline std::string utf8_to_local(const char* pText)
		{
			std::wstring ws = utf8_to_wide(pText);
			char* pANSI = wide_to_mtbytes(CP_ACP, ws.c_str());
			if (pANSI == NULL) {
				return "";
			}
			std::string r = pANSI;
			delete[]pANSI;
			return r;
		}

		std::string CFormatChangeClass::Utf8ToLocal(const char * pText)
		{
			return utf8_to_local(pText);
		}

		std::string CFormatChangeClass::Utf8ToLocal(const std::string& text)
		{
			return Utf8ToLocal(text.c_str());
		}

		std::string CFormatChangeClass::LocalToUtf8(const char * pText)
		{
			return local_to_utf8(pText);
		}

		std::string CFormatChangeClass::LocalToUtf8(const std::string & text)
		{
			return LocalToUtf8(text.c_str());
		}

		std::wstring CFormatChangeClass::Utf8ToWide(const char * pText)
		{
			return utf8_to_wide(pText);
		}

		std::string CFormatChangeClass::WideToUtf8(const wchar_t * pText)
		{
			return wide_to_utf8(pText);
		}
	}
}

