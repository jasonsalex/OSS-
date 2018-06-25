#pragma once
#include <string>
#include <Windows.h>
#include <curl.h>

#define HTTP_INVALID -1

typedef int(*PROGRESS_EVENT_PROC)(void* param,const int &total, const int &recv);

static size_t GetFileSize(FILE * fp)
{
	size_t cur = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size_t ret = ftell(fp);
	fseek(fp, cur, SEEK_CUR);
	return ret;
}

static size_t GetFileSize(const std::string &filePath)
{
	size_t total = 0;
	FILE *file = fopen(filePath.c_str(), "rb+");
	if (file != NULL)
	{
		total = GetFileSize(file);
		fclose(file);
	}
	return total;
}

static void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

static std::string GetPathOrURLShortName(std::string strFullName)
{
	if (strFullName.empty())
	{
		return "";
	}

	string_replace(strFullName, "/", "\\");

	std::string::size_type iPos = strFullName.find_last_of('\\') + 1;

	return strFullName.substr(iPos, strFullName.length() - iPos);
}

class CHttpClientModule
{
public:
	CHttpClientModule();
	~CHttpClientModule();

	static size_t DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
	static size_t UploadCallback(void * ptr, size_t size, size_t nmemb, void * stream);
	static int DownloadProgressCallback(void * clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	//static int UploadProgressCallback(void * pParam, double dltotal, double dlnow, double ultotal, double ulnow);
	
	int GetDownloadFileSize(const std::string url);

	/*!
	@brief download http file to disk
	@param[in] url  http url
	@param[in] outFile savedata do disk with from http data
	@timeOut[out] request http timeout with millisecond
	@return request a http status
	*/
	bool DownLoadFile(const std::string url, const std::string outFile, const bool breakPoint = false, const int timeOut = INT16_MAX);
	bool DownLoadData(const std::string url, char **recvBuf, int *size, const bool breakPoint = false, const int timeOut = INT16_MAX);

	//bool UpLoadFile(const std::wstring url, curl_httppost* formpost, const bool breakPoint = false, const int timeOut = 10);

	void SetProgressEvent(PROGRESS_EVENT_PROC proc, void *param = nullptr)
	{
		progressEventProc = proc;
		progressEventProcParam = param;
	}

private:
	PROGRESS_EVENT_PROC progressEventProc = nullptr;
	void *progressEventProcParam = nullptr;
};

