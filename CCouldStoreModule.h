#pragma once
#include <string>
#include <aos_log.h>
#include <aos_util.h>
#include <aos_string.h>
#include <aos_status.h>
#include <oss_auth.h>
#include <oss_util.h>
#include <oss_api.h>
#include <CFastLogger.h>
#include <resumable_io.h>
#include <rs.h>


typedef void (*PERCENTAGE)(int64_t consumed_bytes, int64_t total_bytes);

struct CsmStatus {
	bool ok;
	std::string downloadUrl;
};


class CCouldStoreModule
{
public:

	static void DownloadCallback(int64_t consumed_bytes, int64_t total_bytes);
	static void UploadCallback(int64_t consumed_bytes, int64_t total_bytes);

	void SetProgressEvent(PERCENTAGE download, PERCENTAGE upload) { downloadCallback = download; uploadCallback = upload; }
	
	void SetResumableAttr(const std::string resumebleFilePath, const int64_t partSize=256*1024, const int threadNum=3)
	{
		this->partSize = partSize;
		this->threadNum = threadNum;
		this->resumebleFilePath = resumebleFilePath;
	}

	virtual bool Init(const std::string &url, const std::string &keyId, const std::string &keySecret) = 0;
	virtual CsmStatus UploadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true) = 0;
	virtual CsmStatus UploadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const char *buf, const int bufSize) = 0;
	virtual bool DownloadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true) = 0;
	virtual bool DownloadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, char **buf, long *bufSize) = 0;
	virtual bool DeleteObject(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName) = 0;

	virtual ~CCouldStoreModule()=default;

protected:
	PERCENTAGE downloadCallback = nullptr, uploadCallback = nullptr;
	
	std::string resumebleFilePath;

	int64_t partSize = 256 * 1024;
	int threadNum = 1;

	std::string url;
	std::string accessKeyId;
	std::string accessKeySecret;
};

class CAliCouldStoreModule : public CCouldStoreModule //∞¢¿Ôsdk
{
public:
	CAliCouldStoreModule();
	bool Init(const std::string &url, const std::string &keyId, const std::string &keySecret)override;
	CsmStatus UploadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true)override;
	CsmStatus UploadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const char *buf, const int bufSize)override;
	bool DownloadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true)override;
	bool DownloadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, char **buf, long *bufSize)override;
	bool DeleteObject(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName)override;

	virtual ~CAliCouldStoreModule();

private:
	void initRequestOptions(oss_request_options_t *options, int is_cname);

};

class CQiniuCouldStoreModule :public CCouldStoreModule // 7≈£sdk
{
public:
	bool Init(const std::string &url, const std::string &keyId, const std::string &keySecret)override;
	CsmStatus UploadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true)override;
	CsmStatus UploadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const char *buf, const int bufSize)override;
	bool DownloadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true)override;
	bool DownloadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, char **buf, long *bufSize)override;
	bool DeleteObject(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName)override;

	virtual ~CQiniuCouldStoreModule();

private:
	bool SaveProgressInfo(Qiniu_Rio_BlkputRet * ret, const int size);
	int ReadProgressInfo(Qiniu_Rio_BlkputRet * ret);

private:

	std::string fileName;
	static int UploadNotify(void* recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet* ret);
	static int DownloadNotify(void* param, const int &total, const int &recv);
	int fileSize = 0;
	int recvCount = 0;
	int lastInx = -1;
	Qiniu_Rio_BlkputRet *blpputRet;
};
