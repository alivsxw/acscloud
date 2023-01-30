#ifndef _ACSCLOUDPIC_H_
#define _ACSCLOUDPIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "acscloudcommon.h"
#include "acscloudshare.h"



#define 	ACS_MSGIDINIT   		"firsttime"
#define 	ACSMQTT_PUBLISHTOPIC 	"deviceMqtt/server"

#define 	ACSPICBASEMAXLEND 		(((ACSPICMAXLEND+2)/3*4)+32)				//PicBaseLen + 头
#define 	ACS_JSON_CODE_SUCCESS   200

extern ACS_Curl_WriteFuncCallback g_Acs_RomoteUpgrade;//远程升级下载升级包处理函数

struct AcsMemoryStruct {
  ACS_CHAR *memory;
  size_t size;
};

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


typedef ACS_INT (*Callback_Deal_ResponseGet)(ACS_CHAR *,ACS_CHAR *,ACS_MESSAGE_S *);
typedef ACS_INT (*Callback_Deal_ResponsePost)(ACS_CHAR *, ACS_CHAR *);
typedef ACS_INT (*Callback_Deal_ResponsePost_v2)(ACS_CHAR *, ACS_CHAR *,ACS_MESSAGE_S *);



/**
* fun:设置同步平台的时间 写入到文件
* @nOpe-in:输入操作标志 ACS_CD_FACES
* @return:0-success;-1-fail
**/
ACS_VOID ACSHTTP_Set_SyncMark(ACS_BYTE nOpe);

/**
* fun:获取同步平台的时间
* @nOpe-in:输入操作标志 ACS_CD_FACES
* @pstSynctime-in:输入时间
* @return:0-success;-1-fail
**/
ACS_VOID  ACSHTTP_Get_SyncMark(ACS_BYTE nOpe,const ACS_CHAR *pSynctime,const ACS_CHAR *pFeild);

/**
* fun:获取是否推送设备数据库数量标志位 1-推 0-不推
* @nOpe-in:输入操作标志 
* @return:0-success;-1-fail
**/
ACS_BYTE ACSHTTP_Get_DataNumIfPush(ACS_VOID);

/**
* fun:设置是否推送设备数据库数量标志位 1-推 0-不推
* @nOpe-in:输入操作标志 
* @return:0-success;-1-fail
**/
ACS_VOID ACSHTTP_Set_DataNumIfPush(ACS_BYTE byDataNUm);

/**
* fun:获取logver
* @nOpe-in:输入操作标志 ACS_CD_FACES
* @return:0-success;-1-fail
**/
ACS_BYTE ACSHTTP_Get_Logver(ACS_VOID);

/**
* fun:获取图片后缀
* @pPicPath:输入图片路径名
* @pSuffix:输出图片后缀(.jpg/.png/.bmp)
* @return:ACS_ERR_E;
**/
ACS_INT ACSHTTP_GetImageSuffix(ACS_CHAR *pPicPath, ACS_CHAR *pSuffix);


/**
* fun:下载文件 (内存Ram存储)
* @cmd-in:请求文件地址
* @fileBuf-in:文件保存的数据
* @nTimes-in:设置超时和连接时间
* @return:0;
**/
ACS_INT ACSHTTP_RamDownFile(const ACS_CHAR *url, struct AcsMemoryStruct *fileBuf,ACS_INT nTimes);

/**
* fun:下载文件 (磁盘Disk存储)
* @cmd-in:请求文件地址
* @picPath-in:文件保存路径
* @nTimes-in:设置超时和连接时间
* @return:success文件长度;fail =<0;
**/
ACS_INT ACSHTTP_DiskDownFile(const ACS_CHAR *url,const ACS_CHAR *picPath,ACS_INT nTimes);

/**
* fun:下载人脸图片
* @pDownPic-in:下载的人脸图片路径
* @pImgInfo-in:人脸信息 ACS_FACE_INFO_S *
* @return:100 ACS_ERR_SUCCESS -success;
**/
ACS_INT ACSHTTP_DownOneFacePic(ACS_CHAR *pDownPic,ACS_CHAR *pImgInfo);

/**
* fun:下载图片数据
* @pszfileBuf-in:下载的人脸图片路径指针存储
* @pImgInfo-in:人脸信息 ACS_FACE_INFO_S *
* @return:100 ACS_ERR_SUCCESS -success;
**/
ACS_INT ACSHTTP_DownOnePicData(struct AcsMemoryStruct *pszfileBuf,ACS_CHAR *pImgInfo);


/**
* fun:广告图片下载函数
* @pAddAdInfo-in:输入 ACS_ADVERT_INFO_S
* @pszPicName-in:输入下载的广告图片路径
* @return:ACS_ERR_E;
**/
ACS_INT ACSHTTP_DownAdPic(ACS_ADVERT_INFO_S *pstAdInfo,const ACS_CHAR* pszPicName);


/**
* fun:获取升级固件信息 HTTP
* @pstUpgadeFW-out:获取的固件信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:NULL
**/
ACS_INT ACSHTTP_GetUpgradeFW(ACS_UPGADEFW_INFO_S *pstUpgadeFW,const ACS_CHAR *pMsgId);


/**
* fun:下载升级文件 (磁盘Disk存储)
* @cmd-in:请求文件地址
* @picPath-in:文件保存路径
* @nTimes-in:设置超时和连接时间
* @ACS_Curl_WriteFuncCallback-in:回调处理函数
* @return:success文件长度;fail =<0;
**/
ACS_INT ACSHTTP_UpgradeFileDownload(const ACS_CHAR *url,const ACS_CHAR *picPath,ACS_INT nTimes, ACS_Curl_WriteFuncCallback callback);


/**
* fun:HTTP Post 推送数据检查结果
* @pstCheackData-in:输入operation 输出check的数据 ACS_CHECKDATA_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostCheckDataResult(const ACS_CHAR *pfindData,ACS_CHECKDATA_INFO_S *pstCheackData,const ACS_CHAR *pMsgId);


/**
* fun:呼叫
* @pstPostData-in:post 数据
* @pPic-in:抓拍图片
* @pstDataResult-in:结果输出
* @return:0-success;
**/
ACS_INT ACSHTTP_Post_Call(const ACS_CALL_INFO_S *pstPostData,const ACS_CHAR *pPic,ACS_CALL_RESULT_S *pstDataResult);



/**
* fun:数据在线验证接口(二维码/密码/卡号数据上传)
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_DataCheckOnline(const ACS_DATACHECKONLINE_INFO_S *pPostinfo, ACS_DATACHECKONLINE_RESULT_S *presult,const ACS_CHAR *pMsgId);


/**
* fun:二维码校验接口
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_HealthQRCode_Check(ACS_HEALTH_QRCODE_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId);


/**
* fun:人证健康码身份证校验接口
* @pPostinfo-in:请求参数 ACS_HEALTH_IDCARD_S * 身份证信息外部进行加密 /device/dcit/api/eq/v1/healthauch/Idcard?uuid=%s&token=%s&pcode=%s
* @pHealthinfo-out:解析的健康码信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_HealthCodeIDCard_Check(ACS_HEALTH_IDCARD_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId);



/**
* fun:请求平台同步用户信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 人脸数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncFaces(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId);


/**
* fun:请求平台获取需要同步的卡号信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 卡号数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncCards(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId);

/**
* fun:请求平台获取需要同步的密码信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 密码数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncPwds(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pDataOut,const ACS_CHAR *pMsgId);


/**
* fun:请求平台获取需要同步的广告信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 卡号数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncAds(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId);


/**
* fun:获取考勤补发信息
* @nPage-in:页码(默认 1)
* @nPageSize-in:每页行数(默认10)
* @pReissueOut-Out:输出请求到的记录结果 ACS_REISSUE_RECORD_S*
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_ReissueRecords(int nPage,int nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId);

/**
* fun:请求平台获取需要同步的派梯消息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncElevatorRules(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId);

/**
* fun:同步用户结果上传
* @pstFaceResult-in:输入人脸同步结果结构体信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncFaceResult_Upload(ACS_SYNCFACE_RESULT_S *pstFaceResult,const ACS_CHAR *pMsgId);


/**
* fun:同步卡号结果上传
* @pstCardResult-in:输入卡号同步结果结构体信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncCardResult_Upload(ACS_SYNCCARD_RESULT_S *pstCardResult,const ACS_CHAR *pMsgId);

/**
* fun:同步广告结果上传
* @pstAdResult-in:输入同步广告结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncAdResult_Upload(ACS_ADVERT_RESULT_S *pstAdResult,const ACS_CHAR *pMsgId);

/**
* fun:获取http TokenHeard
* @ACS_VOID-in:
* @return:ACS_ERR_E
**/
ACS_CHAR *ACSHTTP_GetTokenHeard(ACS_CHAR *pszheader);

/**
* fun:上传抓拍图片
* @psnapphotopath:输入图片路径名
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @* @return:ACS_ERR_E;
**/
ACS_INT ACSHTTP_SnapPhoto_Upload(ACS_CHAR *psnapphotopath,const ACS_CHAR *pMsgId);

/**
* fun:HTTP POST请求MQTT的地址等信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT  ACSHTTP_DeviceLogin(const ACS_CHAR *pMsgId);

/**
* fun:开门数据(短视频,图片,开门信息)上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_OpenDoorRecord_Upload(ACS_OPENDOOR_UPLOAD_S *pstUopendoor);

/**
* fun:通话数据上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail 
**/
ACS_INT ACSHTTP_CallRecord_Upload(ACS_CALL_UPLOAD_S *pstCallUpload);

/**
* fun:请求mqtt启动信息 
* @ACS_CLOUDLOGIN_S-in:输入门禁平台登入参数
* @ACS_URLCONFIG_S-in:输入门禁平台URL接口
* @pTimePathName-in:输入门禁云平台生成文件的路径 
* @return:0-success;
**/
ACS_VOID *RequestCloudInfo_Task(ACS_VOID *arg);


/**
* fun:请求平台获取需要同步的参数信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncParam(const ACS_CHAR *pMsgId);

/**
* fun:上传设备参数到平台
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostDevParam(const ACS_CHAR *pMsgId);

/**
* fun:上传设备分机列表到平台
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostDevDcitTalkList(const ACS_CHAR *pMsgId);

/**
* fun:Post数据 上传错误信息
* @pstMsgIn-in:上传的信息     ACS_MESSAGE_S *
* @nType-in:操作类型 ACS_HTTPURL_SYCUSERS 可为0
* @pstDataIn-in:上传的具体信息 可为NULL
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_VOID
**/
ACS_VOID ACSHTTP_Message_upload(ACS_MESSAGE_S *pstMsgIn,ACS_INT nType,ACS_CHAR *pstDataIn,const ACS_CHAR *pMsgId);


/**
* fun:定时同步平台时间 
* @lltimestamp-in: 需要同步的时间 ACS_UINT64
* @return:void;
**/
ACS_VOID ACSHTTP_SyncPlatformTime(ACS_UINT64 lltimestamp);


/**
* fun:呼叫校验(房间号/手机号/管理处)
* @pstCall-in:请求参数 ACS_DEVICE_CALL_S * 呼叫校验结构体 /device/dcit/api/eq/v1/callCheck?t={timestamp}&uuid={uuid}&token={token}&pcode={pcode}
* @pstCallRes-out:解析的呼叫结果
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_CallCheck(ACS_CALL_CHECK_S* pstCall, ACS_CALL_CHECK_RESULT_S* pstCallRes, const ACS_CHAR* pMsgId);


/**
* fun:呼叫手机APP
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体 /device/dcit/api/eq/v1/callPhone?t={timestamp}&uuid={uuid}&token={token}&pcode={pcode}
* @pstResult-out:操作返回结果
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_WebrtcCallPhone(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult, const ACS_CHAR* pMsgId);


/**
* fun:呼叫唤醒(手机APP)
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体 /device/dcit/api/eq/v1/callHangup?t={timestamp}&uuid={uuid}&token={token}&pcode={pcode}
* @pstResult-out:操作返回结果
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_WebrtcHangUp(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult, const ACS_CHAR* pMsgId);


/**
* fun:ACSHTTP_RestartLogin 重新启动门禁云平台 重新登入门禁云平台 同步平台数据用户门卡广告
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_VOID;
**/
ACS_VOID ACSHTTP_RestartLogin(const ACS_CHAR *pMsgId,const ACS_ULONG ltimestamp);



#ifdef __cplusplus
}
#endif

#endif
