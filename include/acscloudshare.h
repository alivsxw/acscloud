#ifndef _ACSCLOUDESHARE_H_
#define _ACSCLOUDESHARE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "acsclouddefine.h"


#define  ACS_LOGPATH			"/log/acslog.txt"		//日志文件
#define  ACS_PRINTFPATH			"/log/acsdebug"			//debug
#define  ACS_PRINTFPATHCURL		"/log/acsdebug1"		//debug1




//类型

#define  ACS_USERS				"user"					//用户操作
#define  ACS_FACES				"face"					//人脸操作
#define  ACS_CARDS				"card"					//卡号操作
#define  ACS_PWDS				"password"				//密码操作
#define  ACS_ADS				"ad"					//广告操作
#define  ACS_REISSUES			"reissue"				//补发考勤
#define  ACS_ELERULES			"elerule"				//派梯规则

//数据库操作
#define  ACSDB_SAVE				"save"				
#define  ACSDB_UPDATE			"update"
#define  ACSDB_DELETE			"delete"
//数据库操作
//开门类型
#define  ACSTYPE_FACE			"FACE"					//人脸
#define  ACSTYPE_APP			"APP"					//远程
#define  ACSTYPE_PWD			"PASSWORD"				//密码
#define  ACSTYPE_CARD			"CARD" 					//门卡
#define  ACSTYPE_RCARD			"RCARD"					//居住证
#define  ACSTYPE_IDCARD			"IDCARD"				//身份证
#define  ACSTYPE_QRCODE			"QRCODE"
//开门类型


#define 	ACSSYNCNUM 			100						//每次请求同步平台的记录条数
#define		REISSUENUM			3						//每次补发考勤数量
#define  	ACSJSONMAXLEN  		(1024*5000)				//防止平台下发的json字段过长
#define 	ACSPICMAXLEND 		(1024*490)				//PicLen 6和8的倍数
#define 	ACSSYNCNUM 			100						//每次请求同步平台的记录条数
#define		ACSNOTPWDLIMIT		999999					//密码无次数限制标志


typedef enum _ACS_INFORMLIBCMD_E
{
	ACSCMDE_LOGIN		= 1,//立即登入
	ACSCMDE_FACTORY	    = 2,//恢复出厂
	ACSCMDE_PARAM		= 3,//参数改变
	ACSCMDE_LOG			= 4,//日志
	ACSCMDE_DATANUM		= 5,//设备数据
}ACS_INFORMLIBCMD_E;/**设备通知服务器命令**/


typedef struct _ACS_DEVCONFIG_S
{
	ACS_BYTE 				byType;				//门禁类型	1.单元机 		2.围墙机
	ACS_BYTE 				byisFirst;			//首次登入标志 0-否 1-是 
	ACS_BYTE 				byislogver;			//0-不限制（默认0）; 1-v1接口; 2-v2接口（建议设置为2）
//	ACS_BYTE 				byAcsState;			//通行状态 0-进 1-出 
	ACS_CHAR				szAddress[128]; 	//门禁地址 门禁名称	//ACS_CHAR				szToken[128];		//门禁接口访问token
	ACS_CHAR				szDeviceName[64]; 	//门禁设备名字 eg-智能人脸识别系统
}ACS_DEVCONFIG_S;//设备信息
typedef struct _ACS_WEBRTCCONFIG_S
{
	ACS_CHAR				cAddrs[128];	//webrtc代理服务器地址
	ACS_CHAR				cLicense[128]; 	//license
	ACS_CHAR				cKey[256]; 	//流媒体初始化string key
}ACS_WEBRTCCONFIG_S;//设备信息

typedef struct _ACS_RONGCLOUD_S
{
	ACS_CHAR				cAppkey[32]; 	//
	ACS_CHAR				cToken[128];	//
}ACS_RONGCLOUD_S;//融云手机对讲


typedef enum _ACS_CALLRULE_E
{
	ACSCRULE_OWNERAPP		= 1,//业主APP
	ACSCRULE_INDOOR	    	= 2,//室内机分机
	ACSCRULE_PROPERTYAPP	= 3,//物管APP
	ACSCRULE_MANAGE			= 4,//管理机
}ACS_CALLRULE_E;

typedef struct _ACS_CALLRULE_S
{
	ACS_INT				bOwner[4]; 	//
	ACS_INT				bProperty[4];	//
}ACS_CALLRULE_S;//呼叫规则




/**sip**/
typedef struct _ASC_SIP_S
{
	ACS_INT  nPort; 				//端口号6050
	ACS_CHAR cAddrs[NCHAR_A128];	//sip服务器地址
	ACS_CHAR cAccount[NCHAR_A128]; 	//用户名
	ACS_CHAR cPassword[NCHAR_A128];	//密码
}ASC_SIP_S;

/**outData**/
typedef struct _ASC_DEVALLCFG_S
{
	ACS_DEVCONFIG_S		stDevInfo;
	ACS_WEBRTCCONFIG_S	stWebRtc;
	ASC_SIP_S  			stSip;
	ACS_RONGCLOUD_S		stRongCloud;
	ACS_CALLRULE_S 		stCallRule;
}ASC_DEVALLCFG_S;






/**
* fun:MQTT信息指令通知设备端处理函数
* @ACS_INT-in:命令
* @ACS_CHAR *-in:结果
* @return:0-success;
**/
typedef ACS_INT (*ACS_HandleCmd_Callback)(ACS_INT,ACS_CHAR*);//通知函数

/**
* fun:远程升级下载文件，libcurl数据接收回调函数
* @ACS_INT-in:命令
* @ACS_CHAR *-in:结果
* @return:0-success;
**/
typedef ACS_INT (*ACS_Curl_WriteFuncCallback)(ACS_VOID *, ACS_INT , ACS_INT , ACS_CURL_WRITE_CALLBACK_USER *);//通知函数



/**
* fun:呼叫/挂断 http
* @pstPostData-in:呼叫消息
* @pPic-in:抓拍图片
* @pDataOut-Out:呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Call(const ACS_CALL_INFO_S *pstPostData,const ACS_CHAR *pPic,ACS_CALL_RESULT_S *pDataOut);

/** 呼叫/挂断 mqtt
* fun:音视频对讲呼叫推送mqtt消息
* @nCallType-in:输入呼叫的类型 ACS_CALLTYPE_E 
* @nCallObject-in:输入呼叫的对象 sip
* @nState-in:输入状态	  XZC_CALLSTATE_START 开始呼叫1、接通2、被拒结束3、接通后挂断结束4、等待超时结束5、异常错误结束6、未接通结束呼叫7
* @return:0-success;
**/
ACS_INT ACS_CallPushState(const ACS_INT nCallType,const ACS_CHAR *pCallObject,const ACS_INT nState);

/**
* fun:呼叫检验(房间号/手机号/管理处)
* @pstCall-in:请求参数 ACS_DEVICE_CALL_S * 呼叫校验结构体
* @pstCallRes-out:解析的呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Device_CallCheck(ACS_CALL_CHECK_S* pstCall, ACS_CALL_CHECK_RESULT_S* pstCallRes);

/**
* fun:发起呼叫(房间号/手机号/管理处)
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体
* @pstResult-out:解析的呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Webrtc_CallPhone(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult);	
	
/**
* fun:呼叫挂断(房间号/手机号/管理处)
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体
* @pstResult-out:解析的呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Webrtc_CallHangUp(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult);

/**
* fun:返回库版本号
* @return:版本号;
**/ 
ACS_CHAR *ACS_LibVerSion(ACS_VOID);

/**
* fun:获取平台状态
* @ACS_VOID-in:输入void
* @return:XZC_SERVER_STATE_E;
**/
ACS_BYTE ACS_Get_ServiceState(ACS_VOID);

/**
* fun:POST数据回调 在线验证密码是否正确
* @pwd-in:输入密码
* @pResult-out:result-1正常;userId-密码持有者
* @return:0-success;
**/
ACS_INT ACS_DataCheckOnline(const ACS_DATACHECKONLINE_INFO_S *pPostinfo,ACS_DATACHECKONLINE_RESULT_S *presult);

/**
* fun:二维码校验接口
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @return:0-success;
**/
ACS_INT ACS_HealthCode_QRCode_Check(ACS_HEALTH_QRCODE_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId);

/**
* fun:人证健康码身份证校验接口
* @pPostinfo-in:请求参数 ACS_HEALTH_IDCARD_S * /device/dcit/api/eq/v1/healthauch/Idcard?uuid=%s&token=%s&pcode=%s
* @pHealthinfo-out:解析的健康码信息
* @return:0-success;<0-fail
**/
ACS_INT ACS_HealthCode_IDCard_Check(ACS_HEALTH_IDCARD_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId);

/**
* fun:开门数据(短视频,图片,开门信息)上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACS_OpenDoorRecord_Upload(ACS_OPENDOOR_UPLOAD_S *pstUopendoor);

/**
* fun:呼叫数据上传 
* @stUopendoor-in:呼叫数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACS_CallRecord_Upload(ACS_CALL_UPLOAD_S *pstCall);

/**
* fun:上传抓拍图片
* @psnapphotopath:输入图片路径名
* @* @return:ACS_ERR_E;
**/
ACS_INT ACS_SnapPhoto_Upload(char *psnapphotopath,const ACS_CHAR *pMsgId);

/**
* fun:设置libcurl处理远程升级文件的回调函数
* @pCallbackFunc-in:回调处理函数
* @return:ACS_ERR_E;
**/
ACS_INT ACS_Curl_SetCallback_Download(ACS_Curl_WriteFuncCallback pCallbackFunc);

/**
* fun:设置库操作参数 
* @XZC_SERVERCMD_E-in:设备通知库命令
* @return:ACS_VOID
**/
ACS_VOID ACS_SetLibHandleCmd(ACS_INFORMLIBCMD_E e_Cmd,ACS_CHAR *pMsg);

/**
* fun:设置门禁云平台命令消息通知的回调函数
* @pCallbackFunc-in:回调处理函数
* @return:ACS_ERR_E;
**/
ACS_INT ACS_SetCallback_HandleCmd(ACS_HandleCmd_Callback pCallbackFunc);

/**
* fun:设置门禁云平台参数
* @ACS_CLOUDLOGIN_S-in:输入门禁云平台登入参数
* @return:ACS_ERR_E;
**/ 
ACS_INT ACS_SetServicePlatformInfo(ACS_CLOUDLOGIN_S *pstLogIn);

/**
* fun:初始化加载门禁云平台资源 在pFilePath下创建acscloud文件夹
* @ACS_URLCONFIG_S-in:输入门禁平台URL接口
* @pFilePath-in:输入门禁云平台生成文件的路径
* @pUPackPath-in:输入门禁云平台升级包路径
* @return:0-success;
**/ 
ACS_INT ACS_ServiceInit(ACS_APICONFIG_S *pstUrlPath,const ACS_CHAR *pFilePath,const ACS_CHAR *pUPackPath);




typedef struct 
{
	/**
	* fun:呼叫/挂断 http
	* @pstPostData-in:呼叫消息
	* @pPic-in:抓拍图片
	* @pDataOut-Out:呼叫结果
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_Call)(const ACS_CALL_INFO_S *pstPostData,const ACS_CHAR *pPic,ACS_CALL_RESULT_S *pDataOut);

	/**	呼叫/挂断 mqtt
	* fun:音视频对讲呼叫推送mqtt消息
	* @nCallType-in:输入呼叫的类型 ACS_CALLTYPE_E 
	* @nCallObject-in:输入呼叫的对象 sip
	* @nState-in:输入状态	  XZC_CALLSTATE_START 开始呼叫1、接通2、被拒结束3、接通后挂断结束4、等待超时结束5、异常错误结束6、未接通结束呼叫7
	* @return:0-success;
	**/
	ACS_INT (*ACS_CallPushState)(const ACS_INT nCallType,const ACS_CHAR *pCallObject,const ACS_INT nState);

	/**
	* fun:呼叫检验(房间号/手机号/管理处)
	* @pstCall-in:请求参数 ACS_DEVICE_CALL_S * 呼叫校验结构体
	* @pstCallRes-out:解析的呼叫结果
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_Device_CallCheck)(ACS_CALL_CHECK_S* pstCall, ACS_CALL_CHECK_RESULT_S* pstCallRes);

	/**
	* fun:发起呼叫(房间号/手机号/管理处)
	* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体
	* @pstResult-out:解析的呼叫结果
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_Webrtc_CallPhone)(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult);	
		
	/**
	* fun:呼叫挂断(房间号/手机号/管理处)
	* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体
	* @pstResult-out:解析的呼叫结果
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_Webrtc_CallHangUp)(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult);

	/**
	* fun:返回库版本号
	* @return:版本号;
	**/	
	ACS_CHAR *(*ACS_LibVerSion)(ACS_VOID);

	/**
	* fun:获取平台状态
	* @ACS_VOID-in:输入void
	* @return:XZC_SERVER_STATE_E;
	**/
	ACS_BYTE (*ACS_Get_ServiceState)(ACS_VOID);

	/**
	* fun:POST数据回调 在线验证密码是否正确
	* @pwd-in:输入密码
	* @pResult-out:result-1正常;userId-密码持有者
	* @return:0-success;
	**/
	ACS_INT (*ACS_DataCheckOnline)(const ACS_DATACHECKONLINE_INFO_S *pPostinfo,ACS_DATACHECKONLINE_RESULT_S *presult);

	/**
	* fun:二维码校验接口
	* @pPostinfo:-in:请求参数
	* @pPostinfo:-out:返回的结果信息
	* @return:0-success;
	**/
	ACS_INT (*ACS_HealthCode_QRCode_Check)(ACS_HEALTH_QRCODE_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId);

	/**
	* fun:人证健康码身份证校验接口
	* @pPostinfo-in:请求参数 ACS_HEALTH_IDCARD_S * /device/dcit/api/eq/v1/healthauch/Idcard?uuid=%s&token=%s&pcode=%s
	* @pHealthinfo-out:解析的健康码信息
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_HealthCode_IDCard_Check)(ACS_HEALTH_IDCARD_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId);

	/**
	* fun:开门数据(短视频,图片,开门信息)上传 
	* @stUopendoor-in:开门数据结构体
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_OpenDoorRecord_Upload)(ACS_OPENDOOR_UPLOAD_S *pstUopendoor);

	/**
	* fun:开门数据(短视频,图片,开门信息)上传 
	* @stUopendoor-in:开门数据结构体
	* @return:0-success;<0-fail
	**/
	ACS_INT (*ACS_CallRecord_Upload)(ACS_CALL_UPLOAD_S *pstUopendoor);
	
	/**
	* fun:上传抓拍图片
	* @psnapphotopath:输入图片路径名
	* @* @return:ACS_ERR_E;
	**/
	ACS_INT (*ACS_SnapPhoto_Upload)(char *psnapphotopath,const ACS_CHAR *pMsgId);

	/**
	* fun:设置libcurl处理远程升级文件的回调函数
	* @pCallbackFunc-in:回调处理函数
	* @return:ACS_ERR_E;
	**/
	ACS_INT (*ACS_Curl_SetCallback_Download)(ACS_Curl_WriteFuncCallback pCallbackFunc);

	/**
	* fun:设置库操作参数 
	* @XZC_SERVERCMD_E-in:设备通知库命令
	* @return:ACS_VOID
	**/
	ACS_VOID (*ACS_SetLibHandleCmd)(ACS_INFORMLIBCMD_E e_Cmd,ACS_CHAR *pMsg);

	/**
	* fun:设置门禁云平台命令消息通知的回调函数
	* @pCallbackFunc-in:回调处理函数
	* @return:ACS_ERR_E;
	**/
	ACS_INT (*ACS_SetCallback_HandleCmd)(ACS_HandleCmd_Callback pCallbackFunc);

	/**
	* fun:设置门禁云平台参数
	* @ACS_CLOUDLOGIN_S-in:输入门禁云平台登入参数
	* @return:ACS_ERR_E;
	**/	
	ACS_INT (*ACS_SetServicePlatformInfo)(ACS_CLOUDLOGIN_S *pstLogIn);

	/**
	* fun:初始化加载门禁云平台资源 在pFilePath下创建acscloud文件夹
	* @ACS_URLCONFIG_S-in:输入门禁平台URL接口
	* @pFilePath-in:输入门禁云平台生成文件的路径
	* @pUPackPath-in:输入门禁云平台升级包路径
	* @return:0-success;
	**/	
	ACS_INT (*ACS_ServiceInit)(ACS_APICONFIG_S *pstUrlPath,const ACS_CHAR *pFilePath,const ACS_CHAR *pUPackPath);
}ACSCLOUD_ST_API;

#ifdef __cplusplus
}
#endif

#endif


