#ifndef _ACSCLOUDCOMMON_H_
#define _ACSCLOUDCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <time.h>

#include "mosquitto.h"
#include "cJSON.h"

#include "acscloudshare.h"
#include "acscloudhttp.h"
#include "acscloudmqtt.h"
#include "acscloudqueue.h"
#include "acscloud_intellect.h"
#include "acsclouddefine.h"

/**************************
v1.0.20-
		参数同步新增远程维护参数设置
		mqtt指令新增恢复出厂设置
		修改远程升级时再同步数据导致升级之后数据不完整的问题 (同步接口加标志位)



v1.0.19-
		登录接口新增呼叫等级设置结构体
		新增上传通话记录
		呼叫校验(房间号/手机号/管理处) 新增callid字段
		设备参数同步 设备参数信息上传新增rongcloud的接口配置 呼叫时间的设置 呼叫优先级的设置

v1.0.18-新增在线验证接口开放超时 新增人脸在线验证
		新增在线验证返回信息提示




v1.0.17-同步人脸卡号密码等数据的接口新增2012错误码(无数据)的判断
		修改日志变量大小 在线验证写日志
		新增上传设备参数mqtt 和http接口 mqtt getDevParam指令发起上传
		新增主动上传设备注册数据数量 
		新增主动上传设备参数
		修改平台下发错误码无数据1012不上传
		新增平台可以下发dataNum
		修复重启app的指令不生效的问题 未测试


v1.0.16-修复"ifclient_id"为1;"szClientId"不生效的问题
		所有的http接口url新增url



v1.0.15-修改支持mqtt下发时间戳去请求同步平台数据 修改密码请求的时间戳不对
	   -新增mqtt指令下发错误提示上传
	   -新增findData的操作指令 id 
	   -新增findData查询详细信息接口           适配宏视客户发送checkData指令也可以查询到人脸库 作用和findData一样
	   -修改人脸卡号等数据请求失败不进入入库流程的判断
**************************/



													 
#define ACSLIB_VERSION  		"V1.0.20-bak"				//此版本号对应协议文档(https://www.showdoc.com.cn/v2release)上的修改记录
#define ACSSYNCCMDNUM 			25						//ACS 命令队列

//服务器发给设备
#define  CMD_MQTTSERVER_ANSWER				"answer"			//服务器的心跳回应
#define  CMD_MQTTSERVER_SYCRESTART 			"restartApp"    	//绑定后重登录同步
//命令操作
#define  CMD_MQTTSERVER_LOGIN				"login"				//登入
#define  CMD_MQTTSERVER_SYNCTIME			"syncTime"			//同步时间
#define  CMD_MQTTSERVER_SYCOPENDOOR			"openDoor"			//开门
#define  CMD_MQTTSERVER_SYCCLEANDA			"cleanData"			//清理数据
#define  CMD_MQTTSERVER_RMDEV				"rmDev"				//删除设备
#define  CMD_MQTTSERVER_FINDDATA			"findData"			//查询库数据   checkData
#define  CMD_MQTTSERVER_SYCSTOP				"stopSync"			//停止同步
#define  CMD_MQTTSERVER_UPGRADEFW			"upgradeFw"			//升级
#define	 CMD_MQTTSERVER_CALLED				"called"			//通知室内机被呼叫（室内机接收）
#define  CMD_MQTTSERVER_CALLEND				"callEnd"			//通知门口机呼叫挂断（门口机）
#define  CMD_MQTTSERVER_REBOOTDEV			"rebootDev"			//设备重启
#define  CMD_MQTTSERVER_SYCPARAM 			"syncParam"			//同步参数
#define  CMD_MQTTSERVER_SNAPPHOTO			"snapphoto"			//抓拍图片
#define  CMD_MQTTSERVER_CALLSYNC			"callSync"			//mqtt对讲信令
#define  CMD_MQTTSERVER_GETDEVPARAM			"getDevParam"		//获取设备参数
#define  CMD_MQTTSERVER_GETDCITTALKLIST		"getTalkIndoorList"	//获取分机列表
#define  CMD_MQTTSERVER_RESETFACTORY		"resetFactory"		//恢复出厂
#define  CMD_MQTTSERVER_TELEMAINTENANCE		"remoteMaintenance"	//远程维护


//数据库写入操作-费时的接口
#define  CMD_MQTTSERVER_SYCUSERS 			"syncUsers"			//同步用户
#define  CMD_MQTTSERVER_SYCCARD				"syncCards"    		//同步门卡
#define  CMD_MQTTSERVER_SYCPWD				"syncPasswords"    	//同步密码
#define  CMD_MQTTSERVER_SETAD				"setAd"				//下发广告
#define  CMD_MQTTSERVER_ELERULES			"syncEleRules"		//同步派梯规则 //定制
#define  CMD_MQTTSERVER_REISSUERECORD		"reissueRecord"		//补发考勤记录 //定制


#define	ACS_SUCCESS	0
#define ACS_FAILED	-1
#define ACS_ITURN   1
#define ACS_FALSE   0

#define ACS_DEBUGX  9
#define ACS_DEBUG   10
#define ACS_INFO    20
#define ACS_ERROR   30


ACS_BYTE ACSCom_GetPrintfLevel(ACS_BYTE logLevel);
ACS_BYTE ACSCom_SetPrintfLevel(ACS_VOID);
ACS_BYTE ACSCOM_GetLogLevel(ACS_VOID);



//颜色宏定义
#define ACSNONE         "\033[0m\n"
#define ACSRED          "\033[1m\033[40;31m"
#define ACSGREEN        "\033[1m\033[40;32m"
#define ACSBLUE         "\033[0m\033[40;34m"
#define ACSLIGHT_BLUE   "\033[1m\033[40;34m"
#define ACSDARY_GRAY    "\033[1m\033[40;30m"
#define ACSCYAN         "\033[0m\033[40;36m"
#define ACSLIGHT_CYAN   "\033[1m\033[40;36m"
#define ACSPURPLE       "\033[0m\033[40;35m"
#define ACSLIGHT_PURPLE "\033[1m\033[40;35m"
#define ACSBROWN        "\033[0m\033[40;33m"
#define ACSYELLOW       "\033[1m\033[40;33m"
#define ACSLIGHT_GRAY   "\033[0m\033[40;37m"
#define ACSWHITE        "\033[1m\033[40;37m"






#define PRT_PRINT(level,msg, arg...)  do{if(ACSCom_GetPrintfLevel(level)) \
                printf("[acsLib]:[%s-%d]---- " msg " ----\n",__func__,__LINE__,##arg); \
        } while (0)


#if 0
#define ACS_WLOG(...)
#else
ACS_INT ACS_WriteLOG(const ACS_CHAR *pfunc,ACS_INT nline,const ACS_CHAR *format, ...);
#ifndef ACS_WLOG
#define ACS_WLOG(format, ...) 	ACS_WriteLOG(__func__,__LINE__,format,##__VA_ARGS__)
#endif
#endif


/*************************************************
二进制计算
*************************************************/
#define ACSBIT_GET(x,y)   			((x) >> (y)&1)										//获取的某一位的值
#define ACSBIT_SET(x,y)  			x|=(1<<y)											//指定的某一位数置1
#define ACSBIT_CLR(x,y)  			x&=~(1<<y)											//指定的某一位数置0
#define ACSBIT_REVERSE(x,y)  		x^=(1<<y)											//指定的某一位数取反


typedef enum _ACS_HTTPURL_E
{
	//服务器发给设备
	ACS_HTTPURL_ANSWER			=	1, //"answer"			//服务器的心跳回应
	ACS_HTTPURL_SYCRESTART 		=	2, //"restartApp"    	//绑定后重登录同步
	
	//命令操作(数据量小 阻塞)
	ACS_HTTPURL_LOGIN			=	20,//"login"			//登入
	ACS_HTTPURL_SYCOPENDOOR		=	21,//"openDoor"			//开门
	ACS_HTTPURL_SYCCLEANDA		=	22,//"cleanData"		//清理数据
	ACS_HTTPURL_RMDEV			=	23,//"rmDev"			//删除设备
	ACS_HTTPURL_CHECKDATA		=	24,//"checkData"		//查询库数据
	ACS_HTTPURL_SYCSTOP			=	25,//"stopSync"			//停止同步
	ACS_HTTPURL_UPGRADEFW		=	26,//"upgradeFw"		//升级
	ACS_HTTPURL_CALLED			=	27,//"called"			//通知室内机被呼叫（室内机接收）
	ACS_HTTPURL_CALLEND			=	28,//"callEnd"			//通知门口机呼叫挂断（门口机）
	ACS_HTTPURL_REBOOTDEV		=	29,//"rebootDev"		//设备重启
	ACS_HTTPURL_SYNCTIME		=	30,//"TIME"				//同步时间
	ACS_HTTPURL_DATACHECKONLINE	=	32,//"dataCheckOnline"	//数据在线验证(获取二维码内容上传平台)
	ACS_HTTPURL_SNAPPHOTO		=	33,//"snapphoto"		//抓拍人脸
	
	
	//数据库写入操作(数据量大费时间 非阻塞)	
	ACS_HTTPURL_GETPARAM		=	39,//"getDevParam"		//上传设备参数到平台
	ACS_HTTPURL_SETPARAM		=	40,//"syncParam"		//同步设备参数
	ACS_HTTPURL_SYCUSERS 		=	41,//"syncUsers"		//同步用户
	ACS_HTTPURL_SYCCARD			=	42,//"syncCards"    	//同步门卡
	ACS_HTTPURL_SETAD			=	43,//"setAd"			//下发广告
	ACS_HTTPURL_SYCPWD			=	44,//"syncPasswords"    //同步密码
	
	//定制的操作
	ACS_HTTPURL_REISSUERECORD	=	51,//"reissueRecord"	//补发考勤记录
	ACS_HTTPURL_QRCODE			=	54,//"QRCODE"			//查询健康码信息-二维码
	ACS_HTTPURL_IDCARD			=	55,//"IDCARD"			//查询健康码信息-身份证
	ACS_HTTPURL_ELEVATORRULES	=	56,//"elevatorRules"	//下发派梯规则
	ACS_HTTPURL_DCITTALKLIST	=	57,//"DcitTalkList"		//上传设备分机列表到平台
	
	//对讲
	ACS_HTTPURL_CALLPHONE		=	60,//"Callphone"		//呼叫手机号
	ACS_HTTPURL_CALLROOM		=	61,//"Callroom"			//呼叫房间号
	ACS_HTTPURL_CALLMANAGEMENT	=	62,//"CallManagement"	//呼叫管理处
	ACS_HTTPURL_CALLHANGUP		=	63,//"callHangup"		//呼叫挂断
	ACS_HTTPURL_CALLCHECK		=	64,//"callCheck"		//呼叫校验
}ACS_HTTPURL_E;


//停止入库标志 非MQTT回调阻塞 费时的需要
typedef enum _ACS_CLEARDATA_E
{
	ACS_CD_NOT				=	0x00,	//闲置 
	ACS_CD_FACE				=	0x01,	//人脸 
	ACS_CD_CARD				=	0x02,	//卡号 
	ACS_CD_AD				=	0x04,	//广告   
	ACS_CD_UPACK			=	0x08,	//升级   
	ACS_CD_REISSUES 		=	0x10,	//考勤补发 
	ACS_CD_PARAM 			=	0x20,	//同步设备参数
	ACS_CD_ELERULES			=	0x40,	//派梯规则
	ACS_CD_PWD				=	0x80,	//密码 
	ACS_CD_GETPARAM			=	0x100,	//获取设备参数 
	ACS_CD_GETDCITTALKLIST	=	0x200,	//获取分机列表 DCITTALKLIST
}ACS_CLEARDATA_E;


//
typedef enum _ACS_SYNCDATA_E
{
	ACS_CDSYNC_NOT				=	0,	//闲置 
	ACS_CDSYNC_FACE				=	1,	//人脸 
	ACS_CDSYNC_CARD				=	2,	//卡号 
	ACS_CDSYNC_AD				=	3,	//广告   
	ACS_CDSYNC_UPACK			=	4,	//升级   
	ACS_CDSYNC_REISSUES 		=	5,	//考勤补发 
	ACS_CDSYNC_PARAM 			=	6,	//同步设备参数
	ACS_CDSYNC_ELERULES			=	7,	//派梯规则
	ACS_CDSYNC_PWD				=	8,	//密码 
	ACS_CDSYNC_GETPARAM			=	9,	//获取设备参数 
	ACS_CDSYNC_GETDCITTALKLIST	=	10,	//获取分机列表 DCITTALKLIST
}ACS_SYNCDATA_E;


//呼叫类型标志
typedef enum _ACS_CALL_STREAM_E
{
	ACS_CALL_WEBRTC = 0x00,	//webrtc传输流
	ACS_CALL_SIP,			//sip传输流
}ACS_CALL_STREAM_E;

//命令队列
typedef struct _ACS_CMD_S
{
	ACS_CLEARDATA_E e_cmd;
	ACS_ULONG utimestamp;
	ACS_CHAR szMsgID[33];
}ACS_CMD_S;

typedef struct _ACS_CALLED_INFO_S
{
	ACS_CHAR szRoom[8];		//目标设备房间号
	ACS_LONG lCallTime;		//发起呼叫时间戳
	ACS_CHAR szMediaId[32];	//媒体账户
}ACS_CALLED_INFO_S;

typedef  struct _DBOPRA_MAP_ACS
{
    ACS_INT  eMDorerror;//ACS_ERR_E
    ACS_INT  nCode;
    ACS_CHAR szMsg[32];
}DBOPRA_MAP_ACS;

//失败信息通知
typedef  struct _INFORM_ERR_ACS
{
	ACS_INT  nCode;		//ACS_ERR_E
    ACS_INT  nID[64];
    ACS_CHAR szMsg[64];
}INFORM_ERR_ACS;



/*********对应数据类型*************/
typedef enum _ACS_JSON_DATATYPE_E_
{
	ACS_JSON_DATA_INT = 0,		//INT
	ACS_JSON_DATA_FLOAT,		//float
	ACS_JSON_DATA_STRING,		//string
	ACS_JSON_DATA_CHAR,			//char
	ACS_JSON_DATA_USHORT,		//unsigned short
	ACS_JSON_DATA_SHORT,		//short
	ACS_JSON_DATA_DOUBLE,		//double
	ACS_JSON_DATA_LONG,			//long
	ACS_JSON_DATA_ULONG,		//unsigned long
}ACS_JSON_DATATYPE_E;

typedef struct _ACS_MARK_TIME_S //更新数据时间信息
{
	ACS_UINT64  face;	  		//上次同步人脸信息时间
	ACS_UINT64  card;     		//上次同步门禁卡时间
	ACS_UINT64  ad;		 		//上次同步广告时间
	ACS_UINT64  reissue;		//上次同步补发考勤记录时间
	ACS_UINT64  erules;			//上次同步派梯时间
	ACS_UINT64  pwd;     		//上次同步密码时间
} ACS_MARK_TIME_S;

typedef struct _ACS_MARK_FIELD_S //更新数据时间信息
{
	ACS_CHAR  face[128];	  		//上次同步人脸信息标志
	ACS_CHAR  card[128];     		//上次同步门禁卡标志
	ACS_CHAR  ad[128];		 		//上次同步广告标志
	ACS_CHAR  reissue[128];			//上次同步补发考勤记录标志
	ACS_CHAR  erules[128];			//上次同步派梯记录标志
	ACS_CHAR  pwd[128];		 		//上次同步密码标志
} ACS_MARK_FIELD_S;

typedef struct _ACS_SYNC_MARK_S //更新数据时间信息
{
	ACS_BYTE	logLevel;					//debug:10;info:20;error:30
	ACS_MARK_TIME_S		stTime;
	ACS_MARK_FIELD_S 	stField;
} ACS_SYNC_MARK_S;



typedef struct _ACS_MQTT_CONFIG_S
{
	ACS_BYTE		 byQos;					//Qos服务质量等级 默认2
	ACS_BYTE		 byIfClientId;			//是否使用client作为mqttClientID 默认0 0-不使用
	ACS_BYTE   		 keepalive;				//保活间隔
	
	ACS_CHAR		 DevcUuid[128];			//设备标识UUID
	ACS_CHAR 		 szIP[64];				//mqtt服务ip
	ACS_CHAR 		 szUsername[64];		//mqtt访问用户名
	ACS_CHAR 		 szMqttPwd[64];			//访问密码
	ACS_CHAR 		 szClientId[128];		//mqtt客户端订阅ID 
	ACS_CHAR 		 szSubopicTopic[128];	//mqtt客户端订阅主题 为空默认 deviceMqtt/{ClientId} //ACS_BYTE   		 subopicm_id;		//mqtt客户端订阅主题id 默认 5
	ACS_CHAR 		 szPublishTopic[128];	//mqtt客户端发布主题
	ACS_ULONG		 nPort;					//mqtt服务端口
}ACS_MQTT_CONFIG_S;


typedef struct _ACS_CONFIG_S
{
	ACS_CLOUDLOGIN_S 	stClouldInfo;	//门禁云平台登入信息 用户输入
	ACS_MQTT_CONFIG_S   stMqttConfig;	//mqtt配置 			 平台获取
	ACS_DEVCONFIG_S 	stDevInfo;		//设备登入信息  			 平台获取
	//ACS_WEBRTCCONFIG_S  stWebrtc;		//webrtc初始化信息		 平台获取 
	//ASC_SIP_S			stSip;			//sip初始化信息		 	 平台获取
}ACS_CONFIG_S;


typedef struct _ACS_EFIELD_S
{
	ACS_CHAR 		 face[32];		//请求人脸数据标识
	ACS_CHAR 		 card[32];
	ACS_CHAR 		 ad[32];
}ACS_EFIELD_S;


/*******************************************************公共的********************************************************/

/*******************************************************公共的********************************************************/

ACS_INT ACSCOM_EnDataQueue(ACS_CLEARDATA_E e_cmd,const ACS_CHAR *pMsgId,const ACS_ULONG utimestamp);


ACS_INT ACSCOM_DeDataQueue(ACS_CMD_S *pstCmdInfo);


ACS_INT ACSCOM_DeAllDataQueue(ACS_VOID);


ACS_VOID ACSCOM_DestroyDataSequeue(ACS_VOID);


ACS_CHAR* ACSCOM_GetAcsPath(ACS_VOID);


ACS_CHAR* ACSCOM_GetUploadPath(ACS_VOID);

/**
* fun:获取云平台使能开关接口
* @void-void
* @return:0-关闭;1-开启;
**/
ACS_INT ACSCOM_GetClouldEnable(ACS_VOID);

/**
* fun:系统调用
* @cmdstring-in:命令
* @return:success-0 ACS_SUCCESS;fail:-1 ACS_FAILED;
**/
ACS_INT ACSCOM_COMMON_System(const ACS_CHAR * cmdstring);

/**
* fun:由设备错误码转为平台错误码
* @pnError-in:操作错误码
* @pszMsg-in:操作错误消息 如为NULL返回pnError对应的错误消息
* @return:success-平台错误码;失败返回ACS_ERR_E;
**/
ACS_INT ACSCOM_OperateResult(ACS_INT *pnError,ACS_CHAR *pszMsg);

/**
* fun:获取mqtt的登入信息
* @pstSynctime-out:输出 ACS_MQTT_CONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_MQTTCfg(ACS_MQTT_CONFIG_S *pstMqttConfig);


/**
* fun:获取同步数据的时间
* @pstSynctime-out:输出 ACS_SYNC_MARK_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_SyncTimeCfg(ACS_SYNC_MARK_S *pstSynctime);


/**
* fun:设置同步数据的时间
* @pstSynctime-in:输入 ACS_SYNC_MARK_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Set_SyncTimeCfg(ACS_SYNC_MARK_S *pstSynctime);


/**
* fun:获取提示数据库数量的标志位 0-不推送 1-推送 默认1
* @ACS_VOID-in:
* @return:0-不推送 1-推送
**/
ACS_VOID ACSCOM_Set_ifDataNum(ACS_BYTE bIfPush);


/**
* fun:获取提示数据库数量的标志位 0-不推送 1-推送
* @ACS_VOID-in:
* @return:0-不推送 1-推送
**/
ACS_BYTE ACSCOM_Get_ifDataNum(ACS_VOID);


/**
* fun:获取所有门禁云平台参数信息
* @pstMjapiconfig-out:输出 ACS_CONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_Config(ACS_CONFIG_S *pstMjapiconfig);

/**
* fun:获取设备信息
* @pstDevInfo-in:输入 ACS_DEVCONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_GetDevInfo_Config(ACS_DEVCONFIG_S *pstDevInfo);


/**
* fun:设置设备信息
* @pstDevInfo-in:输入 ACS_DEVCONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_SetDevInfo_Config(ACS_DEVCONFIG_S *pstDevInfo);

/**
* fun:获取设备数据数量
* @pstDevInfo-in:输入 ACS_DATANUM_S *
* @return:ACS_ERR_E
**/
ACS_VOID  ACSCOM_GetDataNum_Config(ACS_DATANUM_S *pstDataNum);


/**
* fun:清空设备数据数量
* @pstDevInfo-in:输入 ACS_DATANUM_S *
* @return:ACS_ERR_E
**/
ACS_VOID  ACSCOM_ClearDataNum_Config(ACS_VOID);


/**
* fun:设置设备数据数量
* @pstDevInfo-in:输入 ACS_DATANUM_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_SetDataNum_Config(ACS_DATANUM_S *pstDataNum);


/**
* fun:设置门禁云平台登入信息
* @ACS_CLOUDLOGIN_S-in:输入 ACS_CLOUDLOGIN_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Set_DoorClouldConfig(ACS_CLOUDLOGIN_S *pstClouldInfo);

/**
* fun:获取云平台信息
* @ACS_CLOUDLOGIN_S-out:输出 ACS_CLOUDLOGIN_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_DoorClouldConfig(ACS_CLOUDLOGIN_S *pstCloudCfg);



/**
* fun:设置平台状态并且通知设备
* @ACS_SERVER_STATE_E-in:输入状态值
* @return:ACS_SERVER_STATE_E;
**/
ACS_INT ACSCOM_Set_ServiceState(ACS_SERVER_STATE_E state);

/**
* fun:获取平台状态
* @ACS_VOID-in:输入void
* @return:ACS_SERVER_STATE_E;
**/
ACS_INT ACSCOM_Get_ServiceState(ACS_VOID);


/**
* fun:是否在同步数据 1-是 0-否
* @nCmd-in:输入 nCmd 
* @bData-in:输入 bData 1/0 
* @return:ACS_ERR_E
**/
ACS_VOID  ACSCOM_Set_SyncDataFlag(ACS_SYNCDATA_E nCmd,ACS_BYTE bData);

/**
* fun:获取是否在同步数据 1-是 0-否
* @nCmd-in:输入 nCmd 
* @return:ACS_ERR_E
**/
ACS_DWORD  ACSCOM_Get_SyncDataFlag(ACS_SYNCDATA_E nCmd);



/**
* fun:申请Malloc内存
* @return:ACS_ERR_E;
**/
ACS_INT  ACSCOM_Malloc(ACS_VOID);

/**
* fun:设置门禁云平台操作路径
* @pPath-in:路径
* @pUPackPath-in:升级包路径
* @return:ACS_ERR_E;
**/
ACS_INT  ACSCOM_DirPath(const ACS_CHAR *pPath,const ACS_CHAR *pUPackPath);


/**
* fun:获取人脸的malloc地址
* @return:0-ACS_CHAR *;
**/
ACS_CHAR *ACSCOM_MallocFaceAddress(ACS_VOID);

/**
* fun:释放人脸的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_FreeFaceAddress(ACS_VOID);

/**
* fun:清空人脸的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_CleanFaceAddress(ACS_VOID);

/**
* fun:获取考勤补发的malloc地址
* @return:success-返回地址char *;fail-NULL
**/
ACS_CHAR *ACSCOM_MallocReissueAddress(ACS_VOID);

/**
* fun:清空考勤补发的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_CleanReissueAddress(ACS_VOID);

/**
* fun:释放考勤补发的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_FreeReissueAddress(ACS_VOID);

/**
* fun:通知设备处理数据
* @eCmd-in:操作命令 ACS_CMD_E
* @pstSyncDb-in:待设备操作的数据
* @return:ACS_INT;
**/
ACS_INT ACSCOM_HandleCmdCallback(ACS_CMD_E eCmd,ACS_CHAR *pstSyncDb);


/**
* fun:下载人脸图片  如失败上传失败信息
* @pstFaceRecord:输入人脸信息ACS_FACE_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_ERR_SUCCESS-success;ACS_ERR_FAIL-fail
**/
ACS_INT  ACSCOM_SyncFacePic(ACS_FACE_INFO_S *pstFaceRecord,const ACS_CHAR *pMsgId);


/**
* fun:下载广告图片  如失败上传失败信息
* @pstAdInfo:输入人脸信息 ACS_ADVERT_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_ERR_E;
**/
ACS_INT ACSCOM_SyncAdPic(ACS_ADVERT_INFO_S *pstAdInfo,const ACS_CHAR *pMsgId);


/**
* fun:通知函数
* @pCallbackFunc-in:回调处理函数
* @return:ACS_VOID;
**/
ACS_VOID ACSCOM_SetCallback_HandleCmd(ACS_HandleCmd_Callback pCallbackFunc);


/**
* fun:保存同步时间 以文件形式保存到设备
* @pPath-in:存储路径
* @stConfig-in:保存的同步时间
* @return:ACS_ERR_SUCCESS-success;ACS_ERR_FAIL-fail
**/
ACS_INT  ACSCOM_Save_SyncTimeCfg(const ACS_CHAR *pPath, ACS_SYNC_MARK_S *stConfig);


/**
* fun:初始化人脸同步信息
* @pPath:输入时间同步的文件路径
* @return:ACS_ERR_E
**/
ACS_INT ACSCOM_Init_SyncTimeCfg(const ACS_CHAR *pPath);

/**
* fun:清理由平台入库的数据
* @return:ACS_VOID
**/
ACS_VOID ACSCOM_CleanData(ACS_CHAR *pOperation);

/**
* fun:重新启动门禁云平台 重新登入门禁云平台 同步平台数据用户门卡广告
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_VOID;

ACS_VOID ACSCOM_RestartLogin(const ACS_CHAR *pMsgId,const ACS_ULONG ltimestamp);
**/

#ifdef __cplusplus
}
#endif

#endif


