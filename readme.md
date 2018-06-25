# 说明
* 目前支持七牛和阿里的接入，统一oss云厂商的sdk的规范，使用更加简单。支持断点上传和下载。

# api函数说明
###### 设置进度事件

* 返回值:void

* downloan:下载回调进度

* upload:上载回调进度

void SetProgressEvent(PERCENTAGE download, PERCENTAGE upload)

###### 设置断点续传属性

* 返回值:void

* resumebleFilePath:断点续传文件路径

* partSize:分片大小，默认256kb

* threadNum:上传线程，默认3

void SetResumableAttr(const std::string resumebleFilePath, const int64_t partSize=256*1024, const int threadNum = 3);

###### 初始化oss属性

* 返回值:bool

* url:oss url

* keyId:秘钥

* keySecret:秘钥

virtual bool Init(const std::string &url, const std::string &keyId, const std::string &keySecret) = 0;

###### 上传文件

* bucketName: 数据池名字

* virtualPath:虚拟目录

* objectName:对象名字

* filePath:上传的文件路径

* resumable:是否使用断点续传，默认使用

virtual CsmStatus UploadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true) = 0;

###### 上传缓冲区

* bucketName: 数据池名字

* virtualPath:虚拟目录

* objectName:对象名字

* buf:上传的缓冲区

*bufSize:缓冲区的大小

virtual CsmStatus UploadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const char *buf, const int bufSize) = 0;

###### 下载文件

* bucketName: 数据池名字

* virtualPath:虚拟目录

* objectName:对象名字

* filePath:下载的文件路径

* resumable:是否使用断点续传，默认使用

virtual bool DownloadDataWithFile(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, const std::string &filePath, const bool resumable = true) = 0;

###### 下载缓冲区

* bucketName: 数据池名字

* virtualPath:虚拟目录

* objectName:对象名字

* buf:下载的缓冲区

*bufSize:缓冲区的大小

virtual bool DownloadDataWithBuffer(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName, char **buf, long *bufSize) = 0;

###### 删除oss里面的文件

* bucketName：数据池

* objectName:文件名

virtual bool DeleteObject(const std::string &bucketName, const std::string &virtualPath, const std::string &objectName) = 0;

# demo示例
```
void download(int64_t consumed_bytes, int64_t total_bytes)
{
	printf("download, %lld/%lld\n", consumed_bytes, total_bytes);
}

void upload(int64_t consumed_bytes, int64_t total_bytes)
{
	printf("upload, %lld/%lld\n", consumed_bytes, total_bytes);
}

void CCouldStoreModuleTestForAli()
{
	CCouldStoreModule *ali = new CAliCouldStoreModule;

	ali->SetProgressEvent(download, upload);
	auto time = hfjy::CGetTimes::CGetTimesClass::CurrentDate();

	ali->Init(OSS_ENDPOINT, ACCESS_KEY_ID, ACCESS_KEY_SECRET);
	ali->UploadDataWithFile(BUCKET_NAME, time, OBJECT_NAME, "F:\\39480.mp3");
	ali->DownloadDataWithFile(BUCKET_NAME, time, OBJECT_NAME, "F:\\39480-test.mp3");
	
	char testStr[] = { "this test ali clouds" };


	ali->UploadDataWithBuffer(BUCKET_NAME, time, OBJECT_NAME, testStr, strlen(testStr));

	char *recvStr;
	long size;
	ali->DownloadDataWithBuffer(BUCKET_NAME, time, OBJECT_NAME, &recvStr, &size);
	
	printf("download buf:%s,size:%d \n", recvStr, size);

	free(recvStr);

	delete ali;
}

void CCouldStoreModuleTestForAliWithResumeble()
{
	CCouldStoreModule *ali = new CAliCouldStoreModule;
	ali->SetProgressEvent(download, upload);
	ali->SetResumableAttr("F:/1111.cup", 1 * 1024 * 1024);

	auto time = hfjy::CGetTimes::CGetTimesClass::CurrentDate();

	ali->Init(OSS_ENDPOINT, ACCESS_KEY_ID, ACCESS_KEY_SECRET);
	ali->UploadDataWithFile(BUCKET_NAME, time, OBJECT_NAME, "F:\\pisrl.model");
	ali->DownloadDataWithFile(BUCKET_NAME, time, OBJECT_NAME, "F:\\godoc_test.model");

	delete ali;
}


void CCouldStoreModuleTestForQiniu()
{
	CCouldStoreModule *qiniu = new CQiniuCouldStoreModule;
	qiniu->SetProgressEvent(download, upload);
	qiniu->SetResumableAttr("F:/", 1*1024*1024);
	auto time = hfjy::CGetTimes::CGetTimesClass::CurrentDate();

	qiniu->Init("ogbpegup0.bkt.clouddn.com", "", "");
	//qiniu->UploadDataWithFile("hyphen-test", time, "hfjy-test-1", "F:\\godoc.exe");
	qiniu->DownloadDataWithFile("hyphen-test", time, "hfjy-test-1", "F:\\39480.mp3.qiniu");


	char testStr[] = { "this test qiniu clouds" };


	qiniu->UploadDataWithBuffer("hyphen-test", time, "test_buf111", testStr, strlen(testStr));

	char *recvStr;
	long size;
	qiniu->DownloadDataWithBuffer("hyphen-test", time, "test_buf111", &recvStr, &size);

	printf("download buf:%s,size:%d \n", recvStr, size);

	free(recvStr);
	
	qiniu->DeleteObject("hyphen-test", time, "test_buf111");

	delete qiniu;

}
```