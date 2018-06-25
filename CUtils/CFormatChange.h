#pragma once

#include <windows.h>
#include <sstream>

namespace hfjy
{
	namespace CFormatChange
	{
		class CFormatChangeClass
		{
		public:
			static std::string Utf8ToLocal(const char* pText);
			static std::string Utf8ToLocal(const std::string& text);
			static std::string LocalToUtf8(const char* pText);
			static std::string LocalToUtf8(const std::string& text);
			static std::wstring Utf8ToWide(const char* pText);
			static std::string WideToUtf8(const wchar_t* pText);						
		};
	}
}


