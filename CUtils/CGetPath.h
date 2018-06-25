#pragma once

#include "CFormatChange.h"

namespace hfjy
{
	namespace CGetPath
	{
		class CGetPathClass
		{
		public:
			static std::wstring GetApplicationDirW();
			static std::string GetApplicationDir();
		};
	}
}

