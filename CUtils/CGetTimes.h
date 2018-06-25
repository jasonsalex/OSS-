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
			static double GetTime();												// ��sΪ��λ
			static DWORD GetTickCount();											// ��msΪ��λ

			static std::string CurrentDateTime();
			static std::string CurrentDate();
			static std::string TimeDiffToString(double dtime);
			static std::string getTimeStamp();									//��ȡʱ���

			static std::string ConvertGMTTime(const time_t time);
		};
	}
}
