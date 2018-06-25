#pragma once

#include <windows.h>
#include <time.h>
#include <chrono>
#include <list>
#include <sstream>

namespace hfjy
{
	namespace CGetTimes
	{
		class CGetTimesClass
		{
		public:
			static void InitializeClock();
			static double GetTime();												// 以s为单位
			static DWORD GetTickCount();											// 以ms为单位

			static std::string CurrentDateTime();
			static std::string CurrentDate();
			static std::string TimeDiffToString(double dtime);
			static std::string getTimeStamp();									//获取时间戳

			static std::string ConvertGMTTime(const time_t time);
		};
	}
}
