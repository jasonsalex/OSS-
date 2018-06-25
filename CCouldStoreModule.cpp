#include "CCouldStoreModule.h"
#include <CHttpClientModule.h>
#include <CFormatChange.h>
#include <CJson.hpp>


CAliCouldStoreModule::CAliCouldStoreModule()
{

}

bool CAliCouldStoreModule::Init(const std::string & url, const std::string & keyId, const std::string & keySecret)
{
	// initialize http io system, call it olny once
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK)
	{
		return false;
	}
	
	this->url = url;
	accessKeyId = keyId;
	accessKeySecret = keySecret;

	return true;
}

CAliCouldStoreModule::~CAliCouldStoreModule()
{
	aos_http_io_deinitialize();
}

void CAliCouldStoreModule::initRequestOptions(oss_request_options_t * options, int is_cname)
{
	options->config = oss_config_create(options->pool);

	aos_str_set(&options->config->endpoint, url.c_str());
	aos_str_set(&options->config->access_key_id, accessKeyId.c_str());
	aos_str_set(&options->config->access_key_secret, accessKeySecret.c_str());

	options->config->is_cname = is_cname;
	options->ctl = aos_http_controller_create(options->pool, 0);
}


CsmStatus CAliCouldStoreModule::UploadDataWithFile(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, const std::string & filePath, const bool resumable)
{
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	int is_cname = 0;
	aos_table_t *resp_headers = NULL;
	aos_table_t *headers = NULL;
	oss_request_options_t *options = NULL;
	aos_status_t *s = NULL;
	aos_string_t file;
	aos_list_t resp_body;


	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	initRequestOptions(options, is_cname);

	headers = aos_table_make(p, 0);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	aos_str_set(&bucket, bucketName.c_str());
	aos_str_set(&object, objectPath.c_str());
	aos_str_set(&file, filePath.c_str());
	aos_list_init(&resp_body);

	if (resumable)
	{
		oss_resumable_clt_params_t *clt_params;

		auto checkPoint = resumebleFilePath + objectName + "_upload.cup";

		clt_params = oss_create_resumable_clt_params_content(p, partSize, threadNum, AOS_TRUE, resumebleFilePath.c_str());
		
		s = oss_resumable_upload_file(options, &bucket, &object, &file, headers, 0,
			clt_params, uploadCallback, &resp_headers, &resp_body);
	}
	else
	{
		s = oss_do_put_object_from_file(options, &bucket, &object, &file,
			headers, 0, uploadCallback, &resp_headers, &resp_body);
	}

	CsmStatus ret;
	
	ret.ok = false;

	if (aos_status_is_ok(s)) 
	{
		ret.ok = true;
		DLOG(INFO)<<"put object from file succeeded, object name:"<<objectName;
	}
	else
	{
		ret.ok = false;
		LOG(ERROR) << "put object from file failed, code:" << s->code;
	}

	aos_pool_destroy(p);


	ret.downloadUrl = bucketName+ "." + this->url + "/" + objectPath;

	return ret;
}

CsmStatus CAliCouldStoreModule::UploadDataWithBuffer(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, const char * buf, const int bufSize)
{
	aos_string_t bucket;
	aos_string_t object;
	aos_list_t buffer;
	aos_buf_t *content = NULL;
	aos_table_t *resp_headers = NULL;
	aos_status_t *s = NULL;
	aos_pool_t *p = NULL;
	oss_request_options_t *options = NULL;
	aos_list_t resp_body;

	int is_cname = 0;


	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	initRequestOptions(options, is_cname);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	aos_str_set(&bucket, bucketName.c_str());
	aos_str_set(&object, objectPath.c_str());

	aos_list_init(&buffer);
	content = aos_buf_pack(options->pool, buf, bufSize);
	aos_list_add_tail(&content->node, &buffer);

	s = oss_do_put_object_from_buffer(options, &bucket, &object,
			&buffer, 0, 0, uploadCallback, &resp_headers, &resp_body);

	CsmStatus ret;

	ret.ok = false;

	if (aos_status_is_ok(s)) 
	{
		ret.ok = true;
		DLOG(INFO) << "put object from file succeeded, object name:" << objectName;
	}
	else 
	{
		ret.ok = false;
		LOG(ERROR) << "put object from file failed, code:" << s->code;
	}

	aos_pool_destroy(p);

	ret.downloadUrl = bucketName + "." + this->url + "/" + objectPath;

	return ret;
}

bool CAliCouldStoreModule::DownloadDataWithFile(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, const std::string & filePath, const bool resumable)
{
	aos_string_t object;
	int is_cname = 0;
	oss_request_options_t *options = NULL;
	aos_table_t *params = NULL;
	aos_table_t *resp_headers = NULL;
	aos_table_t *headers = NULL;
	aos_status_t *s = NULL;
	aos_string_t file;
	aos_pool_t *p = NULL;
	aos_string_t bucket;

	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	initRequestOptions(options, is_cname);

	headers = aos_table_make(p, 0);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	aos_str_set(&bucket, bucketName.c_str());
	aos_str_set(&object, objectPath.c_str());
	aos_str_set(&file, filePath.c_str());

	if (resumable)
	{
		oss_resumable_clt_params_t *clt_params;

		auto checkPoint = resumebleFilePath + objectName + "_download.cup";

		clt_params = oss_create_resumable_clt_params_content(p, partSize, threadNum, AOS_TRUE, checkPoint.c_str());

		s = oss_resumable_download_file(options, &bucket, &object, &file, headers, 0,
			clt_params, downloadCallback, &resp_headers);
	}
	else
	{
		s = oss_do_get_object_to_file(options, &bucket, &object, headers, 0,
			&file, downloadCallback, 0);
	}


	bool ret = false;

	if (aos_status_is_ok(s)) {
		ret = true;
		DLOG(INFO) << "get object from file succeeded, object name:" << objectName;

	}
	else {
		LOG(ERROR) << "get object from file failed, code:" << s->code;
		ret = false;
	}

	return ret;
}

bool CAliCouldStoreModule::DownloadDataWithBuffer(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, char ** buf, long * bufSize)
{
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	int is_cname = 0;
	oss_request_options_t *options = NULL;
	aos_table_t *params = NULL;
	aos_table_t *resp_headers = NULL;
	aos_status_t *s = NULL;
	aos_list_t buffer;
	aos_buf_t *content = NULL;
	int64_t len = 0;
	int64_t size = 0;
	int64_t pos = 0;

	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	initRequestOptions(options, is_cname);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	aos_str_set(&bucket, bucketName.c_str());
	aos_str_set(&object, objectPath.c_str());
	aos_list_init(&buffer);

	s = oss_do_get_object_to_buffer(options, &bucket, &object, 0, 0,
		&buffer, downloadCallback, 0);

	bool ret = false;

	if (aos_status_is_ok(s)) {
		ret = true;
		DLOG(INFO) << "get object from file succeeded, object name:" << objectName;
	}
	else {
		ret = false;
		LOG(ERROR) << "get object from file failed, code:" << s->code << "error_code:" << s->error_code << ", error_msg:"
			<< s->error_msg << ", request_id:" << s->req_id;
	}

	//get buffer len
	aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
		len += aos_buf_size(content);
	}

	*bufSize = len;
	*buf = (char*)aos_pcalloc(p, (apr_size_t)(len));

	//copy buffer content to memory
	aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
		size = aos_buf_size(content);
		memcpy(*buf + pos, content->pos, (size_t)size);
		pos += size;
	}

	aos_pool_destroy(p);
	return ret;
}


bool CAliCouldStoreModule::DeleteObject(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName)
{
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	int is_cname = 0;
	oss_request_options_t *options = NULL;
	aos_table_t *resp_headers = NULL;
	aos_status_t *s = NULL;

	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	initRequestOptions(options, is_cname);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	aos_str_set(&bucket, bucketName.c_str());
	aos_str_set(&object, objectPath.c_str());

	s = oss_delete_object(options, &bucket, &object, &resp_headers);

	bool ret = false;

	if (aos_status_is_ok(s)) {
		DLOG(INFO) << "put object from file succeeded, object name:" << objectName;
		ret = true;
	}
	else {
		LOG(ERROR) << "put object from file failed, code:" << s->code << "error_code:" << s->error_code << ", error_msg:"
			<< s->error_msg << ", request_id:" << s->req_id;
		ret = false;
	}

	aos_pool_destroy(p);

	return ret;
}


CQiniuCouldStoreModule::~CQiniuCouldStoreModule()
{
	Qiniu_Global_Cleanup();
}

int CQiniuCouldStoreModule::DownloadNotify(void * param, const int & total, const int & recv)
{
	printf("total:%d, recv:%d\n", total, recv);

	auto self = (CQiniuCouldStoreModule*)param;

	if (self->downloadCallback)
	{
		self->downloadCallback(recv, total);
	}

	return 0;
}

int CQiniuCouldStoreModule::UploadNotify(void * recvr, int blkIdx, int blkSize, Qiniu_Rio_BlkputRet * ret)
{

	auto self = (CQiniuCouldStoreModule*)recvr;
	
	auto btSize = Qiniu_Rio_BlockCount(self->fileSize);
	
	Qiniu_Rio_BlkputRet *blpputRet = self->blpputRet + blkIdx;

	blpputRet->checksum = _strdup(ret->checksum);
	blpputRet->crc32 = ret->crc32;
	blpputRet->ctx = _strdup(ret->ctx);
	blpputRet->host = _strdup(ret->host);
	blpputRet->offset = ret->offset;

	if (self->SaveProgressInfo(self->blpputRet, btSize) == false)
	{
		LOG(ERROR) << "open resumele file is error, file path:"<< self->resumebleFilePath + "/" + self->fileName;
	}
	
	if (self->lastInx != blkIdx)
	{
		self->recvCount += blkSize;
		self->lastInx = blkIdx;
	}

	if (self->recvCount >= self->fileSize)
	{				//上传完成删除文件
		remove((self->resumebleFilePath + "/" + self->fileName).c_str());
	}

	if (self->uploadCallback)
	{
		self->uploadCallback(self->recvCount, self->fileSize);
	}

	return 0;
}

bool CQiniuCouldStoreModule::Init(const std::string & url, const std::string & keyId, const std::string & keySecret)
{
	Qiniu_Global_Init(-1);

	this->url = url;
	accessKeyId = keyId;
	accessKeySecret = keySecret;

	return true;
}

CsmStatus CQiniuCouldStoreModule::UploadDataWithFile(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, const std::string & filePath, const bool resumable)
{

	Qiniu_Rio_PutRet putRet;
	Qiniu_Client client;
	Qiniu_Mac mac;

	Qiniu_RS_PutPolicy putPolicy = Qiniu_RS_PutPolicy();

	mac.accessKey = accessKeyId.c_str();
	mac.secretKey = accessKeySecret.c_str();

	putPolicy.scope = bucketName.c_str();
	putPolicy.expires = INT_MAX;

	char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);
	
	Qiniu_Client_InitMacAuth(&client, 1024, &mac);

	Qiniu_Error error;

	fileSize = GetFileSize(filePath);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	fileName = objectName;
	recvCount = 0;

	if (resumable)
	{
		Qiniu_Rio_PutExtra extra;
		Qiniu_Rio_Settings rioSettings;
		
		Qiniu_Zero(rioSettings);
		Qiniu_Zero(extra);

		rioSettings.workers = threadNum;
		rioSettings.chunkSize = partSize;
		Qiniu_Rio_SetSettings(&rioSettings);

		//extra.upHost = url.c_str();
		extra.notify = UploadNotify;
		extra.notifyRecvr = this; 

		auto btSize = Qiniu_Rio_BlockCount(fileSize);

		std::shared_ptr<Qiniu_Rio_BlkputRet> bputRet(new Qiniu_Rio_BlkputRet[btSize]);

		memset(bputRet.get(), 0, sizeof(Qiniu_Rio_BlkputRet)*btSize);

		this->blpputRet = bputRet.get();

		auto line = ReadProgressInfo(bputRet.get());

		if (line > 0)
		{
			extra.progresses = bputRet.get();
			extra.blockCnt = line;
		}

		error = Qiniu_Rio_PutFile(&client, &putRet, uptoken, objectPath.c_str(), filePath.c_str(), &extra);
		
	}else
	{
		Qiniu_Io_PutExtra extra;
		Qiniu_Zero(extra);

		error = Qiniu_Io_PutFile(&client, &putRet, uptoken, objectPath.c_str(), filePath.c_str(), &extra);
	}
	
	CsmStatus ret;
	
	if (error.code != 200) {
		LOG(ERROR)<< "upload file is error:"<<error.message;
		ret.ok = false;
	}
	else {
		DLOG(INFO) << "upload file:" << objectName;
		ret.ok = true;
	}

	Qiniu_Free(uptoken);
	Qiniu_Client_Cleanup(&client);

	ret.downloadUrl = this->url + "/" + objectPath;

	return ret;
}


CsmStatus CQiniuCouldStoreModule::UploadDataWithBuffer(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, const char * buf, const int bufSize)
{
	
	Qiniu_Rio_PutRet putRet;
	Qiniu_Client client;
	Qiniu_Mac mac;

	Qiniu_RS_PutPolicy putPolicy;

	mac.accessKey = accessKeyId.c_str();
	mac.secretKey = accessKeySecret.c_str();

	Qiniu_Zero(putPolicy);

	putPolicy.scope = bucketName.c_str();
	putPolicy.expires = INT_MAX;

	char *uptoken = Qiniu_RS_PutPolicy_Token(&putPolicy, &mac);

	Qiniu_Client_InitMacAuth(&client, 1024, &mac);

	Qiniu_Error error;

	Qiniu_Io_PutExtra extra;
	Qiniu_Zero(extra);
	
	fileSize = bufSize;
	recvCount = 0;

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;


	error = Qiniu_Io_PutBuffer(&client, &putRet, uptoken, objectPath.c_str(), buf, bufSize,&extra);

	CsmStatus ret;

	if (error.code != 200) {
		LOG(ERROR) << "upload file is error:" << error.message;
		ret.ok = false;
	}
	else {
		DLOG(INFO) << "upload file:" << objectName;
		ret.ok = true;
	}

	Qiniu_Free(uptoken);
	Qiniu_Client_Cleanup(&client);

	ret.downloadUrl = this->url + "/" + objectPath;

	return ret;
}


bool CQiniuCouldStoreModule::DownloadDataWithFile(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, const std::string & filePath, const bool resumable)
{
	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	auto rurl = this->url + "/" + objectPath;

	auto *http = new CHttpClientModule;
	http->SetProgressEvent(DownloadNotify, this);

	bool ret = http->DownLoadFile(rurl, filePath, resumable);

	if (ret)
		DLOG(INFO) << "download file:" << objectName;
	else
		LOG(ERROR) << "download file is error:" << objectName;

	delete http;

	return ret;
}

bool CQiniuCouldStoreModule::DownloadDataWithBuffer(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName, char ** buf, long * bufSize)
{
	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;

	auto rurl = this->url + "/" + objectPath;

	auto *http = new CHttpClientModule;
	http->SetProgressEvent(DownloadNotify, this);

	bool ret = http->DownLoadData(rurl, buf, (int*)bufSize);

	if (ret)
		DLOG(INFO) << "download buffer:" << objectName;
	else
		LOG(ERROR) << "download buffer is error:" << objectName;

	delete http;

	return ret;
}

bool CQiniuCouldStoreModule::DeleteObject(const std::string & bucketName, const std::string &virtualPath, const std::string & objectName)
{
	Qiniu_Client client;
	Qiniu_Mac mac;

	mac.accessKey = accessKeyId.c_str();
	mac.secretKey = accessKeySecret.c_str();

	Qiniu_Client_InitMacAuth(&client, 1024, &mac);

	std::string objectPath;

	if (!virtualPath.empty())
		objectPath = virtualPath + "/" + objectName;
	else
		objectPath = objectName;


	Qiniu_Error error = Qiniu_RS_Delete(&client, bucketName.c_str(), objectPath.c_str());

	if (error.code != 200) {
		LOG(ERROR)<<"delete file is failed, file:"<< objectName <<", bucket:"<< bucketName<<", error:"<<error.message;
		return false;
	}
	else {
		DLOG(INFO)<<"delete file:"<< objectName <<", bucket:" << objectName;
	}

	return true;
}


bool CQiniuCouldStoreModule::SaveProgressInfo(Qiniu_Rio_BlkputRet * ret, const int size)
{
	FILE *file = fopen((this->resumebleFilePath + "/" + fileName).c_str(), "w+");

	if (file)
	{
		nlohmann::json json;

		for (int i = 0; i < size; ++i)
		{
			auto array = nlohmann::json();

			auto  ctx = ret[i];
			
			if (ctx.ctx != nullptr)
			{
				array["ctx"] = ret[i].ctx;
				array["checksum"] = ret[i].checksum;
				array["crc32"] = ret[i].crc32;
				array["offset"] = ret[i].offset;
				array["host"] = ret[i].host;
			}
			else
			{
				array["ctx"] = "";
				array["checksum"] = "";
				array["crc32"] = 0;
				array["offset"] = 0;
				array["host"] = "";
			}

			json.push_back(array);
		}

		auto dump = json.dump();
		
		fwrite(dump.c_str(), 1, dump.size(), file);
		fflush(file);
		fclose(file);

		return true;
	}
	return false;
}

int CQiniuCouldStoreModule::ReadProgressInfo(Qiniu_Rio_BlkputRet * ret)
{

	FILE *file = fopen((this->resumebleFilePath + "/" + fileName).c_str(), "r+");


	if (file)
	{
		int fileSize = GetFileSize((this->resumebleFilePath + "/" + fileName).c_str());
		int count = 0;

		std::shared_ptr<char> buf (new char[fileSize]);
				
		size_t fdfdsfds = fread(buf.get(), 1, fileSize, file);

		fclose(file);

		char *aaa = buf.get();

		auto json = nlohmann::json::parse(std::string(buf.get(), fileSize));

		for (auto arr : json)
		{
			if (arr["ctx"].get<std::string>().empty()) break;

			ret[count].ctx = strdup((arr["ctx"].get<std::string>()).c_str());
			ret[count].checksum = strdup((arr["checksum"].get<std::string>()).c_str());
			ret[count].crc32 = arr["crc32"].get<int>();
			ret[count].offset = arr["offset"].get<int>();
			ret[count].host = strdup((arr["host"].get<std::string>()).c_str());

			count++;
		}

		return json.size();
	}

	return 0;
}
