#include "acscloudcommon.h"
#include "acscloudhttp.h"
#include "acscloudmqtt.h"
#include "acscloudqueue.h"
#include "acscloudshare.h"
#include "acscloud_intellect.h"

//pthread_mutex_t gMutexService = PTHREAD_MUTEX_INITIALIZER;

static ACS_BYTE g_bACSMQTT_IFSUCCESS = ACS_FALSE;
static ACS_LONG ulTimestamp = 0;
static ACS_UINT64 g_byDataDealFlag = 0;
static struct mosquitto *g_Mosqhandl = NULL;

extern ACS_CONFIG_S g_stDoor_Info;

/**
* fun:停止入库状态设置 
* @byType-in://1-face//2-card//3-ad
* @byOpe-in://1-add//0-sub
* @return:NULL
**/
ACS_VOID Mqtt_Set_DataDealFlag(unsigned ACS_CHAR byType,unsigned ACS_CHAR byOpe)
{
	if(byType)
	{
		if(byOpe)
		{
			if((Mqtt_Get_DataDealFlag() & byType) != byType)
				g_byDataDealFlag += byType;
		}
		else
		{
			if((Mqtt_Get_DataDealFlag() & byType) == byType)
				g_byDataDealFlag -= byType;
		}
	}
	else
	{
		g_byDataDealFlag = 0;
	}	
	return;
}

/**
* fun:停止入库状态获取 
* @ACS_UINT64-in:
* @return:NULL
**/
ACS_UINT64 Mqtt_Get_DataDealFlag(ACS_VOID)
{
	return g_byDataDealFlag;
}

ACS_VOID Mqttlib_message_callback(struct mosquitto *mosq, ACS_VOID *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen)
	{
		//printf("%s %s\n", message->topic, (ACS_CHAR * ) message->payload);
		Mqtt_Deal_MqttSevereMsg((ACS_CHAR*)message->payload,strlen((ACS_CHAR*)message->payload));
		//printf("Mqtt_Deal_MqttSevereMsg end\n");
	}
	else
	{
		PRT_PRINT(ACS_DEBUG,"acslib %s (null)", message->topic);
	}
	return;
}


ACS_VOID Mqttlib_subscribe_callback(struct mosquitto *mosq, ACS_VOID *userdata, ACS_INT mid, ACS_INT qos_count, const ACS_INT *granted_qos)
{
	PRT_PRINT(ACS_DEBUGX,"acslib subscribe_call mid:%d;qos_count:%d;granted_qos:%d",(ACS_CHAR *)userdata,mid,qos_count,*granted_qos);
	return;
}

ACS_VOID Mqttlib_log_callback(struct mosquitto *mosq, ACS_VOID *userdata, ACS_INT level, const ACS_CHAR *str)
{
	PRT_PRINT(ACS_DEBUGX,"acslib loging_call userdata:%s;level:%d;str:%s",(ACS_CHAR *)userdata,level,str);//ACS_WLOG("MQTT LogCallBack userdata=%s,str=%s\n",(ACS_CHAR *)userdata,str);
	return;
}

ACS_VOID Mqttlib_disconnect_callback(struct mosquitto *mosq, ACS_VOID *obj, ACS_INT rc)
{
	PRT_PRINT(ACS_INFO,"acslib Disconnect_callback !!! Server rc==%d",rc);//0-正常断开成功 1-内存不足, 7-断网,其它- 5-用户名和密码给错了 MOSQ_ERR_CONN_PENDING 
	ACS_WLOG("MQTT Disconnect_callback !!! Server rc==%d", rc);
	if(rc != 0)
		ACSCOM_Set_ServiceState(ACS_SERVER_FAIL); // ACS_SERVER_FAIL
	else
		ACSCOM_Set_ServiceState(ACS_SERVER_INIT);

	if(NULL == g_Mosqhandl)
	{
		PRT_PRINT(ACS_DEBUG,"mosquitto disconnect g_Mosqhandl:NULL");
	}
	rc = mosquitto_disconnect(g_Mosqhandl);
	if(NULL == g_Mosqhandl)
	{
		PRT_PRINT(ACS_ERROR,"mosquitto disconnect %d g_Mosqhandl:%p;NULL end",rc,g_Mosqhandl);
	}
	else
	{
		PRT_PRINT(ACS_INFO,"mosquitto disconnect success %d g_Mosqhandl:%p;end",rc,g_Mosqhandl);
	}
	g_bACSMQTT_IFSUCCESS = ACS_FALSE;
	return;
}

ACS_VOID Mqttlib_connect_callback(struct mosquitto *mosq, ACS_VOID *userdata, ACS_INT result)
{
	ACS_INT Subopicmid = 5;
	//ACS_CHAR Subopic[134]= {0};
	//ACS_MQTT_CONFIG_S stMqttSvconfig = {0};
	//ACSCOM_Get_MQTTCfg(&stMqttSvconfig);
	ACSCOM_Set_ServiceState(ACS_SERVER_OK);

	if(!result)
	{
		/** 订阅主题 Subscribe to broker information topics on successful connect. **/ //2  //0-请求的QoS:最多一次交付(Fire and Forget) (0) 2-请求的QoS:仅一次交付(有保证的交付)(2)
		ACS_INT ret = mosquitto_subscribe(mosq, &Subopicmid, g_stDoor_Info.stMqttConfig.szSubopicTopic, g_stDoor_Info.stMqttConfig.byQos);
		PRT_PRINT(ACS_INFO,"Subopic:%s;szClientId:%s;ret:%d;byQos:%d",\
			g_stDoor_Info.stMqttConfig.szSubopicTopic,g_stDoor_Info.stMqttConfig.szClientId,ret, g_stDoor_Info.stMqttConfig.byQos);
	}
	else
	{
		PRT_PRINT(ACS_ERROR,"acslib Connect failed");
	}
	return;
}


ACS_INT Mqtt_Pushmsg(const ACS_CHAR *pmsg)
{
	if(NULL == pmsg)
		return -1;
	ACS_INT ret = ACS_ERR_FAIL;
	ACS_INT len = 0;
	
	static ACS_LONG i = 0;
	static ACS_INT snLen = 0;
	
	len = strlen(pmsg);
	
	//PRT_PRINT(ACS_INFO,"acslib Push Msg to MQTT:%s;len:%d",pmsg,len);

	ret = mosquitto_publish(g_Mosqhandl,NULL,g_stDoor_Info.stMqttConfig.szPublishTopic,len,pmsg,g_stDoor_Info.stMqttConfig.byQos,0);
	
	if(len != snLen)
	{
		ACS_WLOG("PushMQTTMsg to Server ret:%d PutMsg:%s",ret,pmsg);
		snLen = len;
	}

	if(i%60 == 0)
	{
		ACS_WLOG("mosquitto publish pmsg:%s; ret:%d;MOSQ_ERR_SUCCESS:%d;Service Get State():%d",pmsg,ret,MOSQ_ERR_SUCCESS,ACSCOM_Get_ServiceState());

		PRT_PRINT(ACS_INFO,"mosquitto publish pmsg:%s; ret:%d;MOSQ_ERR_SUCCESS:%d;Service Get State():%d",pmsg,ret,MOSQ_ERR_SUCCESS,ACSCOM_Get_ServiceState());
		
		i = 1;
	}
	
	PRT_PRINT(ACS_DEBUG,"mosquitto publish %ld;[%ld] pmsg:%s; ret:%d;MOSQ_ERR_SUCCESS:%d;Service Get State():%d",i%60,i%600,pmsg,ret,MOSQ_ERR_SUCCESS,ACSCOM_Get_ServiceState());

	i++;
	if((ret == 3)&&(i>2))
	{
		ACSCOM_Set_ServiceState(ACS_SERVER_FAIL);
	}
	return ret;
}
#if 0
/**
* fun:回复给mqtt消息发送指令
* @dstmsgid-in:消息ID		 eg-csddfdf12sd14
* @srcid-in:发送者			 eg-{DEVSN}
* @cmdmsg-in:消息命令cmd eg-answer/heartbeat/logOut
* @dsttype-in:接收者		 eg-server
* @rescmd-in:消息子命令  	 eg-syncUsers
* @return:ACS_ERR_SUCCESS;
**/
ACS_INT  Mqtt_AnswerServer(const ACS_CHAR *dstmsgid,const      ACS_CHAR*srcid,const ACS_CHAR*cmdmsg,const ACS_CHAR *dsttype,const ACS_CHAR *rescmd,NULL)
{
	//ACS_CHAR ansmsg[256] = {0};
	//ACS_CONFIG_S stMjCfg = {0};
	//ACSCOM_Get_Config(&stMjCfg);
	//memset(ansmsg,0,256);
	cJSON* pUplodmsg = cJSON_CreateObject();
	if(pUplodmsg)
	{
		cJSON_AddStringToObject(pUplodmsg, "c",cmdmsg); //CMD
		cJSON_AddStringToObject(pUplodmsg, "r",rescmd); //消息子命令
		cJSON_AddStringToObject(pUplodmsg, "f",srcid);	//发送者
		cJSON_AddStringToObject(pUplodmsg, "t",dsttype);//接收者
		cJSON* pdstid = cJSON_CreateObject();
		if(pdstid)
		{
			cJSON_AddStringToObject(pdstid, "t",dstmsgid);//消息ID
			cJSON_AddItemToObject(pUplodmsg, "m",pdstid);
		}
		
		if((strcmp(cmdmsg, CMD_MQTT_RES_SV_HREATBEAT) == 0)||(strcmp(cmdmsg, CMD_MQTT_RES_SV_LOGOUT) == 0))
		{
			if(strlen(g_stDoor_Info.stClouldInfo.szPcode) > 0)
			{
				cJSON_AddStringToObject(pUplodmsg, "pcode", g_stDoor_Info.stClouldInfo.szPcode);
			}
			cJSON_AddNumberToObject(pUplodmsg, "ts",ulTimestamp);
		}
		
		ACS_CHAR* p = cJSON_PrintUnformatted(pUplodmsg);
		if(p)
		{
			cJSON_Minify(p);//memcpy(ansmsg,p,256);
			Mqtt_Pushmsg(p);
			free(p);
		}
		cJSON_Delete(pUplodmsg);
	}

	return	ACS_ERR_SUCCESS;
}
#else

/**
* fun:回复给mqtt消息发送指令
* @dstmsgid-in:消息ID		 eg-csddfdf12sd14
* @srcid-in:发送者			 eg-{DEVSN}
* @cmdmsg-in:消息命令cmd eg-answer/heartbeat/logOut
* @dsttype-in:接收者		 eg-server
* @rescmd-in:消息子命令  	 eg-syncUsers
* @return:ACS_ERR_SUCCESS;
**/
ACS_INT  Mqtt_AnswerServer(const ACS_CHAR *MsgID,const      ACS_CHAR*Send,const ACS_CHAR*Cmd,const ACS_CHAR *Receiver,const ACS_CHAR *SonsCmd,const ACS_CHAR *pData)
{
	//ACS_CHAR ansmsg[256] = {0};
	//ACS_CONFIG_S stMjCfg = {0};
	//ACSCOM_Get_Config(&stMjCfg);//
	//memset(ansmsg,0,256);
	ACS_INT ret = ACS_ERR_SUCCESS;
	cJSON* pPushMsg = cJSON_CreateObject();
	if(pPushMsg)
	{
		cJSON_AddStringToObject(pPushMsg, "c",Cmd); 		//CMD
		cJSON_AddStringToObject(pPushMsg, "f",Send);		//发送者
		cJSON_AddStringToObject(pPushMsg, "t",Receiver);	//接收者
		cJSON_AddStringToObject(pPushMsg, "r",SonsCmd); 	//消息子命令
		
		cJSON* pMsgid = cJSON_CreateObject();
		if(pMsgid)
		{
			cJSON_AddStringToObject(pMsgid, "t",MsgID);		//消息ID
			cJSON_AddItemToObject(pPushMsg, "m",pMsgid);
		}
		
		if((strcmp(Cmd, CMD_MQTT_RES_SV_HREATBEAT) == 0)||(strcmp(Cmd, CMD_MQTT_RES_SV_LOGOUT) == 0))
		{
			if(strlen(g_stDoor_Info.stClouldInfo.szPcode) > 0)
			{
				cJSON_AddStringToObject(pPushMsg, "pcode", g_stDoor_Info.stClouldInfo.szPcode);
			}
			cJSON_AddNumberToObject(pPushMsg, "ts",ulTimestamp);
		}
		else if(strcmp(Cmd, CMD_MQTT_RES_SV_DATANUM) == 0)
		{
			ACS_DATANUM_S *pstData = (ACS_DATANUM_S *)pData;
			cJSON* pDatajson = cJSON_CreateObject();
			if(pstData && pDatajson)
			{
				cJSON_AddNumberToObject(pDatajson, "user",pstData->person_count);
				cJSON_AddNumberToObject(pDatajson, "face",pstData->face_count);
				cJSON_AddNumberToObject(pDatajson, "card",pstData->card_cont);
				cJSON_AddNumberToObject(pDatajson, "password",pstData->pwd_cont);
				cJSON_AddNumberToObject(pDatajson, "ad",pstData->ad_cont);
				cJSON_AddItemToObject(pPushMsg, "data",pDatajson);
			}
		}
		
		ACS_CHAR* p = cJSON_PrintUnformatted(pPushMsg);
		if(p)
		{
			cJSON_Minify(p);
			ret = Mqtt_Pushmsg(p);
			free(p);
		}
		cJSON_Delete(pPushMsg);
	}

	return	ret;
}

#endif




ACS_VOID  Mqtt_Deal_Message_upload(const ACS_CHAR *pMsgCmd,const ACS_CHAR *pMsgid)
{
	//这里可能会超时阻塞
	ACS_MESSAGE_S stMsg = {0};
	sprintf(stMsg.szCmd,"%s","MQTT");
	snprintf(stMsg.szDetails,sizeof(stMsg.szDetails),"%s",pMsgCmd);
	snprintf(stMsg.szID,sizeof(stMsg.szID),"%s",pMsgid);
	stMsg.nCode = ACS_ERR_DATAERR;
	ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgid);
	return;
}

ACS_VOID Mqtt_ResetFactory(cJSON* pMqttMsg)
{
	ACS_WLOG("resetFactory %d ...",Mqtt_Get_DataDealFlag());
	PRT_PRINT(ACS_INFO,"resetFactory %d ...",Mqtt_Get_DataDealFlag());

	//停止所有入库操作
	if((Mqtt_Get_DataDealFlag() & ACS_CD_UPACK) != ACS_CD_UPACK)
	{
		Mqtt_Set_DataDealFlag(ACS_CD_NOT,0);
		PRT_PRINT(ACS_INFO,"resetFactory in...");
		ACSCOM_DeAllDataQueue();
		ACS_RESETFACTORY_S resetFactory = {0};

		cJSON* pnet = cJSON_GetObjectItem(pMqttMsg,"net");
		if((pnet)&&(pnet->type == cJSON_Number))
		{
			resetFactory.dwNet = pnet->valueint;
		}

		cJSON* paccount = cJSON_GetObjectItem(pMqttMsg,"account");
		if((paccount)&&(paccount->type == cJSON_Number))
		{
			resetFactory.dwAccount = paccount->valueint;
		}

		cJSON* pfacelist = cJSON_GetObjectItem(pMqttMsg,"facelist");
		if((pfacelist)&&(pfacelist->type == cJSON_Number))
		{
			resetFactory.dwFacelist = pfacelist->valueint;
		}

		cJSON* precord = cJSON_GetObjectItem(pMqttMsg,"record");
		if((precord)&&(precord->type == cJSON_Number))
		{
			resetFactory.dwRecord = precord->valueint;
		}
		ACSCOM_HandleCmdCallback(ACS_CMD_RESETFACTORY,(ACS_CHAR *)&resetFactory);
	}
	return;
}


ACS_VOID Mqtt_RemoteMaintenance(cJSON* pMqttMsg)
{
	cJSON* prtty = cJSON_GetObjectItem(pMqttMsg, "rtty");
	if((prtty) && (prtty->type == cJSON_Object))
	{
		ACS_DEVCFG_INFO_S stAcsDevinfo;
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
	return;
}


/**
* fun:接收MQTT的数据->处理POST/GET HTTP->回复给MQTT
* @pbuf-in:收到的MQTT服务器消息实体
* @nlen-in:收到的MQTT服务器消息长度
* @return:0-success;-1-fail
**/
ACS_INT  Mqtt_Deal_MqttSevereMsg(ACS_CHAR *pbuf,ACS_INT nlen)
{
	ACS_INT ret = -1;
	ACS_INT changf = 1;
	//static ACS_LONG nstate = 0;//筛选重复日志
	cJSON* pMsgid = NULL;
	ACS_CHAR cMsgID[2] = "";

	ACS_ULONG ltimestamp = 0;
	
	

	if((pbuf == NULL) || (nlen <= 0) )
		return ret;

	//PRT_PRINT("acslib Mqtt Severe Msg==%s\n",pbuf);

	cJSON* pJsonRecv = cJSON_Parse(pbuf);
	if (NULL == pJsonRecv)
	{
		ret = -1;
		if(pJsonRecv != NULL)
		{
			cJSON_Delete(pJsonRecv);
		}
		return ret;
	}

	cJSON* pMsgCmd = cJSON_GetObjectItem(pJsonRecv, "c");
	if(pMsgCmd == NULL)
	{
		PRT_PRINT(ACS_ERROR,"acslib 'Template_id' parameter Null.");
		ret = -1;
		if(pJsonRecv != NULL)
		{
			cJSON_Delete(pJsonRecv);
		}
		return ret;
	}
	
	cJSON* pMsgFrom = cJSON_GetObjectItem(pJsonRecv, "m");
	if(pMsgFrom != NULL)
	{
		pMsgid = cJSON_GetObjectItem(pMsgFrom, "t");//printf("acslib [%s %d] pMsgid->valuestring==%s\n", __func__, __LINE__, pMsgid->valuestring);
	}

	if((NULL == pMsgid) || (NULL == pMsgid->valuestring))
	{
		pMsgid = pMsgCmd;
		pMsgid->valuestring = cMsgID;
		PRT_PRINT(ACS_DEBUG,"pMsgid->valuestring:%s",pMsgid->valuestring);
	}
#if 0
	if((strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_ANSWER) != 0))
	{
		nstate = 0;
		ACS_WLOG("Received Mqtt Sever Msg:%s",pMsgCmd->valuestring);
	}
	else if(nstate < 2)
	{
		nstate ++;
		ACS_WLOG("Received Mqtt Sever Msg:%s",pMsgCmd->valuestring);
	}
#endif	

	cJSON* ptimestamp = cJSON_GetObjectItem(pJsonRecv,"ts");
	if((ptimestamp)&&(ptimestamp->type == cJSON_Number))
	{
		//ACS_CHAR szTime[16] = {0};
		//sprintf(szTime,"%0.0f",ptimestamp->valuedouble);
		//ltimestamp = (ACS_ULONG)atol(szTime);
		ltimestamp = (ACS_ULONG)ptimestamp->valueint;
	}
	
	
	//if(strcmp(pMsgCmd->valuestring, "answer") != 0)
	PRT_PRINT(ACS_DEBUG,"Received Mqtt Sever Msg:%s",pMsgCmd->valuestring);

	
	if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCUSERS) == 0)//同步用户
	{
		Mqtt_Set_DataDealFlag(ACS_CD_FACE,1);
		ACSCOM_EnDataQueue(ACS_CD_FACE,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCCARD) == 0)//同步卡号
	{
		Mqtt_Set_DataDealFlag(ACS_CD_CARD,1);
		ACSCOM_EnDataQueue(ACS_CD_CARD,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCPWD) == 0)//同步密码 pagesize
	{
		Mqtt_Set_DataDealFlag(ACS_CD_PWD,1);
		ACSCOM_EnDataQueue(ACS_CD_PWD,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SETAD) == 0)//同步广告
	{
		Mqtt_Set_DataDealFlag(ACS_CD_AD,1);
		ACSCOM_EnDataQueue(ACS_CD_AD,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCOPENDOOR) == 0)//平台开门
	{
		cJSON* puserId = cJSON_GetObjectItem(pJsonRecv,"userId");
		if((puserId)&&(puserId->valuestring))
		{
			ACSCOM_HandleCmdCallback(ACS_CMD_SET_OPENDOOR,puserId->valuestring);
		}
		else
		{
			PRT_PRINT(ACS_INFO,"MQTTServerCMD openDoor...");
			ACS_WLOG("openDoor userid NULL");
			Mqtt_Deal_Message_upload("userId",pMsgid->valuestring);
		}
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCCLEANDA) == 0)//平台解绑设备
	{
		PRT_PRINT(ACS_INFO,"acslib MQTTServerCMD cleanData...");//解绑房屋 不能断mqtt 继续发心跳 cleanData 删除人员 门卡 广告
		//ACSCOM_Set_ServiceState(ACS_SERVER_UNBIND); 
		ACSCOM_CleanData(NULL);
		PRT_PRINT(ACS_DEBUG,"acslib MQTTServerCMD cleanData end");
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCRESTART) == 0)//重新绑定房屋
	{
		PRT_PRINT(ACS_INFO,"acslib MQTTServerCMD restartApp...");
		ACSHTTP_RestartLogin(pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_RMDEV) == 0) //平台删除设备
	{
		PRT_PRINT(ACS_INFO,"acslib MQTTServerCMD rmDev...");
		memset(&g_stDoor_Info.stMqttConfig,0,sizeof(ACS_MQTT_CONFIG_S));
		ACSCOM_CleanData(NULL);
		//ACSCOM_Set_ServiceState(ACS_SERVER_INIT);
		Mqtt_DisConnect(ACS_ITURN);
	}
	else if((strcmp(pMsgCmd->valuestring, "checkData") == 0)||(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_FINDDATA) == 0))//检查设备数据
	{
		cJSON* poperation = cJSON_GetObjectItem(pJsonRecv,"operation");
		if((poperation)&&(poperation->valuestring))
		{
			if((strcmp(poperation->valuestring,ACS_FACES)==0)||(strcmp(poperation->valuestring,ACS_USERS)==0)\
				||(strcmp(poperation->valuestring,ACS_CARDS)==0)||(strcmp(poperation->valuestring,ACS_PWDS)==0)\
				||(strcmp(poperation->valuestring,ACS_ADS)==0))
			{
				ACS_CHECKDATA_INFO_S stCheackData = {0};//memset(&stCheackData,0,sizeof(ACS_CHECKDATA_INFO_S));
				cJSON* puserId = cJSON_GetObjectItem(pJsonRecv,"id");
				if((puserId)&&(puserId->valuestring))
				{
					snprintf(stCheackData.userId,32,puserId->valuestring);
				}
				sprintf(stCheackData.operation,"%s",poperation->valuestring);
				sprintf(stCheackData.ID,"%s",pMsgid->valuestring);
				stCheackData.stList[0].pstData = NULL;
				stCheackData.stList[1].pstData = NULL;
				stCheackData.stList[2].pstData = NULL;

				ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_CHECKDATA,(ACS_CHAR *)&stCheackData);
				ACSHTTP_PostCheckDataResult(pMsgCmd->valuestring,&stCheackData,pMsgid->valuestring);
				
				PRT_PRINT(ACS_INFO,"acslib CheckData Result Upload Back");
			}
			else
			{
				PRT_PRINT(ACS_INFO,"acslib operation null!");
				Mqtt_Deal_Message_upload("operation",pMsgid->valuestring);
			}
		}
		else
		{
			PRT_PRINT(ACS_INFO,"acslib operation null!");
			Mqtt_Deal_Message_upload("operation",pMsgid->valuestring);
		}
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCSTOP) == 0)//停止同步 {face card ...}
	{
		cJSON* poperation = cJSON_GetObjectItem(pJsonRecv,"operation");
		if((NULL != poperation)&&(poperation->valuestring != NULL))
		{
			if(strcmp(poperation->valuestring,ACS_FACES)==0)
			{
				Mqtt_Set_DataDealFlag(ACS_CD_FACE,0);//face
			}
			else if(strcmp(poperation->valuestring,ACS_CARDS)==0)
			{
				Mqtt_Set_DataDealFlag(ACS_CD_CARD,0);//card
			}
			else if(strcmp(poperation->valuestring,ACS_PWDS)==0)
			{
				Mqtt_Set_DataDealFlag(ACS_CD_PWD,0);//pwd
			}
			else if(strcmp(poperation->valuestring,ACS_ADS)==0)
			{
				Mqtt_Set_DataDealFlag(ACS_CD_AD,0);//ad
			}
			else if(strcmp(poperation->valuestring,ACS_REISSUES)==0)
			{
				Mqtt_Set_DataDealFlag(ACS_CD_REISSUES,0);//reissue
			}
			else if(strcmp(poperation->valuestring,ACS_ELERULES)==0)
			{
				Mqtt_Set_DataDealFlag(ACS_CD_ELERULES,0);//elerule
			}
			ACSCOM_HandleCmdCallback(ACS_CMD_DEV_SYNCSTOP,poperation->valuestring);
			
			PRT_PRINT(ACS_INFO,"stopsys Result Upload Back");
		}
		else
		{
			PRT_PRINT(ACS_INFO,"stopsys operation null!");
			Mqtt_Deal_Message_upload("operation",pMsgid->valuestring);
		}
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_UPGRADEFW) == 0)//升级固件
	{
		PRT_PRINT(ACS_INFO,"MQTTServerCMD UpgadeFW...");

		//停止所有入库操作
		if((Mqtt_Get_DataDealFlag() & ACS_CD_UPACK) != ACS_CD_UPACK)
		{
			Mqtt_Set_DataDealFlag(ACS_CD_NOT,0);
			Mqtt_Set_DataDealFlag(ACS_CD_UPACK,1);
			PRT_PRINT(ACS_INFO,"MQTTServerCMD UpgadeFW end...");
		}

		Mqtt_Set_DataDealFlag(ACS_CD_NOT,0);
		ACSCOM_DeAllDataQueue();
		ACSCOM_EnDataQueue(ACS_CD_UPACK,pMsgid->valuestring,ltimestamp);	

		//Mqtt_Set_DataDealFlag(ACS_CD_NOT,0);
		
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_RESETFACTORY) == 0)//恢复出厂
	{
		Mqtt_ResetFactory(pJsonRecv);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_REBOOTDEV) == 0)//设备重启
	{
		ACS_INT ncmd = 0;
		PRT_PRINT(ACS_INFO,"CMD MQTTSERVER_DEV_reboot......");
		cJSON* poperation = cJSON_GetObjectItem(pJsonRecv,"operation");
		if((poperation)&&(poperation->type == cJSON_Number))
		{
			ncmd = poperation->valueint;
		}
		ACSCOM_HandleCmdCallback(ACS_CMD_DEV_REBOOT,(ACS_CHAR *)&ncmd);
		PRT_PRINT(ACS_INFO,"CMD MQTTSERVER_DEV_reboot end......ncmd:%d",ncmd);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_ANSWER) == 0)//服务器的心跳回应
	{
		changf = 0;
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_CALLED) == 0)//设备被呼叫
	{
		
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_CALLEND) == 0)//通话结束
	{

	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_REISSUERECORD) == 0)//考勤补发
	{
		Mqtt_Set_DataDealFlag(ACS_CD_REISSUES,1);
		ACSCOM_EnDataQueue(ACS_CD_REISSUES,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SNAPPHOTO) == 0)//一键抓拍
	{
		ACSCOM_HandleCmdCallback(ACS_CMD_SNAPPHOTO,pMsgid->valuestring);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYCPARAM) == 0)//同步参数
	{
		ACSCOM_EnDataQueue(ACS_CD_PARAM,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_GETDEVPARAM) == 0)//上传设备参数到平台
	{
		ACSCOM_EnDataQueue(ACS_CD_GETPARAM,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_GETDCITTALKLIST) == 0)//获取分机列表
	{
		ACSCOM_EnDataQueue(ACS_CD_GETDCITTALKLIST,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_SYNCTIME) == 0)//同步时间戳
	{
		ACS_UINT64 lltimestamp = (ACS_UINT64)ltimestamp;
		ACSHTTP_SyncPlatformTime(lltimestamp);
	}
	else if (strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_CALLSYNC) == 0)
	{
		ACS_WLOG("Recv CMD_MQTTSERVER_CALLSYNC");
		cJSON* pcallresult = cJSON_GetObjectItem(pJsonRecv, "r");
		if (pcallresult)
		{
			cJSON* pid = cJSON_GetObjectItem(pcallresult, "id");
			if (pid && pid->valuestring) 
			{
				ACS_CALL_CMD_MQTT_S stCallCmdMqtt = {0};//memset(&stCallCmdMqtt, 0, sizeof(stCallCmdMqtt));
				snprintf(stCallCmdMqtt.phone, 16, "%s", pid->valuestring);
				cJSON* pcmd = cJSON_GetObjectItem(pcallresult, "cmd");
				if(pcmd && pcmd->valuestring)
				{
					snprintf(stCallCmdMqtt.cmd, 16, "%s", pcmd->valuestring);
				}				
				cJSON* psessionId = cJSON_GetObjectItem(pcallresult, "sessionId");
				if (psessionId && psessionId->valuestring) 
				{
					snprintf(stCallCmdMqtt.sessionId, 128, "%s", psessionId->valuestring);
				}				
				ACSCOM_HandleCmdCallback(ACS_CMD_CALLCMD_UPPARAM, (ACS_CHAR*)&stCallCmdMqtt);
			}
		}
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_ELERULES) == 0)//派梯规则
	{
		Mqtt_Set_DataDealFlag(ACS_CD_ELERULES,1);
		ACSCOM_EnDataQueue(ACS_CD_ELERULES,pMsgid->valuestring,ltimestamp);
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTT_RES_SV_DATANUM) == 0)//上传数量
	{
		PRT_PRINT(ACS_INFO,"acslib msgCmd Cmd[%s];%d",CMD_MQTT_RES_SV_DATANUM,ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_NOT));
		if(ACS_FALSE != ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_NOT))
		{
			ACSCOM_Set_ifDataNum(ACS_ITURN);
			ACSCOM_ClearDataNum_Config();
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)CMD_MQTT_RES_SV_DATANUM);
			sleep(1);
			if(ACS_FALSE != ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_NOT))
			{
				ACSCOM_Set_ifDataNum(ACS_FALSE);
			}
		}
		else
		{
			ACSCOM_ClearDataNum_Config();
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)CMD_MQTT_RES_SV_DATANUM);
		}
	}
	else if(strcmp(pMsgCmd->valuestring, CMD_MQTTSERVER_TELEMAINTENANCE) == 0)//远程维护
	{
		PRT_PRINT(ACS_INFO,"acslib msgCmd Cmd[%s];",pMsgCmd->valuestring);
		Mqtt_RemoteMaintenance(pJsonRecv);
	}
	else
	{
		changf = 0;
		PRT_PRINT(ACS_INFO,"acslib msgCmd nothing Cmd[%s]",pMsgCmd->valuestring);
		Mqtt_Deal_Message_upload(pMsgCmd->valuestring,pMsgid->valuestring);
	}

	if(changf)
	{
		//ACS_MQTT_CONFIG_S stMqttSvconfig = {0};
		//ACSCOM_Get_MQTTCfg(&stMqttSvconfig);
		Mqtt_AnswerServer(pMsgid->valuestring, g_stDoor_Info.stMqttConfig.szClientId, CMD_MQTT_RES_SV_ANSWER,CMD_MQTT_RES_SV_SERVER,pMsgCmd->valuestring,NULL);
	}

	if(pJsonRecv != NULL)
	{
		cJSON_Delete(pJsonRecv);
	}

	return  ret;

}

/**
* fun:断开MQTT
* @byIfClearTime-in:是否清空同步数据的时间/标志 ACS_ITURN-清空 ACS_FALSE-不清空
* @return:0-success;
**/
ACS_INT Mqtt_DisConnect(const ACS_BYTE byIfClearTime)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	if(g_bACSMQTT_IFSUCCESS)
	{
		ACS_INT i = 0;
		ACS_CHAR msgid[33] = {0};
		const ACS_CHAR map[] = "0123456789abcdefghijklmnopqrstuvwxyz";
		//ACS_MQTT_CONFIG_S stMqttConfig;
		//ACSCOM_Get_MQTTCfg(&stMqttConfig);

		srand((unsigned)time(NULL));
		for (i = 0; i < 32; i++)
		{
			msgid[i] = map[rand()%36];
		}
		ACSCOM_Set_ServiceState(ACS_SERVER_INIT);
		Mqtt_AnswerServer(msgid, g_stDoor_Info.stMqttConfig.szClientId, CMD_MQTT_RES_SV_LOGOUT,CMD_MQTT_RES_SV_SERVER,CMD_MQTT_RES_SV_LOGOUT,NULL);
		sleep(2);
		if(NULL == g_Mosqhandl)
		{
			PRT_PRINT(ACS_DEBUG,"mosquitto disconnect g_Mosqhandl:NULL");
		}
		ret = mosquitto_disconnect(g_Mosqhandl);
		if(NULL == g_Mosqhandl)
		{
			PRT_PRINT(ACS_DEBUG,"mosquitto disconnect g_Mosqhandl:NULL end");
		}
		PRT_PRINT(ACS_DEBUG,"mosquitto disconnect ret:%d",ret);

		if(byIfClearTime)
		{
			//每次登录将请求时间戳置零
			ACS_SYNC_MARK_S     stSysctime ;
			ACSCOM_Get_SyncTimeCfg(&stSysctime);
			ACS_BYTE byLog = stSysctime.logLevel;
			memset(&stSysctime,0,sizeof(ACS_SYNC_MARK_S));
			stSysctime.logLevel = byLog;
			ACSCOM_Set_SyncTimeCfg(&stSysctime);
		}
	}
	return ret;
}

/**
* fun:连接mqtt
* @ACS_VOID-in:ACS_VOID
* @return:0-success;
**/
ACS_INT Mqtt_Connect(ACS_VOID)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	if(NULL == g_Mosqhandl)
	{
		PRT_PRINT(ACS_DEBUG,"mosquitto Connect g_Mosqhandl:NULL");
		return -1;
	}
	//连接mqtt
	mosquitto_username_pw_set(g_Mosqhandl, (const ACS_CHAR*)g_stDoor_Info.stMqttConfig.szUsername, (const ACS_CHAR*)g_stDoor_Info.stMqttConfig.szMqttPwd);
	ret = mosquitto_connect(g_Mosqhandl, (const ACS_CHAR*)g_stDoor_Info.stMqttConfig.szIP, g_stDoor_Info.stMqttConfig.nPort,g_stDoor_Info.stMqttConfig.keepalive);//
	if(ret)
	{
		ACSCOM_Set_ServiceState(ACS_SERVER_FAIL);
		PRT_PRINT(ACS_INFO,"acslib Unable to connect MQTT(IP:%s, port:%d);ret:%d.", g_stDoor_Info.stMqttConfig.szIP, g_stDoor_Info.stMqttConfig.nPort,ret);
		ACS_WLOG("Unable to connect MQTT(IP:%s, port:%d).", g_stDoor_Info.stMqttConfig.szIP, g_stDoor_Info.stMqttConfig.nPort);
		//return NULL;//return -4;
		//usleep(200*1000);
		sleep(3);
	}
	else
	{
		ACSCOM_Set_ServiceState(ACS_SERVER_OK);
		g_bACSMQTT_IFSUCCESS = ACS_ITURN;
		PRT_PRINT(ACS_INFO,"acslib connect MQTT Success.");
		ACS_WLOG("connect MQTT Success.");
		mosquitto_will_set(g_Mosqhandl, "will-topic", strlen("will message"), "will message",g_stDoor_Info.stMqttConfig.byQos, false);//2 -原来是2 爱多纷改为0
		PRT_PRINT(ACS_INFO,"acslib Login Success !!!");
		ACS_WLOG("Login Success !!!");
	}
	return ret;
}

/**
* fun:60s一次发送心跳 
* @pszClientId-in:输入平台的ClientId
* @return:ACS_VOID;
**/
ACS_VOID Mqtt_Heartbeat(const ACS_CHAR *pszClientId)
{
	//ACS_MQTT_CONFIG_S stMqttConfig={0};
	//ACSCOM_Get_MQTTCfg(&stMqttConfig);
	static unsigned ACS_CHAR byTime = 60;
	const ACS_CHAR map[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	ACS_CHAR msgid[33] = {0};
	//PRT_PRINT("byTime:%d, keepalive:%d",byTime, stMqttConfig.keepalive);
	if(byTime > g_stDoor_Info.stMqttConfig.keepalive)
	{		
		ulTimestamp = time(NULL);
		srand((unsigned)ulTimestamp);
		for (ACS_INT i = 0; i < 32; i++)
		{
			msgid[i] = map[rand()%36];
		}
		Mqtt_AnswerServer(msgid, g_stDoor_Info.stMqttConfig.szClientId,CMD_MQTT_RES_SV_HREATBEAT,CMD_MQTT_RES_SV_SERVER,CMD_MQTT_RES_SV_HREATBEAT,NULL);
		byTime = 0;
	}
	byTime ++;
}

/**
* fun:初始化MQTT服务 阻塞循环
* @arg-in:void
* @return:ACS_VOID;
**/
ACS_VOID *Mqtt_MsgServe_Task(ACS_VOID *arg)
{
	ACS_INT ret = 0;
	PRT_PRINT(ACS_DEBUG,"MqttServe_Task begin;");
	while(1)
	{
		//if(strlen(g_stDoor_Info.stClouldInfo.szUuid) > 0)//if((strlen(g_stDoor_Info.stMqttConfig.szClientId) > 0) && (strlen(g_stDoor_Info.stClouldInfo.szUuid) > 0))
		if(strlen(g_stDoor_Info.stMqttConfig.szMqttPwd) > 0)
		{
			ret = mosquitto_lib_init();
			if(ret < 0 )
			{
				PRT_PRINT(ACS_ERROR,"acslib mosquitto_lib_init error");
				ACS_WLOG("mosquitto_lib_init error(%d)", ret);
				return NULL;//return -2;
			}
			PRT_PRINT(ACS_INFO,"mosquittoLib init end;byIfClientId:%d;szClientId:%s",g_stDoor_Info.stMqttConfig.byIfClientId,g_stDoor_Info.stMqttConfig.szClientId);
			
			if((g_stDoor_Info.stMqttConfig.byIfClientId)&&(strlen(g_stDoor_Info.stMqttConfig.szClientId) > 0))
			{
				g_Mosqhandl = mosquitto_new(g_stDoor_Info.stMqttConfig.szClientId, ACS_ITURN, g_stDoor_Info.stMqttConfig.szClientId);//断开后是否保留订阅信息 true 不保留/ false 保留
			}
			else if(strlen(g_stDoor_Info.stClouldInfo.szUuid) > 0)
			{
				g_Mosqhandl = mosquitto_new(g_stDoor_Info.stClouldInfo.szUuid, ACS_ITURN, g_stDoor_Info.stMqttConfig.szClientId);
			}
			
			if(!g_Mosqhandl)
			{
				PRT_PRINT(ACS_INFO,"acslib Error: Out of memory.");
				ACS_WLOG("mosquitto_new error!");
				return NULL;//return -3;
			}
			break;
		}
		usleep(200*1000);
	}
	
	mosquitto_log_callback_set(g_Mosqhandl, Mqttlib_log_callback);
	mosquitto_connect_callback_set(g_Mosqhandl, Mqttlib_connect_callback);//订阅主题
    mosquitto_disconnect_callback_set(g_Mosqhandl,Mqttlib_disconnect_callback);//mqtt断开连接
	mosquitto_message_callback_set(g_Mosqhandl, Mqttlib_message_callback);//消息接收
	mosquitto_subscribe_callback_set(g_Mosqhandl,Mqttlib_subscribe_callback);
	
	
	while(1)
	{
		PRT_PRINT(ACS_INFO,"mosquitto ServiceState");
		while(ACSCOM_Get_ServiceState() != ACS_SERVER_OK)//等待mqtt连接/重新连接成功
		{
			sleep(1);
		}

		//创建入库线程
		
		PRT_PRINT(ACS_INFO,"mosquitto loop_forever begin");
		ret = mosquitto_loop_forever(g_Mosqhandl, -1, 1);
		PRT_PRINT(ACS_INFO,"mosquitto loop forever ret:%d",ret);

		//销毁入库线程
		
		sleep(5);
	}

	if(!g_Mosqhandl)
		mosquitto_destroy(g_Mosqhandl);
	mosquitto_lib_cleanup();
	
	return NULL;
}

#if 0
ACS_VOID Mqtt_PushHeartbeat(ACS_VOID)
{
	ACS_INT i = 0;
	ACS_CHAR map[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	ACS_CHAR msgid[33] = {0};
	ACS_MQTT_CONFIG_S stMqttConfig;
	ACSCOM_Get_MQTTCfg(&stMqttConfig);

	if(ACSCOM_Get_ServiceState() == ACS_SERVER_OK)
	{
		srand((unsigned)time(NULL));
	    
		for (i = 0; i < 32; i++)
		{
			msgid[i] = map[rand()%36];
		}
		//printf("msgid = %s\n", msgid);
		
		Mqtt_AnswerServer(msgid, stMqttConfig.szClientId, CMD_MQTT_RES_SV_HREATBEAT,CMD_MQTT_RES_SV_SERVER,CMD_MQTT_RES_SV_HREATBEAT);
		sleep(60);
	}
	return;
}
#endif

