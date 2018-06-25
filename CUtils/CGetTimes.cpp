#include "CGetTimes.h"

namespace hfjy
{
	namespace CGetTimes
	{
#define TIMESTRLEN 128
		static int usePerformanceCounter_ = 0;
		static double secondsPerTick_;

		double CGetTimesClass::GetTime()
		{
			if (usePerformanceCounter_) {
				LARGE_INTEGER time;
				QueryPerformanceCounter(&time);
				return time.QuadPart * secondsPerTick_;
			}
			return GetTickCount() * .001;
		}

		DWORD CGetTimesClass::GetTickCount()
		{
			if (usePerformanceCounter_) {
				LARGE_INTEGER time;
				QueryPerformanceCounter(&time);
				return (DWORD)(time.QuadPart * secondsPerTick_ / .001);
			}
			return ::GetTickCount();
		}

		void CGetTimesClass::InitializeClock()
		{
			LARGE_INTEGER ticksPerSecond;
			if (QueryPerformanceFrequency(&ticksPerSecond) != 0) {
				usePerformanceCounter_ = 1;
				secondsPerTick_ = 1.0 / (double)ticksPerSecond.QuadPart;
			}
			else {
				usePerformanceCounter_ = 0;
			}
		}

		std::string CGetTimesClass::CurrentDateTime()
		{
			char sztTime[TIMESTRLEN] = { 0 };
			SYSTEMTIME st;
			GetLocalTime(&st);
			sprintf_s(sztTime, TIMESTRLEN, "%d-%02d-%02d %02d:%02d:%02d.%03d",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			return sztTime;
		}

		std::string CGetTimesClass::CurrentDate()
		{
			char sztTime[TIMESTRLEN] = { 0 };
			SYSTEMTIME st;
			GetLocalTime(&st);
			sprintf_s(sztTime, TIMESTRLEN, "%d%02d%02d",
				st.wYear, st.wMonth, st.wDay);
			return sztTime;
		}

		std::string CGetTimesClass::TimeDiffToString(double dtime)
		{
			int64_t  ltime;
			int   imsc;
			float fmsc;
			struct tm *tiempo;
			ltime = (int64_t)dtime;
			tiempo = gmtime(&ltime);
			fmsc = (float)((dtime - (double)ltime) * 1000.0);
			imsc = (int)fmsc;
			if (fmsc - (float)imsc >= 0.5) imsc++;

			char sztTime[TIMESTRLEN] = { 0 };
			sprintf_s(sztTime, TIMESTRLEN, "%02d:%02d:%02d.%03d", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, imsc);
			return sztTime;
		}

		std::string CGetTimesClass::getTimeStamp()
		{
			std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
			auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
			auto timestamp = tmp.count();
			std::string str;
			std::ostringstream os;
			os << timestamp;
			str = os.str();
			return str;
		}


		std::string CGetTimesClass::ConvertGMTTime(const time_t t)
		{
			char buf[32];
			strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
			return std::string(buf);
		}
	}
}