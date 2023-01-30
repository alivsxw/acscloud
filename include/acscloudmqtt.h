
#ifndef _ACSCLOUDEMQTT_H_
#define _ACSCLOUDEMQTT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "acscloudhttp.h"
#include "acscloudmqtt.h"
#include "acscloudqueue.h"
#include "acscloudshare.h"
#include "acscloud_intellect.h"
#include "acscloudcommon.h"

//设备发给服务端
#define  CMD_MQTT_RES_SV_ANSWER         "answer"        //接收回应
#define  CMD_MQTT_RES_SV_HREATBEAT      "heartbeat"     //心跳
#define  CMD_MQTT_RES_SV_LOGOUT		    "logOut"    	//退出回应
#define  CMD_MQTT_RES_SV_SERVER         "server"        //发送目标
#define  CMD_MQTT_RES_SV_DATANUM        "dataNum"       //数据数量


//数据清理标志
typedef struct _ACS_DataIng_S 
{
	unsigned ACS_CHAR g_byFacesDataIngFlag;
	unsigned ACS_CHAR g_byCardsDataIngFlag;
	unsigned ACS_CHAR g_byAdsDataIngFlag;
	unsigned ACS_CHAR g_byReissueRecordDataIngFlag;
} ACS_DataIng_S;

/**
* fun:入库状态设置 
* @byType-in://1-face//2-card//3-ad
* @byOpe-in://1-add//0-sub
* @return:NULL
**/
ACS_VOID  Mqtt_Set_DataDealFlag(unsigned ACS_CHAR byType,unsigned ACS_CHAR byOpe);

/**
* fun:入库状态获取 
* @ACS_UINT64-in:
* @return:NULL
**/
ACS_UINT64 Mqtt_Get_DataDealFlag(ACS_VOID);

/**
* fun:60s一次发送心跳 
* @pszClientId-in:输入平台的ClientId
* @return:ACS_VOID;
**/
ACS_VOID Mqtt_Heartbeat(const ACS_CHAR *pszClientId);

/**
* fun:接收MQTT的数据->处理POST/GET HTTP->回复给MQTT
* @pbuf-in:收到的MQTT服务器消息实体
* @nlen-in:收到的MQTT服务器消息长度
* @return:0-success;-1-fail
**/
ACS_INT  Mqtt_Deal_MqttSevereMsg(ACS_CHAR *pbuf,ACS_INT nlen);

/**
* fun:回复给mqtt消息发送指令
* @dstmsgid-in:消息ID		 eg-csddfdf12sd14
* @srcid-in:发送者			 eg-{DEVSN}
* @cmdmsg-in:消息命令cmd eg-answer/heartbeat/logOut
* @dsttype-in:接收者		 eg-server
* @rescmd-in:消息子命令  	 eg-syncUsers
* @return:ACS_ERR_SUCCESS;
**/
ACS_INT  Mqtt_AnswerServer(const ACS_CHAR *MsgID,const      ACS_CHAR*Send,const ACS_CHAR*Cmd,const ACS_CHAR *Receiver,const ACS_CHAR *SonsCmd,const ACS_CHAR *pData);

/**
* fun:断开MQTT
* @byIfClearTime-in:是否清空同步数据的时间/标志 ACS_ITURN-清空 ACS_FALSE-不清空
* @return:0-success;
**/
ACS_INT  Mqtt_DisConnect(ACS_BYTE byIfClearTime);

/**
* fun:连接mqtt
* @ACS_VOID-in:ACS_VOID
* @return:0-success;
**/
ACS_INT  Mqtt_Connect();

/**
* fun:初始化mqtt服务 阻塞循环
* @arg-in:void
* @return:ACS_VOID;
**/
ACS_VOID *Mqtt_MsgServe_Task(ACS_VOID *arg);


#ifdef __cplusplus
}
#endif

#endif


