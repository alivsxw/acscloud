#include "acscloudhttp.h"
#include "acscloudqueue.h"
#include "acscloud_intellect.h"
#include "acscloudcommon.h"
#include "acscloudshare.h"



/**
* fun:获取平台状态
* @ACS_VOID-in:输入void
* @return:XZC_SERVER_STATE_E;
**/
ACS_BYTE ACS_Get_ServiceState(ACS_VOID)
{
	return ACSCOM_Get_ServiceState();
}



/**
* fun:开门数据(短视频,图片,开门信息)上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACS_OpenDoorRecord_Upload(ACS_OPENDOOR_UPLOAD_S *pstUopendoor)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	return ACSHTTP_OpenDoorRecord_Upload(pstUopendoor);
}


/**
* fun:呼叫数据上传 
* @stUopendoor-in:呼叫数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACS_CallRecord_Upload(ACS_CALL_UPLOAD_S *pstCall)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	return ACSHTTP_CallRecord_Upload(pstCall);
}


/**
* fun:请求平台检验数据 
* @pPostinfo-in:数据(PASSWORD,APP,FACE,CARD,RCARD,IDCARD,QRCODE)	
* @presult-out:数据
* @return:0-success;
**/
ACS_INT ACS_DataCheckOnline(const ACS_DATACHECKONLINE_INFO_S *pPostinfo,ACS_DATACHECKONLINE_RESULT_S *presult)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;

	return ACSHTTP_DataCheckOnline(pPostinfo,presult,"");
}

//{“c”:”heartbeat”,”f”:”{{client_id}}”,”m”:{“t”:”9d2764aa-8770-47d5-a379-69068cb6dcaf”},”t”:”server”,”timestamp”:1658124953}
//{“c”:”hangUp”,”f”:”{{client_id}}”,”m”:{“t”:”9d2764aa-8770-47d5-a379-69068cb6dcaf”},”t”:”server”,”timestamp”:1658124953}
//{“c”:”datanum”,”f”:”{{client_id}}”,”m”:{“t”:”9d2764aa-8770-47d5-a379-69068cb6dcaf”},”t”:”server”,”timestamp”:1658124953,"data":{"user":11,"card":12,"password":120,"ad":42}}

/**
* fun:音视频对讲呼叫推送mqtt消息
* @nCallType-in:输入呼叫的类型 ACS_CALLTYPE_E 
* @nCallObject-in:输入呼叫的对象 sip
* @nState-in:输入状态	  XZC_CALLSTATE_START 开始呼叫1、接通2、被拒结束3、接通后挂断结束4、等待超时结束5、异常错误结束6、未接通结束呼叫7
* @return:0-success;
**/
ACS_INT ACS_CallPushState(const ACS_INT nCallType,const ACS_CHAR *pCallObject,const ACS_INT nState)
{
	ACS_CHAR msgid[33] = {0};	
	const ACS_CHAR map[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	ACS_MQTT_CONFIG_S stMqttSvconfig = {0};
	ACSCOM_Get_MQTTCfg(&stMqttSvconfig);
	
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	else if(ACSCOM_Get_ServiceState() != ACS_SERVER_OK)
		return ACS_ERR_SERVERFAIL;

	ACS_INT ret = ACS_ERR_FAIL;
	ACS_CHAR szCmd[16] = {0};

	if((ACS_CALL_HANGUP == nState) || (ACS_CALL_TIMEOUT == nState))
	{
		memcpy(szCmd, "hangUp",16);
	}
	
	srand((unsigned)time(NULL));
	for (int i = 0; i < 32; i++)
	{
		msgid[i] = map[rand()%36];
	}
	
	ret = Mqtt_AnswerServer(msgid, stMqttSvconfig.szClientId, szCmd,CMD_MQTT_RES_SV_SERVER,pCallObject,NULL);
	
	return ret;
}


/**
* fun:设置库操作参数 
* @XZC_SERVERCMD_E-in:设备通知库命令
* @return:ACS_VOID
**/
ACS_VOID ACS_SetLibHandleCmd(ACS_INFORMLIBCMD_E e_Cmd,ACS_CHAR *pMsg)
{
	switch(e_Cmd)
	{
		case ACSCMDE_LOGIN:
		{
			
			break;
		}
		case ACSCMDE_FACTORY:
		{
			
			break;
		}
		case ACSCMDE_PARAM:
		{
			
			break;
		}
		case ACSCMDE_LOG:
		{
			ACSCom_SetPrintfLevel();
			break;
		}
		case ACSCMDE_DATANUM:
		{
			ACS_DATANUM_S *pstDataNum = (ACS_DATANUM_S *)pMsg;
			if(pstDataNum && (ACS_ITURN == ACSCOM_Get_ifDataNum()))
			{
				ACSCOM_SetDataNum_Config(pstDataNum);
			}
			break;
		}
		default:
		{
			break;
		}
	}
	
	return;
}




/**
* fun:设置门禁云平台命令消息通知的回调函数
* @pCallbackFunc-in:回调处理函数
* @return:ACS_ERR_E;
**/
ACS_INT ACS_SetCallback_HandleCmd(ACS_HandleCmd_Callback pCallbackFunc)
{
	if(pCallbackFunc)
	{
		ACSCOM_SetCallback_HandleCmd(pCallbackFunc);
		return ACS_ERR_SUCCESS;
	}
	else
		return ACS_ERR_PARAMNULL;
}


/**
* fun:设置libcurl处理远程升级文件的回调函数
* @pCallbackFunc-in:回调处理函数
* @return:ACS_ERR_E;
**/
ACS_INT ACS_Curl_SetCallback_Download(ACS_Curl_WriteFuncCallback pCallbackFunc)
{
	if(pCallbackFunc)
	{
		g_Acs_RomoteUpgrade = pCallbackFunc;
		return ACS_ERR_SUCCESS;
	}
	else
		return ACS_ERR_PARAMNULL;
}


/**
* fun:设置门禁云平台参数
* @ACS_CLOUDLOGIN_S-in:输入门禁云平台登入参数
* @return:ACS_ERR_E;
**/	
ACS_INT ACS_SetServicePlatformInfo(ACS_CLOUDLOGIN_S *pstLogIn)
{
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);

	if((pstLogIn == NULL) || (strlen(pstLogIn->szHttpUrl) < 4) || (strlen(pstLogIn->szUuid) < 1))
	{
		PRT_PRINT(ACS_ERROR,"acslib ServerUrl or Guid NULL!");
		return ACS_ERR_PARAMERR;
	}
	
	
	
	//stClouldInfo.nDeviceType = pstLogIn->nDeviceType;	//设备类型
	memcpy(stClouldInfo.szHttpUrl, pstLogIn->szHttpUrl, sizeof(stClouldInfo.szHttpUrl));  //42.193.106.43:8799
	memcpy(stClouldInfo.szUuid, pstLogIn->szUuid, sizeof(stClouldInfo.szUuid));
	memcpy(stClouldInfo.szPcode, pstLogIn->szPcode, sizeof(stClouldInfo.szPcode));
	memcpy(stClouldInfo.szSoftVersion, pstLogIn->szSoftVersion, sizeof(stClouldInfo.szSoftVersion));
	memcpy(stClouldInfo.szDevWiredIP, pstLogIn->szDevWiredIP, sizeof(stClouldInfo.szDevWiredIP));
	memcpy(stClouldInfo.szDevType, pstLogIn->szDevType, sizeof(stClouldInfo.szDevType));

	memcpy(stClouldInfo.szcpuID, pstLogIn->szcpuID, sizeof(stClouldInfo.szcpuID));

	PRT_PRINT(ACS_INFO,"nEnable:%d;raw:%d;SerUrl:%s;szSoftVersion:%s;Guid:%s;cpu:%s",pstLogIn->nEnable,stClouldInfo.nEnable,pstLogIn->szHttpUrl,pstLogIn->szSoftVersion,pstLogIn->szUuid,stClouldInfo.szcpuID);

	stClouldInfo.nEnable = pstLogIn->nEnable;
	ACSCOM_Set_DoorClouldConfig(&stClouldInfo);
	
	return ACS_ERR_SUCCESS;
}


/**
* fun:初始化加载门禁云平台资源 在pFilePath下创建acscloud文件夹
* @ACS_URLCONFIG_S-in:输入门禁平台URL接口
* @pFilePath-in:输入门禁云平台生成文件的路径
* @pUPackPath-in:输入门禁云平台升级包路径
* @return:0-success;
**/	
ACS_INT ACS_ServiceInit(ACS_APICONFIG_S *pstUrlPath,const ACS_CHAR *pFilePath,const ACS_CHAR *pUPackPath)
{
	ACS_INT ret = 0;
	ret = ACSCOM_DirPath(pFilePath,pUPackPath);
	if(ACS_ERR_SUCCESS != ret)
		return ret;
	
	ret = ACSCOM_Malloc();
	if(ACS_ERR_SUCCESS != ret)
		return ret;

	
	pthread_t RequestCloudInfo_t;
	ret = pthread_create(&RequestCloudInfo_t, NULL, RequestCloudInfo_Task, NULL);
	if(ret == ACS_SUCCESS)
	{
		pthread_detach(RequestCloudInfo_t);
	}
	else
	{
		ACS_WLOG("Create p_thread RequestCloudInfo_Task fail");
		PRT_PRINT(ACS_ERROR,"Create p_thread RequestCloudInfo_Task fail");
		return ACS_ERR_FAIL;
	}
	
	pthread_t MsgServe_t;
	ret = pthread_create(&MsgServe_t, NULL, Mqtt_MsgServe_Task, NULL);
	if(ret == ACS_SUCCESS)
	{
		pthread_detach(MsgServe_t);
	}
	else
	{
		ACS_WLOG("Create p_thread MqttServe_Task fail");
		PRT_PRINT(ACS_ERROR,"Create p_thread MqttServe_Task fail");
		return ACS_ERR_FAIL;
	}

	pthread_t DataDeal_t;
	ret = pthread_create(&DataDeal_t, NULL, DataDeal_Task, NULL);
	if(ret == ACS_SUCCESS)
	{
		pthread_detach(DataDeal_t);
	}
	else
	{
		ACS_WLOG("Create p_thread DataDeal_Task fail");
		PRT_PRINT(ACS_ERROR,"Create p_thread DataDeal_Task fail");
		return ACS_ERR_FAIL;
	}

	
	printf("ACS Cloud Service_Init version :%s\n",ACSLIB_VERSION);//PRT_PRINT(ACS_ERROR,"ACS Cloud Service_Init version 1:%s;",ACSLIB_VERSION);
	ACS_WLOG("ACS Cloud Service_Init version 1:%s;",ACSLIB_VERSION);
	
	
	usleep(1000);

	return ACS_ERR_SUCCESS;
}

/**
* fun:返回库版本号
* @return:版本号;
**/	
ACS_CHAR *ACS_LibVerSion(ACS_VOID)
{
	return (ACS_CHAR *)ACSLIB_VERSION;
}


/**
* fun:二维码校验接口
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACS_HealthCode_QRCode_Check(ACS_HEALTH_QRCODE_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	return ACSHTTP_HealthQRCode_Check(pPostinfo,pHealthinfo,pMsgId);
}

/**
* fun:人证健康码身份证校验接口
* @pPostinfo-in:请求参数 ACS_HEALTH_IDCARD_S * 身份证信息外部进行加密
* @pHealthinfo-out:解析的健康码信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACS_HealthCode_IDCard_Check(ACS_HEALTH_IDCARD_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId)
{
	return ACSHTTP_HealthCodeIDCard_Check(pPostinfo,pHealthinfo,pMsgId);
}

/**
* fun:上传抓拍图片
* @psnapphotopath:输入图片路径名
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @* @return:ACS_ERR_E;
**/
ACS_INT ACS_SnapPhoto_Upload(ACS_CHAR *psnapphotopath,const ACS_CHAR *pMsgId)
{
	return ACSHTTP_SnapPhoto_Upload(psnapphotopath,pMsgId);
}


/**
* fun:呼叫检验(房间号/手机号/管理处) 注意 pstCallRes->dwDataLen != 0时 pstData 需要释放
* @pstCall-in:请求参数 ACS_DEVICE_CALL_S * 呼叫校验结构体
* @pstCallRes-out:解析的呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Device_CallCheck(ACS_CALL_CHECK_S* pstCall, ACS_CALL_CHECK_RESULT_S* pstCallRes)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	return ACSHTTP_CallCheck(pstCall, pstCallRes, "");
}

/**
* fun:发起呼叫(房间号/手机号/管理处)
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体
* @pstResult-out:解析的呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Webrtc_CallPhone(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	return ACSHTTP_WebrtcCallPhone(pstCall, pstResult, "");
}

/**
* fun:呼叫挂断(房间号/手机号/管理处)
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体
* @pstResult-out:解析的呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Webrtc_CallHangUp(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult)
{
	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	return ACSHTTP_WebrtcHangUp(pstCall, pstResult, "");
}


/**
* fun:呼叫
* @pstPostData-in:呼叫消息
* @pPic-in:抓拍图片
* @pDataOut-Out:呼叫结果
* @return:0-success;<0-fail
**/
ACS_INT ACS_Call(const ACS_CALL_INFO_S *pstPostData,const ACS_CHAR *pPic,ACS_CALL_RESULT_S *pDataOut)
{
	ACS_INT ret = ACS_ERR_FAIL;
	if(ACSCOM_GetClouldEnable() != ACS_ITURN)
	{
		return ACS_ERR_SERVERNOT;
	}
	else if(ACSCOM_Get_ServiceState() != ACS_SERVER_OK)
	{
		return ACS_ERR_SERVERFAIL;
	}
	
	ret = ACSHTTP_Post_Call(pstPostData,pPic,pDataOut);
	
	return ret;
}


const ACSCLOUD_ST_API ACSCLOUD={
	ACS_Call,
	ACS_CallPushState,
	ACS_Device_CallCheck,
	ACS_Webrtc_CallPhone,
	ACS_Webrtc_CallHangUp,
	*ACS_LibVerSion,
	ACS_Get_ServiceState,
	ACS_DataCheckOnline,
	ACS_HealthCode_QRCode_Check,
	ACS_HealthCode_IDCard_Check,
	ACS_OpenDoorRecord_Upload,
	ACS_CallRecord_Upload,
	ACS_SnapPhoto_Upload,
	ACS_Curl_SetCallback_Download,
	ACS_SetLibHandleCmd,
	ACS_SetCallback_HandleCmd,
	ACS_SetServicePlatformInfo,
	ACS_ServiceInit,
};

/**
const ACSCLOUD_ST_API ACSCLOUD={
	ACS_Get_ServiceState,
	ACS_DataCheckOnline,
	ACS_HealthCode_QRCode_Check,
	ACS_HealthCode_IDCard_Check,
	ACS_OpenDoorRecord_Upload,
	ACS_SnapPhoto_Upload,
	ACS_SetCallback_HandleCmd,
	ACS_Curl_SetCallback_Download,
	ACS_SetServicePlatformInfo,
	ACS_ServiceInit,
};
**/
