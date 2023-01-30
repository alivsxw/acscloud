#include <curl/curl.h>
#include "cJSON.h"

#include "acscloudcommon.h"
#include "acscloudhttp.h"
#include "acscloudqueue.h"
#include "acscloud_intellect.h"
#include "acscloudmqtt.h"
#include "acsclouddefine.h"


#define RECONETMAX  3   //http  重连次数

static ACS_BYTE g_byDataNumifPush = ACS_FALSE;
extern ACS_CONFIG_S g_stDoor_Info;
static ACS_SYNC_MARK_S g_stTempSysctime = {0};
static ACS_BYTE g_bAcsState = ACS_FALSE;//通行状态 0-进 1-出 

static ACS_CHAR g_szUrl[128] = {0};
static ACS_CHAR g_bfirst = ACS_ITURN;


ACS_Curl_WriteFuncCallback g_Acs_RomoteUpgrade = NULL;	//远程升级下载升级包处理函数

ACS_VOID ACS_freeJson(ACS_VOID** buffer,const ACS_CHAR *strfunc)
{
	if(*buffer != NULL)
	{
		cJSON_Delete((cJSON *)*buffer);
		*buffer = NULL;
	}
	return ;
}

ACS_VOID ACS_cJSONDelete(cJSON *buffer)
{
	if(buffer != NULL)
	{
		cJSON_Delete(buffer);
		buffer = NULL;
	}
	return ;
}

ACS_INT ACS_WriteMemoryCallback(ACS_VOID *contents, ACS_INT size, ACS_INT nmemb, ACS_VOID *userp)
{
	ACS_INT realsize = size * nmemb;
	struct AcsMemoryStruct *mem = (struct AcsMemoryStruct *)userp;
	ACS_CHAR *ptr = (ACS_CHAR *)realloc(mem->memory, mem->size + realsize + 1);
	if(ptr == NULL) 
	{
		PRT_PRINT(ACS_DEBUG,"acslib not enough memory (realloc returned NULL)");/* out of memory! */ 
		return 0;
	}
	
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

size_t ACS_WritePicFileCallback(ACS_VOID *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}


ACS_VOID ACS_base64_encode(ACS_CHAR *src, ACS_INT src_len, ACS_CHAR *dst, ACS_INT *dst_len)
{
	ACS_INT i = 0, j = 0;
	ACS_CHAR base64_map[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	if(src == NULL || dst == NULL)
		return ;
	for (; i < src_len - src_len % 3; i += 3) 
	{
	    dst[j++] = base64_map[(src[i]  >> 2) & 0x3F];
	    dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
	    dst[j++] = base64_map[((src[i + 1] << 2) & 0x3C) + ((src[i + 2] >> 6) & 0x3)];
	    dst[j++] = base64_map[src[i + 2] & 0x3F];
	}

	if (src_len % 3 == 1) 
	{
	    dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
	    dst[j++] = base64_map[(src[i] << 4) & 0x30];
	    dst[j++] = '=';
	    dst[j++] = '=';
	}
    	else if (src_len % 3 == 2) 
	{
	    dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
	    dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
	    dst[j++] = base64_map[(src[i + 1] << 2) & 0x3C];		
	    dst[j++] = '=';
	}    	
	dst[j] = '\0';
	if (dst_len)
		*dst_len = j;
}






/**
* fun:获取同步平台的时间
* @nOpe-in:输入操作标志 ACS_CD_FACES
* @pstSynctime-in:输入时间
* @return:0-success;-1-fail
**/
ACS_VOID  ACSHTTP_Get_SyncMark(ACS_BYTE byOpe,const ACS_CHAR *pSynctime,const ACS_CHAR *pFeild)
{
	ACS_UINT64 ullTime = atoi(pSynctime);
	switch (byOpe)
	{
		case ACS_CD_FACE:
		{
			g_stTempSysctime.stTime.face = ullTime;
			memcpy(g_stTempSysctime.stField.face,pFeild,128);
			break;
		}
		case ACS_CD_CARD:
		{
			g_stTempSysctime.stTime.card = ullTime;
			memcpy(g_stTempSysctime.stField.card,pFeild,128);
			break;
		}
		case ACS_CD_AD:
		{
			g_stTempSysctime.stTime.ad = ullTime;
			memcpy(g_stTempSysctime.stField.ad,pFeild,128);
			break;
		}
		case ACS_CD_PWD:
		{
			g_stTempSysctime.stTime.pwd = ullTime;
			memcpy(g_stTempSysctime.stField.pwd,pFeild,128);
			break;
		}
		case ACS_CD_REISSUES:
		{
			g_stTempSysctime.stTime.reissue = ullTime;
			memcpy(g_stTempSysctime.stField.reissue,pFeild,128);
			break;
		}
		default:
		{
			break;
		}
	}
	PRT_PRINT(ACS_DEBUG,"byOpe:%d;pstSynctime:%s;pfeild:%s",byOpe,pSynctime,pFeild);
	return;
}


/**
* fun:设置同步平台的时间 写入到文件
* @nOpe-in:输入操作标志 ACS_CD_FACES
* @return:0-success;-1-fail
**/
ACS_VOID ACSHTTP_Set_SyncMark(ACS_BYTE byOpe)
{
	ACS_SYNC_MARK_S stSysctime;
	ACSCOM_Get_SyncTimeCfg(&stSysctime);
	if(stSysctime.logLevel == ACS_FALSE)
		stSysctime.logLevel = ACS_ERROR;
	switch (byOpe)
	{
		case ACS_CD_FACE:
		{
			stSysctime.stTime.face = g_stTempSysctime.stTime.face;
			memcpy(stSysctime.stField.face,g_stTempSysctime.stField.face,128);
			break;
		}
		case ACS_CD_CARD:
		{
			stSysctime.stTime.card = g_stTempSysctime.stTime.card;
			memcpy(stSysctime.stField.card,g_stTempSysctime.stField.card,128);
			break;
		}
		case ACS_CD_AD:
		{
			stSysctime.stTime.ad = g_stTempSysctime.stTime.ad;
			memcpy(stSysctime.stField.ad,g_stTempSysctime.stField.ad,128);
			break;
		}
		case ACS_CD_REISSUES:
		{
			stSysctime.stTime.reissue = g_stTempSysctime.stTime.reissue;
			memcpy(stSysctime.stField.reissue,g_stTempSysctime.stField.reissue,128);
			break;
		}
		case ACS_CD_PWD:
		{
			stSysctime.stTime.pwd = g_stTempSysctime.stTime.pwd;
			memcpy(stSysctime.stField.pwd,g_stTempSysctime.stField.pwd,128);
			break;
		}
		default:
		{
			break;
		}
	}
	ACSCOM_Set_SyncTimeCfg(&stSysctime);
	return;
}


/**
* fun:获取是否推送设备数据库数量标志位 1-推 0-不推
* @nOpe-in:输入操作标志 
* @return:0-success;-1-fail
**/
ACS_BYTE ACSHTTP_Get_DataNumIfPush(ACS_VOID)
{
	return g_byDataNumifPush;
}

/**
* fun:设置是否推送设备数据库数量标志位 1-推 0-不推
* @nOpe-in:输入操作标志 
* @return:0-success;-1-fail
**/
ACS_VOID ACSHTTP_Set_DataNumIfPush(ACS_BYTE byDataNUm)
{
	//if(g_byDataNumifPush != byDataNUm)
		g_byDataNumifPush = byDataNUm;
	return;
}



/**
* fun:获取logver
* @nOpe-in:输入操作标志 ACS_CD_FACES
* @return:0-success;-1-fail
**/
ACS_BYTE ACSHTTP_Get_Logver(ACS_VOID)
{
	//PRT_PRINT("bylogver:%d",g_stDoor_Info.stDevInfo.bylogver);
	return g_stDoor_Info.stDevInfo.byislogver;
}


/**
* fun:获取http Heard
* @ACS_VOID-in:
* @return:ACS_ERR_E
**/
ACS_CHAR *ACSHTTP_GetTokenHeard(ACS_CHAR *pszheader)
{
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);
	snprintf(pszheader,256,"token:%s",stClouldInfo.szToken);
	return NULL;
}

/**
* fun:获取http Heard
* @ACS_VOID-in:
* @return:ACS_ERR_E
**/
ACS_CHAR *ACSHTTP_GetHeard(ACS_CHAR *pszheader)
{
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);
	snprintf(pszheader,512,"header:{\"acslib_ver\":\"%s\",\"pcode\":\"%s\",\"uuid\":\"%s\",\"softversion\":\"%s\",\"devtype\":\"%s\",\"devwiredIP\":\"%s\",\"cpuid\":\"%s\"}"\
	,ACSLIB_VERSION,stClouldInfo.szPcode,stClouldInfo.szUuid,stClouldInfo.szSoftVersion,stClouldInfo.szDevType,stClouldInfo.szDevWiredIP,stClouldInfo.szcpuID);
	return NULL;
}


ACS_INT ACSHTTP_FileToData(ACS_CHAR* buf,const ACS_CHAR *picpath)//获取抓拍图片数据
{
	FILE* fd = NULL;
	ACS_INT nlen = 0;
	ACS_INT nFileLen = 0;
	ACS_INT re = 0;

	if(picpath == NULL)
	{
		return 0;
	}
	PRT_PRINT(ACS_DEBUG,"path = %s",picpath);
	fd = fopen(picpath,"rb");
	if(fd == NULL)
	{
		PRT_PRINT(ACS_ERROR,"fopen fail path = %s",picpath);
		return 0;
	}

	fseek(fd,0,SEEK_END);
	nFileLen = ftell(fd);
	fseek(fd,0,SEEK_SET);
	
	if(nFileLen >= ACSPICMAXLEND)
	{
		ACS_WLOG("ACS_FileToData nFileLen[%d] > [%d]",nFileLen,ACSPICMAXLEND);
		PRT_PRINT(ACS_ERROR,"ACS_FileToData nFileLen[%d] > [%d]",nFileLen,ACSPICMAXLEND);
		fclose(fd);
		fd = NULL;
		return -1;
	}

	while((re = fread(&buf[nlen],1,10240,fd)) > 0)
	{
		nlen += re;
	}
	PRT_PRINT(ACS_DEBUG,"read len:%d, seek len:%d", nlen, nFileLen);
		
	if(nlen >= ACSPICMAXLEND)
	{
		ACS_WLOG("ACS_FileToData nlen[%d] > [%d]",nlen,ACSPICMAXLEND);
		PRT_PRINT(ACS_ERROR,"ACS_FileToData nlen[%d] > [%d]",nlen,ACSPICMAXLEND);
		fclose(fd);
		fd = NULL;
		return -1;
	}
	
	fclose(fd);
	fd = NULL;
	return nlen;
}


/**
* fun:post数据回调     人脸结果 门卡结果 广告结果 设备库数据结果 错误信息上传
* @CallData-in:服务器返回的结果数据
* @CallDataOut-Out:输出服务器返回的结果数据 
* @return:ACS_ERR_E 0-success; ACS_ERR_SUCCESS  ACS_ERR_DATANULL ACS_ERR_DATAMAX ACS_ERR_DATAERR
**/
ACS_INT  Curl_DealCommonPost_Back(ACS_CHAR *CallData,ACS_CHAR *pCallDataOut)
{
	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);

	ACS_INT ret = ACS_ERR_SUCCESS;
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_ERROR,"CommonPost CallData null");
		ACS_WLOG("CommonPost CallData NULL");
		return ACS_ERR_DATANULL;
	}

	if(strstr(CallData,"502 Bad Gateway") != NULL)
	{
		ACS_WLOG("CommonPost BackData 502 Bad Gateway");
	}
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		ACS_WLOG("resurlt pRecvJsonRoot NULL");
		return ACS_ERR_DATANULL;	
	}

	cJSON* pstatus = cJSON_GetObjectItem(pRecvJsonRoot,"status");//异常会收到这个数据
	if((pstatus)&&(pstatus->type == cJSON_Number))
	{
		ret = pstatus->valueint;
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		ACS_WLOG("CommonPost BackData status:%d",ret);
		return ACS_ERR_DATANULL;
	}

	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(pcode == NULL)//(pcode->valueint != ACS_ERR_SUCCESS)
	{
		PRT_PRINT(ACS_ERROR,"CommonPost retCode NULL");
		ACS_WLOG("CommonPost retCode NULL");
		ret =  ACS_ERR_DATAERR;
	}
	
	if((pcode) && (pcode->valueint != ACS_JSON_CODE_SUCCESS))
	{
		//ret = pcode->valueint;
		//ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		ACS_WLOG("CommonPost BackData retCode:%d",pcode->valueint);
		//return	ret;
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ret;
}



/**
* fun:GET数据回调 设备登录数据回调
* @CallData-in:设备登入服务器返回的结果数据
* @return:ACS_ERR_SUCCESS-success;>0-Fail ACS_ERR_DATAERR ACS_ERR_DATANULL
**/
ACS_INT Curl_DealLogin_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	static ACS_BYTE sbytime = 0;
	ACS_INT ret = ACS_ERR_SUCCESS;
	ASC_DEVALLCFG_S  stDevAll = {0};
	ACS_CONFIG_S stDoorInfo = {0};
	ACSCOM_Get_Config(&stDoorInfo);
	
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib CallData NULL");
		return ACS_ERR_DATAERR;
	}

	PRT_PRINT(ACS_DEBUG,"Login BackData json:%s",CallData);
	sbytime ++;
	if(sbytime < 3)
	{
		ACS_WLOG("login_callback:%s",CallData);//nLen = strlen(CallData);
	}
	
	cJSON* pRecvJsonRoot = cJSON_Parse(CallData);
	if(pRecvJsonRoot)
	{
		cJSON* pretCode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
		if(pretCode && (pretCode->type == cJSON_Number))
		{
			if(pretCode->valueint != ACS_JSON_CODE_SUCCESS)
			{
				cJSON* pmsg = cJSON_GetObjectItem(pRecvJsonRoot,"message");
				if(pmsg && (pmsg->valuestring))
				{
					if(sbytime < 3)
						ACS_WLOG("code:%d,msg=%s",pretCode->valueint,pmsg->valuestring);
				}
				
				if(ACS_ERRSER_NOTREGISTER == pretCode->valueint)
				{
					ret = ACS_ERR_DEVNOTREGISTER;
					ACS_cJSONDelete(pRecvJsonRoot);
					return ret;
				}
			}
			else
			{
				sbytime = 0;
			}
		}		
		
		cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
		if(pdata)//&&(pdata->valuestring != NULL)
		{
			cJSON* paddress = cJSON_GetObjectItem(pdata,"address"); 	
			if(paddress && paddress->valuestring)
			{
				snprintf(stDoorInfo.stDevInfo.szAddress,sizeof(stDoorInfo.stDevInfo.szAddress),"%s",paddress->valuestring);
			}

			cJSON* pDeviceName = cJSON_GetObjectItem(pdata,"DeviceName"); 	
			if(pDeviceName && pDeviceName->valuestring)
			{
				snprintf(stDoorInfo.stDevInfo.szDeviceName,sizeof(stDoorInfo.stDevInfo.szDeviceName),"%s",pDeviceName->valuestring);
			}

			cJSON* pmqtt = cJSON_GetObjectItem(pdata,"mqtt");	
			if(pmqtt)
			{
				cJSON* pmqttip = cJSON_GetObjectItem(pmqtt,"ip");	
				if(pmqttip && pmqttip->valuestring)
				{
					snprintf(stDoorInfo.stMqttConfig.szIP,sizeof(stDoorInfo.stMqttConfig.szIP), pmqttip->valuestring); 
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","ip");
				}
				
				cJSON* pmqttport = cJSON_GetObjectItem(pmqtt,"port");	
				if(pmqttport && pmqttport->valuestring)
				{
					stDoorInfo.stMqttConfig.nPort = atoi(pmqttport->valuestring);
				}
				else if(pmqttport && (pmqttport->type == cJSON_Number))
				{
					stDoorInfo.stMqttConfig.nPort = pmqttport->valueint;
				}
				else
				{
					stDoorInfo.stMqttConfig.nPort = 1883;
				}
				
				cJSON* pmqttusername = cJSON_GetObjectItem(pmqtt,"username");	
				if(pmqttusername && pmqttusername->valuestring)
				{
					snprintf(stDoorInfo.stMqttConfig.szUsername, sizeof(stDoorInfo.stMqttConfig.szUsername),pmqttusername->valuestring);
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","username");
				}
				
				cJSON* pmqtt_pwd = cJSON_GetObjectItem(pmqtt,"mqtt_pwd");	
				if(pmqtt_pwd && pmqtt_pwd->valuestring)
				{				
					snprintf(stDoorInfo.stMqttConfig.szMqttPwd, sizeof(stDoorInfo.stMqttConfig.szMqttPwd), pmqtt_pwd->valuestring);
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","mqtt_pwd");
				}

				cJSON* pmqttclient_id = cJSON_GetObjectItem(pmqtt,"client_id");	
				if(pmqttclient_id && pmqttclient_id->valuestring)
				{
					snprintf(stDoorInfo.stMqttConfig.szClientId, sizeof(stDoorInfo.stMqttConfig.szClientId), pmqttclient_id->valuestring);
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","client_id");
				}
				
				cJSON* pmqttclient_keepalive = cJSON_GetObjectItem(pmqtt,"keeplive");	
				if(pmqttclient_keepalive && pmqttclient_keepalive->type == cJSON_Number)
				{
					stDoorInfo.stMqttConfig.keepalive = pmqttclient_keepalive->valueint;
				}
				else
				{
					stDoorInfo.stMqttConfig.keepalive = 60;
				}

				cJSON* pQos = cJSON_GetObjectItem(pmqtt,"qos");		
				if(pQos && pQos->type == cJSON_Number)
				{
					stDoorInfo.stMqttConfig.byQos = pQos->valueint;		
				}
				else
				{
					stDoorInfo.stMqttConfig.byQos = 2;
				}

				cJSON* pbyIfClientId = cJSON_GetObjectItem(pmqtt,"ifclient_id");		
				if(pbyIfClientId && pbyIfClientId->type == cJSON_Number)
				{
					stDoorInfo.stMqttConfig.byIfClientId = pbyIfClientId->valueint;		
				}
				
				/**
				cJSON* psubopicm_id = cJSON_GetObjectItem(pmqtt,"subopicm_id");		
				if(psubopicm_id && psubopicm_id->type == cJSON_Number)
				{
					stDoorInfo.stMqttConfig.subopicm_id = psubopicm_id->valueint;		
				}
				**/

				cJSON* psubopic = cJSON_GetObjectItem(pmqtt,"subscribe_topic");
				if(psubopic && psubopic->valuestring)
				{
					snprintf(stDoorInfo.stMqttConfig.szSubopicTopic, sizeof(stDoorInfo.stMqttConfig.szSubopicTopic), psubopic->valuestring);
				}
				else
				{
					snprintf(stDoorInfo.stMqttConfig.szSubopicTopic,sizeof(stDoorInfo.stMqttConfig.szSubopicTopic),"deviceMqtt/%s",stDoorInfo.stMqttConfig.szClientId);
				}
				
				cJSON* ppublish = cJSON_GetObjectItem(pmqtt,"publish_topic");
				if(ppublish && ppublish->valuestring)
				{
					snprintf(stDoorInfo.stMqttConfig.szPublishTopic, sizeof(stDoorInfo.stMqttConfig.szPublishTopic), ppublish->valuestring);
				}
				else
				{
					memcpy(stDoorInfo.stMqttConfig.szPublishTopic,ACSMQTT_PUBLISHTOPIC,strlen(ACSMQTT_PUBLISHTOPIC));
				}
			}
			else
			{
				ret = ACS_ERR_DATAERR;
				sprintf(pstMsgOut->szDetails,"%s","mqtt");
			}
			
			cJSON* pisFirst = cJSON_GetObjectItem(pdata,"isFirst");		
			if(pisFirst && pisFirst->type == cJSON_Number)
			{
				stDoorInfo.stDevInfo.byisFirst = pisFirst->valueint;			
			}

			cJSON* plogver = cJSON_GetObjectItem(pdata,"logver");		
			if(plogver && plogver->type == cJSON_Number)
			{
				stDoorInfo.stDevInfo.byislogver = plogver->valueint;		
			}

/**
			cJSON* phttp = cJSON_GetObjectItem(pdata,"http");	
			if((phttp) && (phttp->type == cJSON_Object))
			{
				cJSON* phttpGetConnectTime = cJSON_GetObjectItem(phttp,"httpGetConnectTime");		
				if(phttpGetConnectTime && phttpGetConnectTime->type == cJSON_Number)
				{
					g_bHttpConnectTime = phttpGetConnectTime->valueint;		
				}
				else
				{
					g_bHttpConnectTime = 5;
				}
			}
**/
			cJSON* pid = cJSON_GetObjectItem(pdata,"id");		
			if(pid && pid->valuestring)
			{
				snprintf(pstMsgOut->szID, sizeof(pstMsgOut->szID), pid->valuestring);
			}
			
			cJSON *psip = cJSON_GetObjectItem(pdata, "sip");
			if(psip) 
			{
				cJSON* pserver_addrs = cJSON_GetObjectItem(psip, "server_addrs");
				if (pserver_addrs && pserver_addrs->valuestring)
				{
					snprintf(stDevAll.stSip.cAddrs, sizeof(stDevAll.stSip.cAddrs), pserver_addrs->valuestring);
				}

				cJSON* paccount = cJSON_GetObjectItem(psip, "account");
				if (paccount && paccount->valuestring)
				{
					snprintf(stDevAll.stSip.cAccount, sizeof(stDevAll.stSip.cAccount), paccount->valuestring);
				}

				cJSON* ppassword = cJSON_GetObjectItem(psip, "password");
				if (ppassword && ppassword->valuestring)
				{
					snprintf(stDevAll.stSip.cPassword, sizeof(stDevAll.stSip.cPassword), ppassword->valuestring);
				}
				
				cJSON* pserver_port = cJSON_GetObjectItem(psip,"server_port");		
				if(pserver_port && pserver_port->type == cJSON_Number)
				{
					stDevAll.stSip.nPort = pserver_port->valueint;			
				}
			}
			
			cJSON* pwebrtObj = cJSON_GetObjectItem(pdata, "webrtc");
			if(pwebrtObj) 
			{
				cJSON* pwebrtcaddrs = cJSON_GetObjectItem(pwebrtObj, "media_addrs");
				if (pwebrtcaddrs && pwebrtcaddrs->valuestring)
				{
					snprintf(stDevAll.stWebRtc.cAddrs, sizeof(stDevAll.stWebRtc.cAddrs), pwebrtcaddrs->valuestring);
				}

				cJSON* pwebrtlics = cJSON_GetObjectItem(pwebrtObj, "media_license");
				if (pwebrtlics && pwebrtlics->valuestring)
				{
					snprintf(stDevAll.stWebRtc.cLicense, sizeof(stDevAll.stWebRtc.cLicense), pwebrtlics->valuestring);
				}

				cJSON* pwebrtinitkey = cJSON_GetObjectItem(pwebrtObj, "media_initkey");
				if (pwebrtinitkey && pwebrtinitkey->valuestring)
				{
					snprintf(stDevAll.stWebRtc.cKey, sizeof(stDevAll.stWebRtc.cKey), pwebrtinitkey->valuestring);
				}
			}


			cJSON* prongcloud = cJSON_GetObjectItem(pdata, "rongcloud");
			if(prongcloud) 
			{
				cJSON* pappkey = cJSON_GetObjectItem(prongcloud, "appkey");
				if (pappkey && pappkey->valuestring)
				{
					snprintf(stDevAll.stRongCloud.cAppkey, sizeof(stDevAll.stRongCloud.cAppkey), pappkey->valuestring);
				}
				cJSON* ptoken = cJSON_GetObjectItem(prongcloud, "token");
				if (ptoken && ptoken->valuestring)
				{
					snprintf(stDevAll.stRongCloud.cToken, sizeof(stDevAll.stRongCloud.cToken), ptoken->valuestring);
				}
			}


		//	stDevAll.stCallRule.bOwner[0] = ACSCRULE_OWNERAPP;
		//	stDevAll.stCallRule.bProperty[0] = ACSCRULE_PROPERTYAPP;
		//	stDevAll.stCallRule.bOwner[1] = ACSCRULE_INDOOR;
		//	stDevAll.stCallRule.bProperty[1] = ACSCRULE_MANAGE;

			cJSON* prule = cJSON_GetObjectItem(pdata, "callrule");
			if(prule) 
			{
				cJSON* powner = cJSON_GetObjectItem(prule,"owner"); 
				if(powner && powner->type==cJSON_Array)
				{
					ACS_INT nowners = cJSON_GetArraySize(powner);
					if(nowners < 4)
					{
						for(ACS_INT e = 0;e<nowners;e++)
						{
							cJSON *plevel = cJSON_GetArrayItem(powner,e);
							if((plevel) && (plevel->type == cJSON_Number))
							{
								stDevAll.stCallRule.bOwner[e] = plevel->valueint;
								PRT_PRINT(ACS_DEBUG,"callrule e[%d].bOwner:%d;nlevel:%d",e,stDevAll.stCallRule.bOwner[e], plevel->valueint);
							}
						}
					}
				}
				
				cJSON* pproperty = cJSON_GetObjectItem(prule,"property"); 
				if(pproperty && pproperty->type==cJSON_Array)
				{
					ACS_INT npropertys = cJSON_GetArraySize(pproperty);
					if(npropertys < 4)
					{
						for(ACS_INT e = 0;e<npropertys;e++)
						{
							cJSON *plevel2 = cJSON_GetArrayItem(pproperty,e);
							if((plevel2) && (plevel2->type == cJSON_Number))
							{
								stDevAll.stCallRule.bProperty[e] = plevel2->valueint;
								PRT_PRINT(ACS_DEBUG,"callrule e[%d].bProperty:%d;nlevel:%d",e,stDevAll.stCallRule.bProperty[e], plevel2->valueint);
							}
						}
					}
				}
			}

			cJSON* pmjtoken = cJSON_GetObjectItem(pdata,"token");	
			if(pmjtoken && pmjtoken->valuestring)
			{
				snprintf(stDoorInfo.stClouldInfo.szToken, sizeof(stDoorInfo.stClouldInfo.szToken), pmjtoken->valuestring);//不能用memcpy 字符超出会有回车字符
			}
			
			if(memcmp(&g_stDoor_Info,&stDoorInfo,sizeof(ACS_CONFIG_S)) != 0)
			{
				memset(&g_stDoor_Info,0,sizeof(ACS_CONFIG_S));
				memcpy(&g_stDoor_Info, &stDoorInfo, sizeof(ACS_CONFIG_S));
				memcpy(&stDevAll.stDevInfo, &stDoorInfo.stDevInfo, sizeof(ACS_DEVCONFIG_S));
			}
			ACSCOM_HandleCmdCallback(ACS_CMD_SET_UPPARAM,(ACS_CHAR *)&stDevAll);
		}
		else
		{
			ret = ACS_ERR_DATAERR;
			sprintf(pstMsgOut->szDetails,"%s","data");
		}
		
		ACS_cJSONDelete(pRecvJsonRoot);//ACSCOM_Set_MQTTCfg(&stDoorInfo);
		
	}
	else
	{
		ret = ACS_ERR_DATANULL;
	}

	return ret;
}



/**
* fun:GET数据回调 解析服务器返回的同步广告信息发给设备
* @CallData-in:服务器返回的同步广告信息
* @return:0-success;
**/
ACS_INT Curl_DealSyncAd_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_ERROR,"CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if(pCallOut == NULL)
	{
		PRT_PRINT(ACS_ERROR,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_ERROR,"pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}

	ACS_INT i = 0;
	ACS_INT ret = 0;
	ACS_INT nAdsNum = 0;
	ACS_CHAR sztime[16] = {0};
	ACS_CHAR eField[128] = {0};
	ACS_ADVERT_INFO_S *pstAd = NULL;
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_ERROR,"Card pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}
		
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)
	{
		PRT_PRINT(ACS_ERROR,"not retCode");
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}
	
	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
		if((peField) && (peField->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
			ACS_WLOG("syncAd BackData peField = %s" ,peField->valuestring);
			snprintf(eField,128,peField->valuestring);
		}
			
		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if((ptimestamp) && (ptimestamp->valuestring))
		{
			PRT_PRINT(ACS_INFO,"pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACS_WLOG("syncad BackData pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACSHTTP_Get_SyncMark(ACS_CD_AD,ptimestamp->valuestring,eField);
		}
		else
		{
			sprintf(sztime,"%ld",time(NULL));
			ACSHTTP_Get_SyncMark(ACS_CD_AD,sztime,eField);
		}
		
		cJSON* pAds = cJSON_GetObjectItem(pdata, "ads");
		if((pAds) && (pAds->type == cJSON_Array))
		{
			nAdsNum = cJSON_GetArraySize(pAds);			
			PRT_PRINT(ACS_INFO,"syncAds nAdsNum:%d",nAdsNum);
			ACS_WLOG("SyncAds nAdsNum:%d;nTotal:%d",nAdsNum,pstCallDataOut->nTotal);
			if(10 < nAdsNum)
			{
				nAdsNum = 10;
				//ret = ACS_ERR_DATAERR;
			}
			pstCallDataOut->nPageSize = nAdsNum;
			pstCallDataOut->nTotal = nAdsNum;
			
			pstAd = (ACS_ADVERT_INFO_S *)malloc(sizeof(ACS_ADVERT_INFO_S)*nAdsNum);
			if(pstAd == NULL)
			{
				ACS_WLOG("pSyncAdData malloc %d error",sizeof(ACS_ADVERT_INFO_S)*nAdsNum);
				PRT_PRINT(ACS_ERROR,"pSyncAdData malloc %d error",sizeof(ACS_ADVERT_INFO_S)*nAdsNum);
				pstCallDataOut->nPageSize = -1;				
			}
			
			if(pstAd)
			{
				pstCallDataOut->pRecord = (ACS_CHAR *)pstAd;
				pstCallDataOut->nPageSize = nAdsNum;
				PRT_PRINT(ACS_DEBUG,"pSyncAdData malloc %p;pstAd:%p",pstCallDataOut->pRecord,pstAd);			
				memset(pstAd,0,(sizeof(ACS_ADVERT_INFO_S)*nAdsNum));
				for(i=0; i<nAdsNum; i++)
				{
					cJSON* pAd = cJSON_GetArrayItem(pAds, i);
					if(pAd)
					{
						cJSON* poperator = cJSON_GetObjectItem(pAd, "operation");
						if((poperator)&&(poperator->valuestring))
						{
							snprintf(pstAd[i].operation,sizeof(pstAd[i].operation),poperator->valuestring);//memcpy(pstAd[i].operation, poperator->valuestring, sizeof(pstAd[i].operation));
							PRT_PRINT(ACS_DEBUGX,"stAdCfg.operation=%s",pstAd[i].operation);
						}
						else
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","operation");
							break;
						}

						cJSON* padUrl = cJSON_GetObjectItem(pAd, "adImageUrl");
						if((padUrl) && (padUrl->valuestring))
						{
							snprintf(pstAd[i].adurl,sizeof(pstAd[i].adurl),padUrl->valuestring);//memcpy(pstAd[i].adurl, padUrl->valuestring, sizeof(pstAd[i].adurl));
							/**ret = ACSHTTP_GetImageSuffix(pstAd[i].adurl,szSuffix);
							if(ACS_ERR_SUCCESS != ret)
							{
								printf("GetTheImageSuffix ret[%d] Error!!![%s:%d]\n",ret,__func__,__LINE__);
							}**/
							PRT_PRINT(ACS_DEBUGX,"stAdCfg.adurl=%s",pstAd[i].adurl);
							ACS_WLOG("Ad adurl:%s",pstAd[i].adurl);
						}
						else if(strcmp(pstAd[i].operation,ACSDB_SAVE) == 0)
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","adImageUrl");
							break;
						}

						cJSON* pid = cJSON_GetObjectItem(pAd, "id");
						if(pid && pid->valuestring)
						{
							snprintf(pstAd[i].ID,sizeof(pstAd[i].ID),pid->valuestring);//memcpy(pstAd[i].ID, pid->valuestring, sizeof(pstAd[i].ID));
							PRT_PRINT(ACS_DEBUGX,"stAdCfg.ID=%s",pstAd[i].ID);
							sprintf(pstMsgOut->szID,"%s",pid->valuestring);
						}

						cJSON* padId = cJSON_GetObjectItem(pAd, "adId");
						if(padId && padId->valuestring)
						{
							snprintf(pstAd[i].adId,sizeof(pstAd[i].adId),padId->valuestring);//memcpy(pstAd[i].adId, padId->valuestring, sizeof(pstAd[i].adId));
							PRT_PRINT(ACS_DEBUGX,"stAdCfg.adId=%s", pstAd[i].adId);
						}
						else if(strcmp(pstAd[i].operation,ACSDB_DELETE) != 0)
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","adId");
							break;
						}

						cJSON* padBegin = cJSON_GetObjectItem(pAd, "adBegin");
						if((padBegin) && (padBegin->type == cJSON_Number))
						{
							pstAd[i].adBegin = padBegin->valueint;
							PRT_PRINT(ACS_DEBUGX,"Ad adBegin:%d;%lf",padBegin->valueint,padBegin->valuedouble);
						}
						else
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","adBegin");
							break;
						}

						cJSON* padEnd = cJSON_GetObjectItem(pAd, "adEnd");
						if((padEnd) && (padEnd->type == cJSON_Number))
						{
							pstAd[i].adEnd = padEnd->valueint;
							PRT_PRINT(ACS_DEBUGX,"Ad adEnd:%d;%lf",padEnd->valueint,padEnd->valuedouble);
						}
						else
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","adEnd");
							break;
						}

						cJSON* pspeed = cJSON_GetObjectItem(pAd, "speed");
						if((pspeed) && (pspeed->type == cJSON_Number))
						{
							pstAd[i].speed = pspeed->valueint;
							PRT_PRINT(ACS_DEBUGX,"Ad speed:%d",pspeed->valueint);
						}
						else
						{
							pstAd[i].speed = 5;
						}

						cJSON* pname = cJSON_GetObjectItem(pAd, "name");
						if((pname) && (pname->valuestring))
						{
							snprintf(pstAd[i].adPicName,sizeof(pstAd[i].adPicName),pname->valuestring);
							PRT_PRINT(ACS_DEBUGX,"Ad name:%s",pname->valuestring);
						}

						cJSON* pformat = cJSON_GetObjectItem(pAd, "format");
						if((pformat) && (pformat->valuestring))
						{
							snprintf(pstAd[i].format,sizeof(pstAd[i].format),pformat->valuestring);
							PRT_PRINT(ACS_DEBUGX,"Ad format:%s",pformat->valuestring);
						}
						else
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","format");
							break;
						}

						cJSON* plevel = cJSON_GetObjectItem(pAd, "level");
						if((plevel) && (plevel->type == cJSON_Number))
						{
							pstAd[i].level = plevel->valueint;
							PRT_PRINT(ACS_DEBUGX,"Ad level:%d",plevel->valueint);
						}

						cJSON* ptype = cJSON_GetObjectItem(pAd, "type");
						if((ptype) && (ptype->type == cJSON_Number))
						{
							pstAd[i].type = ptype->valueint;
							PRT_PRINT(ACS_DEBUGX,"Ad type:%d",ptype->valueint);
						}
					}
					
					//平台下发停止入库
					if((Mqtt_Get_DataDealFlag() & ACS_CD_AD) != ACS_CD_AD)
					{
						//pstCallDataOut->nPageSize = -1;
						PRT_PRINT(ACS_INFO,"syncAds stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ACS_WLOG("syncAds stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ret = ACS_ERR_NODATA;
						break;
					}
				}
			}
			else
			{
				ret = ACS_ERR_MALLOCFAIL;
			}
		}
		else
		{
			ret = ACS_ERR_DATAERR;
			sprintf(pstMsgOut->szDetails,"%s","ads");
		}
	}
	else
	{
		ret = ACS_ERR_DATAERR;
		sprintf(pstMsgOut->szDetails,"%s","data");
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ret;
}

#if 0
/**
* fun:GET数据回调 同步人员数据信息数据回调
* @CallData-in:服务器返回的结果数据
* @CallDataOut-Out:输出服务器返回的结果数据 ACS_RECORD_S*
* @return:0-success;<0-Fail -1:回调的json为空;-2:传入的结构体为空
**/
ACS_INT  Curl_DealSyncCard_Back(ACS_CHAR *CallData,ACS_CHAR *pCallDataOut,ACS_MESSAGE_S *pstMsgOut)
{
	ACS_INT nCardsNum = 0,ret = 0;
	ACS_INT i = 0;
	ACS_CHAR eField[128] = {0};
	ACS_CHAR sztime[16] = {0};
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if(pCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallDataOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}

	
	ACS_CARD_INFO_S *pstCard = (ACS_CARD_INFO_S *)pstCallDataOut->pRecord;
	if(pstCard == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstCard null");
		return ACS_ERR_PARAMNULL;
	}
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"MjAcsCloud Card pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}

	//memset(&stSysctime,0,sizeof(ACS_SYNCTIME_CONFIG_S));
	//ACSCOM_Get_SyncTimeCfg(&stSysctime);
		
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)// || (ACS_ERR_SUCCESS != pcode->valueint))
	{
		//ret = pcode->valueint;
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
		if((peField) && (peField->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
			ACS_WLOG("syncCard BackData peField = %s" ,peField->valuestring);
			snprintf(eField,128,peField->valuestring);
		}
			
		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if (ptimestamp && ptimestamp->valuestring)
		{
			PRT_PRINT(ACS_DEBUG,"pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACS_WLOG("syncCard BackData pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACSHTTP_Get_SyncMark(ACS_CD_CARD,ptimestamp->valuestring,eField);
		}
		else
		{
			sprintf(sztime,"%ld",time(NULL));
			ACSHTTP_Get_SyncMark(ACS_CD_CARD,sztime,eField);
		}
		
		cJSON* ptotal = cJSON_GetObjectItem(pdata,"total");
		if (ptotal && ptotal->type == cJSON_Number)
		{
			pstCallDataOut->nTotal = ptotal->valueint;
			PRT_PRINT(ACS_DEBUG,"syncCard nTotal:%d",pstCallDataOut->nTotal);
		}
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","total");
			ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
			return ACS_ERR_DATAERR;
		}
		
		cJSON* pcards = cJSON_GetObjectItem(pdata,"cards"); 
		if(pcards && pcards->type == cJSON_Array)
		{
			nCardsNum = cJSON_GetArraySize(pcards);
			PRT_PRINT(ACS_DEBUG,"syncCard nCardNum:%d",nCardsNum);
			ACS_WLOG("SyncCards: nCardNum:%d;nTotal:%d",nCardsNum,pstCallDataOut->nTotal);
			if(ACSSYNCNUM < nCardsNum)
				nCardsNum = ACSSYNCNUM;
			
			for(i = 0;i<nCardsNum;i++)
			{
				cJSON* ppcards  =  cJSON_GetArrayItem(pcards,i);
				if(ppcards != NULL)
				{		
					cJSON* pID = cJSON_GetObjectItem(ppcards, "id");
					if(pID && pID->valuestring)
					{
						snprintf(pstCard[i].ID,sizeof(pstCard[i].ID),pID->valuestring);//memcpy(pstCard[i].ID, pID->valuestring, sizeof(pstCard[i].ID));
					}
					
					cJSON* puserId = cJSON_GetObjectItem(ppcards,"userId");
					if (puserId && puserId->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"puserId->valuestring = %s" ,puserId->valuestring);
						snprintf(pstCard[i].userId,sizeof(pstCard[i].userId),puserId->valuestring);//memcpy(pstCard[i].userId, puserId->valuestring, sizeof(pstCard[i].userId));
					}
					else
					{
						sprintf(pstMsgOut->szDetails,"%s","userId");
						ret = ACS_ERR_DATAERR;
						break;
					}

					cJSON* ptype = cJSON_GetObjectItem(ppcards,"type");
					if (NULL != ptype && ptype->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"ptype->valuestring = %s" ,ptype->valuestring);
						snprintf(pstCard[i].type,sizeof(pstCard[i].type),ptype->valuestring);//memcpy(pstCard[i].type, ptype->valuestring, sizeof(pstCard[i].type));
					}
					else
					{
						memcpy(pstCard[i].type, "CARD", sizeof(pstCard[i].type));
					}

					cJSON* pcard = cJSON_GetObjectItem(ppcards,"card");
					if (NULL != pcard && pcard->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"pcard->valuestring = %s" ,pcard->valuestring);
						snprintf(pstCard[i].card,sizeof(pstCard[i].card),pcard->valuestring);
					}
					else
					{
						sprintf(pstMsgOut->szDetails,"%s","card");
						ret = ACS_ERR_DATAERR;
						break;
					}

					cJSON* pexpired = cJSON_GetObjectItem(ppcards,"expired");
					if(NULL != pexpired && pexpired->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"pexpired->valuestring = %s" ,pexpired->valuestring);
						snprintf(pstCard[i].expired,sizeof(pstCard[i].expired),pexpired->valuestring);						
					}
					else
					{
						memcpy(pstCard[i].expired, "2147483647", sizeof(pstCard[i].expired));
					}

					cJSON* poperation = cJSON_GetObjectItem(ppcards,"operation");
					if (NULL != poperation && poperation->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"poperation->valuestring = %s" ,poperation->valuestring);
						snprintf(pstCard[i].operation,sizeof(pstCard[i].operation),poperation->valuestring);//memcpy(pstCard[i].operation, poperation->valuestring, sizeof(pstCard[i].operation));						
					}
					else
					{
						sprintf(pstMsgOut->szDetails,"%s","operation");
						ret = ACS_ERR_DATAERR;
						break;
					}

					ACS_WLOG("SyncCards:operator:%s, userId:%s, card:%s, expired:%s", \
					pstCard[i].operation, pstCard[i].userId, pstCard[i].card, pstCard[i].expired);
				}

				//平台下发停止入库
				if((Mqtt_Get_DataDealFlag() & ACS_CD_CARD) != ACS_CD_CARD)
				{
					pstCallDataOut->nPageSize = -1;
					PRT_PRINT(ACS_DEBUG,"synccard stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
					ACS_WLOG("synccard stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
					break;
				}
			}
		}	
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","cards");
			ret = ACS_ERR_DATAERR;
		}
	}

	if((ACS_ERR_SUCCESS == ret)&&(-1 != pstCallDataOut->nPageSize))
		pstCallDataOut->nPageSize = nCardsNum;
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ret;
}
#endif

/**
* fun:GET数据回调 同步人员数据信息数据回调
* @CallData-in:服务器返回的结果数据
* @CallDataOut-Out:输出服务器返回的结果数据 ACS_RECORD_S*
* @return:0-success;<0-Fail -1:回调的json为空;-2:传入的结构体为空
**/
ACS_INT  Curl_DealSyncCard_Back(ACS_CHAR *CallData,ACS_CHAR *pCallDataOut,ACS_MESSAGE_S *pstMsgOut)
{
	ACS_INT nCardsNum = 0,ret = 0;
	ACS_INT i = 0;
	ACS_INT nMallocLen = 0;
	ACS_CHAR eField[128] = {0};
	ACS_CHAR sztime[16] = {0};
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if(pCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallDataOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);

	
	ACS_CARD_INFO_S *pstCard = NULL;//(ACS_CARD_INFO_S *)pstCallDataOut->pRecord;

	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"MjAcsCloud Card pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}

	//memset(&stSysctime,0,sizeof(ACS_SYNCTIME_CONFIG_S));
	//ACSCOM_Get_SyncTimeCfg(&stSysctime);
		
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)// || (ACS_ERR_SUCCESS != pcode->valueint))
	{
		//ret = pcode->valueint;
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}

	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
		if((peField) && (peField->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
			ACS_WLOG("syncCard BackData peField = %s" ,peField->valuestring);
			snprintf(eField,128,peField->valuestring);
		}
			
		cJSON* ptotal = cJSON_GetObjectItem(pdata,"total");
		if(ptotal && ptotal->type == cJSON_Number)
		{
			pstCallDataOut->nTotal = ptotal->valueint;
			PRT_PRINT(ACS_INFO,"syncCard nTotal:%d",pstCallDataOut->nTotal);
		}
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","total");
			ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
			return ACS_ERR_DATAERR;
		}

		

		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if (ptimestamp && ptimestamp->valuestring)
		{
			PRT_PRINT(ACS_DEBUG,"pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACS_WLOG("syncCard BackData pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACSHTTP_Get_SyncMark(ACS_CD_CARD,ptimestamp->valuestring,eField);
		}
		else
		{
			sprintf(sztime,"%ld",time(NULL));
			ACSHTTP_Get_SyncMark(ACS_CD_CARD,sztime,eField);
		}

		
		cJSON* pcards = cJSON_GetObjectItem(pdata,"cards"); 
		if(pcards && pcards->type == cJSON_Array)
		{
			nCardsNum = cJSON_GetArraySize(pcards);
			PRT_PRINT(ACS_DEBUG,"syncCard nCardNum:%d",nCardsNum);
			ACS_WLOG("SyncCards: nCardNum:%d;nTotal:%d",nCardsNum,pstCallDataOut->nTotal);
			if(ACSSYNCNUM < nCardsNum)
				nCardsNum = ACSSYNCNUM;
			
			if(nCardsNum > 0)
			{
				nMallocLen = sizeof(ACS_CARD_INFO_S)*nCardsNum;
				PRT_PRINT(ACS_DEBUG,"syncCard syncCard:%d;nMallocLen:%d",nCardsNum,nMallocLen);
				ACS_WLOG("syncCard: nCardsNum:%d;nMallocLen:%d;nTotal:%d",nCardsNum,nMallocLen,pstCallDataOut->nTotal);
				pstCard = (ACS_CARD_INFO_S *)malloc(nMallocLen);
				PRT_PRINT(ACS_DEBUG,"syncCard malloc psteRules:%p",pstCard);
			}
			
			if(pstCard)
			{
				pstCallDataOut->nPageSize = nCardsNum;
				memset(pstCard,0,(nMallocLen));
				pstCallDataOut->pRecord = (ACS_CHAR *)pstCard;
				PRT_PRINT(ACS_DEBUG,"syncCard nPageSize:%d;pstCard:%p,%p",pstCallDataOut->nPageSize,pstCard,pstCallDataOut->pRecord);
				
				for(i = 0;i<nCardsNum;i++)
				{
					cJSON* ppcards  =  cJSON_GetArrayItem(pcards,i);
					if(ppcards != NULL)
					{		
						cJSON* pID = cJSON_GetObjectItem(ppcards, "id");
						if(pID && pID->valuestring)
						{
							snprintf(pstCard[i].ID,sizeof(pstCard[i].ID),pID->valuestring);//memcpy(pstCard[i].ID, pID->valuestring, sizeof(pstCard[i].ID));
						}
						
						cJSON* puserId = cJSON_GetObjectItem(ppcards,"userId");
						if (puserId && puserId->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"puserId->valuestring = %s" ,puserId->valuestring);
							snprintf(pstCard[i].userId,sizeof(pstCard[i].userId),puserId->valuestring);
						}
						else
						{
							sprintf(pstMsgOut->szDetails,"%s","userId");
							ret = ACS_ERR_DATAERR;
							break;
						}

						cJSON* ptype = cJSON_GetObjectItem(ppcards,"type");
						if (NULL != ptype && ptype->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"ptype->valuestring = %s" ,ptype->valuestring);
							snprintf(pstCard[i].type,sizeof(pstCard[i].type),ptype->valuestring);
						}
						else
						{
							memcpy(pstCard[i].type, "CARD", sizeof(pstCard[i].type));
						}

						cJSON* pcard = cJSON_GetObjectItem(ppcards,"card");
						if (NULL != pcard && pcard->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"pcard->valuestring = %s" ,pcard->valuestring);
							snprintf(pstCard[i].card,sizeof(pstCard[i].card),pcard->valuestring);
						}
						else
						{
							sprintf(pstMsgOut->szDetails,"%s","card");
							ret = ACS_ERR_DATAERR;
							break;
						}

						cJSON* pexpired = cJSON_GetObjectItem(ppcards,"expired");
						if(NULL != pexpired && pexpired->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"pexpired->valuestring = %s" ,pexpired->valuestring);
							snprintf(pstCard[i].expired,sizeof(pstCard[i].expired),pexpired->valuestring);						
						}
						else
						{
							memcpy(pstCard[i].expired, "2147483647", sizeof(pstCard[i].expired));
						}

						cJSON* poperation = cJSON_GetObjectItem(ppcards,"operation");
						if (NULL != poperation && poperation->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"poperation->valuestring = %s" ,poperation->valuestring);
							snprintf(pstCard[i].operation,sizeof(pstCard[i].operation),poperation->valuestring);
						}
						else
						{
							sprintf(pstMsgOut->szDetails,"%s","operation");
							ret = ACS_ERR_DATAERR;
							break;
						}

					//	ACS_WLOG("SyncCards:operator:%s, userId:%s, card:%s, expired:%s", 
					//	pstCard[i].operation, pstCard[i].userId, pstCard[i].card, pstCard[i].expired);
					}

					//平台下发停止入库
					if((Mqtt_Get_DataDealFlag() & ACS_CD_CARD) != ACS_CD_CARD)
					{
						PRT_PRINT(ACS_DEBUG,"synccard stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ACS_WLOG("synccard stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());

						ret = ACS_ERR_NODATA;
						break;
					}
				}
			}
			else
			{
				if(nCardsNum)
				{
					ret = ACS_ERR_MALLOCFAIL;
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","cards");
				}
			}
		}	
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","cards");
			ret = ACS_ERR_DATAERR;
		}
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ret;
}



/**
* fun:GET数据回调 同步人员数据信息数据回调
* @CallData-in:服务器返回的结果数据
* @CallDataOut-Out:输出服务器返回的结果数据 ACS_RECORD_S*
* @return:0-success;<0-Fail -1:回调的json为空;-2:传入的结构体为空
**/
ACS_INT  Curl_DealSyncPwd_Back(ACS_CHAR *CallData,ACS_CHAR *pCallDataOut,ACS_MESSAGE_S *pstMsgOut)
{
	ACS_INT nPwdsNum = 0,ret = 0;
	ACS_INT i = 0;
	ACS_INT nMallocLen = 0;
	ACS_CHAR eField[128] = {0};
	ACS_CHAR sztime[16] = {0}; 
		
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if(pCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallDataOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}

	ACS_PWD_INFO_S *pstPwds = NULL;//(ACS_PWD_INFO_S *)pstCallDataOut->pRecord;

	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"Pwd pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}
		
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)// || (ACS_ERR_SUCCESS != pcode->valueint))
	{
		//ret = pcode->valueint;
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}

	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
		if((peField) && (peField->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
			ACS_WLOG("syncPwd BackData peField = %s" ,peField->valuestring);
			snprintf(eField,128,peField->valuestring);
		}
			
		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if (ptimestamp && ptimestamp->valuestring)
		{
			PRT_PRINT(ACS_DEBUG,"pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACS_WLOG("syncPwd BackData pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACSHTTP_Get_SyncMark(ACS_CD_PWD,ptimestamp->valuestring,eField);
		}
		else
		{
			sprintf(sztime,"%ld",time(NULL));
			ACSHTTP_Get_SyncMark(ACS_CD_PWD,sztime,eField);
		}
		
		cJSON* ptotal = cJSON_GetObjectItem(pdata,"total");
		if (ptotal && ptotal->type == cJSON_Number)
		{
			pstCallDataOut->nTotal = ptotal->valueint;
			PRT_PRINT(ACS_DEBUG,"syncPwd nTotal:%d",pstCallDataOut->nTotal);
		}
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","total");
			ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
			return ACS_ERR_DATAERR;
		}
		
		cJSON* ppasswordList = cJSON_GetObjectItem(pdata,"passwords"); 
		if(ppasswordList && ppasswordList->type == cJSON_Array)
		{
			nPwdsNum = cJSON_GetArraySize(ppasswordList);
			PRT_PRINT(ACS_DEBUG,"syncPwd nPwdsNum:%d",nPwdsNum);
			ACS_WLOG("syncPwd: nPwdsNum:%d;nTotal:%d",nPwdsNum,pstCallDataOut->nTotal);
			
			if(ACSSYNCNUM < nPwdsNum)
				nPwdsNum = ACSSYNCNUM;

			if(nPwdsNum)
			{
				nMallocLen = sizeof(ACS_PWD_INFO_S)*nPwdsNum;
				PRT_PRINT(ACS_DEBUG,"syncPwd syncPwd:%d;nMallocLen:%d",nPwdsNum,nMallocLen);
				ACS_WLOG("syncPwd: nPwdsNum:%d;nMallocLen:%d;nTotal:%d",nPwdsNum,nMallocLen,pstCallDataOut->nTotal);
				pstPwds = (ACS_PWD_INFO_S *)malloc(nMallocLen);
				PRT_PRINT(ACS_DEBUG,"syncPwd malloc psteRules:%p",pstPwds);
			}
			
			if(pstPwds)
			{
				pstCallDataOut->nPageSize = nPwdsNum;
				memset(pstPwds,0,(nMallocLen));
				pstCallDataOut->pRecord = (ACS_CHAR *)pstPwds;
				PRT_PRINT(ACS_DEBUG,"syncPwd nPageSize:%d;pstPwds:%p,%p",pstCallDataOut->nPageSize,pstPwds,pstCallDataOut->pRecord);
				for(i = 0;i<nPwdsNum;i++)
				{
					cJSON* pPassWords = cJSON_GetArrayItem(ppasswordList,i);
					PRT_PRINT(ACS_DEBUG,"syncPwd nPwdsNum:%d;type:%d",nPwdsNum,pPassWords->type);
					if(pPassWords)
					{
						cJSON* pID = cJSON_GetObjectItem(pPassWords, "id");
						if(pID && pID->valuestring)
						{
							snprintf(pstPwds[i].ID,sizeof(pstPwds[i].ID),pID->valuestring);//memcpy(pstCard[i].ID, pID->valuestring, sizeof(pstCard[i].ID));
						}

						cJSON* plimitnum = cJSON_GetObjectItem(pPassWords,"limitnum");		
						if(plimitnum && plimitnum->type == cJSON_Number)
						{
							pstPwds[i].limitnum = plimitnum->valueint;		
						}
						else
						{
							pstPwds[i].limitnum = ACSNOTPWDLIMIT;
						}
						
						cJSON* puserId = cJSON_GetObjectItem(pPassWords,"userId");
						if (NULL != puserId && puserId->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"puserId->valuestring = %s" ,puserId->valuestring);
							snprintf(pstPwds[i].userId,sizeof(pstPwds[i].userId),puserId->valuestring);
						}
						else
						{
							sprintf(pstMsgOut->szDetails,"%s","userId");
							ret = ACS_ERR_DATAERR;
							break;
						}

						cJSON* ppwd = cJSON_GetObjectItem(pPassWords,"password");
						if (NULL != ppwd && ppwd->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"pstPwd->valuestring = %s" ,ppwd->valuestring);
							snprintf(pstPwds[i].pwd,sizeof(pstPwds[i].pwd),ppwd->valuestring);
						}
						else
						{
							sprintf(pstMsgOut->szDetails,"%s","password");
							ret = ACS_ERR_DATAERR;
							break;
						}

						cJSON* pexpired = cJSON_GetObjectItem(pPassWords,"expired");
						if (NULL != pexpired && pexpired->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"pexpired->valuestring = %s" ,pexpired->valuestring);
							snprintf(pstPwds[i].expired,sizeof(pstPwds[i].expired),pexpired->valuestring);					
						}

						cJSON* poperation = cJSON_GetObjectItem(pPassWords,"operation");
						if (NULL != poperation && poperation->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"poperation->valuestring = %s" ,poperation->valuestring);
							snprintf(pstPwds[i].operation,sizeof(pstPwds[i].operation),poperation->valuestring);						
						}
						else
						{
							sprintf(pstMsgOut->szDetails,"%s","operation");
							ret = ACS_ERR_DATAERR;
							break;
						}
						
					//	ACS_WLOG("syncPwd:operator:%s, userId:%s, pwd:%s, expired:%s",pstPwds[i].operation, pstPwds[i].userId, pstPwds[i].pwd, pstPwds[i].expired);						
					}
					
					//平台下发停止入库
					if((Mqtt_Get_DataDealFlag() & ACS_CD_PWD) != ACS_CD_PWD)
					{
						PRT_PRINT(ACS_DEBUG,"syncPwd stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ACS_WLOG("syncPwd stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ret = ACS_ERR_NODATA;
						break;
					}
				}
			}
			else
			{
				if(nPwdsNum)
					ret = ACS_ERR_MALLOCFAIL;
				else
					ret = ACS_ERR_DATAERR;
			}		
		}	
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","passwords");
			ret = ACS_ERR_DATAERR;
		}
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ret;
}




/**
* fun:GET数据回调 同步人员数据信息数据回调
* @CallData-in:服务器返回的结果数据
* @CallDataOut-Out:输出服务器返回的结果数据 ACS_RECORD_S*
* @return:0-success;<0-Fail -1:回调的json为空;-2:传入的结构体为空
**/
ACS_INT  Curl_DealSyncFace_Back(ACS_CHAR *CallData,ACS_CHAR *pCallDataOut,ACS_MESSAGE_S *pstMsgOut)
{
	PRT_PRINT(ACS_DEBUG,"Curl_DealSyncFace_callback begin");
	
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}

	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);
	
#if 0	
	if(ACSJSONMAXLEN < strlen(CallData))
	{
		PRT_PRINT("CallData ACSJSONMAXLEN");
		return ACS_ERR_DATAMAX;
	}
#endif	
	if(pCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallDataOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}

	ACS_INT i = 0;
	ACS_INT e = 0;
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT nFaceNum = 0;
	//ACS_INT neRules = 0;
	ACS_INT neDisRules = 0;
	ACS_CHAR eField[128] = {0};
	ACS_CHAR sztime[16] = {0};
	cJSON* peFloor  =  NULL;
	ACS_ELEDISRULES_S *psteRules = NULL;//ACS_SYNCTIME_CONFIG_S stSysctime;
	ACS_FACE_INFO_S *pstFace = (ACS_FACE_INFO_S *)pstCallDataOut->pRecord;
	if(pstFace == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstFace null");
		return ACS_ERR_PARAMNULL;
	}
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);

//printf("\033[1m\033[40;31m face cJSON_Parse psteRules:%p;[%s:%d]\033[0m\n",pRecvJsonRoot,__func__, __LINE__);

	
	if(pRecvJsonRoot == NULL)
	{
		ACS_WLOG("SyncFaceThread pRecvJsonRoot NULL");
		return ACS_ERR_DATANULL;	
	}

	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if (pcode == NULL)//(pcode->valueint != ACS_ERR_SUCCESS)
	{
		PRT_PRINT(ACS_DEBUG,"SyncFace retCode NULL");
		ACS_WLOG("SyncFaceThread retCode NULL");

		cJSON* pstatus = cJSON_GetObjectItem(pRecvJsonRoot,"status");//"status":406 判断
		if((pstatus)&&(pstatus->type == cJSON_Number))
		{
			sprintf(pstMsgOut->szDetails,"status:%d",pstatus->valueint);
		}
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","retCode");
		}
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}


	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}
	

	//PRT_PRINT("pstFace:%p;pstCallDataOut->pRecord:%p",pstFace,pstCallDataOut->pRecord);

	//ACSCOM_Get_SyncTimeCfg(&stSysctime);
	
	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
		if((peField) && (peField->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
			ACS_WLOG("syncface BackData peField = %s" ,peField->valuestring);
			snprintf(eField,128,peField->valuestring);
		}
	
		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if((ptimestamp) && (ptimestamp->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACS_WLOG("syncface BackData pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACSHTTP_Get_SyncMark(ACS_CD_FACE,ptimestamp->valuestring,eField);
		}
		else
		{
			sprintf(sztime,"%ld",time(NULL));
			ACSHTTP_Get_SyncMark(ACS_CD_FACE,sztime,eField);
		}

		
		cJSON* ptotal = cJSON_GetObjectItem(pdata,"total");
		if(ptotal && ptotal->type == cJSON_Number)
		{
			pstCallDataOut->nTotal = ptotal->valueint;
			PRT_PRINT(ACS_DEBUG,"syncface nTotal:%d",pstCallDataOut->nTotal);
		}
		/**
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","total");
			ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
			return ACS_ERR_DATAERR;
		}**/

		cJSON* ppdata = cJSON_GetObjectItem(pdata,"data"); 
		if(ppdata && ppdata->type==cJSON_Array)
		{
			nFaceNum = cJSON_GetArraySize(ppdata);
			PRT_PRINT(ACS_DEBUG,"syncface nFaceNum:%d",nFaceNum);
			
			ACS_WLOG("SyncFaces: nFaceNum:%d;nTotal:%d",nFaceNum,pstCallDataOut->nTotal);

			if(ACSSYNCNUM < nFaceNum)
			{
				nFaceNum = ACSSYNCNUM;
				//ret = ACS_ERR_DATANULL;
			}
		
			pstCallDataOut->nPageSize = nFaceNum;
			for(i = 0;i<nFaceNum;i++)
			{
				cJSON* pppadd  =  cJSON_GetArrayItem(ppdata,i);
				if(pppadd)
				{
					cJSON* pID = cJSON_GetObjectItem(pppadd, "id");
					if(pID && pID->valuestring)
					{
						snprintf(pstFace[i].ID,sizeof(pstFace[i].ID),pID->valuestring);
					}
					
					cJSON* puserId = cJSON_GetObjectItem(pppadd,"userId");
					if (puserId && puserId->valuestring)
					{
						snprintf(pstFace[i].userId,sizeof(pstFace[i].userId),puserId->valuestring);
						PRT_PRINT(ACS_DEBUG,"puserId = %s" ,puserId->valuestring);
					}
					else
					{
						ret = ACS_ERR_DATAERR;
						sprintf(pstMsgOut->szDetails,"%s","userId");
						break;
					}
					
					cJSON* puserName = cJSON_GetObjectItem(pppadd,"userName");
					if (NULL != puserName && puserName->valuestring)
					{
						snprintf(pstFace[i].userName,sizeof(pstFace[i].userName),puserName->valuestring);
						PRT_PRINT(ACS_DEBUG,"puserName:%s" ,puserName->valuestring);
					}

					cJSON* puserType = cJSON_GetObjectItem(pppadd,"userType");
					if (NULL != puserType && puserType->valuestring)
					{
						snprintf(pstFace[i].userType,sizeof(pstFace[i].userType),puserType->valuestring);
						PRT_PRINT(ACS_DEBUG,"puserType:%s" ,puserType->valuestring);
					}

					cJSON* ppicurl = cJSON_GetObjectItem(pppadd,"url");
					if (NULL != ppicurl && ppicurl->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"url:%s",ppicurl->valuestring);
						snprintf(pstFace[i].url,sizeof(pstFace[i].url),ppicurl->valuestring);
					}/**
					else
					{
						sprintf(pstMsgOut->szDetails,"%s","url");
						ret = ACS_ERR_DATAERR;
						break;
					}**/


					cJSON* pcard = cJSON_GetObjectItem(pppadd,"card");
					if (NULL != pcard && pcard->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"pcard->valuestring = %s" ,pcard->valuestring);
						snprintf(pstFace[i].card,sizeof(pstFace[i].card),pcard->valuestring);
					}

					cJSON* ppassword = cJSON_GetObjectItem(pppadd,"password");
					if (NULL != ppassword && ppassword->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"ppassword->valuestring = %s" ,ppassword->valuestring);
						snprintf(pstFace[i].password,sizeof(pstFace[i].password),ppassword->valuestring);
					}
					
					cJSON* puserIdCard = cJSON_GetObjectItem(pppadd,"userIdCard");
					if (NULL != puserIdCard && puserIdCard->valuestring)
					{
						PRT_PRINT(ACS_DEBUG,"userIdCard->valuestring = %s" ,puserIdCard->valuestring);
						snprintf(pstFace[i].userIdCard,sizeof(pstFace[i].userIdCard),puserIdCard->valuestring);
					}

					cJSON* poperation = cJSON_GetObjectItem(pppadd,"operation");
					if (NULL != poperation && poperation->valuestring)
					{
						snprintf(pstFace[i].operation,sizeof(pstFace[i].operation),poperation->valuestring);
						PRT_PRINT(ACS_DEBUG,"poperation->valuestring = %s" ,poperation->valuestring);
					}
					else
					{
						sprintf(pstMsgOut->szDetails,"%s","operation");
						ret = ACS_ERR_DATAERR;
						break;
					}

					
					cJSON* pValidEnable = cJSON_GetObjectItem(pppadd,"validtimeenable");
					if (NULL != pValidEnable && pValidEnable->type == cJSON_Number)
					{
						pstFace[i].nValidEnable = pValidEnable->valueint;
						PRT_PRINT(ACS_DEBUG,"pValidEnable->valueint = %d" ,pValidEnable->valueint);
					}

					cJSON* pValidtime = cJSON_GetObjectItem(pppadd,"validtime");
					if (NULL != pValidtime && pValidtime->valuestring != NULL)
					{
						snprintf(pstFace[i].validtime, sizeof(pstFace[i].validtime), pValidtime->valuestring);
						PRT_PRINT(ACS_DEBUG,"pValidtime->valuestring = %s" ,pValidtime->valuestring);
					}
					
					cJSON* pValidtimeend = cJSON_GetObjectItem(pppadd,"validtimeend");
					if (NULL != pValidtimeend && pValidtimeend ->valuestring != NULL)
					{
						snprintf(pstFace[i].validtimeend, sizeof(pstFace[i].validtimeend), pValidtimeend ->valuestring);
						PRT_PRINT(ACS_DEBUG,"pValidtimeend ->valuestring = %s" ,pValidtimeend ->valuestring);
					}
					
	#if 0
					cJSON* peRulesList = cJSON_GetObjectItem(pppadd,"eRules"); 
					if(peRulesList && peRulesList->type==cJSON_Array)
					{
						neRules = cJSON_GetArraySize(peRulesList);
						if(neRules)
						{
							PRT_PRINT(ACS_DEBUG,"syncface neRulesNum:%d",neRules);
							ACS_WLOG("SyncFaces: neRulesNum:%d;nTotal:%d",neRules,pstCallDataOut->nTotal);
							psteRules = (ACS_ELERULES_S *)malloc(sizeof(ACS_ELERULES_S)*neRules);
							PRT_PRINT(ACS_DEBUG,"syncface malloc psteRules:%p",psteRules);
						}
						if(psteRules)
						{
							pstFace[i].neRules = neRules;
							pstFace[i].peRulesData = (ACS_CHAR *)psteRules;
							for(e = 0;e<neRules;e++)
							{
								cJSON* peRules  =  cJSON_GetArrayItem(peRulesList,e);
								if(peRules)
								{
									cJSON* peleRuleid = cJSON_GetObjectItem(peRules, "eleRuleid");
									if(peleRuleid && peleRuleid->valuestring)
									{
										snprintf(psteRules[e].eleRuleid,sizeof(psteRules[e].eleRuleid),peleRuleid->valuestring);
									}
									cJSON* peleType = cJSON_GetObjectItem(peRules, "eleType");
									if(peleType && peleType->type == cJSON_Number)
									{
										psteRules[e].eleType = peleType->valueint;
									}
								}
							}
						}
					}
	#endif
					//平台下发停止入库
					if((Mqtt_Get_DataDealFlag() & ACS_CD_FACE) != ACS_CD_FACE)
					{
						//pstCallDataOut->bifStop = ACS_ITURN;
						PRT_PRINT(ACS_DEBUG,"syncface stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ACS_WLOG("SyncFaces stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ret = ACS_ERR_NODATA;
						break;
					}
					
					cJSON* peledisruleList = cJSON_GetObjectItem(pppadd,"eledisrules"); 
					if(peledisruleList && peledisruleList->type==cJSON_Array)
					{
						neDisRules = cJSON_GetArraySize(peledisruleList);
						if(neDisRules)
						{
							PRT_PRINT(ACS_DEBUG,"syncface eDisRules:%d",neDisRules);
							ACS_WLOG("SyncFaces: eDisRules:%d;nTotal:%d",neDisRules,pstCallDataOut->nTotal);
							psteRules = (ACS_ELEDISRULES_S *)malloc(sizeof(ACS_ELEDISRULES_S)*neDisRules);							
							PRT_PRINT(ACS_DEBUG,"syncface malloc psteRules:%p",psteRules);
						}
						if(psteRules)
						{
							memset(psteRules,0,(sizeof(ACS_ELEDISRULES_S)*neDisRules));
							pstFace[i].neDisRules = neDisRules;
							pstFace[i].peDisRulesData = (ACS_CHAR *)psteRules;
							for(e = 0;e<neDisRules;e++)
							{
								peFloor = cJSON_GetArrayItem(peledisruleList,e);
								if((peFloor) && (peFloor->type == cJSON_Number))
								{
									psteRules[e].eleFloor = peFloor->valueint;
									PRT_PRINT(ACS_DEBUG,"syncface psteRules[%d].eleFloor:%d;peFloor:%d",e,psteRules[e].eleFloor, peFloor->valueint);
								}
							}
						}
					}
					
					
				//	ACS_WLOG("SyncFaces: operation:%s, userName:%s, userId:%s, url:%s", pstFace[i].operation, pstFace[i].userName, pstFace[i].userId, pstFace[i].url);
					//usleep(200);
				}			
				else
				{
					sprintf(pstMsgOut->szDetails,"%s","data");
					ret = ACS_ERR_DATAERR;
					break;
				}
			}
		}	
		else
		{
			sprintf(pstMsgOut->szDetails,"%s","data");
			ret = ACS_ERR_DATAERR;
		}
	}
	else
	{
		sprintf(pstMsgOut->szDetails,"%s","data");
		ret = ACS_ERR_DATAERR;
	}
	


	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	return ret;
}


/**
* fun:GET数据回调 同步考勤记录补发信息数据回调
* @CallData-in:服务器返回的结果数据
* @CallDataOut-Out:输出服务器返回的结果数据 ACS_RECORD_S*
* @return:0-success;<0-Fail -1:回调的json为空;-2:传入的结构体为空
**/
ACS_INT Curl_DealSyncReissues_Back(ACS_CHAR *CallData,ACS_CHAR *pCallDataOut, ACS_MESSAGE_S *pstMsgOut)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"ReissueRecords CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if (pCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"ReissueRecords pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallDataOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"ReissueRecord pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_REISSUE_RECORD_S *pstReissueInfo = (ACS_REISSUE_RECORD_S *)pstCallDataOut->pRecord;
	if(pstReissueInfo == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"ReissueRecord pstReissueInfo null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_INT ret = 0;
	ACS_INT nReissuesNum = 0;
	ACS_INT i = 0;
	ACS_CHAR eField[128] = {0};
	ACS_CHAR sztime[16] = {0};

	cJSON* pRecvJsonRoot = cJSON_Parse((char *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"ReissueRecord pRecvJsonRoot null");
		return ACS_ERR_DATAERR;
	}
	
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if (pcode == NULL)//(pcode->valueint != ACS_ERR_SUCCESS)
	{
		PRT_PRINT(ACS_DEBUG,"SyncFace retCode NULL");
		ACS_WLOG("SyncFaceThread retCode NULL");
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}

	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata == NULL)
	{
		pdata = cJSON_GetObjectItem(pRecvJsonRoot,"result");
		if(pdata == NULL)
		{
			ACS_freeJson((void**)&pRecvJsonRoot,__func__);
			return ACS_ERR_DATAERR;
		}
	}

	cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
	if((peField) && (peField->valuestring))
	{
		PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
		ACS_WLOG("syncReissue BackData peField = %s" ,peField->valuestring);
		snprintf(eField,128,peField->valuestring);
	}
		
	cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
	if(ptimestamp && ptimestamp->valuestring)
	{
		ACSHTTP_Get_SyncMark(ACS_CD_REISSUES,ptimestamp->valuestring,eField);
	}
	else
	{
		sprintf(sztime,"%ld",time(NULL));
		ACSHTTP_Get_SyncMark(ACS_CD_REISSUES,sztime,eField);
	}
	
	cJSON* ptotal = cJSON_GetObjectItem(pdata,"total");
	if (ptotal && ptotal->type == cJSON_Number)
	{
		pstCallDataOut->nTotal = ptotal->valueint;
		PRT_PRINT(ACS_DEBUG,"SyncReissues nTotal:%d",pstCallDataOut->nTotal);
	}
	
	cJSON* pReissueInfos = cJSON_GetObjectItem(pdata,"reissues"); 
	if(pReissueInfos != NULL && pReissueInfos->type == cJSON_Array)
	{
		nReissuesNum = cJSON_GetArraySize(pReissueInfos);
		ACS_WLOG("SyncReissues nTotal:%d;nReissuesNum:%d",pstCallDataOut->nTotal,nReissuesNum);

		if(nReissuesNum > REISSUENUM)
		{
			nReissuesNum = REISSUENUM;
		}
		pstCallDataOut->nPageSize = nReissuesNum;

		for(i = 0;i<nReissuesNum;i++)
		{
			cJSON* pReissues  =  cJSON_GetArrayItem(pReissueInfos,i);
			if(pReissues != NULL)
			{	
				cJSON* pworkRole = cJSON_GetObjectItem(pReissues,"workRole");
				if (pworkRole && pworkRole->valuestring)
				{
					pstReissueInfo[i].nWorkRole = atoi(pworkRole->valuestring);
				}
				
				cJSON* pdirection = cJSON_GetObjectItem(pReissues,"direction");
				if (pdirection && pdirection->valuestring)
				{
					snprintf(pstReissueInfo[i].szDirection, sizeof(pstReissueInfo[i].szDirection), pdirection->valuestring);
				}

				cJSON* pswipeTimestamp = cJSON_GetObjectItem(pReissues,"swipeTimestamp");
				if(pswipeTimestamp && pswipeTimestamp->valuestring)
				{
					snprintf(pstReissueInfo[i].szSwipeTime,sizeof(pstReissueInfo[i].szSwipeTime),pswipeTimestamp->valuestring);
				}

				cJSON* pidCardNumber = cJSON_GetObjectItem(pReissues,"idCardNumber");
				if (pidCardNumber && pidCardNumber->valuestring)
				{
					snprintf(pstReissueInfo[i].szIdCardNumber, sizeof(pstReissueInfo[i].szIdCardNumber), pidCardNumber->valuestring);
				}

				cJSON* puserId = cJSON_GetObjectItem(pReissues,"userId");
				if (puserId && puserId->valuestring)
				{
					snprintf(pstReissueInfo[i].szUserId, sizeof(pstReissueInfo[i].szUserId), puserId->valuestring);						
				}

				cJSON* pchannel = cJSON_GetObjectItem(pReissues,"channel");
				if (pchannel && pchannel->valuestring)
				{
					snprintf(pstReissueInfo[i].szChannel, sizeof(pstReissueInfo[i].szChannel), pchannel->valuestring);
				}

				cJSON* pPicUrl = cJSON_GetObjectItem(pReissues,"url");
				if (pPicUrl && pPicUrl->valuestring)
				{
					snprintf(pstReissueInfo[i].szPicUrl, sizeof(pstReissueInfo[i].szPicUrl),pPicUrl->valuestring);
				}

				cJSON* pphoto = cJSON_GetObjectItem(pReissues,"photo");
				if (pphoto && pphoto->valuestring)
				{
					snprintf(pstReissueInfo[i].szPicBase, sizeof(pstReissueInfo[i].szPicBase),pphoto->valuestring);
				}

				//ACS_WLOG("Reissue:WorkRole:%d;Direction:%s;userId:%s;IdCard:%s;Channel:%s", 
				//pstReissueInfo[i].nWorkRole,pstReissueInfo[i].szDirection,pstReissueInfo[i].szUserId, pstReissueInfo[i].szIdCardNumber, pstReissueInfo[i].szChannel);
				//usleep(50*1000);
			}
		}
	}	
	ACS_freeJson((void**)&pRecvJsonRoot,__func__);
	return ret;
}




/**
* fun:GET 数据回调 升级固件数据回调
* @CallData-in:服务器返回的结果数据
* @return:>0-success;<0-Fail
**/
ACS_INT Curl_DealUpgadeFW_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if(pCallOut == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_UPGADEFW_INFO_S *pstUpgadeFW = (ACS_UPGADEFW_INFO_S *)pCallOut;
	if(pstUpgadeFW == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"pstUpgadeFW null");
		return ACS_ERR_PARAMNULL;
	}

	if(strcmp(CallData,"502 Bad Gateway") == 0)
	{
		ACS_WLOG("Upload_Record error: 502 Bad Gateway");
	}
	else
	{
		ACS_WLOG("Upload_Record:%s",CallData);
	}
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if(pRecvJsonRoot)
	{
		cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
		if(pcode)
		{
			cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
			if(pdata != NULL)
			{
				cJSON* purl = cJSON_GetObjectItem(pdata,"url");
				if((NULL != purl) && (purl->valuestring != NULL))
				{
					PRT_PRINT(ACS_DEBUG,"acslib url = %s" ,purl->valuestring);
					ACS_WLOG("UpgadeFW BackData url = %s",purl->valuestring);
					snprintf(pstUpgadeFW->szUrl,sizeof(pstUpgadeFW->szUrl),"%s",purl->valuestring);
					//nCheck ++;
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","url");
				}
				cJSON* pversion = cJSON_GetObjectItem(pdata,"version");
				if((NULL != pversion) && (pversion->valuestring != NULL))
				{
					PRT_PRINT(ACS_DEBUG,"acslib version = %s" ,pversion->valuestring);
					ACS_WLOG("UpgadeFW BackData version = %s",pversion->valuestring);
					snprintf(pstUpgadeFW->szVersion,sizeof(pstUpgadeFW->szVersion),"%s",pversion->valuestring);
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","version");
				}
				
				cJSON* psize = cJSON_GetObjectItem(pdata,"size");
				if((psize != NULL) && (psize->type==cJSON_Number))
				{
					ACS_WLOG("UpgadeFW BackData size = %d",psize->valueint);
					pstUpgadeFW->nSize = psize->valueint;
					//nCheck ++;
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","size");
				}
				
				cJSON* pmd5 = cJSON_GetObjectItem(pdata,"md5");
				if((pmd5 != NULL) && (pmd5->valuestring != NULL))
				{
					ACS_WLOG("UpgadeFW BackData md5 = %s",pmd5->valuestring);
					snprintf(pstUpgadeFW->szMd5,sizeof(pstUpgadeFW->szMd5),"%s",pmd5->valuestring);
					//nCheck ++;
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","md5");
				}
				
				#if 0
				cJSON* pName = cJSON_GetObjectItem(pdata,"name");
				if((pName != NULL) && (pName->valuestring != NULL))
				{
					ACS_WLOG("UpgadeFW szFileName:%s",pName->valuestring);
					snprintf(pstUpgadeFW->szFileName,sizeof(pstUpgadeFW->szFileName),"%s",pName->valuestring);
					//nCheck ++;
				}
				else
				{
					ret = ACS_ERR_DATAERR;
					sprintf(pstMsgOut->szDetails,"%s","name");
				}
				#endif
#if 0
				//回调函数实现同步人脸并上传同步结果
				if(g_stDoor_Info.funcUpgadeFW != NULL)
					g_stDoor_Info.funcUpgadeFW(&stUpgadeFW);
				else
					printf("acslib no set function to syncAd!\n");
#endif
			}
			else
			{
				ret = ACS_ERR_DATAERR;
				sprintf(pstMsgOut->szDetails,"%s","data");
			}
		}	
		else
		{
			ret = ACS_ERR_DATAERR;
			sprintf(pstMsgOut->szDetails,"%s","retCode");
		}
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	}
	else
	{
		ret = ACS_ERR_DATANULL;
	}
	return ret;
}


/**
* fun:GET 数据回调 获取平台时间戳数据回调
* @CallData-in:服务器返回的结果数据
* @return:0-success;<0-Fail
**/
ACS_INT  Curl_DealPlatformTime_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	if(NULL == CallData)
	{
	   return ACS_ERR_DATANULL;
	}
	
	if(NULL == pCallOut)
	{
	   return ACS_ERR_PARAMNULL;
	}

	if(strcmp(CallData,"502 Bad Gateway") == 0)
	{
		ACS_WLOG("Upload_Record error: 502 Bad Gateway");
	}
	//else
	//{
		//ACS_WLOG("Upload_Record:%s",CallData);
	//}
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
		return ACS_ERR_DATANULL;	
	
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if((pcode == NULL) || (pcode->type != cJSON_Number))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}

	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata != NULL)
	{
		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if((NULL != ptimestamp) && (ptimestamp->valuestring != NULL))
		{
			*(ACS_UINT64 *)pCallOut = atoll(ptimestamp->valuestring);
			//pCallOut = (ACS_CHAR *)&timestate;
			//PRT_PRINT("ptimestamp=%s;pCallOut:%lld\n",ptimestamp->valuestring,*(ACS_UINT64 *)pCallOut);
			//ACS_WLOG("syncTime ptimestamp=%s",ptimestamp->valuestring);
		}
		else
		{
			ret = ACS_ERR_DATAERR;
			sprintf(pstMsgOut->szDetails,"%s","timestamp");
		}
	}
	else
	{
		ret = ACS_ERR_DATAERR;
		sprintf(pstMsgOut->szDetails,"%s","data");
	}

	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	 
	return ret;

}


/**
* fun:POST数据回调     开门记录(识别记录)数据回调
* @CallData-in:服务器返回的结果数据
* @return:ACS_ERR_SUCCESS-success;<0-Fail  ACS_ERR_DATANULL ACS_ERR_DATAERR
**/
ACS_INT  Curl_DealUploadRecord_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut)
{
	if(CallData == NULL)
	{
	   return ACS_ERR_DATANULL;
	}
	
	//PRT_PRINT("Upload_Record CallData:%s",CallData);
	if(strcmp(CallData,"502 Bad Gateway") == 0)
	{
		ACS_WLOG("Upload_Record error: 502 Bad Gateway");
	}
	//else
	//{
	//	ACS_WLOG("Upload_Record:%s",CallData);
	//}
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
		return ACS_ERR_DATANULL;	
	
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(pcode == NULL)
	{
		 ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}

	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	 
	return ACS_ERR_SUCCESS;

}




/**
* fun:POST数据回调 解析服务器返回的健康码信息
* @CallData-in:服务器返回的信息
* @pCallOut-out:解析到的信息
* @return:0-success;
**/
int Curl_DealSnapPhoto_Back(char *CallData ,char *pCallOut)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}

	cJSON* pRecvJsonRoot = cJSON_Parse(CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"MjAcsCloud Snapphoto pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if (pcode == NULL)
	{
		ACS_freeJson((void**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
	cJSON *pMsg = cJSON_GetObjectItem(pRecvJsonRoot, "message");
	if(pMsg && pMsg->valuestring)
	{
		ACS_WLOG("CallResult retCode=%d, message=%s",pcode->valueint, pMsg->valuestring);
	}

	ACS_freeJson((void**)&pRecvJsonRoot,__func__);

	return 0;
}

ACS_VOID Curl_DealSyncParam_json(cJSON* pRecvJsonRoot,ACS_MESSAGE_S *pstMsgOut)
{
	ACS_DEVCFG_INFO_S stAcsDevinfo;
	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* pHealthcloudCfgV4 = cJSON_GetObjectItem(pdata, "healthcloudCfgV4");
		if((pHealthcloudCfgV4) && (pHealthcloudCfgV4->type == cJSON_Object))
		{
			ACS_HCODECLOUD_S stAcsHcodeCloud;
			memset(&stAcsHcodeCloud,0,sizeof(ACS_HCODECLOUD_S));
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_HCODECLOUD;
			stAcsDevinfo.dwDataLen = sizeof(ACS_HCODECLOUD_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsHcodeCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pSceneSerialID = cJSON_GetObjectItem(pHealthcloudCfgV4, "SceneSerialID");
			if(pSceneSerialID && pSceneSerialID->valuestring)
			{
				snprintf(stAcsHcodeCloud.szSceneSerialID,sizeof(stAcsHcodeCloud.szSceneSerialID),"%s",pSceneSerialID->valuestring);
			}
	
			cJSON* pServerIP = cJSON_GetObjectItem(pHealthcloudCfgV4, "ServerIP");
			if(pServerIP && pServerIP->valuestring)
			{
				snprintf(stAcsHcodeCloud.szServerIP,sizeof(stAcsHcodeCloud.szServerIP),"%s",pServerIP->valuestring);
			}

			cJSON* pAuthKey = cJSON_GetObjectItem(pHealthcloudCfgV4, "AuthKey");
			if(pAuthKey && pAuthKey->valuestring)
			{
				snprintf(stAcsHcodeCloud.szAuthKey,sizeof(stAcsHcodeCloud.szAuthKey),"%s",pAuthKey->valuestring);
			}

			cJSON* pAuthIV = cJSON_GetObjectItem(pHealthcloudCfgV4, "AuthIV");
			if(pAuthIV && pAuthIV->valuestring)
			{
				snprintf(stAcsHcodeCloud.szAuthIV,sizeof(stAcsHcodeCloud.szAuthIV),"%s",pAuthIV->valuestring);
			}

			cJSON* pSceneCategory = cJSON_GetObjectItem(pHealthcloudCfgV4, "SceneCategory");
			if(pSceneCategory && pSceneCategory->valuestring)
			{
				snprintf(stAcsHcodeCloud.szSceneCategory,sizeof(stAcsHcodeCloud.szSceneCategory),"%s",pSceneCategory->valuestring);
			}

			stAcsDevinfo.nType = ACS_DEV_PARAM_HCODECLOUD+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.dwDataLen = sizeof(ACS_HCODECLOUD_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsHcodeCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}
		
		cJSON* pCloudCfg = cJSON_GetObjectItem(pdata, "CloudCfg");
		if((pCloudCfg) && (pCloudCfg->type == cJSON_Object))
		{
			ACS_CLOUDCFG_S stAcsCloud;
			memset(&stAcsCloud,0,sizeof(ACS_CLOUDCFG_S));
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_CLOUDCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_CLOUDCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);		

			cJSON* pCloudEnable = cJSON_GetObjectItem(pCloudCfg, "CloudEnable");
			if(pCloudEnable && pCloudEnable->type == cJSON_Number)
			{
				stAcsCloud.byEnable = (pCloudEnable->valueint?1:0);
			}

			cJSON* pCloudIPAddr = cJSON_GetObjectItem(pCloudCfg, "CloudIPAddrCfg");
			if(pCloudIPAddr && pCloudIPAddr->valuestring)
			{
				snprintf(stAcsCloud.szCloudIPAddr,sizeof(stAcsCloud.szCloudIPAddr),"%s",pCloudIPAddr->valuestring);
			}

			cJSON* pCloudID = cJSON_GetObjectItem(pCloudCfg, "CloudID");
			if(pCloudID && pCloudID->valuestring)
			{
				snprintf(stAcsCloud.szCloudID,sizeof(stAcsCloud.szCloudID),"%s",pCloudID->valuestring);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_CLOUDCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pAcsCfg = cJSON_GetObjectItem(pdata, "AcsCfg");
		if((pAcsCfg) && (pAcsCfg->type == cJSON_Object))
		{
			ACS_ACSCFG_S stAcsCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_ACSCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_ACSCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pAcsState = cJSON_GetObjectItem(pAcsCfg, "AcsState");
			if(pAcsState && pAcsState->type == cJSON_Number)
			{
				stAcsCfg.dwAcsState = pAcsState->valueint;
				g_bAcsState = stAcsCfg.dwAcsState;
			}

			cJSON* pRecoredPicEnable = cJSON_GetObjectItem(pAcsCfg, "RecoredPicEnable");
			if(pRecoredPicEnable && pRecoredPicEnable->type == cJSON_Number)
			{
				stAcsCfg.dwRecoredPicEnable = pRecoredPicEnable->valueint;
			}

			cJSON* pStrangeRecEnable = cJSON_GetObjectItem(pAcsCfg, "StrangeRecEnable");
			if(pStrangeRecEnable && pStrangeRecEnable->type == cJSON_Number)
			{
				stAcsCfg.dwStrangeRecEnable = pStrangeRecEnable->valueint;
			}

			cJSON* pStrangeTik = cJSON_GetObjectItem(pAcsCfg, "StrangeTik");
			if(pStrangeTik && pStrangeTik->type == cJSON_Number)
			{
				stAcsCfg.dwStrangeTik = pStrangeTik->valueint;
			}

			cJSON* pRelay1DelayTime = cJSON_GetObjectItem(pAcsCfg, "Relay1DelayTime");
			if(pRelay1DelayTime && pRelay1DelayTime->type == cJSON_Number)
			{
				stAcsCfg.dwDelay1Time = pRelay1DelayTime->valueint;
			}

			cJSON* pRelay1IoType = cJSON_GetObjectItem(pAcsCfg, "Relay1IoType");
			if(pRelay1IoType && pRelay1IoType->type == cJSON_Number)
			{
				stAcsCfg.dwIo1Type = pRelay1IoType->valueint;
			}
			
			cJSON* pConfirmMode = cJSON_GetObjectItem(pAcsCfg, "ConfirmMode");
			if(pConfirmMode && pConfirmMode->type == cJSON_Number)
			{
				stAcsCfg.dwConfirmMode = pConfirmMode->valueint;
			}

			cJSON* pConfirmWayMask = cJSON_GetObjectItem(pAcsCfg, "ConfirmWayMask");
			if(pConfirmWayMask && pConfirmWayMask->type == cJSON_Number)
			{
				stAcsCfg.dwConfirmWayMask = pConfirmWayMask->valueint;
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_ACSCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}
#if 1

		cJSON* pAudioStreamCfg = cJSON_GetObjectItem(pdata, "AudioStream");
		if((pAudioStreamCfg) && (pAudioStreamCfg->type == cJSON_Object))
		{
			ACS_AUDIOSTREAM_S stAudiosCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_AUDIOCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_AUDIOSTREAM_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAudiosCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pEnable = cJSON_GetObjectItem(pAudioStreamCfg, "Enable");
			if(pEnable && pEnable->type == cJSON_Number)
			{
				stAudiosCfg.byEnable = pEnable->valueint;
			}
			cJSON* pVolumeIn = cJSON_GetObjectItem(pAudioStreamCfg, "VolumeIn");
			if(pVolumeIn && pVolumeIn->type == cJSON_Number)
			{
				stAudiosCfg.byVolumeIn = pVolumeIn->valueint;
			}
			cJSON* pVolumeOut = cJSON_GetObjectItem(pAudioStreamCfg, "VolumeOut");
			if(pVolumeOut && pVolumeOut->type == cJSON_Number)
			{
				stAudiosCfg.byVolumeOut = pVolumeOut->valueint;
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_AUDIOCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAudiosCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pCameraCfg = cJSON_GetObjectItem(pdata, "CameraConfig");
		if((pCameraCfg) && (pCameraCfg->type == cJSON_Object))
		{
			ACS_CAMCFG_S stCameraCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_CAMCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_CAMCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stCameraCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pDeviceName = cJSON_GetObjectItem(pCameraCfg, "DeviceName");
			if(pDeviceName && pDeviceName->valuestring)
			{
				snprintf(stCameraCfg.szDevName,sizeof(stCameraCfg.szDevName),"%s",pDeviceName->valuestring);
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_CAMCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stCameraCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pSfaceCfg = cJSON_GetObjectItem(pdata, "SfaceCfg");
		if((pSfaceCfg) && (pSfaceCfg->type == cJSON_Object))
		{
			ACS_SFACECFG_S stsFaceCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_SFACECFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_SFACECFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stsFaceCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pAcsScene = cJSON_GetObjectItem(pSfaceCfg, "AcsScene");
			if(pAcsScene && pAcsScene->type == cJSON_Number)
			{
				stsFaceCfg.dwAcsScene = pAcsScene->valueint;
			}
			cJSON* pCmpSingleThreshold = cJSON_GetObjectItem(pSfaceCfg, "CmpSingleThreshold");
			if(pCmpSingleThreshold && pCmpSingleThreshold->type == cJSON_Number)
			{
				stsFaceCfg.dwCmpSingleThreshold = pCmpSingleThreshold->valueint;
			}
			cJSON* pCmpThreshold = cJSON_GetObjectItem(pSfaceCfg, "CmpThreshold");
			if(pCmpThreshold && pCmpThreshold->type == cJSON_Number)
			{
				stsFaceCfg.dwCmpThreshold = pCmpThreshold->valueint;
			}
			cJSON* pFaceMaxSize = cJSON_GetObjectItem(pSfaceCfg, "FaceMaxSize");
			if(pFaceMaxSize && pFaceMaxSize->type == cJSON_Number)
			{
				stsFaceCfg.dwFaceMaxSize = pFaceMaxSize->valueint;
			}
			cJSON* pFaceMinSize = cJSON_GetObjectItem(pSfaceCfg, "FaceMinSize");
			if(pFaceMinSize && pFaceMinSize->type == cJSON_Number)
			{
				stsFaceCfg.dwFaceMinSize = pFaceMinSize->valueint;
			}
			cJSON* pFaceSensitivity = cJSON_GetObjectItem(pSfaceCfg, "FaceSensitivity");
			if(pFaceSensitivity && pFaceSensitivity->type == cJSON_Number)
			{
				stsFaceCfg.dwFaceSensitivity = pFaceSensitivity->valueint;
			}
			cJSON* pHacknessenble = cJSON_GetObjectItem(pSfaceCfg, "Hacknessenble");
			if(pHacknessenble && pHacknessenble->type == cJSON_Number)
			{
				stsFaceCfg.dwHacknessenble = pHacknessenble->valueint;
			}
			cJSON* pHacknessthreshold = cJSON_GetObjectItem(pSfaceCfg, "Hacknessthreshold");
			if(pHacknessthreshold && pHacknessthreshold->type == cJSON_Number)
			{
				stsFaceCfg.dwHacknessthreshold = pHacknessthreshold->valueint;
			}
			cJSON* pMaskDetec = cJSON_GetObjectItem(pSfaceCfg, "MaskDetec");
			if(pMaskDetec && pMaskDetec->type == cJSON_Number)
			{
				stsFaceCfg.dwMaskEnable = pMaskDetec->valueint;
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_SFACECFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stsFaceCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
        }


		cJSON* pWirednetwork = cJSON_GetObjectItem(pdata, "Wirednetwork");
		if((pWirednetwork) && (pWirednetwork->type == cJSON_Object))
		{
			ACS_WIREDNET_S wirednet = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_WIREDNETCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_WIREDNET_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&wirednet;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pipAddress = cJSON_GetObjectItem(pWirednetwork, "ipAddress");
			if(pipAddress && pipAddress->valuestring)
			{
				snprintf(wirednet.ipAddress,sizeof(wirednet.ipAddress),"%s",pipAddress->valuestring);
			}
			cJSON* pdhcp = cJSON_GetObjectItem(pWirednetwork, "dhcp");
			if(pdhcp && pdhcp->type == cJSON_Number)
			{
				wirednet.bDHCP = (pdhcp->valueint?1:0);
			}
			cJSON* psubnetMask = cJSON_GetObjectItem(pWirednetwork, "subnetMask");
			if(psubnetMask && psubnetMask->valuestring)
			{
				snprintf(wirednet.subnetMask,sizeof(wirednet.subnetMask),"%s",psubnetMask->valuestring);
			}
			cJSON* pSecondaryDNS = cJSON_GetObjectItem(pWirednetwork, "SecondaryDNS");
			if(pSecondaryDNS && pSecondaryDNS->valuestring)
			{
				snprintf(wirednet.SecondaryDNS,sizeof(wirednet.SecondaryDNS),"%s",pSecondaryDNS->valuestring);
			}
			cJSON* pPrimaryDNS = cJSON_GetObjectItem(pWirednetwork, "PrimaryDNS");
			if(pPrimaryDNS && pPrimaryDNS->valuestring)
			{
				snprintf(wirednet.PrimaryDNS,sizeof(wirednet.PrimaryDNS),"%s",pPrimaryDNS->valuestring);
			}
			
			cJSON* pMACAddress = cJSON_GetObjectItem(pWirednetwork, "MACAddress");
			if(pMACAddress && pMACAddress->valuestring)
			{
				snprintf(wirednet.MACAddress,sizeof(wirednet.MACAddress),"%s",pMACAddress->valuestring);
			}
			
			cJSON* pDefaultGateway = cJSON_GetObjectItem(pWirednetwork, "DefaultGateway");
			if(pDefaultGateway && pDefaultGateway->valuestring)
			{
				snprintf(wirednet.DefaultGateway,sizeof(wirednet.DefaultGateway),"%s",pDefaultGateway->valuestring);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_WIREDNETCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&wirednet;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}


		cJSON* pSysteminfoVersion = cJSON_GetObjectItem(pdata, "SysteminfoVersion");
		if((pSysteminfoVersion) && (pSysteminfoVersion->type == cJSON_Object))
		{
			ACS_SYSTEMVERSION_S systemVer = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_SYNTEMVERCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_SYSTEMVERSION_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&systemVer;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* puuid = cJSON_GetObjectItem(pSysteminfoVersion, "uuid");
			if(puuid && puuid->valuestring)
			{
				snprintf(systemVer.uuid,sizeof(systemVer.uuid),"%s",puuid->valuestring);
			}
			cJSON* pcpuid = cJSON_GetObjectItem(pSysteminfoVersion, "cpuid");
			if(pcpuid && pcpuid->valuestring)
			{
				snprintf(systemVer.cpuid,sizeof(systemVer.cpuid),"%s",pcpuid->valuestring);
			}
			cJSON* pDeviceAlgoVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceAlgoVersion");
			if(pDeviceAlgoVersion && pDeviceAlgoVersion->valuestring)
			{
				snprintf(systemVer.DeviceAlgoVersion,sizeof(systemVer.DeviceAlgoVersion),"%s",pDeviceAlgoVersion->valuestring);
			}
			cJSON* pDeviceOnvifVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceOnvifVersion");
			if(pDeviceOnvifVersion && pDeviceOnvifVersion->valuestring)
			{
				snprintf(systemVer.DeviceOnvifVersion,sizeof(systemVer.DeviceOnvifVersion),"%s",pDeviceOnvifVersion->valuestring);
			}
			cJSON* pDeviceSoftVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceSoftVersion");
			if(pDeviceSoftVersion && pDeviceSoftVersion->valuestring)
			{
				snprintf(systemVer.DeviceSoftVersion,sizeof(systemVer.DeviceSoftVersion),"%s",pDeviceSoftVersion->valuestring);
			}
			cJSON* pDeviceUiVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceUiVersion");
			if(pDeviceUiVersion && pDeviceUiVersion->valuestring)
			{
				snprintf(systemVer.DeviceUiVersion,sizeof(systemVer.DeviceUiVersion),"%s",pDeviceUiVersion->valuestring);
			}
			cJSON* pDeviceWebVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceWebVersion");
			if(pDeviceWebVersion && pDeviceWebVersion->valuestring)
			{
				snprintf(systemVer.DeviceWebVersion,sizeof(systemVer.DeviceWebVersion),"%s",pDeviceWebVersion->valuestring);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_SYNTEMVERCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&systemVer;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}


		cJSON* pBuildingCfg = cJSON_GetObjectItem(pdata, "BuildingCfg");
		if((pBuildingCfg) && (pBuildingCfg->type == cJSON_Object))
		{
			ACS_BUILDINGINFO_S building = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_BUILDINGCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_BUILDINGINFO_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&building;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pAControlIp = cJSON_GetObjectItem(pBuildingCfg, "AControlIp");
			if(pAControlIp && pAControlIp->valuestring)
			{
				snprintf(building.AControlIp,sizeof(building.AControlIp),"%s",pAControlIp->valuestring);
			}
			cJSON* pAreaID = cJSON_GetObjectItem(pBuildingCfg, "AreaID");
			if(pAreaID && pAreaID->valuestring)
			{
				snprintf(building.AreaID,sizeof(building.AreaID),"%s",pAreaID->valuestring);
			}
			cJSON* pBuildingID = cJSON_GetObjectItem(pBuildingCfg, "BuildingID");
			if(pBuildingID && pBuildingID->valuestring)
			{
				snprintf(building.BuildingID,sizeof(building.BuildingID),"%s",pBuildingID->valuestring);
			}
			cJSON* pBusDevName = cJSON_GetObjectItem(pBuildingCfg, "BusDevName");
			if(pBusDevName && pBusDevName->valuestring)
			{
				snprintf(building.BusDevName,sizeof(building.BusDevName),"%s",pBusDevName->valuestring);
			}
			cJSON* pCenterIP = cJSON_GetObjectItem(pBuildingCfg, "CenterIP");
			if(pCenterIP && pCenterIP->valuestring)
			{
				snprintf(building.CenterIP,sizeof(building.CenterIP),"%s",pCenterIP->valuestring);
			}
			cJSON* pDevNumber = cJSON_GetObjectItem(pBuildingCfg, "DevNumber");
			if(pDevNumber && pDevNumber->valuestring)
			{
				snprintf(building.DevNumber,sizeof(building.DevNumber),"%s",pDevNumber->valuestring);
			}
			cJSON* pRoomID = cJSON_GetObjectItem(pBuildingCfg, "RoomID");
			if(pRoomID && pRoomID->valuestring)
			{
				snprintf(building.RoomID,sizeof(building.RoomID),"%s",pRoomID->valuestring);
			}
			cJSON* pUnitID = cJSON_GetObjectItem(pBuildingCfg, "UnitID");
			if(pUnitID && pUnitID->valuestring)
			{
				snprintf(building.UnitID,sizeof(building.UnitID),"%s",pUnitID->valuestring);
			}
			cJSON* pBusDevType = cJSON_GetObjectItem(pBuildingCfg, "BusDevType");
			if(pBusDevType && pBusDevType->type == cJSON_Number)
			{
				building.bBusDevType = (pBusDevType->valueint);
			}			
			cJSON* pSlaveType = cJSON_GetObjectItem(pBuildingCfg, "SlaveType");
			if(pSlaveType && pSlaveType->type == cJSON_Number)
			{
				building.bSlaveType = (pSlaveType->valueint);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_BUILDINGCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&building;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pCallCfg = cJSON_GetObjectItem(pdata, "CallCfg");
		if((pCallCfg) && (pCallCfg->type == cJSON_Object))
		{
			ACS_TALKPARAM_S talkParam = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_CALLSETING;
			stAcsDevinfo.dwDataLen = sizeof(ACS_TALKPARAM_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&talkParam;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);				

			cJSON* pRingToneDuration = cJSON_GetObjectItem(pCallCfg, "RingToneDuration");//响铃时间
			if(pRingToneDuration && pRingToneDuration->type == cJSON_Number)
			{
				talkParam.dwRingToneDuration = (pRingToneDuration->valueint);
			}			
			cJSON* pTalkDuration = cJSON_GetObjectItem(pCallCfg, "TalkDuration");//通话时间
			if(pTalkDuration && pTalkDuration->type == cJSON_Number)
			{
				talkParam.dwTalkDuration = (pTalkDuration->valueint);
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_CALLSETING+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&talkParam;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
		}
		
		cJSON* pCallProtocolCfg = cJSON_GetObjectItem(pdata, "CallProtocolCfg");
		if((pCallProtocolCfg) && (pCallProtocolCfg->type == cJSON_Object))
		{
			ASC_DEVALLCFG_S CallProtocol = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_CALLPROTOCOL;
			stAcsDevinfo.dwDataLen = sizeof(ASC_DEVALLCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&CallProtocol;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);

			cJSON* prule = cJSON_GetObjectItem(pCallProtocolCfg, "callrule");
			if(prule) 
			{
				cJSON* powner = cJSON_GetObjectItem(prule,"owner"); 
				if(powner && powner->type==cJSON_Array)
				{
					ACS_INT nowners = cJSON_GetArraySize(powner);
					if(nowners < 4)
					{
						for(ACS_INT e = 0;e<nowners;e++)
						{
							cJSON *plevel = cJSON_GetArrayItem(powner,e);
							if((plevel) && (plevel->type == cJSON_Number))
							{
								CallProtocol.stCallRule.bOwner[e] = plevel->valueint;
								PRT_PRINT(ACS_DEBUG,"callrule e[%d].bOwner:%d;nlevel:%d",e,CallProtocol.stCallRule.bOwner[e], plevel->valueint);
							}
						}
					}
				}
				
				cJSON* pproperty = cJSON_GetObjectItem(prule,"property"); 
				if(pproperty && pproperty->type==cJSON_Array)
				{
					ACS_INT npropertys = cJSON_GetArraySize(pproperty);
					if(npropertys < 4)
					{
						for(ACS_INT e = 0;e<npropertys;e++)
						{
							cJSON *plevel2 = cJSON_GetArrayItem(pproperty,e);
							if((plevel2) && (plevel2->type == cJSON_Number))
							{
								CallProtocol.stCallRule.bProperty[e] = plevel2->valueint;
								PRT_PRINT(ACS_DEBUG,"callrule e[%d].bProperty:%d;nlevel:%d",e,CallProtocol.stCallRule.bProperty[e], plevel2->valueint);
							}
						}
					}
				}
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_CALLPROTOCOL+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&CallProtocol;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}
/**
		cJSON* pTelemaintenanceCfg = cJSON_GetObjectItem(pdata, "RemoteMaintenanceCfg");
		if((pTelemaintenanceCfg) && (pTelemaintenanceCfg->type == cJSON_Object))
		{
			cJSON* prtty = cJSON_GetObjectItem(pTelemaintenanceCfg, "rtty");
			if(prtty)
			{
				ACS_TELEMAINTENANCE_S telemaintenanceParam = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_TELEMAINTENANCE;
				stAcsDevinfo.dwDataLen = sizeof(ACS_TELEMAINTENANCE_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&telemaintenanceParam;
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				cJSON* penable = cJSON_GetObjectItem(prtty, "enable");//开关 默认关
				if(penable && penable->type == cJSON_Number)
				{
					telemaintenanceParam.dwEnable = (penable->valueint);
				}
				cJSON* pdMaintenanceDuration = cJSON_GetObjectItem(prtty, "maintenanceDuration");//维护时间 单位分钟
				if(pdMaintenanceDuration && pdMaintenanceDuration->type == cJSON_Number)
				{
					telemaintenanceParam.dMaintenanceDuration = (pdMaintenanceDuration->valueint);
				}

				cJSON* pport = cJSON_GetObjectItem(prtty, "port");//维护时间 单位分钟
				if(pport && pport->type == cJSON_Number)
				{
					telemaintenanceParam.dPort = (pport->valueint);
				}

				cJSON* paddress = cJSON_GetObjectItem(prtty, "address");
				if(paddress && paddress->valuestring)
				{
					snprintf(telemaintenanceParam.cServerAddress,sizeof(telemaintenanceParam.cServerAddress),"%s",paddress->valuestring);
				}
				
				cJSON* ptoken = cJSON_GetObjectItem(prtty, "token");
				if(ptoken && ptoken->valuestring)
				{
					snprintf(telemaintenanceParam.cToken,sizeof(telemaintenanceParam.cToken),"%s",ptoken->valuestring);
				}
				
				stAcsDevinfo.nType = ACS_DEV_PARAM_TELEMAINTENANCE+ACS_TYPE_SET_OFFSET;
				stAcsDevinfo.szData  = (ACS_CHAR *)&telemaintenanceParam;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			}
		}
**/		
		
#endif
	}
	else
	{
		if(pstMsgOut)
			sprintf(pstMsgOut->szDetails,"%s","data");
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return;// ACS_ERR_DATAERR;
	}
	return;
}

/**
* fun:GET数据回调 解析服务器返回的同步参数信息发给设备
* @CallData-in:服务器返回的同步广告信息
* @return:0-success;
**/
ACS_INT Curl_DealSyncParam_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}

	//ACS_DEVCFG_INFO_S stAcsDevinfo;
	//memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));

	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);

	cJSON* pRecvJsonRoot = cJSON_Parse(CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"MjAcsCloud SyncParam pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}

	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}
	
	Curl_DealSyncParam_json(pRecvJsonRoot,pstMsgOut);

	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return 0;
}



/**
* fun:Post数据回调 解析服务器返回的同步参数信息发给设备
* @CallData-in:服务器返回的同步广告信息
* @return:0-success;
**/
ACS_INT Curl_PostDealSyncParam_Back(ACS_CHAR *CallData,ACS_CHAR *pMsgOut)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"CallData null");
		return ACS_ERR_DATANULL;
	}

	//ACS_DEVCFG_INFO_S stAcsDevinfo;
	//memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));

	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);

	cJSON* pRecvJsonRoot = cJSON_Parse(CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"PostDealSyncParam pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}

	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		//ACS_MESSAGE_S *pstMsgOut = (ACS_MESSAGE_S *)pMsgOut;
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}
#if 0	
	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* pHealthcloudCfgV4 = cJSON_GetObjectItem(pdata, "healthcloudCfgV4");
		if((pHealthcloudCfgV4) && (pHealthcloudCfgV4->type == cJSON_Object))
		{
			ACS_HCODECLOUD_S stAcsHcodeCloud;
			memset(&stAcsHcodeCloud,0,sizeof(ACS_HCODECLOUD_S));
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_HCODECLOUD;
			stAcsDevinfo.dwDataLen = sizeof(ACS_HCODECLOUD_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsHcodeCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pSceneSerialID = cJSON_GetObjectItem(pHealthcloudCfgV4, "SceneSerialID");
			if(pSceneSerialID && pSceneSerialID->valuestring)
			{
				snprintf(stAcsHcodeCloud.szSceneSerialID,sizeof(stAcsHcodeCloud.szSceneSerialID),"%s",pSceneSerialID->valuestring);
			}
	
			cJSON* pServerIP = cJSON_GetObjectItem(pHealthcloudCfgV4, "ServerIP");
			if(pServerIP && pServerIP->valuestring)
			{
				snprintf(stAcsHcodeCloud.szServerIP,sizeof(stAcsHcodeCloud.szServerIP),"%s",pServerIP->valuestring);
			}

			cJSON* pAuthKey = cJSON_GetObjectItem(pHealthcloudCfgV4, "AuthKey");
			if(pAuthKey && pAuthKey->valuestring)
			{
				snprintf(stAcsHcodeCloud.szAuthKey,sizeof(stAcsHcodeCloud.szAuthKey),"%s",pAuthKey->valuestring);
			}

			cJSON* pAuthIV = cJSON_GetObjectItem(pHealthcloudCfgV4, "AuthIV");
			if(pAuthIV && pAuthIV->valuestring)
			{
				snprintf(stAcsHcodeCloud.szAuthIV,sizeof(stAcsHcodeCloud.szAuthIV),"%s",pAuthIV->valuestring);
			}

			cJSON* pSceneCategory = cJSON_GetObjectItem(pHealthcloudCfgV4, "SceneCategory");
			if(pSceneCategory && pSceneCategory->valuestring)
			{
				snprintf(stAcsHcodeCloud.szSceneCategory,sizeof(stAcsHcodeCloud.szSceneCategory),"%s",pSceneCategory->valuestring);
			}

			stAcsDevinfo.nType = ACS_DEV_PARAM_HCODECLOUD+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.dwDataLen = sizeof(ACS_HCODECLOUD_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsHcodeCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}
		
		cJSON* pCloudCfg = cJSON_GetObjectItem(pdata, "CloudCfg");
		if((pCloudCfg) && (pCloudCfg->type == cJSON_Object))
		{
			ACS_CLOUDCFG_S stAcsCloud;
			memset(&stAcsCloud,0,sizeof(ACS_CLOUDCFG_S));
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_CLOUDCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_CLOUDCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);		

			cJSON* pCloudEnable = cJSON_GetObjectItem(pCloudCfg, "CloudEnable");
			if(pCloudEnable && pCloudEnable->type == cJSON_Number)
			{
				stAcsCloud.byEnable = (pCloudEnable->valueint?1:0);
			}

			cJSON* pCloudIPAddr = cJSON_GetObjectItem(pCloudCfg, "CloudIPAddrCfg");
			if(pCloudIPAddr && pCloudIPAddr->valuestring)
			{
				snprintf(stAcsCloud.szCloudIPAddr,sizeof(stAcsCloud.szCloudIPAddr),"%s",pCloudIPAddr->valuestring);
			}

			cJSON* pCloudID = cJSON_GetObjectItem(pCloudCfg, "CloudID");
			if(pCloudID && pCloudID->valuestring)
			{
				snprintf(stAcsCloud.szCloudID,sizeof(stAcsCloud.szCloudID),"%s",pCloudID->valuestring);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_CLOUDCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCloud;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pAcsCfg = cJSON_GetObjectItem(pdata, "AcsCfg");
		if((pAcsCfg) && (pAcsCfg->type == cJSON_Object))
		{
			ACS_ACSCFG_S stAcsCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_ACSCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_ACSCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pAcsState = cJSON_GetObjectItem(pAcsCfg, "AcsState");
			if(pAcsState && pAcsState->type == cJSON_Number)
			{
				stAcsCfg.dwAcsState = pAcsState->valueint;
				g_bAcsState = stAcsCfg.dwAcsState;
			}

			cJSON* pRecoredPicEnable = cJSON_GetObjectItem(pAcsCfg, "RecoredPicEnable");
			if(pRecoredPicEnable && pRecoredPicEnable->type == cJSON_Number)
			{
				stAcsCfg.dwRecoredPicEnable = pRecoredPicEnable->valueint;
			}

			cJSON* pStrangeRecEnable = cJSON_GetObjectItem(pAcsCfg, "StrangeRecEnable");
			if(pStrangeRecEnable && pStrangeRecEnable->type == cJSON_Number)
			{
				stAcsCfg.dwStrangeRecEnable = pStrangeRecEnable->valueint;
			}

			cJSON* pStrangeTik = cJSON_GetObjectItem(pAcsCfg, "StrangeTik");
			if(pStrangeTik && pStrangeTik->type == cJSON_Number)
			{
				stAcsCfg.dwStrangeTik = pStrangeTik->valueint;
			}

			cJSON* pRelay1DelayTime = cJSON_GetObjectItem(pAcsCfg, "Relay1DelayTime");
			if(pRelay1DelayTime && pRelay1DelayTime->type == cJSON_Number)
			{
				stAcsCfg.dwDelay1Time = pRelay1DelayTime->valueint;
			}

			cJSON* pRelay1IoType = cJSON_GetObjectItem(pAcsCfg, "Relay1IoType");
			if(pRelay1IoType && pRelay1IoType->type == cJSON_Number)
			{
				stAcsCfg.dwIo1Type = pRelay1IoType->valueint;
			}
			
			cJSON* pConfirmMode = cJSON_GetObjectItem(pAcsCfg, "ConfirmMode");
			if(pConfirmMode && pConfirmMode->type == cJSON_Number)
			{
				stAcsCfg.dwConfirmMode = pConfirmMode->valueint;
			}

			cJSON* pConfirmWayMask = cJSON_GetObjectItem(pAcsCfg, "ConfirmWayMask");
			if(pConfirmWayMask && pConfirmWayMask->type == cJSON_Number)
			{
				stAcsCfg.dwConfirmWayMask = pConfirmWayMask->valueint;
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_ACSCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}
#if 1

		cJSON* pAudioStreamCfg = cJSON_GetObjectItem(pdata, "AudioStream");
		if((pAudioStreamCfg) && (pAudioStreamCfg->type == cJSON_Object))
		{
			ACS_AUDIOSTREAM_S stAudiosCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_AUDIOCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_AUDIOSTREAM_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAudiosCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pEnable = cJSON_GetObjectItem(pAudioStreamCfg, "Enable");
			if(pEnable && pEnable->type == cJSON_Number)
			{
				stAudiosCfg.byEnable = pEnable->valueint;
			}
			cJSON* pVolumeIn = cJSON_GetObjectItem(pAudioStreamCfg, "VolumeIn");
			if(pVolumeIn && pVolumeIn->type == cJSON_Number)
			{
				stAudiosCfg.byVolumeIn = pVolumeIn->valueint;
			}
			cJSON* pVolumeOut = cJSON_GetObjectItem(pAudioStreamCfg, "VolumeOut");
			if(pVolumeOut && pVolumeOut->type == cJSON_Number)
			{
				stAudiosCfg.byVolumeOut = pVolumeOut->valueint;
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_AUDIOCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stAudiosCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pCameraCfg = cJSON_GetObjectItem(pdata, "CameraConfig");
		if((pCameraCfg) && (pCameraCfg->type == cJSON_Object))
		{
			ACS_CAMCFG_S stCameraCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_CAMCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_CAMCFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stCameraCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pDeviceName = cJSON_GetObjectItem(pCameraCfg, "DeviceName");
			if(pDeviceName && pDeviceName->valuestring)
			{
				snprintf(stCameraCfg.szDevName,sizeof(stCameraCfg.szDevName),"%s",pDeviceName->valuestring);
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_CAMCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stCameraCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pSfaceCfg = cJSON_GetObjectItem(pdata, "SfaceCfg");
		if((pSfaceCfg) && (pSfaceCfg->type == cJSON_Object))
		{
			ACS_SFACECFG_S stsFaceCfg = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
			stAcsDevinfo.nType = ACS_DEV_PARAM_SFACECFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_SFACECFG_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&stsFaceCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pAcsScene = cJSON_GetObjectItem(pSfaceCfg, "AcsScene");
			if(pAcsScene && pAcsScene->type == cJSON_Number)
			{
				stsFaceCfg.dwAcsScene = pAcsScene->valueint;
			}
			cJSON* pCmpSingleThreshold = cJSON_GetObjectItem(pSfaceCfg, "CmpSingleThreshold");
			if(pCmpSingleThreshold && pCmpSingleThreshold->type == cJSON_Number)
			{
				stsFaceCfg.dwCmpSingleThreshold = pCmpSingleThreshold->valueint;
			}
			cJSON* pCmpThreshold = cJSON_GetObjectItem(pSfaceCfg, "CmpThreshold");
			if(pCmpThreshold && pCmpThreshold->type == cJSON_Number)
			{
				stsFaceCfg.dwCmpThreshold = pCmpThreshold->valueint;
			}
			cJSON* pFaceMaxSize = cJSON_GetObjectItem(pSfaceCfg, "FaceMaxSize");
			if(pFaceMaxSize && pFaceMaxSize->type == cJSON_Number)
			{
				stsFaceCfg.dwFaceMaxSize = pFaceMaxSize->valueint;
			}
			cJSON* pFaceMinSize = cJSON_GetObjectItem(pSfaceCfg, "FaceMinSize");
			if(pFaceMinSize && pFaceMinSize->type == cJSON_Number)
			{
				stsFaceCfg.dwFaceMinSize = pFaceMinSize->valueint;
			}
			cJSON* pFaceSensitivity = cJSON_GetObjectItem(pSfaceCfg, "FaceSensitivity");
			if(pFaceSensitivity && pFaceSensitivity->type == cJSON_Number)
			{
				stsFaceCfg.dwFaceSensitivity = pFaceSensitivity->valueint;
			}
			cJSON* pHacknessenble = cJSON_GetObjectItem(pSfaceCfg, "Hacknessenble");
			if(pHacknessenble && pHacknessenble->type == cJSON_Number)
			{
				stsFaceCfg.dwHacknessenble = pHacknessenble->valueint;
			}
			cJSON* pHacknessthreshold = cJSON_GetObjectItem(pSfaceCfg, "Hacknessthreshold");
			if(pHacknessthreshold && pHacknessthreshold->type == cJSON_Number)
			{
				stsFaceCfg.dwHacknessthreshold = pHacknessthreshold->valueint;
			}
			cJSON* pMaskDetec = cJSON_GetObjectItem(pSfaceCfg, "MaskDetec");
			if(pMaskDetec && pMaskDetec->type == cJSON_Number)
			{
				stsFaceCfg.dwMaskEnable = pMaskDetec->valueint;
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_SFACECFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&stsFaceCfg;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
        }


		cJSON* pWirednetwork = cJSON_GetObjectItem(pdata, "Wirednetwork");
		if((pWirednetwork) && (pWirednetwork->type == cJSON_Object))
		{
			ACS_WIREDNET_S wirednet = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_WIREDNETCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_WIREDNET_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&wirednet;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pipAddress = cJSON_GetObjectItem(pWirednetwork, "ipAddress");
			if(pipAddress && pipAddress->valuestring)
			{
				snprintf(wirednet.ipAddress,sizeof(wirednet.ipAddress),"%s",pipAddress->valuestring);
			}
			cJSON* pdhcp = cJSON_GetObjectItem(pWirednetwork, "dhcp");
			if(pdhcp && pdhcp->type == cJSON_Number)
			{
				wirednet.bDHCP = (pdhcp->valueint?1:0);
			}
			cJSON* psubnetMask = cJSON_GetObjectItem(pWirednetwork, "subnetMask");
			if(psubnetMask && psubnetMask->valuestring)
			{
				snprintf(wirednet.subnetMask,sizeof(wirednet.subnetMask),"%s",psubnetMask->valuestring);
			}
			cJSON* pSecondaryDNS = cJSON_GetObjectItem(pWirednetwork, "SecondaryDNS");
			if(pSecondaryDNS && pSecondaryDNS->valuestring)
			{
				snprintf(wirednet.SecondaryDNS,sizeof(wirednet.SecondaryDNS),"%s",pSecondaryDNS->valuestring);
			}
			cJSON* pPrimaryDNS = cJSON_GetObjectItem(pWirednetwork, "PrimaryDNS");
			if(pPrimaryDNS && pPrimaryDNS->valuestring)
			{
				snprintf(wirednet.PrimaryDNS,sizeof(wirednet.PrimaryDNS),"%s",pPrimaryDNS->valuestring);
			}
			
			cJSON* pMACAddress = cJSON_GetObjectItem(pWirednetwork, "MACAddress");
			if(pMACAddress && pMACAddress->valuestring)
			{
				snprintf(wirednet.MACAddress,sizeof(wirednet.MACAddress),"%s",pMACAddress->valuestring);
			}
			
			cJSON* pDefaultGateway = cJSON_GetObjectItem(pWirednetwork, "DefaultGateway");
			if(pDefaultGateway && pDefaultGateway->valuestring)
			{
				snprintf(wirednet.DefaultGateway,sizeof(wirednet.DefaultGateway),"%s",pDefaultGateway->valuestring);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_WIREDNETCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&wirednet;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}


		cJSON* pSysteminfoVersion = cJSON_GetObjectItem(pdata, "SysteminfoVersion");
		if((pSysteminfoVersion) && (pSysteminfoVersion->type == cJSON_Object))
		{
			ACS_SYSTEMVERSION_S systemVer = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_SYNTEMVERCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_SYSTEMVERSION_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&systemVer;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* puuid = cJSON_GetObjectItem(pSysteminfoVersion, "uuid");
			if(puuid && puuid->valuestring)
			{
				snprintf(systemVer.uuid,sizeof(systemVer.uuid),"%s",puuid->valuestring);
			}
			cJSON* pcpuid = cJSON_GetObjectItem(pSysteminfoVersion, "cpuid");
			if(pcpuid && pcpuid->valuestring)
			{
				snprintf(systemVer.cpuid,sizeof(systemVer.cpuid),"%s",pcpuid->valuestring);
			}
			cJSON* pDeviceAlgoVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceAlgoVersion");
			if(pDeviceAlgoVersion && pDeviceAlgoVersion->valuestring)
			{
				snprintf(systemVer.DeviceAlgoVersion,sizeof(systemVer.DeviceAlgoVersion),"%s",pDeviceAlgoVersion->valuestring);
			}
			cJSON* pDeviceOnvifVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceOnvifVersion");
			if(pDeviceOnvifVersion && pDeviceOnvifVersion->valuestring)
			{
				snprintf(systemVer.DeviceOnvifVersion,sizeof(systemVer.DeviceOnvifVersion),"%s",pDeviceOnvifVersion->valuestring);
			}
			cJSON* pDeviceSoftVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceSoftVersion");
			if(pDeviceSoftVersion && pDeviceSoftVersion->valuestring)
			{
				snprintf(systemVer.DeviceSoftVersion,sizeof(systemVer.DeviceSoftVersion),"%s",pDeviceSoftVersion->valuestring);
			}
			cJSON* pDeviceUiVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceUiVersion");
			if(pDeviceUiVersion && pDeviceUiVersion->valuestring)
			{
				snprintf(systemVer.DeviceUiVersion,sizeof(systemVer.DeviceUiVersion),"%s",pDeviceUiVersion->valuestring);
			}
			cJSON* pDeviceWebVersion = cJSON_GetObjectItem(pSysteminfoVersion, "DeviceWebVersion");
			if(pDeviceWebVersion && pDeviceWebVersion->valuestring)
			{
				snprintf(systemVer.DeviceWebVersion,sizeof(systemVer.DeviceWebVersion),"%s",pDeviceWebVersion->valuestring);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_SYNTEMVERCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&systemVer;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}


		cJSON* pBuildingCfg = cJSON_GetObjectItem(pdata, "BuildingCfg");
		if((pBuildingCfg) && (pBuildingCfg->type == cJSON_Object))
		{
			ACS_BUILDINGINFO_S building = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_BUILDINGCFG;
			stAcsDevinfo.dwDataLen = sizeof(ACS_BUILDINGINFO_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&building;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
			
			cJSON* pAControlIp = cJSON_GetObjectItem(pBuildingCfg, "AControlIp");
			if(pAControlIp && pAControlIp->valuestring)
			{
				snprintf(building.AControlIp,sizeof(building.AControlIp),"%s",pAControlIp->valuestring);
			}
			cJSON* pAreaID = cJSON_GetObjectItem(pBuildingCfg, "AreaID");
			if(pAreaID && pAreaID->valuestring)
			{
				snprintf(building.AreaID,sizeof(building.AreaID),"%s",pAreaID->valuestring);
			}
			cJSON* pBuildingID = cJSON_GetObjectItem(pBuildingCfg, "BuildingID");
			if(pBuildingID && pBuildingID->valuestring)
			{
				snprintf(building.BuildingID,sizeof(building.BuildingID),"%s",pBuildingID->valuestring);
			}
			cJSON* pBusDevName = cJSON_GetObjectItem(pBuildingCfg, "BusDevName");
			if(pBusDevName && pBusDevName->valuestring)
			{
				snprintf(building.BusDevName,sizeof(building.BusDevName),"%s",pBusDevName->valuestring);
			}
			cJSON* pCenterIP = cJSON_GetObjectItem(pBuildingCfg, "CenterIP");
			if(pCenterIP && pCenterIP->valuestring)
			{
				snprintf(building.CenterIP,sizeof(building.CenterIP),"%s",pCenterIP->valuestring);
			}
			cJSON* pDevNumber = cJSON_GetObjectItem(pBuildingCfg, "DevNumber");
			if(pDevNumber && pDevNumber->valuestring)
			{
				snprintf(building.DevNumber,sizeof(building.DevNumber),"%s",pDevNumber->valuestring);
			}
			cJSON* pRoomID = cJSON_GetObjectItem(pBuildingCfg, "RoomID");
			if(pRoomID && pRoomID->valuestring)
			{
				snprintf(building.RoomID,sizeof(building.RoomID),"%s",pRoomID->valuestring);
			}
			cJSON* pUnitID = cJSON_GetObjectItem(pBuildingCfg, "UnitID");
			if(pUnitID && pUnitID->valuestring)
			{
				snprintf(building.UnitID,sizeof(building.UnitID),"%s",pUnitID->valuestring);
			}
			cJSON* pBusDevType = cJSON_GetObjectItem(pBuildingCfg, "BusDevType");
			if(pBusDevType && pBusDevType->type == cJSON_Number)
			{
				building.bBusDevType = (pBusDevType->valueint);
			}			
			cJSON* pSlaveType = cJSON_GetObjectItem(pBuildingCfg, "SlaveType");
			if(pSlaveType && pSlaveType->type == cJSON_Number)
			{
				building.bSlaveType = (pSlaveType->valueint);
			}
			stAcsDevinfo.nType = ACS_DEV_PARAM_BUILDINGCFG+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&building;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		cJSON* pCallCfg = cJSON_GetObjectItem(pdata, "CallCfg");
		if((pCallCfg) && (pCallCfg->type == cJSON_Object))
		{
			ACS_TALKPARAM_S talkParam = {0};
			memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
			stAcsDevinfo.nType = ACS_DEV_PARAM_CALLSETING;
			stAcsDevinfo.dwDataLen = sizeof(ACS_TALKPARAM_S);
			stAcsDevinfo.szData  = (ACS_CHAR *)&talkParam;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);				

			cJSON* pRingToneDuration = cJSON_GetObjectItem(pCallCfg, "RingToneDuration");//响铃时间
			if(pRingToneDuration && pRingToneDuration->type == cJSON_Number)
			{
				talkParam.dwRingToneDuration = (pRingToneDuration->valueint);
			}			
			cJSON* pTalkDuration = cJSON_GetObjectItem(pCallCfg, "TalkDuration");//通话时间
			if(pTalkDuration && pTalkDuration->type == cJSON_Number)
			{
				talkParam.dwTalkDuration = (pTalkDuration->valueint);
			}
			
			stAcsDevinfo.nType = ACS_DEV_PARAM_CALLSETING+ACS_TYPE_SET_OFFSET;
			stAcsDevinfo.szData  = (ACS_CHAR *)&talkParam;		
			ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);	
		}

		
#endif
	}
	else
	{
	//	sprintf(pstMsgOut->szDetails,"%s","data");
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
#endif
	Curl_DealSyncParam_json(pRecvJsonRoot,NULL);

	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return 0;
}





/**
* fun:POST数据回调 解析服务器返回的健康码信息
* @CallData-in:服务器返回的健康码信息
* @healthInfo-out:解析到的健康码信息
* @return:0-success;
**/
ACS_INT Curl_DealHealthCodeInfo_Back(ACS_CHAR *CallData, ACS_CHAR *healthInfo)
{
	if(CallData == NULL)
	{
	   return ACS_ERR_DATANULL;
	}
	
	PRT_PRINT(ACS_DEBUG,"healthCode_callback: %s",CallData);
	ACS_HEALTHCODE_INFO_S *healthCodeInfo = (ACS_HEALTHCODE_INFO_S *)healthInfo;
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
		return ACS_ERR_MALLOCFAIL;

	cJSON* pRetCode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if (pRetCode == NULL)
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
	
	if(pRetCode->valueint != ACS_JSON_CODE_SUCCESS)
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		if(pRetCode->valueint == 500)//未授权
			return -2;
		return ACS_ERR_PARAMERR;
	}

	cJSON* ptimestamp = cJSON_GetObjectItem(pRecvJsonRoot,"timestamp");
	cJSON* pmsg = cJSON_GetObjectItem(pRecvJsonRoot,"message");
	if (NULL != pmsg && NULL != ptimestamp)
	{
		PRT_PRINT(ACS_DEBUG,"MjCloudLib %s time:%d code:%d msg=%s",__func__,ptimestamp->valueint, pRetCode->valueint ,pmsg->valuestring);
	}
	
	cJSON* pData = cJSON_GetObjectItem(pRecvJsonRoot, "data");
	if(pData)
	{
		cJSON* pName = cJSON_GetObjectItem(pData, "name");
		if(pName && pName->valuestring)
		{
			memcpy(healthCodeInfo->name, pName->valuestring, sizeof(healthCodeInfo->name));
		}
		
		cJSON* pId = cJSON_GetObjectItem(pData, "id");
		if(pId && pId->valuestring)
		{
			memcpy(healthCodeInfo->id, pId->valuestring, sizeof(healthCodeInfo->id));
		}
		
		cJSON* pStatus = cJSON_GetObjectItem(pData, "status");
		if(pStatus && pStatus->valuestring)
		{
			memcpy(healthCodeInfo->status, pStatus->valuestring, sizeof(healthCodeInfo->status));
		}
		
		cJSON* pDate = cJSON_GetObjectItem(pData, "date");
		if(pDate && pDate->valuestring)
		{
			memcpy(healthCodeInfo->date, pDate->valuestring, sizeof(healthCodeInfo->date));
		}
		
		cJSON* pHours= cJSON_GetObjectItem(pData, "hours");
		if(pHours && (pHours->type == cJSON_Number))
		{
			healthCodeInfo->hours = pHours->valueint;
		}
		
		cJSON* pVaccine = cJSON_GetObjectItem(pData, "vaccine");
		if(pVaccine && (pVaccine->type == cJSON_Number))
		{
			healthCodeInfo->vaccine = pVaccine->valueint;
		}
		
		cJSON* pVaccineDate = cJSON_GetObjectItem(pData, "vaccineDate");
		if(pVaccineDate && pVaccineDate->valuestring)
		{
			memcpy(healthCodeInfo->vaccineDate, pVaccineDate->valuestring, sizeof(healthCodeInfo->vaccineDate));
		}
		
		cJSON* pReason = cJSON_GetObjectItem(pData, "reason");
		if(pReason && pReason->valuestring)
		{
			memcpy(healthCodeInfo->reason, pReason->valuestring, sizeof(healthCodeInfo->reason));
		}
		
		cJSON* pStopOverCity = cJSON_GetObjectItem(pData, "stopOverCity");
		if(pStopOverCity && pStopOverCity->valuestring)
		{
			memcpy(healthCodeInfo->stopOverCity, pStopOverCity->valuestring, sizeof(healthCodeInfo->stopOverCity));
		}
	}
	else
	{
		ACS_WLOG("data null");
		PRT_PRINT(ACS_DEBUG,"data null");
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	return ACS_ERR_SUCCESS;

}


/**
* fun:POST数据回调 解析服务器返回的数据上传接口
* @CallData-in:服务器返回的健康码信息
* @healthInfo-out:解析到的健康码信息
* @return:0-success;
**/
ACS_INT Curl_DealDataCheckOnline_Back(ACS_CHAR *CallData, ACS_CHAR *DataUploadOut)
{
	if(CallData == NULL)
	{
	   return ACS_ERR_DATANULL;
	}
	
	//PRT_PRINT(ACS_DEBUG,"DatacheckOnline_callback: %s",CallData);

	ACS_WLOG("DatacheckOnline_callback:%s",CallData);

	
	ACS_DATACHECKONLINE_RESULT_S *pDataUpload = (ACS_DATACHECKONLINE_RESULT_S *)DataUploadOut;
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if(pRecvJsonRoot == NULL)
		return ACS_ERR_PARAMNULL;

	cJSON* pRetCode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if((pRetCode == NULL) || (pRetCode->type != cJSON_Number))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
	
	if(pRetCode->valueint != ACS_JSON_CODE_SUCCESS)
	{
		pDataUpload->retCode = pRetCode->valueint;
		cJSON* pmessage = cJSON_GetObjectItem(pRecvJsonRoot,"message");
		if (pmessage && pmessage->valuestring)
		{
			snprintf(pDataUpload->message,sizeof(pDataUpload->message),"%s",pmessage->valuestring);
		}
		ACS_WLOG("retCode:%d;message:%s",pRetCode->valueint,pDataUpload->message);
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}

	cJSON* ptimestamp = cJSON_GetObjectItem(pRecvJsonRoot,"timestamp");
	if (ptimestamp && ptimestamp->valuestring)
	{
		snprintf(pDataUpload->timestamp,sizeof(pDataUpload->timestamp),"%s",ptimestamp->valuestring);
	}
	
	cJSON* pData = cJSON_GetObjectItem(pRecvJsonRoot, "data");
	if(pData)
	{
		cJSON* pName = cJSON_GetObjectItem(pData, "name");
		if(pName && pName->valuestring)
		{
			snprintf(pDataUpload->name,sizeof(pDataUpload->name),pName->valuestring);
		}
		
		cJSON* puserId = cJSON_GetObjectItem(pData, "userId");
		if(puserId && puserId->valuestring)
		{
			snprintf(pDataUpload->userId,sizeof(pDataUpload->userId),puserId->valuestring);
		}

		cJSON* popendoor = cJSON_GetObjectItem(pData, "opendoor");
		if((popendoor) && (popendoor->type == cJSON_Number))
		{
			pDataUpload->opendoorEn = popendoor->valueint;
		}

		cJSON* presult = cJSON_GetObjectItem(pData, "result");
		if((presult) && (presult->type == cJSON_Number))
		{
			pDataUpload->result = presult->valueint;
		}
		
		cJSON* pmsg = cJSON_GetObjectItem(pData, "msg");
		if(pmsg && pmsg->valuestring)
		{
			snprintf(pDataUpload->msg,sizeof(pDataUpload->msg),pmsg->valuestring);
		}
	}
	else
	{
		ACS_WLOG("data null");
		PRT_PRINT(ACS_DEBUG,"data null");
	}
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	return ACS_ERR_SUCCESS;
}


/**
* fun:POST数据回调 解析服务器返回的检验结果
* @CallData-in:服务器返回的检验结果
* @callRes-out:解析到的结果
* @return:0-success;
**/
ACS_INT Curl_DealCallCheck_Back(ACS_CHAR* CallData, ACS_CHAR* callRes)
{
	ACS_CALL_CHECK_RESULT_S* pstCallRes;
	pstCallRes = (ACS_CALL_CHECK_RESULT_S*)callRes;
	if(CallData == NULL)
		return ACS_ERR_DATANULL;

	PRT_PRINT(ACS_DEBUG,"Curl_Call_Check_callback: %s", CallData);
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR*)CallData);
	if (pRecvJsonRoot == NULL)
		return ACS_ERR_PARAMNULL;

	cJSON* pretCode = cJSON_GetObjectItem(pRecvJsonRoot, "retCode");
	if((pretCode)&&(pretCode->type == cJSON_Number))
	{
		pstCallRes->retCode = pretCode->valueint;
	}

	cJSON* pRecvData = cJSON_GetObjectItem(pRecvJsonRoot, "data");
	if(pRecvData) 
	{
		cJSON* pringtime = cJSON_GetObjectItem(pRecvData, "ringtime");
		if((pringtime)&&(pringtime->type == cJSON_Number))
		{
			pstCallRes->ringtime = pringtime->valueint;
		}

		cJSON* presult = cJSON_GetObjectItem(pRecvData, "result");
		if((presult)&&(presult->type == cJSON_Number))
		{
			pstCallRes->result = presult->valueint;
		}
		else
		{
			pstCallRes->result = -1;
		}
		
		cJSON* pmsg = cJSON_GetObjectItem(pRecvData, "msg");
		if(pmsg && pmsg->valuestring)
		{
			snprintf(pstCallRes->message, 128, "%s", pmsg->valuestring);//memset(pstCallRes->message, 0, 128);
		}

		if((ACS_ERRSER_SUCCESS == pstCallRes->result) || (ACS_ERRSER_CALLSUCCESS == pstCallRes->result))
		{
			cJSON* pcallid = cJSON_GetObjectItem(pRecvData,"callid"); 
			if(pcallid && pcallid->type==cJSON_Array)
			{
				ACS_INT nowners = cJSON_GetArraySize(pcallid);
				
				if(nowners > 0)
				{
					PRT_PRINT(ACS_DEBUG,"nowners [%d]",nowners);
					ACS_CALL_USERID_S *pCall = (ACS_CALL_USERID_S *)malloc(nowners*sizeof(ACS_CALL_USERID_S));
					if(pCall)
					{
						pstCallRes->dwDataLen = nowners;
						pstCallRes->pstData = (ACS_CHAR*)pCall;
						for(ACS_INT e = 0;e<nowners;e++)
						{
							cJSON *pscallid = cJSON_GetArrayItem(pcallid,e);
							if((pscallid) && (pscallid->valuestring))
							{
								snprintf(pCall[e].callid, 64, "%s", pscallid->valuestring);
								PRT_PRINT(ACS_DEBUG,"callid e[%d].callid:%s;callid:%s",e,pCall[e].callid, pscallid->valuestring);
							}
						}
					}
					else
					{
						ACS_WLOG("DealCallCheck malloc fail");
					}
				}
			}
		}

		cJSON* pcheckItem = NULL;
		cJSON* pwebrtcArray = cJSON_GetObjectItem(pRecvData, "webrtc");
		if((pwebrtcArray) && (pwebrtcArray->type == cJSON_Array))
		{
			for(ACS_INT i = 0; i < cJSON_GetArraySize(pwebrtcArray); i++)
			{
				pcheckItem = cJSON_GetArrayItem(pwebrtcArray, i);
				if(pcheckItem) 
				{
					cJSON* ptemp = cJSON_GetObjectItem(pcheckItem, "userType");
					if (ptemp  && ptemp->valuestring && strcmp(ptemp->valuestring, "O") == 0)
						break;
				}
				pcheckItem = NULL;
			}

			if(!pcheckItem)
				pcheckItem = cJSON_GetArrayItem(pwebrtcArray, 0); //如果没有业主,默认呼叫第一个

			if((pcheckItem)&&(pcheckItem->type == cJSON_Object))
			{
				cJSON* puserType = cJSON_GetObjectItem(pcheckItem, "userType");
				if(puserType && puserType->valuestring)
				{
					snprintf(pstCallRes->stCall.userType, 16, "%s", puserType->valuestring);
				}

				cJSON* pphone = cJSON_GetObjectItem(pcheckItem, "phone");
				if(pphone && pphone->valuestring)
				{
					snprintf(pstCallRes->stCall.phone, 16, "%s", pphone->valuestring);
				}

				cJSON* prealName = cJSON_GetObjectItem(pcheckItem, "realName");
				if(prealName && prealName->valuestring)
				{
					snprintf(pstCallRes->stCall.realName, 32, "%s", prealName->valuestring);
				}
			}
		}
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ACS_ERR_SUCCESS;
}

/**
* fun:POST数据回调 解析服务器返回的检验结果
* @inData-in:服务器返回的健康码信息
* @outRes-out:解析到的健康码信息
* @return:0-success;
**/
ACS_INT Curl_DealCallRequest_Back(ACS_CHAR* inData, ACS_CHAR* outRes)
{
	ACS_CALL_RESULT_S* pstResult = NULL;
	pstResult = (ACS_CALL_RESULT_S*)outRes;
	if (pstResult == NULL)
		return ACS_ERR_PARAMNULL;

	PRT_PRINT(ACS_DEBUG,"Curl_Call_Request_callback: %s", inData);
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR*)inData);
	if (pRecvJsonRoot == NULL)
		return ACS_ERR_PARAMNULL;

	cJSON* pret = cJSON_GetObjectItem(pRecvJsonRoot, "retCode");
	if((pret) && (pret->type == cJSON_Number))
	{
		pstResult->retCode = pret->valueint;
	}

	cJSON* pRecvData = cJSON_GetObjectItem(pRecvJsonRoot, "data");
	if (pRecvData) 
	{
		cJSON* ptemp = cJSON_GetObjectItem(pRecvData, "result");
		if((ptemp) && (ptemp->type == cJSON_Number))
		{
			pstResult->nResult = ptemp->valueint;
		}
	}
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	return ACS_ERR_SUCCESS;
}


/**
* fun:GET数据回调 解析服务器返回的同步信息发给设备
* @CallData-in:服务器返回的同步信息
* @return:0-success;
**/
ACS_INT Curl_DealSyncElevatorRules_Back(ACS_CHAR *CallData ,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_ERROR,"CallData null");
		return ACS_ERR_DATANULL;
	}
	
	if(pCallOut == NULL)
	{
		PRT_PRINT(ACS_ERROR,"pCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}
	
	PRT_PRINT(ACS_DEBUG,"CallData:%s",CallData);
	
	ACS_RECORD_S *pstCallDataOut = (ACS_RECORD_S *)pCallOut;
	if(pstCallDataOut == NULL)
	{
		PRT_PRINT(ACS_ERROR,"pstCallDataOut null");
		return ACS_ERR_PARAMNULL;
	}

	ACS_INT i = 0;
	ACS_INT j = 0;
	ACS_INT eleRulesNum = 0;
	ACS_INT floorsNum = 0;
	ACS_INT ret = 0;
	ACS_CHAR eField[128] = {0};
	ACS_CHAR sztime[16] = {0};
	ACS_ELERULES_INFO_S *pstEleRules = NULL;
	ACS_FLOORS_INFO_S *pstFloorss = NULL;
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if (pRecvJsonRoot == NULL)
	{
		PRT_PRINT(ACS_ERROR,"prules pRecvJsonRoot null");
		return ACS_ERR_DATANULL;
	}
		
	cJSON* pcode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if(NULL == pcode)
	{
		PRT_PRINT(ACS_ERROR,"not retCode");
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		sprintf(pstMsgOut->szDetails,"%s","retCode");
		return ACS_ERR_DATAERR;
	}

	if((cJSON_Number == pcode->type) && (ACS_ERRSER_NODATASYNC == pcode->valueint))
	{
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_NODATA;
	}
	
	cJSON* pdata = cJSON_GetObjectItem(pRecvJsonRoot,"data");
	if(pdata)
	{
		cJSON* peField = cJSON_GetObjectItem(pdata,"eField");
		if((peField) && (peField->valuestring))
		{
			PRT_PRINT(ACS_DEBUG,"peField = %s" ,peField->valuestring);
			ACS_WLOG("syncEleRules BackData peField = %s" ,peField->valuestring);
			snprintf(eField,128,peField->valuestring);
		}
			
		cJSON* ptimestamp = cJSON_GetObjectItem(pdata,"timestamp");
		if((ptimestamp) && (ptimestamp->valuestring))
		{
			PRT_PRINT(ACS_INFO,"pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACS_WLOG("syncEleRules BackData pptimestamp->valuestring = %s" ,ptimestamp->valuestring);
			ACSHTTP_Get_SyncMark(ACS_CD_ELERULES,ptimestamp->valuestring,eField);
		}
		else
		{
			sprintf(sztime,"%ld",time(NULL));
			ACSHTTP_Get_SyncMark(ACS_CD_ELERULES,sztime,eField);
		}
		
		cJSON* pdistrictfloors = cJSON_GetObjectItem(pdata, "districtfloors");
		if((pdistrictfloors) && (pdistrictfloors->type == cJSON_Array))
		{
			eleRulesNum = cJSON_GetArraySize(pdistrictfloors);
			
			PRT_PRINT(ACS_DEBUG,"syncEleRules eleRules:%d",eleRulesNum);
			ACS_WLOG("syncEleRules eleRules:%d;nTotal:%d",eleRulesNum,pstCallDataOut->nTotal);
			if(ACSSYNCNUM < eleRulesNum)
			{
				eleRulesNum = ACSSYNCNUM;//ret = ACS_ERR_DATAERR;
			}
			pstCallDataOut->nPageSize = eleRulesNum;
			pstCallDataOut->nTotal = eleRulesNum;
			
			pstEleRules = (ACS_ELERULES_INFO_S *)malloc(sizeof(ACS_ELERULES_INFO_S)*eleRulesNum);
			if(pstEleRules == NULL)
			{
				ACS_WLOG("pstEleRules malloc %d error",sizeof(ACS_ELERULES_INFO_S)*eleRulesNum);
				PRT_PRINT(ACS_ERROR,"pstEleRules malloc %d error",sizeof(ACS_ELERULES_INFO_S)*eleRulesNum);
				pstCallDataOut->nPageSize = ACS_FAILED;				
			}
			
			if(pstEleRules)
			{
				pstCallDataOut->pRecord = (ACS_CHAR *)pstEleRules;
				PRT_PRINT(ACS_DEBUG,"pstEleRules malloc %p;pstEleRules:%p",pstCallDataOut->pRecord,pstEleRules);				
				memset(pstEleRules,0,(sizeof(ACS_ELERULES_INFO_S)*eleRulesNum));
				for(i=0; i<eleRulesNum; i++)
				{
					cJSON* pDisFloors = cJSON_GetArrayItem(pdistrictfloors, i);
					if(pDisFloors != NULL)
					{
						cJSON* poperator = cJSON_GetObjectItem(pDisFloors, "operation");
						if((poperator)&&(poperator->valuestring))
						{
							snprintf(pstEleRules[i].operation,sizeof(pstEleRules[i].operation),poperator->valuestring);//memcpy(pstAd[i].operation, poperator->valuestring, sizeof(pstAd[i].operation));
							PRT_PRINT(ACS_DEBUGX,"operation=%s",pstEleRules[i].operation);
						}
						else
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","operation");
							break;
						}
						
						cJSON* peleRulesid = cJSON_GetObjectItem(pDisFloors, "eleRuleid");
						if((peleRulesid)&&(peleRulesid->valuestring))
						{
							snprintf(pstEleRules[i].eleRulesid,sizeof(pstEleRules[i].eleRulesid),peleRulesid->valuestring);//memcpy(pstAd[i].operation, poperator->valuestring, sizeof(pstAd[i].operation));
							PRT_PRINT(ACS_DEBUGX,"eleRulesid=%s",pstEleRules[i].eleRulesid);
						}
						else
						{
							ret = ACS_ERR_DATAERR;
							sprintf(pstMsgOut->szDetails,"%s","eleRulesid");
							break;
						}

						cJSON* ptype = cJSON_GetObjectItem(pDisFloors, "type");
						if((ptype) && (ptype->type == cJSON_Number))
						{
							pstEleRules[i].type = ptype->valueint;
							PRT_PRINT(ACS_DEBUGX,"type:%d",ptype->valueint);
						}

						cJSON* pdefaultfloor = cJSON_GetObjectItem(pDisFloors, "defaultfloor");
						if((pdefaultfloor) && (pdefaultfloor->type == cJSON_Number))
						{
							pstEleRules[i].defaultfloor = pdefaultfloor->valueint;
							PRT_PRINT(ACS_DEBUGX,"defaultfloor:%d",pdefaultfloor->valueint);
						}

						cJSON* pcurfloor = cJSON_GetObjectItem(pDisFloors, "curfloor");
						if((pcurfloor) && (pcurfloor->type == cJSON_Number))
						{
							pstEleRules[i].curfloor = pcurfloor->valueint;
							PRT_PRINT(ACS_DEBUGX,"curfloor:%d",ptype->valueint);
						}
						
						cJSON* pvisitorfloor = cJSON_GetObjectItem(pDisFloors, "visitorfloor");
						if((pvisitorfloor) && (pvisitorfloor->type == cJSON_Number))
						{
							pstEleRules[i].visitorfloor = pvisitorfloor->valueint;
							PRT_PRINT(ACS_DEBUGX,"visitorfloor:%d",pvisitorfloor->valueint);
						}
						
						cJSON* pid = cJSON_GetObjectItem(pDisFloors, "id");
						if((pid)&&(pid->valuestring))
						{
							snprintf(pstEleRules[i].ID,sizeof(pstEleRules[i].ID),pid->valuestring);
							PRT_PRINT(ACS_DEBUGX,"ID=%s",pstEleRules[i].ID);
						}
						
						cJSON* pfloors = cJSON_GetObjectItem(pDisFloors, "floors");
						if((pfloors) && (pfloors->type == cJSON_Array))
						{
							floorsNum = cJSON_GetArraySize(pfloors);
							
							PRT_PRINT(ACS_INFO,"syncEleRules floorsNum:%d",floorsNum);
							ACS_WLOG("syncEleRules floorsNum:%d;nTotal:%d",floorsNum,pstCallDataOut->nTotal);
							if(ACSSYNCNUM < floorsNum)
							{
								floorsNum = ACSSYNCNUM;
							}
							pstEleRules[i].floorTotal = floorsNum;
							
							pstFloorss = (ACS_FLOORS_INFO_S *)malloc(sizeof(ACS_FLOORS_INFO_S)*floorsNum);
							if(pstFloorss == NULL)
							{
								ACS_WLOG("pstFloorss malloc %d error",sizeof(ACS_FLOORS_INFO_S)*floorsNum);
								PRT_PRINT(ACS_ERROR,"pstFloorss malloc %d error",sizeof(ACS_FLOORS_INFO_S)*floorsNum);
								//pstCallDataOut->nPageSize = -1;
								pstEleRules[i].floorTotal = ACS_FAILED;
							}
							
							if(pstFloorss)
							{
								pstEleRules[i].pFloorData = (ACS_CHAR *)pstFloorss;
								PRT_PRINT(ACS_DEBUG,"pstFloorss malloc %p;pstFloorss:%p",pstEleRules[i].pFloorData,pstFloorss);				
								memset(pstFloorss,0,(sizeof(ACS_FLOORS_INFO_S)*floorsNum));
								for(j=0; j<floorsNum; j++)
								{
									cJSON* pFloor = cJSON_GetArrayItem(pfloors, j);
									if(pFloor)
									{
										cJSON* pname = cJSON_GetObjectItem(pFloor, "name");
										if((pname)&&(pname->valuestring))
										{
											snprintf(pstFloorss[j].cName,sizeof(pstFloorss[j].cName),pname->valuestring);
											PRT_PRINT(ACS_DEBUGX,"pname=%s",pstFloorss[j].cName);
										}
										cJSON* pdir = cJSON_GetObjectItem(pDisFloors, "dir");
										if((pdir) && (pdir->type == cJSON_Number))
										{
											pstFloorss[j].bDir = pdir->valueint;
											PRT_PRINT(ACS_DEBUGX,"dir:%d",pdir->valueint);
										}
										cJSON* pfloor = cJSON_GetObjectItem(pDisFloors, "floor");
										if((pfloor) && (pfloor->type == cJSON_Number))
										{
											pstFloorss[j].bFloor = pfloor->valueint;
											PRT_PRINT(ACS_DEBUGX,"floor:%d",pfloor->valueint);
										}
									}
								}
							}
						}
					}
					//平台下发停止入库
					if((Mqtt_Get_DataDealFlag() & ACS_CD_ELERULES) != ACS_CD_ELERULES)
					{
						//pstCallDataOut->bifStop = ACS_ITURN;
						PRT_PRINT(ACS_INFO,"syncEleRules stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						ACS_WLOG("syncEleRules stop Mqtt_Get_DataDealFlag():%d",Mqtt_Get_DataDealFlag());
						break;
					}
				}
			}
			else
			{
				ret = ACS_ERR_MALLOCFAIL;
			}
		}
		else
		{
			ret = ACS_ERR_DATAERR;
			sprintf(pstMsgOut->szDetails,"%s","districtfloors");
		}
	}
	else
	{
		ret = ACS_ERR_DATAERR;
		sprintf(pstMsgOut->szDetails,"%s","data");
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	return ret;
}


/**
* fun:POST数据回调 解析服务器返回的数据上传接口
* @CallData-in:服务器返回的健康码信息
* @healthInfo-out:解析到的健康码信息
* @return:0-success;
**/
ACS_INT Curl_DealCall_Back(ACS_CHAR *CallData, ACS_CHAR *pDataOut,ACS_MESSAGE_S *pstMsg)
{
	if(CallData == NULL)
	{
		PRT_PRINT(ACS_ERROR,"CallData NULL");
		return ACS_ERR_PARAMNULL;
	}

	if(pDataOut == NULL)
	{
		PRT_PRINT(ACS_ERROR,"pDataOut NULL");
		return ACS_ERR_PARAMNULL;
	}
	
	PRT_PRINT(ACS_DEBUG,"Curl_DealCall_Back: %s",CallData);

	
	ACS_INT nsip = 0;
	ACS_INT i = 0;
	ACS_INT ret = ACS_ERR_SUCCESS;
	
	ACS_CALL_RESULT_S *pDataResult = (ACS_CALL_RESULT_S *)pDataOut;
	if(pDataResult == NULL)
	{
		return ACS_ERR_PARAMNULL;
	}
	
	cJSON* pRecvJsonRoot = cJSON_Parse((ACS_CHAR *)CallData);
	if(pRecvJsonRoot == NULL)
		return ACS_ERR_PARAMNULL;

	cJSON* pRetCode = cJSON_GetObjectItem(pRecvJsonRoot,"retCode");
	if((pRetCode == NULL) || (pRetCode->type != cJSON_Number))
	{
		memcpy(pstMsg->szDetails,"retCode",7);
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
	
	if(pRetCode->valueint != ACS_JSON_CODE_SUCCESS)
	{
		pDataResult->retCode = pRetCode->valueint;
		cJSON* pmessage = cJSON_GetObjectItem(pRecvJsonRoot,"message");
		if (pmessage && pmessage->valuestring)
		{
			snprintf(pDataResult->message,sizeof(pDataResult->message),"%s",pmessage->valuestring);
		}
		ACS_WLOG("retCode:%d;message:%s",pRetCode->valueint,pDataResult->message);
		ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
		return ACS_ERR_DATAERR;
	}
	
	cJSON* pData = cJSON_GetObjectItem(pRecvJsonRoot, "data");
	if(pData)
	{
	#if 1
		cJSON* psip = cJSON_GetObjectItem(pData,"sip"); 
		if(psip && psip->type == cJSON_Array)
		{
			nsip = cJSON_GetArraySize(psip);
			pDataResult->nDataLen = -1;

			ACS_WLOG("DealCallRoom_Back bDataLen:%d;nsipNum:%d",pDataResult->nDataLen,nsip);
			#if 0
			pDataResult->nDataLen = nsip;
			ACS_CALL_SIP_S *pstSip = NULL;
			pstSip = (ACS_CALL_SIP_S *)malloc(sizeof(ACS_CALL_SIP_S)*nsip);
			if(pstSip == NULL)
			{
				ACS_WLOG("pSyncAdData malloc %d error",sizeof(ACS_CALL_SIP_S)*nsip);
				PRT_PRINT(ACS_ERROR,"pSyncAdData malloc %d error\n",sizeof(ACS_CALL_SIP_S)*nsip);
				pDataResult->nDataLen = -1;
				nsip = ACS_FALSE;
			}
			else
			#endif
			{
				for(i = 0;i<nsip;i++)
				{
					cJSON* ppsip = cJSON_GetArrayItem(psip,i);
					if(ppsip)
					{		
						cJSON* pphone = cJSON_GetObjectItem(ppsip, "phone");
						if(pphone && pphone->valuestring)
						{
							PRT_PRINT(ACS_DEBUG,"cPhone:%s",pphone->valuestring);//snprintf(pstSip[i].cPhone,sizeof(pstSip[i].cPhone),pphone->valuestring);
						}
						else
						{
							memcpy(pstMsg->szDetails,"phone",5);
							ret = ACS_ERR_DATAERR;
						}
						
						cJSON* pnsip = cJSON_GetObjectItem(ppsip, "sip");
						if((pnsip) && (pnsip->type == cJSON_Number))
						{
							PRT_PRINT(ACS_DEBUG,"sip:%d",pnsip->valueint);//pstSip[i].nSip = psip->valueint;
						}
						else
						{
							memcpy(pstMsg->szDetails,"sip",3);
							ret = ACS_ERR_DATAERR;
						}
					}
				}
				//pDataResult->pData = pstSip;
			}

		}
	#endif 

		cJSON* presult = cJSON_GetObjectItem(pData, "result");
		if((presult) && (presult->type == cJSON_Number))
		{
			pDataResult->nResult = presult->valueint;
		}
		else
		{
			memcpy(pstMsg->szDetails,"result",6);
			ret = ACS_ERR_DATAERR;
		}

		cJSON* pringtime = cJSON_GetObjectItem(pData, "ringtime");
		if((pringtime) && (pringtime->type == cJSON_Number))
		{
			pDataResult->bRingtime = pringtime->valueint;
		}
		else
		{
			pDataResult->bRingtime = 45;
		}

		cJSON* pmsg = cJSON_GetObjectItem(pData, "msg");
		if(pmsg && pmsg->valuestring)
		{
			snprintf(pDataResult->cMsg,sizeof(pDataResult->cMsg),pmsg->valuestring);
		}
	}
	else
	{
		ACS_WLOG("data null");
		PRT_PRINT(ACS_DEBUG,"data null");
	}
	
	ACS_freeJson((ACS_VOID**)&pRecvJsonRoot,__func__);
	
	return ret;
}


/**
* fun:Post请求服务器数据  		  开门数据结果       人脸入库结果 卡号入库结果 广告入库结果 数据检查结果 错误信息提示结果上传
* @url-in:POST请求的URL /device/dcit/api/eq/v1/{action}/result?uuid=xx&token=xxxx&pcode=xxx
* @Callfunc-in:服务器返回的结果
* @Postdata-in:请求数据
* @timeout-in:请求超时时间s
* @return:ACS_ERR_SUCCESS-success;<0-fail  ACS_ERR_MALLOCFAIL ACS_ERR_CURLFIAL ACS_ERR_DATANULL -2
**/
ACS_INT ACSHTTP_Post_Request(const ACS_CHAR *pUrl,Callback_Deal_ResponsePost Callfunc,const ACS_CHAR *Postdata,const ACS_INT timeout)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	CURLcode res;
	CURL *easyhandle = NULL;
	struct curl_slist *custom_headers = NULL;
	struct AcsMemoryStruct chunk;
	chunk.memory = (ACS_CHAR *)malloc(1);
	if(chunk.memory == NULL)
	{
		return ACS_ERR_MALLOCFAIL;//printf("acsLib Http_Post chunk.memory NULL\n");
	}
 	chunk.size = 0;	

	//ACS_CONFIG_S stMjCfg = {0};
	//ACSCOM_Get_Config(&stMjCfg);
	//sprintf(vurl,"%s%s",stMjCfg.stClouldInfo.szHttpUrl,url);

	//ACS_WLOG("HTTP Post Result url:%s\n",vurl);
	//printf("vurl:%s\n",vurl);
	//PRT_PRINT("MjCloudLib PostDataLen=%d", strlen(Postdata));


	
	//printf(ACSRED"curl_easy_init [%s;%d]"ACSNONE,__func__,__LINE__);
	easyhandle = curl_easy_init();
	//printf(ACSRED"curl_easy_init 111[%s;%d]"ACSNONE,__func__,__LINE__);
	if (easyhandle)
	{	
		custom_headers = curl_slist_append(custom_headers,"User-Agent: Mozilla/4.0");			//custom_headers = curl_slist_append(custom_headers,Header);
		custom_headers = curl_slist_append(custom_headers,"Cache-Control: no-cache");
		custom_headers = curl_slist_append(custom_headers,"Accept-Language: zh-CN,zh;q=0.9");
		custom_headers = curl_slist_append(custom_headers,"accept-encoding: gzip, deflate");
		custom_headers = curl_slist_append(custom_headers,"Expect: ");							//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		custom_headers = curl_slist_append(custom_headers,"Connection: keep-alive");
		custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");	//custom_headers = curl_slist_append(custom_headers,"Content-Type: application/x-www-form-urlencoded");

		if(strstr(pUrl,"https://"))
		{
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);
		}
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, custom_headers);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, timeout);//checkData有可能需要5s左右					//设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, timeout);	  							//设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
		curl_easy_setopt(easyhandle, CURLOPT_URL,pUrl);
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, Postdata);
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);//毫秒级超时
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, (ACS_LONG)strlen(Postdata));
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WriteMemoryCallback);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (ACS_VOID *)&chunk);
		//curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);//调试

		
		//curl_easy_setopt(easyhandle, CURLOPT_POST,1);
		res = curl_easy_perform(easyhandle);//PRT_PRINT(ACS_DEBUG,"upload res:%d;",res);
		if (res != CURLE_OK)
		{
			ret = ACS_ERR_CURLFIAL;
			ACS_WLOG("Post curl_easy_strerror:%s,%d;ret:%d;ContentType:%d",curl_easy_strerror(res),res,ret,timeout);//PRT_PRINT(ACS_DEBUG,"Post curl_easy_strerror:%s,%d;ret:%d;ContentType:%d",  curl_easy_strerror(res),res,ret,timeout);
		}

		if(chunk.size > 0)
		{
			#if 1
			if(ACSJSONMAXLEN < chunk.size)
			{
				PRT_PRINT(ACS_DEBUG,"CallData ACSJSONMAXLEN");
				ret = ACS_ERR_DATAMAX;
			}
			else
			#endif
			{
				ret = Callfunc(chunk.memory,NULL);
			}
		}
		else
		{
			if((res == CURLE_OPERATION_TIMEDOUT)||(res == CURLE_COULDNT_CONNECT))//28是服务器收到了数据 返回超时
			{
				ret = -2; //断网续传
			}
			else
			{
				ret = ACS_ERR_DATANULL;
			}
		}
		curl_slist_free_all(custom_headers);
		curl_easy_cleanup(easyhandle);
		//printf(ACSGREEN"curl_easy_init end [%s;%d]"ACSNONE,__func__,__LINE__);
	}
	else
	{
		ret = ACS_ERR_CURLFIAL;
	}

	if(chunk.memory)
	{//printf("\033[1m\033[40;31m face free malloc chunk.memory:%p;[%s:%d]\033[0m\n",chunk.memory,__func__, __LINE__);
		free(chunk.memory);
		chunk.memory = NULL;
	}
	
	//curl_global_cleanup();

	return ret;
}



/**
* fun:Post请求服务器数据  		  开门数据结果       人脸入库结果 卡号入库结果 广告入库结果 数据检查结果 错误信息提示结果上传
* @url-in:POST请求的URL /device/dcit/api/eq/v1/{action}/result?uuid=xx&token=xxxx&pcode=xxx
* @Callfunc-in:服务器返回的结果
* @Postdata-in:请求数据
* @ContentType-in:数据格式 默认1-json 
* @return:ACS_ERR_SUCCESS-success;<0-fail  ACS_ERR_MALLOCFAIL ACS_ERR_CURLFIAL ACS_ERR_DATANULL -2
**/
ACS_INT ACSHTTP_Post_Request_V2(const ACS_CHAR *pUrl,Callback_Deal_ResponsePost_v2 Callfunc,const ACS_CHAR *Postdata,const ACS_INT waitTime, ACS_CHAR *respData,ACS_MESSAGE_S *pMsg)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	CURLcode res;
	CURL *easyhandle = NULL;
	struct curl_slist *custom_headers = NULL;
	struct AcsMemoryStruct chunk;
	chunk.memory = (ACS_CHAR *)malloc(1);
	if(chunk.memory == NULL)
	{
		return ACS_ERR_MALLOCFAIL;//printf("acsLib Http_Post chunk.memory NULL\n");
	}
 	chunk.size = 0;	

	//ACS_CONFIG_S stMjCfg = {0};
	//ACSCOM_Get_Config(&stMjCfg);
	//sprintf(vurl,"%s%s",stMjCfg.stClouldInfo.szHttpUrl,url);

	//ACS_WLOG("HTTP Post Result url:%s\n",vurl);
	//printf("vurl:%s\n",vurl);
	//PRT_PRINT("MjCloudLib PostDataLen=%d", strlen(Postdata));
	
	//printf(ACSRED"curl_easy_init [%s;%d]"ACSNONE,__func__,__LINE__);
	easyhandle = curl_easy_init();
	//printf(ACSRED"curl_easy_init 111[%s;%d]"ACSNONE,__func__,__LINE__);
	if (easyhandle)
	{	
		custom_headers = curl_slist_append(custom_headers,"User-Agent: Mozilla/4.0");			//custom_headers = curl_slist_append(custom_headers,Header);
		custom_headers = curl_slist_append(custom_headers,"Cache-Control: no-cache");
		custom_headers = curl_slist_append(custom_headers,"Accept-Language: zh-CN,zh;q=0.9");
		//custom_headers = curl_slist_append(custom_headers,"accept-encoding: gzip, deflate");
		custom_headers = curl_slist_append(custom_headers, "Expect:");							//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		custom_headers = curl_slist_append(custom_headers,"Connection: keep-alive");
		custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");	//custom_headers = curl_slist_append(custom_headers,"Content-Type: application/x-www-form-urlencoded");

		if(strstr(pUrl,"https://"))
		{
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);
		}
		
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, custom_headers);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, waitTime);//checkData有可能需要5s左右					//设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, waitTime);	  							//设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
		curl_easy_setopt(easyhandle, CURLOPT_URL,pUrl);
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, Postdata);
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, (ACS_LONG)strlen(Postdata));
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WriteMemoryCallback);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (ACS_VOID *)&chunk);

		res = curl_easy_perform(easyhandle);

		if(chunk.size > 0)
		{
			#if 1
			if(ACSJSONMAXLEN < chunk.size)
			{
				PRT_PRINT(ACS_DEBUG,"CallData ACSJSONMAXLEN");
				ret = ACS_ERR_DATAMAX;
			}
			else
			#endif
			{
				ret = Callfunc(chunk.memory,respData,pMsg);
			}
		}
		else
		{
			ACS_WLOG("Post curl_easy_strerror:%s,%d;ret:%d",  curl_easy_strerror(res),res,ret);
			if((res == CURLE_OPERATION_TIMEDOUT)||(res == CURLE_COULDNT_CONNECT))//28是服务器收到了数据 返回超时
			{
				ret = -2; //断网续传
			}
			else
			{
				ret = ACS_ERR_DATANULL;
			}
		}
		curl_slist_free_all(custom_headers);
		curl_easy_cleanup(easyhandle);
		//printf(ACSGREEN"curl_easy_init end [%s;%d]"ACSNONE,__func__,__LINE__);
	}
	else
	{
		ret = ACS_ERR_CURLFIAL;
	}

	if(chunk.memory)
	{
		free(chunk.memory);
		chunk.memory = NULL;
	}
	
	//curl_global_cleanup();

	return ret;
}


/**
* fun:通过不同HTTP API接口 Post 请求服务器数据 查询健康码信息
* @url-in:POST请求的URL {{MJJ_D00RAPI}}/api/qcrverify3?uuid=xxx&token=xxxxx&pcode=xxx
* @Callfunc-in:服务器返回的结果回调函数
* @Postdata-in:请求数据
* @respData-out:由Callfunc函数解析到的数据结果
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_Post_Request_V3(const ACS_CHAR *url,Callback_Deal_ResponsePost Callfunc,const ACS_CHAR *Postdata, ACS_CHAR *respData,const ACS_BYTE bTime)
{
	ACS_INT ret = 0;
	CURLcode res;
	CURL *easyhandle = NULL;
	struct curl_slist *custom_headers = NULL;
	struct AcsMemoryStruct chunk;
	ACS_CHAR Header[256]= {0};
	chunk.memory = (ACS_CHAR *)malloc(1);
	if(chunk.memory == NULL)
	{
		return -1;
	}
 	chunk.size = 0;
	
	//ACS_CONFIG_S stMjCfg = {0};
	//ACSCOM_Get_Config(&stMjCfg);
	//sprintf(vurl,"%s%s",stMjCfg.stClouldInfo.szHttpUrl,url);
	//PRT_PRINT("HTTP POST URL Check PwdData:%s\n",vurl);
	
	PRT_PRINT(ACS_DEBUG,"Postdata:%s",Postdata);

	ACSHTTP_GetTokenHeard(Header);

#if 1
	//curl_global_init(CURL_GLOBAL_ALL);
	easyhandle = curl_easy_init();
	if (easyhandle)
	{	
		custom_headers = curl_slist_append(custom_headers,Header);								//custom_headers = curl_slist_append(custom_headers, "Expect:");//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		custom_headers = curl_slist_append(custom_headers,"User-Agent: Mozilla/4.0");
		custom_headers = curl_slist_append(custom_headers,"Cache-Control: no-cache");
		custom_headers = curl_slist_append(custom_headers,"Accept-Language: zh-CN,zh;q=0.9");	//custom_headers = curl_slist_append(custom_headers,"accept-encoding: gzip, deflate");
		custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");
		custom_headers = curl_slist_append(custom_headers,"Connection: keep-alive");			//custom_headers = curl_slist_append(custom_headers,AcsCom_GetHeard());
				
		if(strstr(url,"https://"))//跳过证书验证
		{
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);
		}
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, custom_headers);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, bTime);	  									//设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, bTime);	  							//设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
		curl_easy_setopt(easyhandle, CURLOPT_URL,url);	 										//指定url
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, Postdata);    							//指定post内容
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, (ACS_LONG)strlen(Postdata));		//post 长度
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WriteMemoryCallback);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (ACS_VOID *)&chunk);					//curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);//调试
		res = curl_easy_perform(easyhandle);
		if(chunk.size > 0)
		{
			PRT_PRINT(ACS_DEBUG,"chunk.size==%d",chunk.size);
			ret = Callfunc(chunk.memory, respData);
		}
		else
		{
			ACS_WLOG("Post curl_easy_strerror:%s,%d;ret:%d",  curl_easy_strerror(res),res,ret);
			PRT_PRINT(ACS_DEBUG,"Post curl_easy_strerror:%s,%d;ret:%d",  curl_easy_strerror(res),res,ret);
			if((res == CURLE_OPERATION_TIMEDOUT)||(res == CURLE_COULDNT_CONNECT))//28是服务器收到了数据 返回超时
			{
				ret = -2; //断网续传
			}
			else
			{
				ret = ACS_ERR_DATANULL;
			}
		}
		
		curl_slist_free_all(custom_headers);
		curl_easy_cleanup(easyhandle);
	}
	else
	{
		ret = ACS_ERR_CURLFIAL;
	}
#endif
	if(chunk.memory != NULL)
	{
		free(chunk.memory);
		chunk.memory = NULL;
	}
	//curl_global_cleanup();
	return ret;
}


//上传开门数据 、录像、和图片
ACS_INT ACSHTTP_PostFile_Request(ACS_CHAR *url,Callback_Deal_ResponsePost Callfunc,ACS_CHAR *Postdata,ACS_CHAR * rcordfilepth,ACS_CHAR *response)
{
	CURL *easyhandle = NULL;
	CURLcode res;
	struct curl_slist *custom_headers=NULL;
	ACS_INT ret = 0;
	//ACS_CHAR Header[256]= {0};
	//ACS_CHAR vurl[512]={0};
	struct AcsMemoryStruct chunk;

	struct curl_httppost* pFormPost = NULL;
	struct curl_httppost* pLastElem = NULL;


	chunk.memory = (ACS_CHAR *)malloc(1);
	if(chunk.memory== NULL)
	{
		return -1;
	}
	chunk.size = 0;

	//curl_global_init(CURL_GLOBAL_ALL);

	//ACS_CONFIG_S stMjCfg = {0};
	//ACSCOM_Get_Config(&stMjCfg);

	//sprintf(vurl,"%s%s",stMjCfg.stClouldInfo.szHttpUrl,url);

	ACS_WLOG("ACSHTTP_PostFile_Request url==%s",url);
	//printf(ACSRED"curl_easy_init [%s;%d]"ACSNONE,__func__,__LINE__);
	easyhandle = curl_easy_init();
	//printf(ACSRED"curl_easy_init 111[%s;%d]"ACSNONE,__func__,__LINE__);
	if (easyhandle)
	{
		custom_headers = curl_slist_append(custom_headers,"User-Agent: Mozilla/4.0");
		custom_headers = curl_slist_append(custom_headers,"Cache-Control: no-cache");
		custom_headers = curl_slist_append(custom_headers,"Accept-Language: zh-CN,zh;q=0.9");
		custom_headers = curl_slist_append(custom_headers,"accept-encoding: gzip, deflate");
		custom_headers = curl_slist_append(custom_headers, "Expect:");//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		//custom_headers = curl_slist_append(custom_headers,Header);

		custom_headers = curl_slist_append(custom_headers,"Connection: keep-alive");

		if(Postdata != NULL)
		{
			curl_formadd(&pFormPost,
				         &pLastElem,
				         CURLFORM_COPYNAME, "msg",
				 		 CURLFORM_PTRCONTENTS, Postdata,
		                 CURLFORM_CONTENTSLENGTH, strlen(Postdata),
		                 CURLFORM_CONTENTTYPE, "application/json",
				         CURLFORM_END);
		}
		
		if (access(rcordfilepth, F_OK) != -1)
		{
			curl_formadd(&pFormPost,
				        &pLastElem,
				        CURLFORM_COPYNAME, "videoFile",
				        CURLFORM_FILE,rcordfilepth,
				        CURLFORM_CONTENTTYPE, "MultipartFile",
				        CURLFORM_END);
		}

		curl_easy_setopt(easyhandle, CURLOPT_HTTPPOST, pFormPost);
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, custom_headers);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, 7);	  //设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, 7);	  //设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WriteMemoryCallback);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (ACS_VOID *)&chunk);
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);
		//curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_URL,url);	 // 指定url

		res = curl_easy_perform(easyhandle);

		if (res != CURLE_OK)
		{
			ret =-1;
		}

		if(chunk.size >0)
		{
			ret = Callfunc(chunk.memory,NULL);
		}
		else
		{
			ret = -1;
		}
		
		curl_formfree(pFormPost);//printf("free acs [%s %d]\n" ,__func__,__LINE__);
		curl_slist_free_all(custom_headers);//printf("free acs [%s %d]\n" ,__func__,__LINE__);
		curl_easy_cleanup(easyhandle);
		//printf(ACSGREEN"curl_easy_init end [%s;%d]"ACSNONE,__func__,__LINE__);

	}
	else
	{
		ret =-1;
	}

	if(chunk.memory != NULL)
	{
		free(chunk.memory);
		chunk.memory = NULL;
	}

	//curl_global_cleanup();

	return ret;
}


/**
* fun:下载文件 (磁盘Disk存储)
* @cmd-in:请求文件地址
* @picPath-in:文件保存路径
* @nTimes-in:设置超时和连接时间
* @return:success文件长度;fail =<0;
**/
ACS_INT ACSHTTP_DiskDownFile(const ACS_CHAR *url,const ACS_CHAR *picPath,ACS_INT nTimes)
{
	CURL *easyhandle = NULL;
	ACS_INT ret = 0;
	CURLcode res;
	//curl_global_init(CURL_GLOBAL_ALL);
	easyhandle = curl_easy_init();
	if (easyhandle)
	{
		//custom_headers = curl_slist_append(custom_headers, "Expect:");//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		//custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");
		//跳过证书验证
		curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
		curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, nTimes);	  //设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, nTimes);	  //设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
		curl_easy_setopt(easyhandle, CURLOPT_URL,url);	 // 指定url
		curl_easy_setopt(easyhandle, CURLOPT_PROGRESSFUNCTION, NULL);
		curl_easy_setopt(easyhandle, CURLOPT_PROGRESSDATA, NULL);
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WritePicFileCallback);

		FILE* fp = fopen(picPath, "wb");
		if (!fp) 
		{
			PRT_PRINT(ACS_DEBUG,"open the file[%s]!!!",picPath);
			//ret = ACS_ERR_FILE_OPENERR;
			ret = -2;
		}
		else
		{
			curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, fp);/* write the page body to this file handle */
			res = curl_easy_perform(easyhandle);/* get it! */
			if (res != CURLE_OK)
			{
				PRT_PRINT(ACS_DEBUG,"curl_easy_perform error [res:%d]!!!",res);
				fclose(fp);
				return -1;
			}
			fseek(fp,0,SEEK_END);			
			ret = ftell(fp);//获取文件长度;
			fseek(fp,0,SEEK_SET);
			fflush(fp);
			fclose(fp);/* close the header file */
		}
		curl_easy_cleanup(easyhandle);
	}
	else
	{
		//ret = ACS_ERR_FILE_DOWNERR;
		ret = -3;
		PRT_PRINT(ACS_DEBUG,"curl easy init fail!!!");
	}
	//curl_global_cleanup();
	return ret;
}



/**
* fun:下载文件 (内存Ram存储)
* @cmd-in:请求文件地址
* @fileBuf-in:文件保存的数据
* @nTimes-in:设置超时和连接时间
* @return:0;
**/
ACS_INT ACSHTTP_RamDownFile(const ACS_CHAR *url, struct AcsMemoryStruct *fileBuf,ACS_INT nTimes)
{
	CURL *easyhandle = NULL;
	struct curl_slist *custom_headers=NULL;
	ACS_INT ret = 0;
	CURLcode res;

	if(fileBuf == NULL)
		return -2;
	
	fileBuf->memory = (ACS_CHAR *)malloc(1);
	if(fileBuf->memory == NULL)
		return -3;
	//PRT_PRINT(ACS_DEBUG,"malloc:%p",fileBuf->memory);
	fileBuf->size = 0; /* no data at this point */

	//PRT_PRINT("DownFile url:%s",url);
	

	//curl_global_init(CURL_GLOBAL_ALL);
	easyhandle = curl_easy_init();
	if (easyhandle)
	{
		custom_headers = curl_slist_append(custom_headers, "Expect: ");//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		//custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, custom_headers);
		if(strstr(url,"https://"))//跳过证书验证
		{
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);

			//printf("https url:%s \n",url);
		}
		/**
		else
		{
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, true);
			curl_easy_setopt(curl,CURLOPT_CAINFO,"C:/ssl/cacert.pem");
			curl_easy_setopt(curl,CURLOPT_SSLCERT,"C:/ssl/client.pem");  
			curl_easy_setopt(curl,CURLOPT_SSLCERTPASSWD,"11111111");  
			curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");  
			curl_easy_setopt(curl,CURLOPT_SSLKEY,"C:/ssl/clientkey.pem");  
			curl_easy_setopt(curl,CURLOPT_SSLKEYPASSWD,"11111111");  
			curl_easy_setopt(curl,CURLOPT_SSLKEYTYPE,"PEM");
		}
		**/
		
		//curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);//调试
		
		curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, nTimes);	  //设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, nTimes);	  //设置连接超时，单位s, (CURLOPT_CONNECTTIMEOUT_MS 毫秒)
		curl_easy_setopt(easyhandle, CURLOPT_URL,url);	 // 指定url
		curl_easy_setopt(easyhandle, CURLOPT_PROGRESSFUNCTION, NULL);
		curl_easy_setopt(easyhandle, CURLOPT_PROGRESSDATA, NULL);
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WriteMemoryCallback);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (ACS_CHAR *)fileBuf);
		//PRT_PRINT("DownFile sizeof=%d",fileBuf->size);
		res = curl_easy_perform(easyhandle);/* get it! */
		if (res != CURLE_OK)
		{
			PRT_PRINT(ACS_DEBUG,"DownFile curl_easy_strerror== %d",res);
			ACS_WLOG("DownFile url[%s] error res[%d]",url,res);
			ret = -1;
		}
		curl_slist_free_all(custom_headers);
		curl_easy_cleanup(easyhandle);
	}
	else
	{
		ret = -1;
		ACS_WLOG("DownFile url[%s] error",url);
	}
	
	//PRT_PRINT("acs malloc pPicData:%p",fileBuf->memory);//sprintf(fileBuf->memory,"%s","abc123");
	
	//curl_global_cleanup();
	return ret;
}


/**
* fun:下载升级文件 (磁盘Disk存储)
* @cmd-in:请求文件地址
* @picPath-in:文件保存路径
* @nTimes-in:设置超时和连接时间
* @ACS_Curl_WriteFuncCallback-in:回调处理函数
* @return:success =0;fail =<0;
**/
ACS_INT ACSHTTP_UpgradeFileDownload(const ACS_CHAR *url,const ACS_CHAR *picPath,ACS_INT nTimes, ACS_Curl_WriteFuncCallback callback)
{
	CURL *easyhandle = NULL;
	ACS_INT ret = 0;
	CURLcode res;
	ACS_CURL_WRITE_CALLBACK_USER user={0};
	//curl_global_init(CURL_GLOBAL_ALL);
	easyhandle = curl_easy_init();
	if (easyhandle)
	{
		//custom_headers = curl_slist_append(custom_headers, "Expect:");//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		//custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");
		//跳过证书验证
		curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
		curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, nTimes);	  //设置连接超时，单位s,
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, nTimes);	  //设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
		curl_easy_setopt(easyhandle, CURLOPT_URL,url);	 // 指定url
		curl_easy_setopt(easyhandle, CURLOPT_PROGRESSFUNCTION, NULL);
		curl_easy_setopt(easyhandle, CURLOPT_PROGRESSDATA, NULL);//curl_easy_setopt(easyhandle, CURLOPT_BUFFERSIZE, 2*1024*1024);
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);//curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, callback);

		FILE* fp = fopen(picPath, "wb");
		if (!fp) 
		{
			PRT_PRINT(ACS_DEBUG,"open the file[%s]!!!",picPath);
			//ret = ACS_ERR_FILE_OPENERR;
			ret = -2;
		}
		else
		{
			user.fp = fp;
			curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &user);/* write the page body to this file handle */
			res = curl_easy_perform(easyhandle);/* get it! */
			if (res != CURLE_OK)
			{
				PRT_PRINT(ACS_DEBUG,"curl_easy_perform error [res:%d]!!!",res);
				ret = -1;
			}
			else
			{
				if(user.nCheckResult != ACS_ERR_SUCCESS)
				{
					ret = -4;
				}
				else
				{
					ret = user.nTotalSize;//传出下载文件长度
				}
			}

			if(fp)
			{
				fclose(fp);
				fp=NULL;
			}
			
		}
		curl_easy_cleanup(easyhandle);
	}
	else
	{
		//ret = ACS_ERR_FILE_DOWNERR;
		ret = -3;
		PRT_PRINT(ACS_DEBUG,"curl easy init fail!!!");
	}
	//curl_global_cleanup();
	return ret;
}



/**
* fun:呼叫记录上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_CallRecord_json(ACS_CALL_UPLOAD_S *pstCallUpload,ACS_CHAR **pp)
{
	ACS_INT ret = ACS_ERR_SUCCESS;

	if(pstCallUpload == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"upload point NULL");
		return ACS_ERR_PARAMNULL;
	}

	ACS_CHAR msgid[33] = {0};	
	const ACS_CHAR map[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	srand((unsigned)time(NULL));
	for (int i = 0; i < 32; i++)
	{
		msgid[i] = map[rand()%36];
	}

	cJSON* ppUplodmsg = cJSON_CreateObject();
	if(ppUplodmsg)
	{
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{
			cJSON_AddStringToObject(pData, "msgid",msgid);
			cJSON_AddStringToObject(pData, "type",pstCallUpload->ctype);
			cJSON_AddStringToObject(pData, "address",g_stDoor_Info.stDevInfo.szAddress);
			cJSON_AddStringToObject(pData, "calledid",pstCallUpload->ccalledid);
			cJSON_AddStringToObject(pData, "number",pstCallUpload->cnumber);
			cJSON_AddStringToObject(pData, "timebegin",pstCallUpload->ctimebegin);
			cJSON_AddStringToObject(pData, "timeend",pstCallUpload->ctimeend);
			cJSON_AddNumberToObject(pData, "callduration",pstCallUpload->dCallduration);
			cJSON_AddNumberToObject(pData, "state",pstCallUpload->bstate);
			cJSON_AddItemToObject(ppUplodmsg, "data", pData);
		}
		else
		{
			ret = ACS_ERR_MALLOCFAIL;
		}
		
		*pp = cJSON_PrintUnformatted(ppUplodmsg);//PRT_PRINT(ACS_DEBUG,"JSON char malloc p:%p",*pp);
		if(NULL == *pp)
		{
			ret = ACS_ERR_FAIL;
		}
		ACS_freeJson((ACS_VOID**)&ppUplodmsg,__func__);
	}
	else
	{
		ret = ACS_ERR_MALLOCFAIL;
	}	

	return ret;
}



/**
* fun:通话数据上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail  ACS_ERR_MALLOCFAIL ACS_ERR_CURLFIAL ACS_ERR_DATANULL -2 //ACS_ERR_DATANULL ACS_ERR_DATAERR
**/
ACS_INT ACSHTTP_CallRecord_Upload(ACS_CALL_UPLOAD_S *pstCallUpload)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT times = 0;
	ACS_CHAR *p = NULL;
	ACS_CHAR urlmsg[512]={0};
	
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);
	ACSHTTP_CallRecord_json(pstCallUpload,&p);

//	printf("dCallduration:%d;bstate:%d\n",pstCallUpload->dCallduration,pstCallUpload->bstate);
//	printf("ctype:%s;ccalledid:%s;cnumber:%s;ctimebegin:%s;ctimeend:%s\n",pstCallUpload->ctype,pstCallUpload->ccalledid,pstCallUpload->cnumber,pstCallUpload->ctimebegin,pstCallUpload->ctimeend);


	
	//PRT_PRINT(ACS_INFO,"\033[1m\033[40;31m	UPLOAD  [%s:%d]\033[0m\n", __func__, __LINE__);

	if(p)
	{
		snprintf(urlmsg,512,"%s/device/dcit/api/eq/v1/record/call?uuid=%s&token=%s&pcode=%s&cpuid=%s",\
		stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken, stClouldInfo.szPcode,stClouldInfo.szcpuID);
		
		PRT_PRINT(ACS_DEBUG,"CallRecord_Upload p:%s",p);
		
		do
		{
			ret = ACSHTTP_Post_Request(urlmsg,Curl_DealUploadRecord_Back,p,3);
			usleep(500*1000);
			times++;
		}while(((ret > ACS_ERR_DATANULL)||(ret == -2)) && (times < RECONETMAX));
		
		free(p);
		p = NULL;

		if(ACS_ERR_SUCCESS != ret)
		{
			ACS_WLOG("CallUpload_Record err ret:%d;times:%d",ret,times);
		}
		
	}
	else
	{
		ACS_WLOG("Upload p pointer NULL");
		ret = ACS_ERR_MALLOCFAIL;
	}	

	return ret;
}



/**
* fun:开门数据(短视频,图片,开门信息)上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_OpenDoorRecord_json(ACS_OPENDOOR_UPLOAD_S *pstUopendoor,ACS_CHAR **pp)
{
	ACS_INT ret = -1;
	ACS_INT nEncodepiclen = 0;	
	ACS_INT nPiclen = 0;
	ACS_CHAR temperature[8] = {0};
	ACS_CHAR picbuf[ACSPICBASEMAXLEND] = {0};
	ACS_CHAR EncodeBuf[ACSPICBASEMAXLEND] = {0};
	if(pstUopendoor == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"upload point NULL");
		return ACS_ERR_PARAMNULL;
	}
	
	cJSON* ppUplodmsg = cJSON_CreateObject();
	if(ppUplodmsg)
	{
		cJSON_AddStringToObject(ppUplodmsg, "userId",pstUopendoor->userId);//PRT_PRINT("JSON Create pcjson:%p",*ppUplodmsg);
		cJSON_AddStringToObject(ppUplodmsg, "type",pstUopendoor->type);

		if(strlen(pstUopendoor->data)<1)
			cJSON_AddStringToObject(ppUplodmsg, "data",pstUopendoor->userId);
		else
			cJSON_AddStringToObject(ppUplodmsg, "data",pstUopendoor->data);

		cJSON_AddStringToObject(ppUplodmsg, "screenTime",pstUopendoor->screenTime);
		
		if(access(pstUopendoor->snapfacepath, F_OK) == 0)
		{
			nPiclen = ACSHTTP_FileToData(picbuf,pstUopendoor->snapfacepath);
			if(nPiclen > 0)
				ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
			#if 1
			memset(picbuf, 0, sizeof(picbuf));
			snprintf(picbuf,sizeof(picbuf),"data:image/jpeg;base64,%s",EncodeBuf);
			cJSON_AddStringToObject(ppUplodmsg, "pic", picbuf);
			#else
			cJSON_AddStringToObject(pUplodmsg, "pic", EncodeBuf);
			#endif
			ACS_WLOG("nPiclen:%d;snapfacePic:%s;nEncodePicLen:%d",nPiclen,pstUopendoor->snapfacepath,nEncodepiclen);
		}
		else
		{
			cJSON_AddStringToObject(ppUplodmsg, "pic", "null");
			ACS_WLOG("snapfacePic [%s] NULL",pstUopendoor->snapfacepath);
		}
		
		sprintf(temperature, "%0.2f",pstUopendoor->temperature);
		cJSON_AddStringToObject(ppUplodmsg, "temperature", temperature);

		cJSON* pIDCardInfo = cJSON_CreateObject();
		if(pIDCardInfo)
		{
			cJSON_AddStringToObject(pIDCardInfo, "name",pstUopendoor->stIDCard.szName);
			cJSON_AddStringToObject(pIDCardInfo, "gender",pstUopendoor->stIDCard.szGender);
			cJSON_AddStringToObject(pIDCardInfo, "national",pstUopendoor->stIDCard.szNational);
			cJSON_AddStringToObject(pIDCardInfo, "birthday",pstUopendoor->stIDCard.szBirthday);
			cJSON_AddStringToObject(pIDCardInfo, "address",pstUopendoor->stIDCard.szAddress);
			cJSON_AddStringToObject(pIDCardInfo, "id",pstUopendoor->stIDCard.szId);
			cJSON_AddStringToObject(pIDCardInfo, "maker",pstUopendoor->stIDCard.szMaker);
			cJSON_AddStringToObject(pIDCardInfo, "startDate",pstUopendoor->stIDCard.szStartDate); 
			cJSON_AddStringToObject(pIDCardInfo, "endDate",pstUopendoor->stIDCard.szEndDate);
			
			if(access(pstUopendoor->stIDCard.szIDCardImg, F_OK) == 0)
			{
				memset(picbuf, 0, sizeof(picbuf));
				memset(EncodeBuf, 0, sizeof(EncodeBuf));
				nPiclen = ACSHTTP_FileToData(picbuf,pstUopendoor->stIDCard.szIDCardImg);
				if(nPiclen > 0)
				ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);

				#if 1
				memset(picbuf, 0, sizeof(picbuf));
				snprintf(picbuf,sizeof(picbuf),"data:image/jpeg;base64,%s",EncodeBuf);
				cJSON_AddStringToObject(ppUplodmsg, "idCardImg", picbuf);
				#else
				cJSON_AddStringToObject(pIDCardInfo, "idCardImg", EncodeBuf);//printf("EncodeBuf:%s\n",EncodeBuf);
				#endif
				ACS_WLOG("nPiclen:%d;idCardImg:%s;nEncodePicLen:%d",nPiclen,pstUopendoor->stIDCard.szIDCardImg,nEncodepiclen);
			}
			else
			{
				ACS_WLOG("idCardImg [%s] NULL",pstUopendoor->stIDCard.szIDCardImg);
				cJSON_AddStringToObject(pIDCardInfo, "idCardImg", "null");
			}
		}
		
		cJSON_AddItemToObject(ppUplodmsg, "idCardInfo", pIDCardInfo);
		cJSON_AddNumberToObject(ppUplodmsg, "openState",pstUopendoor->byOpenDoorEn);
		cJSON_AddNumberToObject(ppUplodmsg, "acsState",g_bAcsState);
		*pp = cJSON_PrintUnformatted(ppUplodmsg);//PRT_PRINT(ACS_DEBUG,"JSON char malloc p:%p",*pp);
		if(NULL == *pp)
		{
			cJSON_Delete(ppUplodmsg);//ACS_freeJson((ACS_VOID**)&ppUplodmsg,__func__);
			return ACS_ERR_FAIL;
		}
		ret = ACS_ERR_SUCCESS;
		cJSON_Delete(ppUplodmsg);
	}
	else
	{
		ret = ACS_ERR_MALLOCFAIL;
	}
	
	return ret;
}


/**
* fun:开门数据(短视频,图片,开门信息)上传 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail  ACS_ERR_MALLOCFAIL ACS_ERR_CURLFIAL ACS_ERR_DATANULL -2 //ACS_ERR_DATANULL ACS_ERR_DATAERR
**/
ACS_INT ACSHTTP_OpenDoorRecord_Upload(ACS_OPENDOOR_UPLOAD_S *pstUopendoor)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT times = 0;
	ACS_CHAR *p = NULL;
	//cJSON *pcjson = NULL;
	ACS_CHAR urlmsg[512]={0};
	//ACS_CONFIG_S stMjapiconfig = {0};
	//ACSCOM_Get_Config(&stMjapiconfig);
//	ACS_CLOUDLOGIN_S stClouldInfo = {0};
//	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);
	ACSHTTP_OpenDoorRecord_json(pstUopendoor,&p);
	
	//PRT_PRINT(ACS_INFO,"\033[1m\033[40;31m	UPLOAD  [%s:%d]\033[0m\n", __func__, __LINE__);

	if(p)//PRT_PRINT(ACS_INFO,  ACS_WLOG(
	{
		ACS_WLOG("UPLOAD:userId:%s,type:%s,data:%s,temp:%0.3f;screenTime:%s;pic:%s;IDCardInfo:szName:%s;cardid:%s;img:%s;OpenDoor:%d,jsonLen:%ld", \
		pstUopendoor->userId,pstUopendoor->type,pstUopendoor->data,pstUopendoor->temperature,pstUopendoor->screenTime, \
		pstUopendoor->snapfacepath,pstUopendoor->stIDCard.szName,pstUopendoor->stIDCard.szId,pstUopendoor->stIDCard.szIDCardImg,pstUopendoor->byOpenDoorEn,strlen(p));
		
		snprintf(urlmsg,512,"%s/device/dcit/api/eq/v1/record/opendoor?uuid=%s&token=%s&pcode=%s&cpuid=%s",\
		g_stDoor_Info.stClouldInfo.szHttpUrl,g_stDoor_Info.stClouldInfo.szUuid,g_stDoor_Info.stClouldInfo.szToken, g_stDoor_Info.stClouldInfo.szPcode,g_stDoor_Info.stClouldInfo.szcpuID);

		//printf("\033[1m\033[40;31m	UPLOAD	szBirthday:%s;szStartDate:%s;szEndDate:%s[%s:%d]\033[0m\n",
		//	pstUopendoor->stIDCard.szBirthday,pstUopendoor->stIDCard.szStartDate,pstUopendoor->stIDCard.szEndDate, __func__, __LINE__);
		
		do
		{
			ret = ACSHTTP_Post_Request(urlmsg,Curl_DealUploadRecord_Back,p,5);
			usleep(500*1000);
			times++;
		}while(((ret > ACS_ERR_DATANULL)||(ret == -2)) && (times < RECONETMAX));

		if(ACS_ERR_SUCCESS != ret)
		{
			ACS_WLOG("Upload_Record err ret:%d;times:%d",ret,times);
		}
		//PRT_PRINT(ACS_DEBUG,"upload free p:%p;p:%s",p,p);
		//printf("\033[1m\033[40;31m	UPLOAD upload free p:%p;p:%s [%s:%d]\033[0m\n",p,p, __func__, __LINE__);
		free(p);
		p = NULL;
	}
	else
	{
		ACS_WLOG("Upload p pointer NULL");
		ret = ACS_ERR_MALLOCFAIL;
	}	

	return ret;
}


//extern ACS_INT ACS_Http_Get_Request(const ACS_CHAR *url,Callback_Deal_ResponseGet Callfunc,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut);
/**
* fun:通过不同HTTP API接口 Get 服务器数据
* @url-in:设备登录的接口地址 /device/dcit/api/eq/v1/device/{action}?{xxx}
* @Callfunc-in:回调函数
* @pCallOut-out:回调函数结果输出
* @return:ACS_ERR_SUCCESS-success;Fail-ACS_ERR_DATAERR ACS_ERR_DATANULL//ACS_ERR_MALLOCFAIL ACS_ERR_TIMEOUT ACS_ERR_CURLFIAL
**/
ACS_INT ACSHTTP_Get_Request(const ACS_CHAR *pUrl,Callback_Deal_ResponseGet Callfunc,ACS_CHAR *pCallOut,ACS_MESSAGE_S *pstMsgOut)
{
	CURL *easyhandle;
	CURLcode res;
	ACS_INT ret = ACS_ERR_SUCCESS;
	
	struct curl_slist *custom_headers = NULL;

	struct AcsMemoryStruct chunk;

 	chunk.memory = (ACS_CHAR *)malloc(1);
	if(chunk.memory == NULL)
	{
		return ACS_ERR_MALLOCFAIL;
	}

    //curl_global_init(CURL_GLOBAL_ALL);
 	chunk.size = 0;						//ACS_WLOG("HTTP Get url:%s\n",pUrl);

	easyhandle = curl_easy_init();
	if(easyhandle)
	{
		custom_headers = curl_slist_append(custom_headers,"Expect:");//添加HTTP的包头Expect：防止数据大于1024个字节需要等待服务响应//也许有Expect: 100-continue，去掉它
		custom_headers = curl_slist_append(custom_headers,"User-Agent: Mozilla/4.0");
		custom_headers = curl_slist_append(custom_headers,"Cache-Control: no-cache");
		custom_headers = curl_slist_append(custom_headers,"Accept-Language: zh-CN,zh;q=0.9");//custom_headers = curl_slist_append(custom_headers,"Lang-Type: zh");//hongshi宏视版本需要这个
		custom_headers = curl_slist_append(custom_headers,"Content-Type: application/json");//custom_headers = curl_slist_append(custom_headers,"Content-Type: application/x-www-form-urlencoded");
		custom_headers = curl_slist_append(custom_headers,"Connection: keep-alive");
		if(NULL == pCallOut)
		{
			ACS_CHAR Header[512]= {0};
			ACSHTTP_GetHeard(Header);
			custom_headers = curl_slist_append(custom_headers,Header);//上传设备属性 设备sn 设备版本号库版本号 
		}
		if(strstr(pUrl,"https://"))
		{
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);	
			curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYHOST, 0);
		}
		//curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);//调试
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, custom_headers);				/******设置头信****/
		curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, 5);	 							/******设置连接超时，单位s****/
		curl_easy_setopt(easyhandle, CURLOPT_CONNECTTIMEOUT, 5);	  					/******设置连接超时，单位s****/
		curl_easy_setopt(easyhandle, CURLOPT_URL,pUrl);									/******设置url****/
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ACS_WriteMemoryCallback);	/******设置信息回调函数****/
		curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1L);								/**多线程使用超时主线程有sleep需要设置**/
		curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (ACS_VOID *)&chunk);
		res = curl_easy_perform(easyhandle);   											/****** 执行****/
		if (res != CURLE_OK)
		{
			if(CURLE_OPERATION_TIMEDOUT == res || CURLE_COULDNT_CONNECT == res)
			{
				ret = ACS_ERR_TIMEOUT;
			}
			else
			{
				ret = ACS_ERR_CURLFIAL;
			}
			PRT_PRINT(ACS_INFO,"curl_easy_strerror:%s,%d;ret:%d",curl_easy_strerror(res), res,ret);
			ACS_WLOG("curl_easy_strerror:%s,%d;ret:%d",curl_easy_strerror(res), res,ret);
		}
		
		if(chunk.size > 0)
		{
			#if 1
			if(ACSJSONMAXLEN < chunk.size)
			{
				PRT_PRINT(ACS_DEBUG,"CallData ACSJSONMAXLEN");
				ret = ACS_ERR_DATAMAX;
			}
			else
			#endif
			{
				ret = Callfunc(chunk.memory,pCallOut,pstMsgOut);
			}
		}
		
		curl_slist_free_all(custom_headers);
		curl_easy_cleanup(easyhandle);
	}
	else
	{
		ret = ACS_ERR_CURLFIAL;
	}

	if(chunk.memory != NULL)
	{
		free(chunk.memory);
		chunk.memory = NULL;
	}

	//curl_global_cleanup();//printf("login ret:%d\n",ret);
	
	return ret;
}


ACS_INT ACSHTTP_PicBase64(const ACS_CHAR *pPicPath,ACS_CHAR *pBase64)
{
	ACS_INT nlen = 0;
	ACS_INT nPiclen = 0;
	ACS_CHAR EncodeBuf[ACSPICBASEMAXLEND] = {0};
	memset(pBase64, 0, ACSPICBASEMAXLEND);
	nPiclen = ACSHTTP_FileToData(pBase64,pPicPath);
	if(nPiclen > 0)
	{
		ACS_base64_encode(pBase64, nPiclen,EncodeBuf,&nlen);
		memset(pBase64, 0, ACSPICBASEMAXLEND);
		snprintf(pBase64,ACSPICBASEMAXLEND,"data:image/jpeg;base64,%s",EncodeBuf);//printf("(picbuf):%s\n",(pBase64));
	}
	
	return nPiclen;
}


/**
* fun:HTTP Post 推送数据检查结果
* @pstCheackData-in:输入operation 输出check的数据 ACS_CHECKDATA_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostCheckDataResult_json(ACS_CHECKDATA_INFO_S *pstCheackData,cJSON **ppUplodmsg,ACS_CHAR **pp,const ACS_CHAR *pUuid)
{
	cJSON* pJsondata = NULL;
	cJSON* informationArray = NULL;
	ACS_INT i = 0;
	ACS_INT j = 0;
	ACS_CHAR TimeStamp[13] = {0};
	ACS_CHAR picbuf[ACSPICBASEMAXLEND] = {0};//ACS_CHAR EncodeBuf[ACSPICBASEMAXLEND] = {0};
	sprintf(TimeStamp,"%ld",time(NULL));
	
	pJsondata = cJSON_CreateObject();
	if(pJsondata)
	{
		cJSON_AddStringToObject(pJsondata, "uuid", pUuid);
		cJSON_AddStringToObject(pJsondata, "timestamp",TimeStamp);
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{	
			PRT_PRINT(ACS_DEBUG,"CheckData operation:%s;size:%d",pstCheackData->operation,pstCheackData->size);
			ACS_WLOG("CheckData operation:%s;size:%d",pstCheackData->operation,pstCheackData->size);
			cJSON_AddStringToObject(pData,"operation",pstCheackData->operation);
			for(j = 0; j < pstCheackData->size; j ++)
			{
				PRT_PRINT(ACS_DEBUG,"list:%d;total:%d",pstCheackData->stList[j].list,pstCheackData->stList[j].total);
				ACS_WLOG("j[%d]:list:%d;total:%d",j,pstCheackData->stList[j].list,pstCheackData->stList[j].total);

				cJSON* whiteArray = cJSON_CreateArray();
				if(whiteArray)
				{
					if((strcmp(pstCheackData->operation,ACS_USERS) == 0)||(strcmp(pstCheackData->operation,ACS_FACES) == 0))
					{
						ACS_CHECKDATA_FACE_S *pCheckData = (ACS_CHECKDATA_FACE_S *)pstCheackData->stList[j].pstData;
						if(pCheckData == NULL)
						{
							PRT_PRINT(ACS_ERROR,"acslib face pCheckData:%p;j[%d]error",pCheckData,j);
							continue;
						}
						
						ACS_INFORMATION_S *pInformation = (ACS_INFORMATION_S *)pstCheackData->stList[j].pstInformation;
						if(pInformation)
						{
							PRT_PRINT(ACS_ERROR,"acslib face pInformation:%p;j[%d]",pInformation,j);
							informationArray = cJSON_CreateArray();
						}
	
						PRT_PRINT(ACS_DEBUG,"face pCheckData:%p;j[%d]",pCheckData,j);
						for(i = 0;i < pstCheackData->stList[j].total;i++)
						{
							cJSON* pUserid = cJSON_CreateString(pCheckData[i].userId);
							if(pUserid)
							{
								cJSON_AddItemToArray(whiteArray,pUserid);
							}
							
							if(informationArray)
							{
								cJSON *information = cJSON_CreateObject();
								if(information)
								{
									cJSON_AddNumberToObject(information,"source",pInformation[i].source);
									//cJSON_AddNumberToObject(information,"userType",pInformation[i].userType);
									//cJSON_AddNumberToObject(information,"age",pInformation[i].age);
									//cJSON_AddNumberToObject(information,"certificateType",pInformation[i].certificateType);
									//cJSON_AddNumberToObject(information,"gender",pInformation[i].gender);
									cJSON_AddNumberToObject(information,"authType",pInformation[i].authType);
									cJSON_AddNumberToObject(information,"validtime_enable",pInformation[i].validtime_enable);
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].validtime);
									cJSON_AddStringToObject(information,"validtime",TimeStamp);
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].validtime_end);
									cJSON_AddStringToObject(information,"validtime_end",TimeStamp);
									cJSON_AddStringToObject(information,"userId",pInformation[i].workId);
									cJSON_AddStringToObject(information,"cardNumber",pInformation[i].cardNumber);
									cJSON_AddStringToObject(information,"password",pInformation[i].password);
									cJSON_AddStringToObject(information,"phone",pInformation[i].phone);
									cJSON_AddStringToObject(information,"certificateNumber",pInformation[i].certificateNumber);
									cJSON_AddStringToObject(information,"name",pInformation[i].name);

									
									#if 1
									cJSON* facesArray = cJSON_CreateArray();
									if(facesArray)
									{
										if(ACSHTTP_PicBase64(pInformation[i].facepath,picbuf) > 0)
										{
											cJSON* pface = cJSON_CreateString(picbuf);
											if(pface)
											{
												cJSON_AddItemToArray(facesArray,pface);
											}
										}
										cJSON_AddItemToObject(information,"faces",facesArray);
									}
									#endif
									cJSON_AddItemToArray(informationArray,information);
								}
							}
							

						}

						if(strcmp(pstCheackData->operation,ACS_FACES) == 0)							
							cJSON_AddNumberToObject(pData,"userTotal",pstCheackData->stList[j].total);
						else
							cJSON_AddNumberToObject(pData,"total",pstCheackData->stList[j].total);
						
						cJSON_AddItemToObject(pData,ACS_USERS,whiteArray);
						if(informationArray)
						{
							cJSON_AddItemToObject(pData,"information",informationArray);
						}
					}
					else if(strcmp(pstCheackData->operation,ACS_CARDS) == 0)
					{
						PRT_PRINT(ACS_DEBUG,"acslib ACS_CARDS:%s",pstCheackData->operation);
						ACS_CHECKDATA_CARD_S *pCheckData = (ACS_CHECKDATA_CARD_S *)pstCheackData->stList[j].pstData;
						if(pCheckData == NULL)
						{
							PRT_PRINT(ACS_INFO,"acslib card pCheckData:%p;j[%d]error",pCheckData,j);
							continue;
						}

						ACS_INFORMATION_CARD_S *pInformation = (ACS_INFORMATION_CARD_S *)pstCheackData->stList[j].pstInformation;
						if(pInformation)
						{
							PRT_PRINT(ACS_ERROR,"acslib card pInformation:%p;j[%d]",pInformation,j);
							informationArray = cJSON_CreateArray();
						}
						
						for(i = 0;i < pstCheackData->stList[j].total;i++)
						{
							cJSON* pCardid = cJSON_CreateString(pCheckData[i].cardId);
							cJSON_AddItemToArray(whiteArray,pCardid);

							if(informationArray)
							{
								cJSON *information = cJSON_CreateObject();
								if(information)
								{
									cJSON_AddNumberToObject(information,"source",pInformation[i].source);
									cJSON_AddNumberToObject(information,"cardType",pInformation[i].cardType);									
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].validtime_end);
									cJSON_AddStringToObject(information,"validtime_enable",TimeStamp);									
									cJSON_AddStringToObject(information,"userId",pInformation[i].workId);									
									cJSON_AddStringToObject(information,"cardNumber",pInformation[i].cardNumber);
									cJSON_AddItemToArray(informationArray,information);
								}
							}
						}
						
						cJSON_AddNumberToObject(pData,"total",pstCheackData->stList[j].total);
						cJSON_AddItemToObject(pData,ACS_CARDS,whiteArray);
						if(informationArray)
						{
							cJSON_AddItemToObject(pData,"information",informationArray);
						}
					}
					else if(strcmp(pstCheackData->operation,ACS_PWDS) == 0)
					{
						PRT_PRINT(ACS_DEBUG,"acslib ACS_PWDS:%s",pstCheackData->operation);
						ACS_CHECKDATA_PWD_S *pCheckData = (ACS_CHECKDATA_PWD_S *)pstCheackData->stList[j].pstData;
						if(pCheckData == NULL)
						{
							PRT_PRINT(ACS_INFO,"acslib pwd pCheckData:%p;j[%d]error",pCheckData,j);
							continue;
						}
						
						ACS_INFORMATION_PWD_S *pInformation = (ACS_INFORMATION_PWD_S *)pstCheackData->stList[j].pstInformation;
						if(pInformation)
						{
							PRT_PRINT(ACS_ERROR,"acslib pwd pInformation:%p;j[%d]",pInformation,j);
							informationArray = cJSON_CreateArray();
						}
						
						for(i = 0;i < pstCheackData->stList[j].total;i++)
						{
							cJSON* ppwd = cJSON_CreateString(pCheckData[i].pwd);
							cJSON_AddItemToArray(whiteArray,ppwd);

							if(informationArray)
							{
								cJSON *information = cJSON_CreateObject();
								if(information)
								{
									cJSON_AddNumberToObject(information,"source",pInformation[i].source);
									cJSON_AddNumberToObject(information,"limitnum",pInformation[i].limitnum);									
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].validtime_end);
									cJSON_AddStringToObject(information,"validtime_enable",TimeStamp);									
									cJSON_AddStringToObject(information,"userId",pInformation[i].workId);									
									cJSON_AddStringToObject(information,"password",pInformation[i].password);
									cJSON_AddItemToArray(informationArray,information);
								}
							}
						}
						
						cJSON_AddNumberToObject(pData,"total",pstCheackData->stList[j].total);
						cJSON_AddItemToObject(pData,ACS_PWDS,whiteArray);
						if(informationArray)
						{
							cJSON_AddItemToObject(pData,"information",informationArray);
						}
					}
					else if(strcmp(pstCheackData->operation,ACS_ADS) == 0)
					{
						PRT_PRINT(ACS_DEBUG,"acslib ACS_ADS:%s",pstCheackData->operation);
						ACS_CHECKDATA_AD_S *pCheckData = (ACS_CHECKDATA_AD_S *)pstCheackData->stList[j].pstData;
						if(pCheckData == NULL)
						{
							PRT_PRINT(ACS_INFO,"acslib ads pCheckData:%p;j[%d]error",pCheckData,j);
							continue;
						}
						
						ACS_INFORMATION_AD_S *pInformation = (ACS_INFORMATION_AD_S *)pstCheackData->stList[j].pstInformation;
						if(pInformation)
						{
							PRT_PRINT(ACS_ERROR,"acslib ads pInformation:%p;j[%d]",pInformation,j);
							informationArray = cJSON_CreateArray();
						}
						
						for(i = 0;i < pstCheackData->stList[j].total;i++)
						{
							cJSON* pad = cJSON_CreateString(pCheckData[i].adworkid);
							cJSON_AddItemToArray(whiteArray,pad);

							if(informationArray)
							{
								cJSON *information = cJSON_CreateObject();
								if(information)
								{
									cJSON_AddNumberToObject(information,"enable",pInformation[i].enable);
									cJSON_AddNumberToObject(information,"source",pInformation[i].source);
									cJSON_AddNumberToObject(information,"level",pInformation[i].level);
									cJSON_AddNumberToObject(information,"secord",pInformation[i].secord);
									cJSON_AddNumberToObject(information,"type",pInformation[i].type);
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].createtime);
									cJSON_AddStringToObject(information,"createtime",TimeStamp);
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].validtime);
									cJSON_AddStringToObject(information,"validtime",TimeStamp);
									memset(TimeStamp,0,sizeof(TimeStamp));
									sprintf(TimeStamp,"%d",pInformation[i].validtime_end);
									cJSON_AddStringToObject(information,"validtime_enable",TimeStamp);
									
									cJSON_AddStringToObject(information,"userId",pInformation[i].workId);									
									cJSON_AddStringToObject(information,"name",pInformation[i].name);
									cJSON_AddStringToObject(information,"format",pInformation[i].format);

									#if 1
									cJSON* facesArray = cJSON_CreateArray();
									if(facesArray)
									{
										if(ACSHTTP_PicBase64(pInformation[i].path,picbuf) > 0)
										{
											cJSON* pface = cJSON_CreateString(picbuf);
											if(pface)
											{
												cJSON_AddItemToArray(facesArray,pface);
											}
										}
										cJSON_AddItemToObject(information,"pics",facesArray);
									}
									#endif
									
									cJSON_AddItemToArray(informationArray,information);
								}
							}
						}
						
						cJSON_AddNumberToObject(pData,"total",pstCheackData->stList[j].total);
						cJSON_AddItemToObject(pData,ACS_ADS,whiteArray);
						if(informationArray)
						{
							cJSON_AddItemToObject(pData,"information",informationArray);
						}
					}
				}	
			
				if(pstCheackData->stList[j].pstInformation)
				{
					PRT_PRINT(ACS_DEBUG,"j:%d;free pstInformation:%p",j,pstCheackData->stList[j].pstInformation);
					free(pstCheackData->stList[j].pstInformation);
					pstCheackData->stList[j].pstInformation = NULL;
				}
				
				PRT_PRINT(ACS_DEBUG,"j:%d;free pstData:%p",j,pstCheackData->stList[j].pstData);
				free(pstCheackData->stList[j].pstData);
				pstCheackData->stList[j].pstData = NULL;
				//printf("acslib j[%d] free[%p] end\n",j,pCheckData);	
				//pCheckData = NULL;	
			}
			
			if((pstCheackData->size == 0))//&& (pstCheackData->code == ACS_ERR_SUCCESS))
			{
				cJSON_AddNumberToObject(pData,"total",pstCheackData->size);
			}

			if(ACS_ERR_SUCCESS == ACSCOM_OperateResult(&pstCheackData->code,pstCheackData->msg))
			{
				cJSON_AddStringToObject(pData,"result","success");
			}
			else
			{
				cJSON_AddStringToObject(pData,"result","fail");
			}

			cJSON_AddNumberToObject(pData,"code",pstCheackData->code);
			cJSON_AddStringToObject(pData,"msg",pstCheackData->msg);
			cJSON_AddStringToObject(pData,"id",pstCheackData->ID);
		}
		cJSON_AddItemToObject(pJsondata,"data",pData);
	}
	
	*pp = cJSON_PrintUnformatted(pJsondata);
	PRT_PRINT(ACS_DEBUG,"print malloc acslib *pp:%p",*pp);
	if (NULL == *pp)
	{
		ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
		return ACS_ERR_PARAMNULL;
	}
	//PRT_PRINT(ACS_DEBUG,"free acslib pJsondata:%p",pJsondata);
	ACS_freeJson((ACS_VOID**)&pJsondata,__func__);

	return ACS_ERR_SUCCESS;
}


/**
* fun:HTTP Post 推送数据检查结果
* @pstCheackData-in:输入operation 输出check的数据 ACS_CHECKDATA_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostCheckDataResult(const ACS_CHAR *pfindData,ACS_CHECKDATA_INFO_S *pstCheackData,const ACS_CHAR *pMsgId)
{

	ACS_INT ret = 0;
	ACS_INT times = 0;
	ACS_CHAR *pp = NULL;
	cJSON *ppUplodmsg = NULL;
	
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo); 
	
	ACSHTTP_PostCheckDataResult_json(pstCheackData,&ppUplodmsg,&pp,stClouldInfo.szUuid);
	
	if(pp)//CheckData
	{
		ACS_CHAR urlmsg[512]={0};
		PRT_PRINT(ACS_DEBUG,"acslib pp:%s",pp);
	
		if(strcmp(CMD_MQTTSERVER_FINDDATA,pfindData) == 0)
		{
			sprintf(urlmsg,"%s/device/dcit/api/eq/v1/findData/result?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
		else
		{
			sprintf(urlmsg,"%s/device/dcit/api/eq/v1/CheckData/result?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
			stClouldInfo.szHttpUrl,stClouldInfo.szUuid, stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
		
		PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);
		
		do
		{
			ret = ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,pp,7);
			usleep(500*1000);
			times++;
		}while (((ret > ACS_ERR_DATANULL) || (ret == -2)) && (times < RECONETMAX));
		PRT_PRINT(ACS_DEBUG,"acslib free pp:%p",pp);
		free(pp);
	}
	
	return ret;
}



/**
* fun:同步用户结果上传
* @pstFaceResult-in:输入人脸同步结果结构体信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncFaceResult_Upload(ACS_SYNCFACE_RESULT_S *pstFaceResult,const ACS_CHAR *pMsgId)
{
	if(pstFaceResult == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib SyncFace pstFaceResult point NULL");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT times = 0;
	cJSON* pData = NULL;
	cJSON* Result1 = NULL;
	cJSON* pJsondata = NULL;
	cJSON* ResultArray = NULL;
	ACS_CHAR TimeStamp [13] = {0};
	ACS_CHAR urlmsg[512]={0};
	
	//ACS_CONFIG_S stMjapiconfig = {0};
	//ACSCOM_Get_Config(&stMjapiconfig);

	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);

	sprintf(TimeStamp,"%ld",time(NULL));

	pJsondata = cJSON_CreateObject();
	if(pJsondata)
	{
		cJSON_AddStringToObject(pJsondata, "uuid", stClouldInfo.szUuid);
		cJSON_AddStringToObject(pJsondata, "timestamp",TimeStamp);
		pData = cJSON_CreateObject();
		if(pData)
		{
			cJSON_AddItemToObject(pJsondata,"data",pData);
			ResultArray = cJSON_CreateArray();
			if(ResultArray)
			{
				cJSON_AddItemToObject(pData,"data",ResultArray);
				Result1 = cJSON_CreateObject();
				if(Result1)
				{
					cJSON_AddItemToArray(ResultArray,Result1);
					cJSON_AddStringToObject(Result1,"id",pstFaceResult->ID);
					cJSON_AddStringToObject(Result1,"userId",pstFaceResult->userId);
					cJSON_AddStringToObject(Result1,"operation",pstFaceResult->operation);
					cJSON_AddStringToObject(Result1,"url",pstFaceResult->url);
					
					if(ACS_ERR_SUCCESS == ACSCOM_OperateResult(&pstFaceResult->code,pstFaceResult->msg))
					{
						cJSON_AddStringToObject(Result1,"result","success");
					}
					else
					{
						cJSON_AddStringToObject(Result1,"result","fail");
					}

					PRT_PRINT(ACS_DEBUG,"code:%d;msg:%s",pstFaceResult->code,pstFaceResult->msg);
					
					cJSON_AddNumberToObject(Result1,"code",pstFaceResult->code);
					cJSON_AddStringToObject(Result1,"msg",pstFaceResult->msg);

					ACS_CHAR* p = cJSON_PrintUnformatted(pJsondata);
					if(p)
					{
						PRT_PRINT(ACS_DEBUG,"FaceResult:%s",p);//sprintf(postData, "%s", p);
						sprintf(urlmsg,"%s/device/dcit/api/eq/v1/face/result?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
							stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
						do
						{
							ret = ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,p,2);
							usleep(500*1000);
							times++;
						}while ((ACS_ERR_DATANULL < ret) && (times < RECONETMAX));
						
						free(p);
					}
				}
			}
		}
		
		ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
		
	}

	return ret;
}


/**
* fun:同步卡号结果上传
* @pstCardResult-in:输入卡号同步结果结构体信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncCardResult_Upload(ACS_SYNCCARD_RESULT_S *pstCardResult,const ACS_CHAR *pMsgId)
{
	if(pstCardResult == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib SyncCard pstCardResult point NULL");
		return ACS_ERR_PARAMNULL;
	}

	//PRT_PRINT(ACS_DEBUG,"acslib SyncCard pstCardResult point NULL");

	
	ACS_INT ret = 0;
	ACS_INT times= 0;
	cJSON* ResultArray;
	cJSON* Result1;
	cJSON* pJsondata = NULL;
	ACS_CHAR TimeStamp[13] = {0};
	ACS_CHAR urlmsg[512]={0};
	ACS_CHAR postData[1024]={0};
	//ACS_CONFIG_S stMjapiconfig = {0};
	//ACSCOM_Get_Config(&stMjapiconfig);
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);
	
	sprintf(TimeStamp,"%ld",time(NULL));
	pJsondata = cJSON_CreateObject();
	if(pJsondata)
	{
		cJSON_AddStringToObject(pJsondata, "uuid", stClouldInfo.szUuid);
		cJSON_AddStringToObject(pJsondata, "timestamp",TimeStamp);
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{
			ResultArray = cJSON_CreateArray();
			if(ResultArray)
			{
			   	Result1 = cJSON_CreateObject();
				if(Result1)
				{
					cJSON_AddStringToObject(Result1,"id",pstCardResult->ID);
					cJSON_AddStringToObject(Result1,"cardId",pstCardResult->cardId);
					cJSON_AddStringToObject(Result1,"userId",pstCardResult->userId);
					cJSON_AddStringToObject(Result1,"operation",pstCardResult->operation);
					if(ACS_ERR_SUCCESS == ACSCOM_OperateResult(&pstCardResult->code,pstCardResult->msg))
					{
						cJSON_AddStringToObject(Result1,"result","success");
					}
					else
					{
						cJSON_AddStringToObject(Result1,"result","fail");
					}
					cJSON_AddNumberToObject(Result1,"code",pstCardResult->code);
					cJSON_AddStringToObject(Result1,"msg",pstCardResult->msg);
					cJSON_AddItemToArray(ResultArray,Result1);
				}
				cJSON_AddItemToObject(pData,"data",ResultArray);
			}
			cJSON_AddItemToObject(pJsondata,"data",pData);
			ACS_CHAR* p = cJSON_PrintUnformatted(pJsondata);
			if(p)
			{
				sprintf(postData, "%s", p);
				free(p);
			}
		}

		ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
	}
	
	sprintf(urlmsg,"%s/device/dcit/api/eq/v1/card/result?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
	stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);

	PRT_PRINT(ACS_DEBUG,"urlmsg==%s",urlmsg);	
	PRT_PRINT(ACS_DEBUG,"postData==%s",postData);
	
	do
	{
		ret = ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,postData,1);
		usleep(500*1000);
		times++;
	}while ((ACS_ERR_DATANULL < ret) && (times < RECONETMAX));

	return ret;
}



/**
* fun:同步广告结果上传
* @pstAdResult-in:输入同步广告结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncAdResult_Upload(ACS_ADVERT_RESULT_S *pstAdResult,const ACS_CHAR *pMsgId)
{
	if(pstAdResult == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib SyncAd pstAdResult point NULL");
		return ACS_ERR_PARAMNULL;
	}
	
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT times = 0;
	cJSON* ResultArray;
	cJSON* Result1;
	cJSON* pJsondata = NULL;
	ACS_CHAR TimeStamp[13] = {0};
	ACS_CHAR szmsg[64] = {0};
	ACS_CHAR urlmsg[512] = {0};
	ACS_CHAR postData[1024] = {0};
	//ACS_CONFIG_S stMjapiconfig;
	//ACSCOM_Get_Config(&stMjapiconfig);
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);
	
	sprintf(TimeStamp,"%ld",time(NULL));
	//PRT_PRINT("acslib Timestamp=%s\n",TimeStamp);

	pJsondata = cJSON_CreateObject();
	if(pJsondata)
	{
		cJSON_AddStringToObject(pJsondata, "uuid", stClouldInfo.szUuid);
		cJSON_AddStringToObject(pJsondata, "timestamp",TimeStamp);
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{
			ResultArray = cJSON_CreateArray();
			if(ResultArray)
			{
				Result1 = cJSON_CreateObject();
				if(Result1)
				{
					cJSON_AddStringToObject(Result1,"id",pstAdResult->ID);
					cJSON_AddStringToObject(Result1,"adId",pstAdResult->adId);
					cJSON_AddStringToObject(Result1,"operation",pstAdResult->operation);
					if(ACS_ERR_SUCCESS == ACSCOM_OperateResult(&pstAdResult->code,szmsg))
					{
						cJSON_AddStringToObject(Result1,"result","success");
					}
					else
					{
						cJSON_AddStringToObject(Result1,"result","fail");
					}
					cJSON_AddNumberToObject(Result1,"code",pstAdResult->code);
					cJSON_AddStringToObject(Result1,"msg",szmsg);
					cJSON_AddItemToArray(ResultArray,Result1);
				}
				cJSON_AddItemToObject(pData,"data",ResultArray);
			}
			cJSON_AddItemToObject(pJsondata,"data",pData);
			ACS_CHAR* p = cJSON_PrintUnformatted(pJsondata);
			if(p)
			{
				sprintf(postData, "%s", p);
				free(p);
			}
		}	
		ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
	}

	//if(strlen(stMjapiconfig.stClouldInfo.szPcode) > 0)
	sprintf(urlmsg,"%s/device/dcit/api/eq/v1/ad/result?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
		stClouldInfo.szHttpUrl,stClouldInfo.szUuid, stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
	//else
	//	sprintf(urlmsg,"%s/device/dcit/api/eq/v1/ad/result?uuid=%s&token=%s&msgid=%s", \stMjapiconfig.stClouldInfo.szHttpUrl, stMjapiconfig.stClouldInfo.szUuid, stMjapiconfig.stDevInfo.szToken,pMsgId);
	PRT_PRINT(ACS_DEBUG,"urlmsg==%s",urlmsg);
	PRT_PRINT(ACS_DEBUG,"postData==%s",postData);

	do
	{
		ret = ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,postData,1);
		usleep(500*1000);
		times++;
	}while ((ret > ACS_ERR_DATANULL) && (times < RECONETMAX));

	return ret;
}


/**
* fun:Post数据 上传错误信息
* @pstMsgIn-in:上传的信息     ACS_MESSAGE_S *
* @nType-in:操作类型 ACS_HTTPURL_SYCUSERS 可为0
* @pstDataIn-in:上传的具体信息 可为NULL
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_VOID
**/
ACS_VOID ACSHTTP_Message_upload(ACS_MESSAGE_S *pstMsgIn,ACS_INT nType,ACS_CHAR *pstDataIn,const ACS_CHAR *pMsgId)
{
	//ACS_INT ret = 0;//ACS_INT times = 0;
	ACS_CHAR szMessage[32] = {0};
	ACS_CHAR szTime[16] = {0};
	ACS_CHAR szMsg[64] = {0};
	ACS_CHAR urlmsg[512] = {0};
	//ACS_CONFIG_S stMjapiconfig = {0};
	//ACSCOM_Get_Config(&stMjapiconfig);
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);

	if(pstMsgIn->nCode == ACS_ERR_DATAERR)
	{
		snprintf(szMsg,sizeof(szMsg),"Missing key fields '%s'",pstMsgIn->szDetails);
	}
	else if(ACS_ERR_CHECKFAIL == pstMsgIn->nCode)
	{
		snprintf(szMsg,sizeof(szMsg),"checkFail '%s'",pstMsgIn->szDetails);
	}

	if(-1 == ACSCOM_OperateResult(&pstMsgIn->nCode,szMessage))
	{
		pstMsgIn->nCode = 1000;
		memset(szMessage,0,sizeof(szMessage));
		sprintf(szMessage,"%s","fail");
	}

	sprintf(szTime,"%ld",time(NULL));
	cJSON* pUplodmsg = cJSON_CreateObject();
	if(pUplodmsg)
	{
		cJSON_AddNumberToObject(pUplodmsg, "retCode",pstMsgIn->nCode);
		cJSON_AddStringToObject(pUplodmsg, "message",szMessage);
		cJSON_AddStringToObject(pUplodmsg, "timestamp",szTime);
		cJSON_AddStringToObject(pUplodmsg, "msg",pstMsgIn->szMsg);
		cJSON* pdata = cJSON_CreateObject();//PRT_PRINT(ACS_DEBUG,"acslib syncCardUpload pCardData point NULL ");
		if(pdata)
		{
			cJSON_AddStringToObject(pdata, "cmd",pstMsgIn->szCmd);
			if((ACS_HTTPURL_SYCUSERS == nType)&&(pstDataIn))
			{
				ACS_FACE_INFO_S *pstData = (ACS_FACE_INFO_S *)pstDataIn;
				cJSON_AddStringToObject(pdata, "userId",pstData->userId);
				cJSON_AddStringToObject(pdata, "operation",pstData->operation);
				cJSON_AddStringToObject(pdata, "userIdCard",pstData->userIdCard);
//				ACS_WLOG("Message Upload: userId:%s, operation:%s, result:%s", pstData->userId, pstData->operation, szMessage);
			}
			else if((ACS_HTTPURL_SYCCARD == nType)&&(pstDataIn))
			{
				ACS_CARD_INFO_S *pstData = (ACS_CARD_INFO_S *)pstDataIn;
				cJSON_AddStringToObject(pdata, "userId",pstData->userId);
				cJSON_AddStringToObject(pdata, "operation",pstData->operation);
				cJSON_AddStringToObject(pdata, "card",pstData->card);
//				ACS_WLOG("Message Upload: userId:%s, operation:%s, result:%s", pstData->userId, pstData->operation, szMessage);
			}
			else if((ACS_HTTPURL_SYCPWD == nType)&&(pstDataIn))
			{
				ACS_PWD_INFO_S *pstData = (ACS_PWD_INFO_S *)pstDataIn;
				cJSON_AddStringToObject(pdata, "userId",pstData->userId);
				cJSON_AddStringToObject(pdata, "operation",pstData->operation);
				cJSON_AddStringToObject(pdata, "password",pstData->pwd);
//				ACS_WLOG("Message Upload: userId:%s,pwd:%s; operation:%s, result:%s", pstData->userId,pstData->pwd, pstData->operation, szMessage);
			}
			else if((ACS_HTTPURL_SETAD == nType)&&(pstDataIn))
			{
				ACS_ADVERT_INFO_S *pstData = (ACS_ADVERT_INFO_S *)pstDataIn;
				cJSON_AddStringToObject(pdata, "adId",pstData->adId);
				cJSON_AddStringToObject(pdata, "operation",pstData->operation);
				cJSON_AddStringToObject(pdata, "adPicName",pstData->adPicName);
//				ACS_WLOG("Message Upload: adId:%s, operation:%s, result:%s", pstData->adId, pstData->operation, szMessage);
			}
			#if 1
			else if((ACS_HTTPURL_ELEVATORRULES == nType)&&(pstDataIn))
			{
				ACS_ELERULES_INFO_S *pstData = (ACS_ELERULES_INFO_S *)pstDataIn;
				cJSON_AddStringToObject(pdata, "eleRuleid",pstData->eleRulesid);
				cJSON_AddStringToObject(pdata, "operation",pstData->operation);
				cJSON_AddNumberToObject(pdata, "defaultfloor",pstData->defaultfloor);
				cJSON_AddNumberToObject(pdata, "curfloor",pstData->curfloor);
				cJSON_AddNumberToObject(pdata, "visitorfloor",pstData->visitorfloor);
				cJSON_AddNumberToObject(pdata, "type",pstData->type);
				cJSON_AddNumberToObject(pdata, "dir",pstData->stFloors.bDir);
				cJSON_AddNumberToObject(pdata, "floor",pstData->stFloors.bFloor);
				cJSON_AddStringToObject(pdata, "name",pstData->stFloors.cName);				
//				ACS_WLOG("Message Upload: eleRuleid:%s, operation:%s, result:%s", pstData->eleRulesid, pstData->operation, szMessage);
			}
			#endif
			cJSON_AddStringToObject(pdata, "detail",szMsg);
			cJSON_AddStringToObject(pdata, "id",pstMsgIn->szID);
		}
		cJSON_AddItemToObject(pUplodmsg, "data", pdata);
		ACS_CHAR *p = cJSON_PrintUnformatted(pUplodmsg);
		PRT_PRINT(ACS_DEBUG,"Post Message:%s",p);
		if(p)
		{
			sprintf(urlmsg,"%s/device/dcit/api/eq/v1/message/error?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken, stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
			//ret = 
				ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,p,2);
			free(p);
		}
		else
		{
			PRT_PRINT(ACS_ERROR,"Post Message p pointer NULL");
			ACS_WLOG("Post Message p pointer NULL");
			//ret = ACS_ERR_MALLOCFAIL;
		}
		cJSON_Delete(pUplodmsg);		
	}
	//else
	//{
		//ret = ACS_ERR_MALLOCFAIL;
	//}
//	ACS_WLOG("Post Message ret:%d",ret);
	return;
}

ACS_VOID ACSHTTP_GET_URL(int nApi,char *pstUrl,ACS_INT nPage,const ACS_CHAR *pMsgId)
{
	ACS_SYNC_MARK_S stSysctime = {0};
	ACSCOM_Get_SyncTimeCfg(&stSysctime);
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);

	switch(nApi)
	{
		case ACS_HTTPURL_SYNCTIME:
			{
				ACS_ULONG ltimeNew = time(NULL);
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/dev/synctime?t=%ld&uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,ltimeNew,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
			}
			break;
		case ACS_HTTPURL_SYCUSERS:
			{
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/face/sync?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&page=%d&pageSize=%d&eField=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stSysctime.stTime.face,stClouldInfo.szUuid,stClouldInfo.szToken,\
					stClouldInfo.szPcode,pMsgId,nPage,ACSSYNCNUM,stSysctime.stField.face,stClouldInfo.szcpuID);
			}
			break;
		case ACS_HTTPURL_SYCCARD:
			{
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/card/sync?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&page=%d&pageSize=%d&eField=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stSysctime.stTime.card,stClouldInfo.szUuid,stClouldInfo.szToken,\
					stClouldInfo.szPcode,pMsgId,nPage,ACSSYNCNUM,stSysctime.stField.card,stClouldInfo.szcpuID);
			}
			break;
		case ACS_HTTPURL_SYCPWD:
			{
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/password/sync?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&page=%d&pageSize=%d&eField=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stSysctime.stTime.pwd,stClouldInfo.szUuid,stClouldInfo.szToken,\
					stClouldInfo.szPcode,pMsgId,nPage,ACSSYNCNUM,stSysctime.stField.pwd,stClouldInfo.szcpuID);
			}
			break;
		case ACS_HTTPURL_SETAD:
			{
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/ad/sync?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&eField=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stSysctime.stTime.ad,stClouldInfo.szUuid,stClouldInfo.szToken,\
					stClouldInfo.szPcode,pMsgId,stSysctime.stField.ad,stClouldInfo.szcpuID);
			}
			break;
		case ACS_HTTPURL_LOGIN:
			{
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/device/login?uuid=%s&devicetype=%d&pcode=%s&msgid=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stClouldInfo.szUuid,1,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
			}
			break;

		case ACS_HTTPURL_UPGRADEFW:
			{
				sprintf(pstUrl,"%s/device/dcit/api/eq/v1/communicate/upgradeFw?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
			}
			break;
		case ACS_HTTPURL_REISSUERECORD:
		{			
			if(stSysctime.stTime.reissue == 0)
				stSysctime.stTime.reissue = time(NULL);
			
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/record/reissue?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&page=%d&pageSize=%d&cpuid=%s",\
				stClouldInfo.szHttpUrl,stSysctime.stTime.reissue,stClouldInfo.szUuid,stClouldInfo.szToken,\
				stClouldInfo.szPcode,pMsgId,nPage,REISSUENUM,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_SNAPPHOTO:
		{		
			ACS_ULONG ltimeNew = time(NULL);
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/snapphoto?t=%ld&uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,ltimeNew,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_SETPARAM:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/param/sync?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;	
		case ACS_HTTPURL_GETPARAM:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/param/post?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;	
		case ACS_HTTPURL_QRCODE:
		{
			sprintf(pstUrl,"%s/device/api/qcrverify3?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;	
		case ACS_HTTPURL_IDCARD:
		{
			sprintf(pstUrl,"%s/device/api/idverifyv3?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_DATACHECKONLINE:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/checkOnline?t=%ld&uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,time(NULL),stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_CALLPHONE:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/callPhone?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_CALLROOM:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/callRoom?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_CALLMANAGEMENT:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/callManagement?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_CALLHANGUP:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/callHangup?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s",\
				stClouldInfo.szHttpUrl,stClouldInfo.szUuid,stClouldInfo.szToken,stClouldInfo.szPcode,pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_CALLCHECK:
		{
			sprintf(pstUrl, "%s/device/dcit/api/eq/v1/callCheck?uuid=%s&token=%s&pcode=%s&msgid=%s&cpuid=%s", \
				stClouldInfo.szHttpUrl, stClouldInfo.szUuid, stClouldInfo.szToken, stClouldInfo.szPcode, pMsgId,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_ELEVATORRULES:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/elevatorRules/sync?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&page=%d&pageSize=%d&eField=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stSysctime.stTime.erules,stClouldInfo.szUuid,stClouldInfo.szToken,\
					stClouldInfo.szPcode,pMsgId,nPage,ACSSYNCNUM,stSysctime.stField.erules,stClouldInfo.szcpuID);
		}
			break;
		case ACS_HTTPURL_DCITTALKLIST:
		{
			sprintf(pstUrl,"%s/device/dcit/api/eq/v1/talklistindoor/post?t=%lld&uuid=%s&token=%s&pcode=%s&msgid=%s&page=%d&pageSize=%d&eField=%s&cpuid=%s",\
					stClouldInfo.szHttpUrl,stSysctime.stTime.erules,stClouldInfo.szUuid,stClouldInfo.szToken,\
					stClouldInfo.szPcode,pMsgId,nPage,ACSSYNCNUM,stSysctime.stField.erules,stClouldInfo.szcpuID);
		}
			break;
		default:
			break;
	}
	return;
}



/**
* fun:HTTP POST请求MQTT的地址等信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_DeviceLogin(const ACS_CHAR *pMsgId)
{
	ACS_INT  ret = 0;
	ACS_CHAR urlmsg[512]={0};
	
	ACS_MESSAGE_S stMsg = {0};

	ACSHTTP_GET_URL(ACS_HTTPURL_LOGIN,urlmsg,0,pMsgId);
	
	PRT_PRINT(ACS_DEBUG,"Login url:%s",urlmsg);
	
	ret = ACSHTTP_Get_Request(urlmsg,Curl_DealLogin_Back,NULL,&stMsg);
	
	if(ACS_ERR_CURLFIAL == ret || ACS_ERR_TIMEOUT == ret || ACS_ERR_DEVNOTREGISTER == ret) 
	{
		
	}
	else if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_LOGIN);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}





/**
* fun:请求平台同步用户信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 人脸数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncFaces(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId)
{
	ACS_MESSAGE_S stMsg = {0};
	ACS_BYTE times= 0;
	ACS_CHAR urlmsg[512]={0};

	ACSHTTP_GET_URL(ACS_HTTPURL_SYCUSERS,urlmsg,nPage,pMsgId);

	PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);

	do
	{
		stMsg.nCode = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncFace_Back,(ACS_CHAR *)pReissueOut,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= stMsg.nCode) || (ACS_ERR_TIMEOUT == stMsg.nCode)) && (times < RECONETMAX));


	if(ACS_ERR_NODATA == stMsg.nCode)
	{
		PRT_PRINT(ACS_INFO,"not Post Upload Message");
	}
	else if(ACS_ERR_SUCCESS != stMsg.nCode)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCUSERS);//stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}
	

	return stMsg.nCode;
}


/**
* fun:请求平台获取需要同步的卡号信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 卡号数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncCards(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};
	ACS_MESSAGE_S stMsg = {0};
	
	ACSHTTP_GET_URL(ACS_HTTPURL_SYCCARD,urlmsg,nPage,pMsgId);
	PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);

	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncCard_Back,(ACS_CHAR *)pReissueOut,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret)) && (times < RECONETMAX));

	if(ACS_ERR_NODATA == ret)
	{
		PRT_PRINT(ACS_INFO,"not Post Upload Message");
	}
	else if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCCARD);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}



/**
* fun:请求平台获取需要同步的密码信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 密码数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncPwds(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pDataOut,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};
	ACS_MESSAGE_S stMsg = {0};
	
	ACSHTTP_GET_URL(ACS_HTTPURL_SYCPWD,urlmsg,nPage,pMsgId);
	PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);

	//memset(urlmsg,0,sizeof(urlmsg));
	//sprintf(urlmsg,"https://mock.apifox.cn/m1/1533213-0-default/device/dcit/api/eq/v1/password/sync?t=%ld",time(NULL));
	
	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncPwd_Back,(ACS_CHAR *)pDataOut,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret)) && (times < RECONETMAX));

	if(ACS_ERR_NODATA == ret)
	{
		PRT_PRINT(ACS_INFO,"not Post Upload Message");
	}
	else if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCPWD);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}



/**
* fun:请求平台获取需要同步的广告信息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 卡号数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;{"timestamp":"2022-10-31T07:35:26.500+00:00","status":404,"error":"Not Found","path":"/sy/device/dcit/api/eq/v1/ad/sync"}
**/
ACS_INT ACSHTTP_Get_SyncAds(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};
	ACS_MESSAGE_S stMsg = {0};
	
	ACSHTTP_GET_URL(ACS_HTTPURL_SETAD,urlmsg,nPage,pMsgId);
	PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);

	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncAd_Back,(ACS_CHAR *)pReissueOut,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret)) && (times < RECONETMAX));

	if(ACS_ERR_NODATA == ret)
	{
		PRT_PRINT(ACS_INFO,"not Post Upload Message");
	}
	else if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SETAD);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}


/**
* fun:获取升级固件信息 HTTP
* @pstUpgadeFW-out:获取的固件信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:NULL
**/
ACS_INT ACSHTTP_GetUpgradeFW(ACS_UPGADEFW_INFO_S *pstUpgadeFW,const ACS_CHAR *pMsgId) 
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};
	ACS_MESSAGE_S stMsg = {0};

	ACSHTTP_GET_URL(ACS_HTTPURL_UPGRADEFW,urlmsg,0,pMsgId);

	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealUpgadeFW_Back,(ACS_CHAR *)pstUpgadeFW,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret)) && (times<RECONETMAX));

	if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_UPGRADEFW);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}



/**
* fun:获取考勤补发信息
* @nPage-in:页码(默认 1)
* @nPageSize-in:每页行数(默认10)
* @pReissueOut-Out:输出请求到的记录结果 ACS_REISSUE_RECORD_S*
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_ReissueRecords(int nPage,int nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};

	ACSHTTP_GET_URL(ACS_HTTPURL_REISSUERECORD,urlmsg,nPage,pMsgId);
	
	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncReissues_Back,(ACS_CHAR *)pReissueOut, NULL);
		usleep(500*1000);
		times++;
	}while(((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret))&& (times < RECONETMAX));
	
	return ret;
}


/**
* fun:请求平台获取需要同步的派梯消息
* @nPage-in:页码
* @nPageSize-in:每页记录数
* @pReissueOut-out:ACS_RECORD_S * 数据
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_Get_SyncElevatorRules(ACS_INT nPage,ACS_INT nPageSize,ACS_RECORD_S *pReissueOut,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};
	ACS_MESSAGE_S stMsg = {0};
	
	ACSHTTP_GET_URL(ACS_HTTPURL_ELEVATORRULES,urlmsg,nPage,pMsgId);

	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncElevatorRules_Back,(ACS_CHAR *)pReissueOut,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret)) && (times < RECONETMAX));

	if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_ELERULES);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}



/**
* fun:呼叫json数据拼接 
* @stUopendoor-in:开门数据结构体
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_Post_Call_json(ACS_CHAR *pUrlmsg,const ACS_CALL_INFO_S *pstPostData,ACS_CHAR **pp,ACS_CHAR *szCall)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_INT nPiclen = 0;
	ACS_INT nEncodepiclen = 0;
	ACS_CHAR picbuf[ACSPICBASEMAXLEND] = {0};
	ACS_CHAR EncodeBuf[ACSPICBASEMAXLEND] = {0};
	cJSON* pDataUpload = cJSON_CreateObject();
	if(pDataUpload)
	{
		cJSON_AddStringToObject(pDataUpload, "timestamp",pstPostData->cScreenTime);
		
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{
			if(ACS_CALL_HANGUP == pstPostData->nCallType)
			{
				//cJSON_AddStringToObject(pData, "cmd","callHangup");
				memcpy(szCall,"callHangup",10);
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLHANGUP,pUrlmsg,0,"");
			}
			else if(ACS_CALL_PHONE == pstPostData->nCallType)
			{
				memcpy(szCall,"callPhone",9);				
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLPHONE,pUrlmsg,0,"");
			}
			else if(ACS_CALL_ROOM == pstPostData->nCallType)
			{
				memcpy(szCall,"callRoom",8);
				//cJSON_AddStringToObject(pData, "cmd","callRoom");
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLROOM,pUrlmsg,0,"");
			}
			else if(ACS_CALL_MANAGEMENT == pstPostData->nCallType)
			{
				memcpy(szCall,"callManagement",14);
				//cJSON_AddStringToObject(pData, "cmd","callManagement");
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLMANAGEMENT,pUrlmsg,0,"");
			}

			cJSON_AddStringToObject(pData, "cmd",szCall);
			
			if((ACS_CALL_HANGUP == pstPostData->nCallType) || (ACS_CALL_PHONE == pstPostData->nCallType))
			{
				cJSON* pwebrtc = cJSON_CreateObject();//phone
				if(pwebrtc)
				{
					cJSON_AddStringToObject(pwebrtc, "phone",pstPostData->cDataUUID);
					cJSON_AddStringToObject(pwebrtc, "peerId",pstPostData->cDataID);
					if(ACS_CALL_PHONE == pstPostData->nCallType)
					{
						ACS_MQTT_CONFIG_S stMqttSvconfig = {0};
						ACSCOM_Get_MQTTCfg(&stMqttSvconfig);
						cJSON_AddStringToObject(pwebrtc, "topic",stMqttSvconfig.szPublishTopic);
					}
					cJSON_AddItemToObject(pData, "webrtc", pwebrtc);
				}
			}

			cJSON* psip = cJSON_CreateObject();//phone
			if(psip)
			{
				if((ACS_CALL_ROOM == pstPostData->nCallType) || (ACS_CALL_HANGUP == pstPostData->nCallType))
				{
					cJSON_AddStringToObject(psip, "roomId",pstPostData->cDataUUID);
				}
				else if(ACS_CALL_PHONE == pstPostData->nCallType)
				{
					cJSON_AddStringToObject(psip, "phone",pstPostData->cDataUUID);
				}
				
				
				if(access(pstPostData->cPhoto, F_OK) == 0)
				{
					nPiclen = ACSHTTP_FileToData(picbuf,pstPostData->cPhoto);
					if(nPiclen > 0)
					{
						ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
					}
					snprintf(picbuf,sizeof(picbuf),"data:image/jpeg;base64,%s",EncodeBuf);

					
					cJSON_AddStringToObject(psip, "image", picbuf);

					
					ACS_WLOG("nPiclen:%d;snapfacePic:%s;nEncodePicLen:%d",nPiclen,pstPostData->cPhoto,nEncodepiclen);
				}
				cJSON_AddItemToObject(pData, "sip", psip);
			}
		}
		
		cJSON_AddItemToObject(pDataUpload, "data", pData);
		*pp = cJSON_PrintUnformatted(pDataUpload);
		PRT_PRINT(ACS_DEBUG,"JSON char malloc p:%p",*pp);

		PRT_PRINT(ACS_DEBUG,"Call JSON:%s",*pp);
		
		cJSON_Delete(pDataUpload);
	}
	return ret;
}


/**
* fun:呼叫
* @pstPostData-in:post 数据
* @pPic-in:抓拍图片
* @pstDataResult-in:结果输出
* @return:0-success;
**/
ACS_INT ACSHTTP_Post_Call(const ACS_CALL_INFO_S *pstPostData,const ACS_CHAR *pPic,ACS_CALL_RESULT_S *pstDataResult)
{
	ACS_INT ret = 0;
	ACS_CHAR *p = NULL;
	//ACS_CHAR *pCall = NULL;
	ACS_CHAR urlmsg[512] = {0};//ACS_CALL_RESULT_S stDataResult = {0};
	ACS_MESSAGE_S stMsg = {0};

	ACSHTTP_Post_Call_json(urlmsg,pstPostData,&p,stMsg.szCmd);
	
	//PRT_PRINT(ACS_DEBUG,"JSON char malloc p:%p;pCall:%s",p,pCall);
	
	PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);

	if(p)
	{
		ret = ACSHTTP_Post_Request_V2(urlmsg,Curl_DealCall_Back,p,3,(ACS_CHAR *)pstDataResult,&stMsg);
		free(p);
	}
	
	if((pstDataResult) && (pstDataResult->nDataLen > 0))
	{
		if(pstDataResult->pData)
		{
			free(pstDataResult->pData);
		}
	}
	
	if(ACS_ERR_SUCCESS != ret)
	{
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,"");
	}
	
	PRT_PRINT(ACS_DEBUG,"nResult:%d;retCode:%d",pstDataResult->nResult,pstDataResult->retCode);

	return ret;
}


/**
* fun:定时同步平台时间 
* @lltimestamp-in: 需要同步的时间 ACS_UINT64
* @return:void;
**/
ACS_VOID ACSHTTP_SyncPlatformTime(ACS_UINT64 lltimestamp)
{
	if(lltimestamp)
	{
		ACS_UINT64  llocalTime = (ACS_UINT64)time(0);PRT_PRINT(ACS_DEBUG,"acslib localTime:%lld;PlatformTimestamp:%lld",llocalTime,lltimestamp);
		ACS_WLOG("acslib localTime:%lld;PlatformTimestamp:%lld",llocalTime,lltimestamp);
		if(llabs(llocalTime - lltimestamp)>10)
		{
			ACSCOM_HandleCmdCallback(ACS_CMD_DEV_SYNCTIME,(char *)&lltimestamp);//通知设备同步时间
		}
	}
	return;
}


/**
* fun:下载人脸图片
* @pDownPic-in:下载的人脸图片路径
* @pImgInfo-in:人脸信息 ACS_FACE_INFO_S
* @return:100 ACS_ERR_SUCCESS -success;
**/
ACS_INT ACSHTTP_DownOneFacePic(ACS_CHAR *pDownPic,ACS_CHAR *pImgInfo)
{
	ACS_INT nImgLen = 0;
	ACS_INT ret = ACS_ERR_SUCCESS;
	if(pImgInfo==NULL)
	{
		PRT_PRINT(ACS_ERROR,"acs_DownAndOneFacePic pImgInfo/pDownPic NULL!!!");
		return -1;
	}
	ACS_FACE_INFO_S *pFaceImgInfo = (ACS_FACE_INFO_S *)pImgInfo;

	//ACS_CHAR szPicBuf[ATTPICMAXLEND]={0};
	
	if(pFaceImgInfo == NULL||pDownPic == NULL)
	{
		PRT_PRINT(ACS_ERROR,"acs_DownAndOneFacePic pFaceImgInfo/pDownPic NULL!!!");
		return -1;
	}

	if((strlen(pFaceImgInfo->url) < 5))
	{
		PRT_PRINT(ACS_ERROR,"acs pDownPicUrl[%s] Down Error!!!",pFaceImgInfo->url);
		return ACS_ERR_PARAMERR;
	}
	
	if(((strlen(pFaceImgInfo->userId) + strlen(pFaceImgInfo->userName)) > 58) || (strlen(pFaceImgInfo->userId) < 1) || (strlen(pFaceImgInfo->userId) > 24) )//64-5-1
	{
		PRT_PRINT(ACS_ERROR,"acs UpdateEmpId UpdateEmpName Error!!!");
		return ACS_ERR_PARAMERR;//return WEBS_PARAINVA;
	}

	//pthread_mutex_lock(&gattPutCmpResultMutex);
	nImgLen = ACSHTTP_DiskDownFile(pFaceImgInfo->url,pDownPic,5);
	//pthread_mutex_unlock(&gattPutCmpResultMutex);
	if(nImgLen < 1)
	{
		PRT_PRINT(ACS_ERROR,"acs pDownPic[%s] Down Error!!!",pDownPic);
		return ACS_ERR_FILE_DOWNERR;
	}
	//ResizePic();
	//ResizePic((ACS_CHAR *)USAATT_PLATFORMPIC_PATH,(ACS_CHAR *)USAATT_PLATFORMPIC_PATH);		
	if(nImgLen >= ACSPICMAXLEND)
	{
		PRT_PRINT(ACS_ERROR,"acs pDownPic[%s] nImgLen:%d; Down Error!!!",pDownPic,nImgLen);
		//att_writeLog("[%s:%d] pszPicLen BIG[%d]!!!\n",__func__,__LINE__,nImgLen);
		return ACS_ERR_FILEBIG;
	}

#if 0	
	FILE* fp = fopen(pDownPic, "rb");
    if (!fp) 
	{
	    printf("acs pDownPic[%s] create Error!!![%s:%d]\n",pDownPic,__func__,__LINE__);
		return DORWBLIST_FILE_OPEN_ERROR;
	}
	
	ACS_CHAR *pszPicMalloc = (ACS_CHAR *)malloc(ACSPICMAXLEND);
    if(pszPicMalloc == NULL)
    {
    	printf("pszPicMalloc error!!![%s:%d]\n",__func__,__LINE__);
		//att_writeLog("[%s:%d] pszPicMalloc error!!!\n",__func__,__LINE__);
		return -1;
    }
	
	{
		fread(pszPicMalloc,1,nImgLen,fp); 
		fclose(fp);
		ret = ACS_AddFace((ACS_CHAR *)pFaceImgInfo,pszPicMalloc,nImgLen);
	}
	
	acs_free(pszPicMalloc);
	
#endif
	return ret;
}



/**
* fun:下载图片数据
* @pszfileBuf-in:下载的人脸图片路径
* @pImgInfo-in:人脸信息 ACS_FACE_INFO_S *
* @return:100 ACS_ERR_SUCCESS -success;
**/
ACS_INT ACSHTTP_DownOnePicData(struct AcsMemoryStruct *pszfileBuf,ACS_CHAR *pUrl)
{
	ACS_INT nImgLen = 0;
	ACS_INT ret = ACS_ERR_SUCCESS;
	
	if(pUrl == NULL||pszfileBuf == NULL)
	{
		PRT_PRINT(ACS_ERROR,"acs_DownOnePicData pDownPicURL NULL!!!");
		return -1;
	}

	if((strlen(pUrl) < 5))
	{
		PRT_PRINT(ACS_ERROR,"acs DownOnePicData[%s] Down Error!!!",pUrl);
		return ACS_ERR_SUCCESS;//ACS_ERR_PARAMERR;
	}
	
	nImgLen = ACSHTTP_RamDownFile(pUrl, pszfileBuf,10);
	if(nImgLen < 0)
	{
		PRT_PRINT(ACS_ERROR,"acs_Http_RamDownFile ret[%d] Down Error!!!",nImgLen);
		//return ACS_ERR_FILE_DOWNERR;
		ret = ACS_ERR_FILE_DOWNERR;
	}

	return ret;
}


/**
* fun:广告图片下载函数
* @pAddAdInfo-in:输入 ACS_ADVERT_INFO_S
* @pszPicName-in:输入下载的广告图片路径
* @return:ACS_ERR_E;
**/
ACS_INT ACSHTTP_DownAdPic(ACS_ADVERT_INFO_S *pstAdInfo,const ACS_CHAR* pszPicName)
{
	if(pstAdInfo == NULL)
	{
		PRT_PRINT(ACS_ERROR,"acs DownAdPic pstAdInfo pointer NULL Down Error!!!");
		return ACS_ERR_PARAMERR;
	}
	
	// /facepic/zlp_adverts/ad3-1623945600-0-20.jpg]
	ACS_INT ret = 0;
	ACS_INT nImgLen = 0;
	//ACS_CHAR szSuffix[8] = {0};

	if(strlen(pstAdInfo->adurl) < 4)
	{
		PRT_PRINT(ACS_ERROR,"acs DownAdPic ret[%d] Down Error!!!\n",ret);
		return ACS_ERR_PARAMERR;
	}

	//ACS_DeletAdPic(pstAdInfo);
	
	

	PRT_PRINT(ACS_DEBUG,"acs adurl:%s;szDownPic:%s.",pstAdInfo->adurl,pszPicName);
	ACS_WLOG("szDownPic:%s.adPicName:%s",pszPicName,pstAdInfo->adPicName);
	

	nImgLen = ACSHTTP_DiskDownFile(pstAdInfo->adurl,pszPicName,8);
	if(nImgLen < 1)
	{
		PRT_PRINT(ACS_ERROR,"acs DownAdPic[%s] Down Error!!",pszPicName);
		return ACS_ERR_FILE_DOWNERR;
	}

	//ResizePic();
	//ResizePic((ACS_CHAR *)USAATT_PLATFORMPIC_PATH,(ACS_CHAR *)USAATT_PLATFORMPIC_PATH);		
	
	if(nImgLen >= ACSPICMAXLEND)
	{
		PRT_PRINT(ACS_INFO,"acs DownAdPic[%s] nImgLen:%d; Down Error!!!",pstAdInfo->adPicName,nImgLen);//att_writeLog("[%s:%d] pszPicLen BIG[%d]!!!\n",__func__,__LINE__,nImgLen);
		return ACS_ERR_FILEBIG;
	}
	
	return ACS_ERR_SUCCESS;
}



/**
* fun:获取图片后缀
* @pPicPath:输入图片路径名
* @pSuffix:输出图片后缀(.jpg/.png/.bmp)
* @return:ACS_ERR_E;
**/
ACS_INT ACSHTTP_GetImageSuffix(ACS_CHAR *pPicPath, ACS_CHAR *pSuffix)
{
	ACS_CHAR *p = NULL;
	const ACS_CHAR pch = '.';
	if(pPicPath == NULL)
	{
		PRT_PRINT(ACS_ERROR,"acs pDownAd pPicPath pointer NULL !!!");
		return ACS_ERR_PARAMERR;
	}
	p = strrchr(pPicPath,pch);
	if(p && p[1] && (pSuffix))
	{
		snprintf(pSuffix,5,"%s",p);
		return ACS_ERR_SUCCESS;
	}
	else
	{
		PRT_PRINT(ACS_ERROR,"acs pDownAd pPicPath PathName[%s] error!!!",pPicPath);
	}
	
	return ACS_ERR_PARAMERR;
}


/**
* fun:上传抓拍图片
* @psnapphotopath:输入图片路径名
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @* @return:ACS_ERR_E;
**/
ACS_INT ACSHTTP_SnapPhoto_Upload(ACS_CHAR *psnapphotopath,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times=0;
	ACS_CHAR urlmsg[256]={0};

	ACSHTTP_GET_URL(ACS_HTTPURL_SNAPPHOTO,urlmsg,0,pMsgId);

	PRT_PRINT(ACS_DEBUG,"urlmsg=%s",urlmsg);
	
	do
	{
		ret = ACSHTTP_PostFile_Request(urlmsg,Curl_DealSnapPhoto_Back,NULL,psnapphotopath,NULL);
		usleep(500*1000);
		times++;
	}while((ret != 0) && (times < RECONETMAX));
	
	return ret;
}


/**
* fun:请求平台获取需要同步的参数信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_SyncParam(const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR urlmsg[512]={0};
	ACS_MESSAGE_S stMsg = {0};
	
	ACSHTTP_GET_URL(ACS_HTTPURL_SETPARAM,urlmsg,0,pMsgId);

	do
	{
		ret = ACSHTTP_Get_Request(urlmsg,Curl_DealSyncParam_Back,NULL,&stMsg);
		usleep(500*1000);
		times++;
	}while (((ACS_ERR_DATANULL <= ret) || (ACS_ERR_TIMEOUT == ret)) && (times < RECONETMAX));

	if(ACS_ERR_SUCCESS != ret)
	{
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCPARAM);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}


/**
* fun:HTTP Post 推送设备参数信息
* @pstCheackData-in:输入operation 输出check的数据 ACS_CHECKDATA_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostDevParam_json(ACS_CHAR **pp)
{
	ACS_CHAR TimeStamp[13] = {0};
	sprintf(TimeStamp,"%ld",time(NULL));
	ACS_DEVCFG_INFO_S stAcsDevinfo = {0};
	cJSON* pJsondata = cJSON_CreateObject();
	if(pJsondata)
	{
		cJSON_AddNumberToObject(pJsondata, "retCode",200);
		cJSON_AddStringToObject(pJsondata, "message","success");
		cJSON_AddStringToObject(pJsondata, "timestamp",TimeStamp);
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{
			cJSON* CameraConfig = cJSON_CreateObject();
			if(CameraConfig)
			{
				ACS_CAMCFG_S stCameraCfg = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
				stAcsDevinfo.nType = ACS_DEV_PARAM_CAMCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_CAMCFG_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&stCameraCfg;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);//printf("\033[1m\033[40;31m	szDevName:%s  [%s:%d]\033[0m\n",stCameraCfg.szDevName, __func__, __LINE__);	
				
				cJSON_AddStringToObject(CameraConfig, "DeviceName", stCameraCfg.szDevName);				
				cJSON_AddItemToObject(pData,"CameraConfig",CameraConfig);
			}

			cJSON* SfaceCfg = cJSON_CreateObject();
			if(SfaceCfg)
			{
				ACS_SFACECFG_S stsFaceCfg = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
				stAcsDevinfo.nType = ACS_DEV_PARAM_SFACECFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_SFACECFG_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&stsFaceCfg;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);

				//printf("\033[1m\033[40;31m	dwAcsScene:%d;dwHacknessenble:%d  [%s:%d]\033[0m\n",stsFaceCfg.dwAcsScene,stsFaceCfg.dwHacknessenble, __func__, __LINE__);	
				
				cJSON_AddNumberToObject(SfaceCfg, "AcsScene",stsFaceCfg.dwAcsScene);
				cJSON_AddNumberToObject(SfaceCfg, "CmpThreshold",stsFaceCfg.dwCmpThreshold);
				cJSON_AddNumberToObject(SfaceCfg, "CmpSingleThreshold",stsFaceCfg.dwCmpSingleThreshold);
				cJSON_AddNumberToObject(SfaceCfg, "FaceMinSize",stsFaceCfg.dwFaceMinSize);
				cJSON_AddNumberToObject(SfaceCfg, "FaceMaxSize",stsFaceCfg.dwFaceMaxSize);
				cJSON_AddNumberToObject(SfaceCfg, "Hacknessenble",stsFaceCfg.dwHacknessenble);
				cJSON_AddNumberToObject(SfaceCfg, "Hacknessthreshold",stsFaceCfg.dwHacknessthreshold);				
				cJSON_AddNumberToObject(SfaceCfg, "HacknessRank",stsFaceCfg.dwHacknessRank);
				cJSON_AddNumberToObject(SfaceCfg, "MaskEnable",stsFaceCfg.dwMaskEnable);
				cJSON_AddNumberToObject(SfaceCfg, "Hacknessthreshold",stsFaceCfg.dwFaceSensitivity);				
				cJSON_AddItemToObject(pData,"SfaceCfg",SfaceCfg);
			}
			
			cJSON* pAcsCfg = cJSON_CreateObject();
			if(pAcsCfg)
			{
				ACS_ACSCFG_S stAcsCfg = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
				stAcsDevinfo.nType = ACS_DEV_PARAM_ACSCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_ACSCFG_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCfg;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				
				//printf("\033[1m\033[40;31m	dwAcsState:%d;dwRecoredPicEnable:%d  [%s:%d]\033[0m\n",stAcsCfg.dwAcsState,stAcsCfg.dwRecoredPicEnable, __func__, __LINE__);	

				cJSON_AddNumberToObject(pAcsCfg, "AcsState",stAcsCfg.dwAcsState);
				cJSON_AddNumberToObject(pAcsCfg, "RecoredPicEnable",stAcsCfg.dwRecoredPicEnable);
				cJSON_AddNumberToObject(pAcsCfg, "StrangeRecEnable",stAcsCfg.dwStrangeRecEnable);
				cJSON_AddNumberToObject(pAcsCfg, "StrangeTik",stAcsCfg.dwStrangeTik);			
				cJSON_AddNumberToObject(pAcsCfg, "Relay1DelayTime",stAcsCfg.dwDelay1Time);
				cJSON_AddNumberToObject(pAcsCfg, "Relay1IoType",stAcsCfg.dwIo1Type);
				cJSON_AddNumberToObject(pAcsCfg, "ConfirmMode",stAcsCfg.dwConfirmMode);
				cJSON_AddNumberToObject(pAcsCfg, "ConfirmWayMask",stAcsCfg.dwConfirmWayMask);
				cJSON_AddItemToObject(pData,"AcsCfg",pAcsCfg);
			}
			
			cJSON* AudioStream = cJSON_CreateObject();
			if(AudioStream)
			{
				ACS_AUDIOSTREAM_S stAudiosCfg = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));	
				stAcsDevinfo.nType = ACS_DEV_PARAM_AUDIOCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_AUDIOSTREAM_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&stAudiosCfg;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				cJSON_AddNumberToObject(AudioStream, "Enable",stAudiosCfg.byEnable);
				cJSON_AddNumberToObject(AudioStream, "VolumeIn",stAudiosCfg.byVolumeIn);
				cJSON_AddNumberToObject(AudioStream, "VolumeOut",stAudiosCfg.byVolumeOut);
				cJSON_AddItemToObject(pData,"AudioStream",AudioStream);
			}
			
			cJSON* pCloudCfg = cJSON_CreateObject();
			if(pCloudCfg)
			{
				ACS_CLOUDCFG_S stAcsCloud = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_CLOUDCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_CLOUDCFG_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&stAcsCloud;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				cJSON_AddNumberToObject(pCloudCfg, "CloudEnable",stAcsCloud.byEnable);
				cJSON_AddStringToObject(pCloudCfg, "CloudIPAddrCfg",stAcsCloud.szCloudIPAddr);
				cJSON_AddStringToObject(pCloudCfg, "CloudID",stAcsCloud.szCloudID);
				cJSON_AddItemToObject(pData,"CloudCfg",pCloudCfg);
			}
	
			cJSON* pWirednetwork = cJSON_CreateObject();
			if(pWirednetwork)
			{
				ACS_WIREDNET_S wirednet = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_WIREDNETCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_WIREDNET_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&wirednet;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				
				cJSON_AddNumberToObject(pWirednetwork, "dhcp",wirednet.bDHCP);
				cJSON_AddStringToObject(pWirednetwork, "ipAddress",wirednet.ipAddress);
				cJSON_AddStringToObject(pWirednetwork, "subnetMask",wirednet.subnetMask);
				cJSON_AddStringToObject(pWirednetwork, "SecondaryDNS",wirednet.SecondaryDNS);
				cJSON_AddStringToObject(pWirednetwork, "PrimaryDNS",wirednet.PrimaryDNS);
				cJSON_AddStringToObject(pWirednetwork, "MACAddress",wirednet.MACAddress);
				cJSON_AddStringToObject(pWirednetwork, "DefaultGateway",wirednet.DefaultGateway);
				cJSON_AddItemToObject(pData,"Wirednetwork",pWirednetwork);
			}

			cJSON* pSysteminfoVersion = cJSON_CreateObject();
			if(pSysteminfoVersion)
			{
				ACS_SYSTEMVERSION_S systemVer = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_SYNTEMVERCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_SYSTEMVERSION_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&systemVer;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				
				cJSON_AddStringToObject(pSysteminfoVersion, "uuid",systemVer.uuid);
				cJSON_AddStringToObject(pSysteminfoVersion, "cpuid",systemVer.cpuid);
				cJSON_AddStringToObject(pSysteminfoVersion, "DeviceAlgoVersion",systemVer.DeviceAlgoVersion);
				cJSON_AddStringToObject(pSysteminfoVersion, "DeviceOnvifVersion",systemVer.DeviceOnvifVersion);
				cJSON_AddStringToObject(pSysteminfoVersion, "DeviceSoftVersion",systemVer.DeviceSoftVersion);
				cJSON_AddStringToObject(pSysteminfoVersion, "DeviceUiVersion",systemVer.DeviceUiVersion);
				cJSON_AddStringToObject(pSysteminfoVersion, "DeviceWebVersion",systemVer.DeviceWebVersion);
				cJSON_AddItemToObject(pData,"SysteminfoVersion",pSysteminfoVersion);
			}

			cJSON* pBuildingCfg = cJSON_CreateObject();
			if(pBuildingCfg)
			{
				ACS_BUILDINGINFO_S building = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_BUILDINGCFG;
				stAcsDevinfo.dwDataLen = sizeof(ACS_BUILDINGINFO_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&building;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);				
				cJSON_AddStringToObject(pBuildingCfg, "AControlIp",building.AControlIp);
				cJSON_AddStringToObject(pBuildingCfg, "AreaID",building.AreaID);
				cJSON_AddStringToObject(pBuildingCfg, "BuildingID",building.BuildingID);
				cJSON_AddStringToObject(pBuildingCfg, "BusDevName",building.BusDevName);
				cJSON_AddNumberToObject(pBuildingCfg, "BusDevType",building.bBusDevType);
				cJSON_AddStringToObject(pBuildingCfg, "CenterIP",building.CenterIP);
				cJSON_AddStringToObject(pBuildingCfg, "DevNumber",building.DevNumber);
				//cJSON_AddNumberToObject(pBuildingCfg, "DeviceType",building.bDeviceType);
				cJSON_AddStringToObject(pBuildingCfg, "RoomID",building.RoomID);
				cJSON_AddNumberToObject(pBuildingCfg, "SlaveType",building.bSlaveType);
				cJSON_AddStringToObject(pBuildingCfg, "UnitID",building.UnitID);
				cJSON_AddItemToObject(pData,"BuildingCfg",pBuildingCfg);
			}
			
			cJSON* pCallProtocolCfg = cJSON_CreateObject();
			if(pCallProtocolCfg)
			{
				ASC_DEVALLCFG_S CallProtocol = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_CALLPROTOCOL;
				stAcsDevinfo.dwDataLen = sizeof(ASC_DEVALLCFG_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&CallProtocol;	
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);
				cJSON* pSipCfg = cJSON_CreateObject();
				if(pSipCfg)
				{
					cJSON_AddStringToObject(pSipCfg, "server_addrs",CallProtocol.stSip.cAddrs);
					cJSON_AddStringToObject(pSipCfg, "account",CallProtocol.stSip.cAccount);
					cJSON_AddStringToObject(pSipCfg, "password",CallProtocol.stSip.cPassword);
					cJSON_AddNumberToObject(pSipCfg, "server_port",CallProtocol.stSip.nPort);
					cJSON_AddItemToObject(pCallProtocolCfg,"sip",pSipCfg);
				}
				cJSON* prongcloudCfg = cJSON_CreateObject();
				if(prongcloudCfg)
				{
					cJSON_AddStringToObject(prongcloudCfg, "appkey",CallProtocol.stRongCloud.cAppkey);
					cJSON_AddStringToObject(prongcloudCfg, "token",CallProtocol.stRongCloud.cToken);
					cJSON_AddItemToObject(pCallProtocolCfg,"rongcloud",prongcloudCfg);
				}

				cJSON* pWebRtcCfg = cJSON_CreateObject();
				if(pWebRtcCfg)
				{
					cJSON_AddStringToObject(pWebRtcCfg, "media_initkey",CallProtocol.stWebRtc.cKey);
					cJSON_AddStringToObject(pWebRtcCfg, "media_addrs",CallProtocol.stWebRtc.cAddrs);
					cJSON_AddStringToObject(pWebRtcCfg, "media_license",CallProtocol.stWebRtc.cLicense);
					cJSON_AddItemToObject(pCallProtocolCfg,"webrtc",pWebRtcCfg);
				}

				cJSON* pcallruleCfg = cJSON_CreateObject();
				if(pcallruleCfg)
				{
					cJSON* pownercfg = cJSON_CreateArray();
					if(pownercfg)
					{
						for(ACS_INT i = 0; i<2; i++)
						{	
							cJSON* powner = cJSON_CreateNumber(CallProtocol.stCallRule.bOwner[i]);
							if(powner)
							{
								cJSON_AddItemToArray(pownercfg,powner);
							}
						}
						cJSON_AddItemToObject(pcallruleCfg, "owner", pownercfg);
					}
					
					cJSON* ppropertycfg = cJSON_CreateArray();
					if(ppropertycfg)
					{
						for(ACS_INT i = 0; i<2; i++)
						{	
							cJSON* pProperty = cJSON_CreateNumber(CallProtocol.stCallRule.bProperty[i]);
							if(pProperty)
							{
								cJSON_AddItemToArray(ppropertycfg,pProperty);
							}
						}
						cJSON_AddItemToObject(pcallruleCfg, "property", ppropertycfg);
					}
					
					cJSON_AddItemToObject(pCallProtocolCfg,"callrule",pcallruleCfg);
				}
				
				cJSON_AddItemToObject(pData,"CallProtocolCfg",pCallProtocolCfg);
			}

			cJSON* pCallCfg = cJSON_CreateObject();
			if(pCallCfg)
			{
				ACS_TALKPARAM_S talkParam = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_CALLSETING;
				stAcsDevinfo.dwDataLen = sizeof(ACS_TALKPARAM_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&talkParam;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);				
				
				cJSON_AddNumberToObject(pCallCfg, "RingToneDuration",talkParam.dwRingToneDuration);//响铃时间
				cJSON_AddNumberToObject(pCallCfg, "TalkDuration",talkParam.dwTalkDuration);//通话时间
				
				cJSON_AddItemToObject(pData,"CallCfg",pCallCfg);
			}
/**
			cJSON* pTelemaintenanceCfg = cJSON_CreateObject();
			if(pTelemaintenanceCfg)
			{
				ACS_TELEMAINTENANCE_S telemaintenanceParam = {0};
				memset(&stAcsDevinfo,0,sizeof(ACS_DEVCFG_INFO_S));
				stAcsDevinfo.nType = ACS_DEV_PARAM_TELEMAINTENANCE;
				stAcsDevinfo.dwDataLen = sizeof(ACS_TELEMAINTENANCE_S);
				stAcsDevinfo.szData  = (ACS_CHAR *)&telemaintenanceParam;		
				ACSCOM_HandleCmdCallback(ACS_CMD_SYNCPARAM,(ACS_CHAR *)&stAcsDevinfo);			
				cJSON_AddNumberToObject(pTelemaintenanceCfg, "enable",telemaintenanceParam.dwEnable);//开关 默认关				
				cJSON_AddItemToObject(pData,"RemoteMaintenanceCfg",pTelemaintenanceCfg);
			}
**/
			
			cJSON_AddItemToObject(pJsondata,"data",pData);
		}

		*pp = cJSON_PrintUnformatted(pJsondata);
		PRT_PRINT(ACS_DEBUG,"print malloc acslib *pp:%p",*pp);
		if (NULL == *pp)
		{
			ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
			return ACS_ERR_PARAMNULL;
		}		
	}
	ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
	return ACS_ERR_SUCCESS;
}


/**
* fun:上传设备参数到平台
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostDevParam(const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR *pp = NULL;
	ACSHTTP_PostDevParam_json(&pp);
	if(pp)
	{
		ACS_CHAR urlmsg[512]={0};
		PRT_PRINT(ACS_DEBUG,"acslib pp:%s",pp);
		ACSHTTP_GET_URL(ACS_HTTPURL_GETPARAM,urlmsg,0,pMsgId);
		PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);
	
		do
		{
			//ret = ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,pp,5);
			ret = ACSHTTP_Post_Request(urlmsg,Curl_PostDealSyncParam_Back,pp,6);
			usleep(500*1000);
			times++;
		}while(((ACS_ERR_DATANULL <= ret) || (-2 == ret)) && (times < RECONETMAX));
		
		PRT_PRINT(ACS_DEBUG,"acslib free pp:%p",pp);
		
		free(pp);
		pp = NULL;
	}

	if(ACS_ERR_SUCCESS != ret)
	{
		ACS_MESSAGE_S stMsg = {0};
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_GETDEVPARAM);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}


/**
* fun:HTTP Post 推送设备参数信息
* @pstCheackData-in:输入operation 输出check的数据 ACS_CHECKDATA_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_VOID ACSHTTP_PostDevDcitTalkList_json(ACS_CHAR **pp)
{
	//ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_CHAR TimeStamp[13] = {0};
	sprintf(TimeStamp,"%ld",time(NULL));
	ACS_CHAR DcitList[ACSPICBASEMAXLEND]={0}; 
	//ret = 
	ACSCOM_HandleCmdCallback(ACS_CMD_DCITTALKLIST,(ACS_CHAR *)&DcitList);
	cJSON* pJsondata = cJSON_CreateObject();
	if(pJsondata)
	{
		cJSON_AddNumberToObject(pJsondata, "retCode",200);
		cJSON_AddStringToObject(pJsondata, "message","success");
		cJSON_AddStringToObject(pJsondata, "timestamp",TimeStamp);
		cJSON* pDcitList = cJSON_CreateString(DcitList);
		if(pDcitList)
		{
			cJSON_AddItemToObject(pJsondata,"data",pDcitList);
		}
		*pp = cJSON_Print(pJsondata);
		PRT_PRINT(ACS_DEBUG,"print malloc acslib *pp:%p",*pp);
		if (NULL == *pp)
		{
			ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
			return;
		}
		ACS_freeJson((ACS_VOID**)&pJsondata,__func__);
	}
	return;
}



/**
* fun:上传设备分机列表到平台
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_PostDevDcitTalkList(const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_INT times= 0;
	ACS_CHAR *pp = NULL;

	ACSHTTP_PostDevDcitTalkList_json(&pp);

	if(pp)
	{
		ACS_CHAR urlmsg[512]={0};
		PRT_PRINT(ACS_DEBUG,"acslib pp:%s",pp);
		ACSHTTP_GET_URL(ACS_HTTPURL_DCITTALKLIST,urlmsg,0,pMsgId);
		PRT_PRINT(ACS_DEBUG,"urlmsg:%s",urlmsg);
		
		do
		{
			ret = ACSHTTP_Post_Request(urlmsg,Curl_DealCommonPost_Back,pp,5);
			usleep(500*1000);
			times++;
		}while(((ACS_ERR_DATANULL <= ret) || (-2 == ret)) && (times < RECONETMAX));
		
		PRT_PRINT(ACS_DEBUG,"print free acslib pp:%p;%p",pp,*pp);
		free(pp);
	}

	if(ACS_ERR_SUCCESS != ret)
	{
		ACS_MESSAGE_S stMsg = {0};
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_GETDCITTALKLIST);
		stMsg.nCode = ret;
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	return ret;
}


/**
* fun:二维码校验接口
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_HealthQRCode_Check(ACS_HEALTH_QRCODE_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_CHAR urlmsg[512]={0};//ACS_CHAR PostData[1024*1024]={0};
	//ACS_CONFIG_S stMjapiconfig = {0};
	//ACSCOM_Get_Config(&stMjapiconfig);
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);

	if(strlen(stClouldInfo.szToken) <= 0)
	{
		return ACS_ERR_PARAMERR;
	}

	cJSON* pQRCode = cJSON_CreateObject();
	if(pQRCode)
	{
		cJSON_AddStringToObject(pQRCode, "hardwareId", stClouldInfo.szUuid);
		cJSON_AddStringToObject(pQRCode, "temp", "0");//cJSON_AddStringToObject(pQRCode, "photo","");
		cJSON_AddStringToObject(pQRCode, "qrCode",pPostinfo->QrValue);//cJSON_AddStringToObject(pQRCode, "checkResult","-1");
		ACS_CHAR* p = cJSON_PrintUnformatted(pQRCode);//sprintf(PostData,"%s",p);
		if(p)
		{
			ACSHTTP_GET_URL(ACS_HTTPURL_QRCODE,urlmsg,0,pMsgId);
			ret = ACSHTTP_Post_Request_V3(urlmsg,Curl_DealHealthCodeInfo_Back,p,(ACS_CHAR *)pHealthinfo,3);
			free(p);
			p = NULL;
		}
		cJSON_Delete(pQRCode);
	}
	
	return ret;
}


/**
* fun:人证健康码身份证校验接口
* @pPostinfo-in:请求参数 ACS_HEALTH_IDCARD_S * 身份证信息外部进行加密
* @pHealthinfo-out:解析的健康码信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_HealthCodeIDCard_Check(ACS_HEALTH_IDCARD_S *pPostinfo, ACS_HEALTHCODE_INFO_S *pHealthinfo,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = 0;
	ACS_CHAR urlmsg[256]={0};//ACS_CHAR PostData[1024*1024]={0};
	//ACS_CONFIG_S stMjapiconfig ={ 0 };
	//ACSCOM_Get_Config(&stMjapiconfig);
	ACS_CLOUDLOGIN_S stClouldInfo = {0};
	ACSCOM_Get_DoorClouldConfig(&stClouldInfo);

	if(strlen(stClouldInfo.szToken) <= 0)
	{
		return ACS_ERR_PARAMERR;
	}

	cJSON* pQRCode = cJSON_CreateObject();
	if(pQRCode)
	{
		cJSON_AddStringToObject(pQRCode, "hardwareId", stClouldInfo.szUuid);
		cJSON_AddStringToObject(pQRCode, "bodyTemperature",	"0");
		cJSON_AddStringToObject(pQRCode, "name", pPostinfo->name);
		cJSON_AddStringToObject(pQRCode, "personId", pPostinfo->CardId);
		cJSON_AddStringToObject(pQRCode, "checkResult", "-1");
		cJSON_AddStringToObject(pQRCode, "faceBase64", "");
		
		ACS_CHAR* p = cJSON_PrintUnformatted(pQRCode);//sprintf(PostData,"%s",p);
		if(p)
		{
			ACSHTTP_GET_URL(ACS_HTTPURL_IDCARD,urlmsg,0,pMsgId);
			ret = ACSHTTP_Post_Request_V3(urlmsg,Curl_DealHealthCodeInfo_Back,p,(ACS_CHAR *)pHealthinfo,3);
			free(p);
			p = NULL;
		}
		cJSON_Delete(pQRCode);
	}


	return ret;
}




/**
* fun:数据在线验证接口(二维码数据上传)
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_DataCheckOnline_json(const ACS_DATACHECKONLINE_INFO_S *pPostinfo, ACS_CHAR **pOut)
{
	ACS_INT nEncodepiclen = 0;	
	ACS_INT nPiclen = 0;
	ACS_CHAR picbuf[ACSPICBASEMAXLEND] = {0};
	ACS_CHAR EncodeBuf[ACSPICBASEMAXLEND] = {0};

	cJSON* pDataUpload = cJSON_CreateObject();
	if(pDataUpload)
	{
		cJSON* pData = cJSON_CreateObject();
		if(pData)
		{
			cJSON_AddStringToObject(pData, "type",pPostinfo->szType);
			if(ACS_ERR_SUCCESS == strncmp(ACSTYPE_FACE,pPostinfo->szType,4))
			{
				if(access(pPostinfo->szPhoto, F_OK) == 0)
				{
					nPiclen = ACSHTTP_FileToData(picbuf,pPostinfo->szPhoto);
					if(nPiclen > ACS_ERR_SUCCESS)
					{
						ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
					}
					memset(picbuf, ACS_ERR_SUCCESS, sizeof(picbuf));
					snprintf(picbuf,sizeof(picbuf),"data:image/jpeg;base64,%s",EncodeBuf);
					cJSON_AddStringToObject(pData, "data", picbuf);
					ACS_WLOG("nPiclen:%d;snapfacePic:%s;nEncodePicLen:%d",nPiclen,pPostinfo->szPhoto,nEncodepiclen);
				}
				else
				{
					cJSON_AddStringToObject(pData, "data", "null");
					ACS_WLOG("snapfacePic [%s] NULL",pPostinfo->szPhoto);
				}
			}
			else if((pPostinfo->dwDataLen)&&(pPostinfo->pData))
			{
				cJSON_AddStringToObject(pData, "data",pPostinfo->pData);
			}
		}
		cJSON_AddItemToObject(pDataUpload, "data", pData);
		cJSON_AddStringToObject(pDataUpload, "timestamp",pPostinfo->szScreenTime);
		*pOut = cJSON_PrintUnformatted(pDataUpload);
		PRT_PRINT(ACS_DEBUG,"print malloc acslib *pp:%p",*pOut);
		if(NULL == *pOut)
		{
			ACS_freeJson((ACS_VOID**)&pDataUpload,__func__);
			return ACS_ERR_PARAMNULL;
		}
		ACS_freeJson((ACS_VOID**)&pDataUpload,__func__);
	}
	return ACS_ERR_SUCCESS;
}


/**
* fun:数据在线验证接口(二维码数据上传)
* @pPostinfo:-in:请求参数
* @pPostinfo:-out:返回的结果信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;
**/
ACS_INT ACSHTTP_DataCheckOnline(const ACS_DATACHECKONLINE_INFO_S *pPostinfo, ACS_DATACHECKONLINE_RESULT_S *presult,const ACS_CHAR *pMsgId)
{
	ACS_CHAR urlmsg[512]={0};
	ACS_CHAR *pp = NULL;
	ACS_INT ret = 0;
	ACS_INT nTime = 3;

	if(pPostinfo->curltimeout)
	{
		nTime = pPostinfo->curltimeout;
	}
	
	ACSHTTP_DataCheckOnline_json(pPostinfo,&pp);
	if(pp)
	{
		ACSHTTP_GET_URL(ACS_HTTPURL_DATACHECKONLINE,urlmsg,0,pMsgId);
		ret = ACSHTTP_Post_Request_V3(urlmsg,Curl_DealDataCheckOnline_Back,pp,(ACS_CHAR *)presult,nTime);
		PRT_PRINT(ACS_DEBUG,"acslib free pp:%p",pp);
		free(pp);
		pp = NULL;
	}
	return ret;
}


/**
* fun:呼叫校验(房间号/手机号/管理处) 注意 pstCallRes->dwDataLen != 0时 pstData 需要释放
* @pstCall-in:请求参数 ACS_DEVICE_CALL_S * 呼叫校验结构体 /device/dcit/api/eq/v1/callCheck?t={timestamp}&uuid={uuid}&token={token}&pcode={pcode}
* @pstCallRes-out:解析的呼叫结果
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_CallCheck(ACS_CALL_CHECK_S* pstCall, ACS_CALL_CHECK_RESULT_S* pstCallRes, const ACS_CHAR* pMsgId)
{
	ACS_INT ret = 0;
	ACS_CHAR urlmsg[512] = {0};
	ACS_CHAR timestamp[16] = {0};
	sprintf(timestamp, "%ld", time(NULL));

	cJSON* pCallReq = cJSON_CreateObject();
	if(pCallReq)
	{
		cJSON_AddStringToObject(pCallReq, "timestamp", timestamp);
		cJSON* pCallReqData = cJSON_CreateObject();
		if(pCallReqData)
		{
			cJSON_AddStringToObject(pCallReqData, "type", pstCall->type);
			cJSON_AddStringToObject(pCallReqData, "data", pstCall->callId);
			cJSON_AddItemToObject(pCallReq, "data", pCallReqData);

			ACS_CHAR* p = cJSON_PrintUnformatted(pCallReq);//sprintf(PostData,"%s",p);
			if(p)
			{
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLCHECK, urlmsg, 0, pMsgId);
				ret = ACSHTTP_Post_Request_V3(urlmsg, Curl_DealCallCheck_Back, p, (ACS_CHAR*)pstCallRes,3);
				free(p);
				p = NULL;
			}
		}
		cJSON_Delete(pCallReq);
	}

	return ret;
}

/**
* fun:呼叫手机APP
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体 /device/dcit/api/eq/v1/callPhone?t={timestamp}&uuid={uuid}&token={token}&pcode={pcode}
* @pstResult-out:操作返回结果
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_WebrtcCallPhone(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult, const ACS_CHAR* pMsgId)
{
	ACS_INT ret = -1;
	ACS_CHAR urlmsg[256] = {0};
	ACS_CHAR timestamp[16] = {0};
	sprintf(timestamp, "%ld", time(NULL));

	ACS_MQTT_CONFIG_S stMqttConfig;
	ACSCOM_Get_MQTTCfg(&stMqttConfig);

	cJSON* pCallReq = cJSON_CreateObject();
	if(pCallReq)
	{
		cJSON* pCallReqData = cJSON_CreateObject();
		cJSON_AddStringToObject(pCallReq, "timestamp", timestamp);
		if(pCallReqData)
		{
			cJSON_AddStringToObject(pCallReqData, "cmd", "callPhone");
			cJSON* pWebrtc = cJSON_CreateObject();
			if(pWebrtc)
			{
				cJSON_AddStringToObject(pWebrtc, "phone", pstCall->phone);
				cJSON_AddStringToObject(pWebrtc, "peerId", pstCall->perrId);
				cJSON_AddStringToObject(pWebrtc, "topic", stMqttConfig.szSubopicTopic);
				cJSON_AddItemToObject(pCallReqData, "webrtc", pWebrtc);
			}
			cJSON_AddItemToObject(pCallReq, "data", pCallReqData);
			ACS_CHAR* p = cJSON_PrintUnformatted(pCallReq);//sprintf(PostData,"%s",p);
			if(p)
			{
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLPHONE, urlmsg, 0, pMsgId);
				ret = ACSHTTP_Post_Request_V3(urlmsg, Curl_DealCallRequest_Back, p, (ACS_CHAR*)pstResult,3);
				free(p);
				p = NULL;
			}
		}
		cJSON_Delete(pCallReq);
	}

	return ret;
}

/**
* fun:呼叫挂断(手机APP)
* @pstCall-in:请求参数 ACS_CALL_WEBRTC_S * 呼叫校验结构体 /device/dcit/api/eq/v1/callHangup?t={timestamp}&uuid={uuid}&token={token}&pcode={pcode}
* @pstResult-out:操作返回结果
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:0-success;<0-fail
**/
ACS_INT ACSHTTP_WebrtcHangUp(ACS_CALL_WEBRTC_S* pstCall, ACS_CALL_RESULT_S* pstResult, const ACS_CHAR* pMsgId)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_CHAR urlmsg[512] = {0};
	ACS_CHAR timestamp[16] = {0};
	sprintf(timestamp, "%ld", time(NULL));

	cJSON* pCallReq = cJSON_CreateObject();
	if(pCallReq)
	{
		cJSON_AddStringToObject(pCallReq, "timestamp", timestamp);
		cJSON* pCallReqData = cJSON_CreateObject();		
		if (pCallReqData)
		{
			cJSON_AddStringToObject(pCallReqData, "cmd", "hangup");
			cJSON* pWebrtc = cJSON_CreateObject();
			if(pWebrtc)
			{
				cJSON_AddStringToObject(pWebrtc, "phone", pstCall->phone);
				cJSON_AddStringToObject(pWebrtc, "peerId", pstCall->perrId);
				cJSON_AddItemToObject(pCallReqData, "webrtc", pWebrtc);
			}
			cJSON_AddItemToObject(pCallReq, "data", pCallReqData);

			ACS_CHAR* p = cJSON_PrintUnformatted(pCallReq);//sprintf(PostData,"%s",p);
			if(p)
			{
				ACSHTTP_GET_URL(ACS_HTTPURL_CALLHANGUP, urlmsg, 0, pMsgId);
				ret = ACSHTTP_Post_Request_V3(urlmsg, Curl_DealCallRequest_Back, p, (ACS_CHAR*)pstResult,3);
				free(p);
				p = NULL;
			}
		}
		cJSON_Delete(pCallReq);
	}

	return ret;
}


/**
* fun:设备数据库数量叫推送mqtt消息
* @ACS_VOID-in:
* @return:0-success;
**/
ACS_INT ACS_DataNum_Push(ACS_VOID)
{

	if(ACSCOM_GetClouldEnable() == 0)
		return ACS_ERR_SERVERNOT;
	else if(ACSCOM_Get_ServiceState() != ACS_SERVER_OK)
		return ACS_ERR_SERVERFAIL;
	
	ACS_INT ret = ACS_ERR_FAIL;
	ACS_CHAR msgid[33] = {0};	
	const ACS_CHAR map[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	ACS_DATANUM_S stDataNum = {0};
	ACSCOM_GetDataNum_Config(&stDataNum);

	//if(stDataNum.person_count == -1)
	{
		//ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)CMD_MQTT_RES_SV_DATANUM);
	//	return ACS_ERR_SUCCESS;//ACS_ERR_SUCCESS;
	}
	
	//ACS_MQTT_CONFIG_S stMqttSvconfig = {0};
	//ACSCOM_Get_MQTTCfg(&stMqttSvconfig);

	srand((unsigned)time(NULL));
	for (int i = 0; i < 32; i++)
	{
		msgid[i] = map[rand()%36];
	}
	
	ret = Mqtt_AnswerServer(msgid, g_stDoor_Info.stMqttConfig.szClientId, CMD_MQTT_RES_SV_DATANUM,CMD_MQTT_RES_SV_SERVER,"",(ACS_CHAR *)&stDataNum);
	return ret;
}

/**
* fun:ACSHTTP_RestartLogin 重新启动门禁云平台 重新登入门禁云平台 同步平台数据用户门卡广告
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_VOID;
**/
ACS_VOID ACSHTTP_RestartLogin(const ACS_CHAR *pMsgId,const ACS_ULONG ltimestamp)
{
	//重新登录//ACSCOM_Set_ServiceState(ACS_SERVER_INIT);
	memset(g_szUrl,0,sizeof(g_szUrl));
	memset(&g_stDoor_Info.stMqttConfig,0,sizeof(ACS_MQTT_CONFIG_S));
	return;
}



/**
* fun:请求mqtt启动信息 
* @ACS_CLOUDLOGIN_S-in:输入门禁平台登入参数
* @ACS_URLCONFIG_S-in:输入门禁平台URL接口
* @pTimePathName-in:输入门禁云平台生成文件的路径 
* @return:0-success;
**/
ACS_VOID *RequestCloudInfo_Task(ACS_VOID *arg)
{
//	ACS_CHAR byfirst = ACS_ITURN;
	ACS_INT  ret = 0;					//ACS_CONFIG_S stDoorConfig = {0};
//	ACS_CHAR szUrl[128] = {0};			//ACS_MQTT_CONFIG_S stMqttConfig = {0};
	curl_global_init(CURL_GLOBAL_ALL);	/**防止多线程竞争导致段错误**/
	
	PRT_PRINT(ACS_DEBUG,"RequestCloudInfo_Task begin");

	while(1)
	{
		if(ACS_ITURN == g_stDoor_Info.stClouldInfo.nEnable)
		{
			//PRT_PRINT(ACS_DEBUG,"acslib szHttpUrl:%s szUrl:%s",g_stDoor_Info.stClouldInfo.szHttpUrl,szUrl);
			if(memcmp(g_szUrl, &g_stDoor_Info.stClouldInfo.szHttpUrl,sizeof(g_szUrl)) != ACS_FALSE)
			{
				if(strlen(g_stDoor_Info.stClouldInfo.szHttpUrl) > 4)
				{
					PRT_PRINT(ACS_DEBUG,"Restatic ACS_Device Login szHttpUrl:%s",g_stDoor_Info.stClouldInfo.szHttpUrl);
					ret = ACSHTTP_DeviceLogin(ACS_MSGIDINIT);//地址改变 请求错误 			
					if(ACS_ERR_SUCCESS != ret)
					{
						if((ACS_ERR_TIMEOUT == ret) || (ACS_ERR_CURLFIAL == ret))
						{
							ACSCOM_Set_ServiceState(ACS_SERVER_NETERR);
						}
						else
						{
							ACSCOM_Set_ServiceState(ACS_SERVER_LOGIN);
						}
						PRT_PRINT(ACS_DEBUG,"acslib Request for MQTT information error!!! ret:%d",ret);
					//	ACS_WLOG("ret=%d, Request for MQTT information error!!", ret);
						sleep(10);
						continue;
					}
				}
				
				if(strlen(g_stDoor_Info.stMqttConfig.szUsername) > 0)
				{

#if 0
					memset(g_stDoor_Info.stClouldInfo.szHttpUrl,0,sizeof(g_stDoor_Info.stClouldInfo.szHttpUrl));
					memcpy(g_stDoor_Info.stClouldInfo.szHttpUrl,"https://mock.apifox.cn/m1/1533213-0-default" ,sizeof("https://mock.apifox.cn/m1/1533213-0-default"));
#endif
					memcpy(g_szUrl, &g_stDoor_Info.stClouldInfo.szHttpUrl, sizeof(g_szUrl));
					Mqtt_DisConnect(ACS_FALSE);//断开上一次的mqtt链接
				}

				PRT_PRINT(ACS_DEBUG,"g_stDoor_Info.stMqttConfig.szClientId:%s;g_stDoor_Info.stClouldInfo.szUuid:%s;szcpuID:%s",\
				g_stDoor_Info.stMqttConfig.szClientId,g_stDoor_Info.stClouldInfo.szUuid,g_stDoor_Info.stClouldInfo.szcpuID);
				
			}
			
			if(ACS_SERVER_OK != ACSCOM_Get_ServiceState())//if((ACS_SERVER_INIT == ACSCOM_Get_ServiceState()) || (ACS_SERVER_FAIL == ACSCOM_Get_ServiceState()) || (ACS_SERVER_NETERR == ACSCOM_Get_ServiceState()) || (ACS_SERVER_UNBINDFAIL == ACSCOM_Get_ServiceState()))
			{
				ret = Mqtt_Connect();
				
				if((ACS_ERR_SUCCESS == ret) && (ACS_ITURN == g_bfirst))//首次登入就更新库
				{
					ACSCOM_EnDataQueue(ACS_CD_GETPARAM,ACS_MSGIDINIT,0);
					Mqtt_Set_DataDealFlag(ACS_CD_FACE,1);
					ACSCOM_EnDataQueue(ACS_CD_FACE,ACS_MSGIDINIT,0);
					Mqtt_Set_DataDealFlag(ACS_CD_CARD,1);
					ACSCOM_EnDataQueue(ACS_CD_CARD,ACS_MSGIDINIT,0);
					Mqtt_Set_DataDealFlag(ACS_CD_PWD,1);
					ACSCOM_EnDataQueue(ACS_CD_PWD,ACS_MSGIDINIT,0);
					Mqtt_Set_DataDealFlag(ACS_CD_AD,1);
					ACSCOM_EnDataQueue(ACS_CD_AD,ACS_MSGIDINIT,0);
					g_bfirst = 0;
				}
				else if(ACS_ERR_SUCCESS == ret)
				{
					ACSHTTP_Set_DataNumIfPush(ACS_ITURN);	
				}
			}

			if((ACSCOM_Get_ServiceState() == ACS_SERVER_OK)||(ACSCOM_Get_ServiceState() == ACS_SERVER_UNBIND))
			{
				if((ACSHTTP_Get_DataNumIfPush()) && (ACS_ERR_SUCCESS == ACS_DataNum_Push()))
				{			
					PRT_PRINT(ACS_INFO,"DataNum_Push success");
					ACSHTTP_Set_DataNumIfPush(ACS_FALSE);
				}
				
				Mqtt_Heartbeat(g_stDoor_Info.stMqttConfig.szClientId);
			}
			sleep(1);
		}
		else
		{
			//PRT_PRINT(ACS_DEBUG,"ACSCOM_Get_ServiceState():%d",ACSCOM_Get_ServiceState());
			Mqtt_DisConnect(ACS_FALSE);
			sleep(5);
		}
	}
	curl_global_cleanup();
	return NULL;
}


