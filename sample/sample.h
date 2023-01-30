#ifndef __ACSCLOUD_SAMPLE_H__
#define __ACSCLOUD_SAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ACS_RECORD_S
{
	//ACS_BYTE bifStop;				//是否停止入库
	ACS_INT  nTotal;				//总数
	ACS_INT  nPageSize;				//当页记录条数
	ACS_CHAR *pRecord;				//输入存储地址 http请求到的数据指针(eg:ACS_FACE_INFO_S *;指向的结构体长度为 nPageSize*ACS_FACE_INFO_S)
}ACS_RECORD_S;


//上传错误提示信息
typedef struct _ACS_MESSAGE_S
{
	ACS_INT	 nCode;					//错误码
	ACS_CHAR szMsg[32];				//提示信息
	ACS_CHAR szCmd[16];				//操作类型触发的API syncUserid/syncCard/syncAd/
//	ACS_CHAR szOperation[16];		//save
	ACS_CHAR szDetails[16];			//具体细节
	ACS_CHAR szID[20];				//用于平台同步校验的ID 触发的指令
}ACS_MESSAGE_S;



#ifdef __cplusplus
}
#endif


#endif
