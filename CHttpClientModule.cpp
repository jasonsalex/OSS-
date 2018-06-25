#include "CHttpClientModule.h"
#include <CFastLogger.h>
#include <CFormatChange.h>
#include <CGetTimes.h>
#include <CJson.hpp>
#include <base64.h>
#include <aos_log.h>
#include <aos_util.h>
#include <aos_string.h>
#include <aos_status.h>
#include <oss_auth.h>
#include <oss_util.h>
#include <oss_api.h>

CHttpClientModule::CHttpClientModule()
{
}


CHttpClientModule::~CHttpClientModule()
{
}

size_t CHttpClientModule::DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam)
{
	return fwrite(pBuffer, nSize, nMemByte, (FILE*)pParam);
}

size_t CHttpClientModule::UploadCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	return fread(ptr, size, nmemb, (FILE *)stream);
}


int CHttpClientModule::DownloadProgressCallback(void *pParam, double dltotal, double dlnow, double ultotal, double ulnow)
{	
	auto p = (CHttpClientModule*)pParam;

	if (p->progressEventProc != nullptr)
	{
		return p->progressEventProc(p->progressEventProcParam, dltotal, dlnow);
	}
	
	return 0;
}


//int CHttpClientModule::UploadProgressCallback(void *pParam, double dltotal, double dlnow, double ultotal, double ulnow)
//{
//	auto p = (CHttpClientModule*)pParam;
//	if (p->progressEventProc != nullptr)
//	{
//		return p->progressEventProc(p->progressEventProcParam, dltotal, dlnow);
//	}
//
//	return 0;
//}



int CHttpClientModule::GetDownloadFileSize(const std::string url)
{
	CURL* curl = curl_easy_init();

	if (curl == nullptr)
	{
		LOG(ERROR) << "init curl is error, url:" << url.c_str();
		return HTTP_INVALID;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

	CURLcode retCode;
	retCode = curl_easy_perform(curl);

	double size = HTTP_INVALID;

	if (retCode != CURLE_OK)
	{
		LOG(ERROR) << "curl error:" << curl_easy_strerror(retCode) << ", url:" << url.c_str();
		size =  HTTP_INVALID;
	}

	retCode = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
	
	if (retCode != CURLE_OK)
	{
		LOG(ERROR) << "curl error:" << curl_easy_strerror(retCode) << ", url:" << url.c_str();
		size = HTTP_INVALID;
	}

	curl_easy_cleanup(curl);

	return size;
}


bool CHttpClientModule::DownLoadFile(const std::string url, const std::string outFile, const bool breakPoint, const int timeOut)
{

	FILE * file = nullptr;
		
	if(breakPoint)
		file = fopen(outFile.c_str(), "ab+");
	else
		file = fopen(outFile.c_str(), "wb+");

	if (file == nullptr)
	{
		LOG(ERROR) << "download event, open file is error, path:" << url.c_str();
		return false;
	}

	auto size = GetDownloadFileSize(url);

	if(size > 0 && size == GetFileSize(file))
	{
		fclose(file);
		return true;
	}


	CURL* curl = curl_easy_init();

	if(curl == nullptr)
	{ 
		LOG(ERROR) << "init curl is error, url:" << url.c_str();
		return false;
	}


	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DownloadProgressCallback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);

	if (breakPoint)
	{
		curl_off_t size = GetFileSize(file); //类型一定要是curl_off_t,否者会出错
		curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, size); //设置断点下载	
	}

	bool ret = true;

	CURLcode retCode;

	retCode = curl_easy_perform(curl);

	if (retCode != CURLE_OK)
	{
		LOG(ERROR)<<"curl error:"<< curl_easy_strerror(retCode)<<", url:"<< url.c_str();
		ret = false;
	}

	fclose(file);
	curl_easy_cleanup(curl);
	
	return ret;
}

bool CHttpClientModule::DownLoadData(const std::string url, char **recvBuf, int *size, const bool breakPoint, const int timeOut)
{
	FILE * file = tmpfile();

	if (file == nullptr)
	{
		LOG(ERROR) << "download event, open file is error, path:" << url.c_str();
		return false;
	}

	CURL* curl = curl_easy_init();

	if (curl == nullptr)
	{
		LOG(ERROR) << "init curl is error, url:" << url.c_str();
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DownloadProgressCallback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);

	CURLcode retcCode = curl_easy_perform(curl);

	bool ret = true;

	if (retcCode != CURLE_OK)
	{
		LOG(ERROR) << "curl error:" << curl_easy_strerror(retcCode) << ", url:" << url.c_str();
		ret = false;
	}
	
	*size = GetFileSize(file);

	*recvBuf = new char[*size];

	fseek(file, 0, SEEK_SET);

	if (fread(*recvBuf, 1, *size, file) != *size)
	{
		LOG(ERROR) << "create recv buffer is error";
		ret = false;
	}

	fclose(file);
	curl_easy_cleanup(curl);

	return ret;
}


//bool CHttpClientModule::UpLoadFile(const std::wstring url, curl_httppost* formpost, const bool breakPoint, const int timeOut)
//{
//	CURL* curl = curl_easy_init();
//
//	if (curl == nullptr)
//	{
//		LOG(ERROR) << "init curl is error, url:" << url.c_str();
//		return false;
//	}
//
//	curl_slist *slist = NULL;
//	slist = curl_slist_append(slist, "Content-Type: multipart/form-data");
//	slist = curl_slist_append(slist, "Expect:");
//
//	//curl_easy_setopt(curl, CURLOPT_HEADER, slist);
//	curl_easy_setopt(curl, CURLOPT_URL, hfjy::CFormatChange::CFormatChangeClass::WideToUtf8(url.c_str()).c_str());
//
//	//curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
//	//curl_easy_setopt(curl, CURLOPT_PUT, 1L);
//	//curl_easy_setopt(curl, CURLOPT_POST, 1);
//	curl_easy_setopt(curl, CURLOPT_POST, formpost);
//
//	//curl_easy_setopt(curl, CURLOPT_READFUNCTION, UploadCallback);
//	//curl_easy_setopt(curl, CURLOPT_READDATA, file);
//
//	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
//	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
//	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
//	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, UploadProgressCallback);
//	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
//	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
//	//curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, GetFileSize(file));
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//
//	CURLcode retcCode = curl_easy_perform(curl);
//	bool ret = true;
//
//	if (retcCode != CURLE_OK)
//	{
//		LOG(ERROR) << "curl error:" << curl_easy_strerror(retcCode) << ", url:" << url.c_str();
//		ret = false;
//	}
//
//	curl_easy_cleanup(curl);
//	return ret;
//}






