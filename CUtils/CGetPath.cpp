#include "CGetPath.h"

namespace hfjy
{
	namespace CGetPath
	{
		std::wstring CGetPathClass::GetApplicationDirW()
		{
			wchar_t path[_MAX_PATH];
			::GetModuleFileNameW(nullptr, path, _MAX_PATH);
			::wcsrchr(path, L'\\')[1] = L'\0';
			return path;
		}

		std::string CGetPathClass::GetApplicationDir()
		{
			std::wstring str = GetApplicationDirW();
			return hfjy::CFormatChange::CFormatChangeClass::WideToUtf8(str.c_str());
		}
	}
}