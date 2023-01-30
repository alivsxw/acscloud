#include "acscloudcommon.h"

#include "acscloudmqtt.h"
#include "acscloudhttp.h"
#include "acscloudqueue.h"
#include "acscloudshare.h"
#include "acscloud_intellect.h"

#define ACS_UPGREDFILE		"upgrade_file.bin"

/**
* fun:获取升级固件信息 HTTP
* @pstUpgadeFW-out:获取的固件信息
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:NULL
**/
ACS_VOID upgradePack(const ACS_CHAR *pMsgId) 
{
	ACS_INT ret = 0;//ACS_CHAR szmd5sum[64] = "ef17b8de520090800d1137b3b3f9ce03";//{0};
	ACS_MESSAGE_S stMsg = {0};
	ACS_CHAR szUpackFile[256] = {0};
	ACS_UPGADEFW_INFO_S stUpgadeFW = {0};
	ret = ACSHTTP_GetUpgradeFW(&stUpgadeFW,pMsgId);
	snprintf(szUpackFile,sizeof(szUpackFile),"%s/%s",ACSCOM_GetUploadPath(),ACS_UPGREDFILE);


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_UPACK,ACS_ITURN);
	PRT_PRINT(ACS_DEBUG,"upgradePack SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_UPACK));

	if(access(szUpackFile, F_OK) == ACS_SUCCESS)
	{
		PRT_PRINT(ACS_DEBUG,"%s access,rm it!", szUpackFile);
		char cmd[128]={0};
		sprintf(cmd, "rm %s", szUpackFile);
		system(cmd);
	}
	
	ret = ACSHTTP_UpgradeFileDownload(stUpgadeFW.szUrl,szUpackFile,300,g_Acs_RomoteUpgrade);//ret = ACS_Http_DiskDownFile(stUpgadeFW.szUrl,szUpackFile,60);
	PRT_PRINT(ACS_DEBUG,"stUpgadeFW.nSize:%d, ret:%d",stUpgadeFW.nSize, ret);
	if((ret >= ACS_SUCCESS))//&& (memcmp(stUpgadeFW.szMd5, szmd5sum, strlen(stUpgadeFW.szMd5)) == 0))
	{
		if(ret != stUpgadeFW.nSize)
		{
			ret = ACS_ERR_CHECKFAIL;
			sprintf(stMsg.szDetails,"Size:%d",stUpgadeFW.nSize);
		}
		else if(ret == stUpgadeFW.nSize)
		{
			ret = ACSCOM_HandleCmdCallback(ACS_CMD_SET_UPGRADEFW,(ACS_CHAR *)&stUpgadeFW);
			if(ret == -1)
			{
				ret = ACS_ERR_CHECKFAIL;
				sprintf(stMsg.szDetails,"Version");
			}
		}
	}
	else
	{
		//升级失败
		PRT_PRINT(ACS_DEBUG,"UpgradePack fail ret:%d",ret);
		ACS_WLOG("UpgradePack fail ret:%d",ret);
		if(-1 == ret)
		{
			ret = ACS_ERR_FILE_DOWNERR;
		}
		else if(-2 == ret)
		{
			ret = ACS_ERR_FILE_OPENERR;
		}
		else if(-3 == ret)
		{
			ret = ACS_ERR_CURLFIAL;
		}
		else if(-4 == ret)
		{
			ret = ACS_ERR_CHECKFAIL;
			sprintf(stMsg.szDetails,"Version");
		}
	}

	if(ACS_ERR_SUCCESS != ret)
	{
		ACSCOM_HandleCmdCallback(ACS_CMD_UPGRADEFW_FAIL,(ACS_CHAR *)&stUpgadeFW);
		sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_UPGRADEFW);//sprintf(stMsg.szID,"%s",stUpgadeFW->ID);
		stMsg.nCode = ret;		
		ACSHTTP_Message_upload(&stMsg,0,NULL,pMsgId);
	}

	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_UPACK,ACS_FALSE);
	PRT_PRINT(ACS_DEBUG,"upgradePack SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_UPACK));
	
	return;
}



ACS_VOID syncFaceResultUpload(ACS_FACE_INFO_S *pFaceData,ACS_INT nCode,const ACS_CHAR *pMsgId)
{
	if(pFaceData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib syncFaceUpload pFaceData point NULL\n");
		return;
	}
	
	if(ACS_LOGVER_TWO != ACSHTTP_Get_Logver())
	{
		ACS_SYNCFACE_RESULT_S stFaceResult = {0};
		memset(&stFaceResult,0,sizeof(ACS_SYNCFACE_RESULT_S));
		stFaceResult.code = nCode;
		snprintf(stFaceResult.ID,sizeof(stFaceResult.ID),"%s",pFaceData->ID);
		sprintf(stFaceResult.userId,"%s", pFaceData->userId);
		sprintf(stFaceResult.url,"%s", pFaceData->url);
		sprintf(stFaceResult.operation, "%s", pFaceData->operation);
		ACSHTTP_SyncFaceResult_Upload(&stFaceResult,pMsgId);
	}
	
	if((ACS_LOGVER_TWO == ACSHTTP_Get_Logver()) || (ACS_LOGVER_ZERO == ACSHTTP_Get_Logver()))
	{
		ACS_MESSAGE_S stMsg = {0};
		//if(ACS_ERR_SUCCESS != nCode)
		{
			sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCUSERS);
			//sprintf(stMsg.szOperation,"%s",pFaceData->operation);
			sprintf(stMsg.szID,"%s",pFaceData->ID);
			stMsg.nCode = nCode;
			ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_SYCUSERS,(ACS_CHAR *)pFaceData,pMsgId);
		}
	}
	return; 
}

ACS_VOID syncFace(const ACS_CHAR *pMsgId)
{
	ACS_INT nPage = 1;
	ACS_INT k = 0;
	ACS_INT ret = ACS_ERR_FAIL;
	ACS_INT getRet = ACS_ERR_FAIL;
	ACS_INT nFirstPageSum = 0;//ACS_INT nPageNum = ACSSYNCNUM;//请求的记录条数
	ACS_ELEDISRULES_S *peleRuleDatas = NULL;
	ACS_RECORD_S stFaceInfo = {0};
	stFaceInfo.pRecord = ACSCOM_MallocFaceAddress();

	PRT_PRINT(ACS_DEBUG,"acs e_cmd:face;pMsgId:%s",pMsgId);
	
	if(NULL == stFaceInfo.pRecord)
	{
		PRT_PRINT(ACS_DEBUG,"ACSCOM_MallocFaceAddress:%p NULL;",stFaceInfo.pRecord);
		return;
	}
	
	ACS_FACE_INFO_S *pSyncFaceData = (ACS_FACE_INFO_S *)stFaceInfo.pRecord;
	if(NULL == pSyncFaceData)
	{
		PRT_PRINT(ACS_DEBUG,"pSyncFaceData:%p NULL;",pSyncFaceData);
		return;
	}
	

	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_FACE,ACS_ITURN);
	PRT_PRINT(ACS_DEBUG,"face SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_FACE));

	
	
	ACSCOM_Set_ifDataNum(ACS_FALSE);//不推

	while(1)//face
	{	

		if((Mqtt_Get_DataDealFlag() & ACS_CD_FACE) == ACS_CD_FACE)
		{
			//获取数据 入队列 获取的数据条数=队列数 
			getRet = ACSHTTP_Get_SyncFaces(nPage,ACSSYNCNUM,&stFaceInfo,pMsgId);//PRT_PRINT("face nPage:%d",nPage);
		}
		else
		{
			stFaceInfo.nPageSize = 0;
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(nPage == 1)
			{
				nFirstPageSum = stFaceInfo.nPageSize;					
			}
		}

		for(k=0;k<stFaceInfo.nPageSize;k++)
		{
			if(ACS_ERR_SUCCESS == getRet)
			{
				if((Mqtt_Get_DataDealFlag() & ACS_CD_FACE) == ACS_CD_FACE)
				{
					ret = ACSCOM_SyncFacePic(&pSyncFaceData[k],pMsgId);
					if(ACS_ERR_SUCCESS == ret)
					{
						ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYNCUSER,(ACS_CHAR *)&pSyncFaceData[k]);
						syncFaceResultUpload(&pSyncFaceData[k],ret,pMsgId);
					}
					
					if(pSyncFaceData[k].pPicData != NULL)
					{
						free(pSyncFaceData[k].pPicData);
						pSyncFaceData[k].pPicData = NULL;
					}
				}
				else
				{
					getRet = ACS_ERR_FAIL;
				}
			}
			
			peleRuleDatas = (ACS_ELEDISRULES_S *)pSyncFaceData[k].peDisRulesData;
			PRT_PRINT(ACS_DEBUG,"syncface free neRules:%d;psteRules:%p;%p;nPageSize:%d;k:%d",pSyncFaceData[k].neDisRules,peleRuleDatas,pSyncFaceData[k].peDisRulesData,stFaceInfo.nPageSize,k);
			if((pSyncFaceData[k].neDisRules) && (pSyncFaceData[k].peDisRulesData))
			{
				free(pSyncFaceData[k].peDisRulesData);
				pSyncFaceData[k].peDisRulesData = NULL;
			}
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(stFaceInfo.nPageSize >= ACS_SUCCESS)	
				ACSHTTP_Set_SyncMark(ACS_CD_FACE);
		}
		
		//平台无数据 数据取完 停止入库
		if((nFirstPageSum < 1)||(stFaceInfo.nPageSize < nFirstPageSum)||(stFaceInfo.nTotal <= ACSSYNCNUM))
		{
			PRT_PRINT(ACS_DEBUG,"acs face nFirstPageSize:%d;nPageSize:%d;nTotal:%d",nFirstPageSum,stFaceInfo.nPageSize,stFaceInfo.nTotal);
			ACSCOM_CleanFaceAddress();
			//PRT_PRINT(ACS_DEBUG,"pSyncFaceData:%p;stRecordInfo.pRecord:%p",pSyncFaceData,stFaceInfo.pRecord);
			nPage = 1;
			break;
		}
		nPage++;
		
	}

	ACSCOM_Set_ifDataNum(ACS_ITURN);//推

	
	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_FACE,ACS_FALSE);
	PRT_PRINT(ACS_DEBUG,"face SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_FACE));
	

	ACSCOM_FreeFaceAddress();
	PRT_PRINT(ACS_DEBUG,"acs e_cmd:face;pMsgId:%s;end",pMsgId);
	return;
}


ACS_VOID syncCardResultUpload(ACS_CARD_INFO_S *pCardData,ACS_INT nCode,const ACS_CHAR *pMsgId)
{
	if(pCardData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib syncCardUpload pCardData point NULL");
		return;
	}
	
	if(ACS_LOGVER_TWO != ACSHTTP_Get_Logver())
	{
		ACS_SYNCCARD_RESULT_S stCardResult = {0};
		memset(&stCardResult,0,sizeof(ACS_SYNCCARD_RESULT_S));
		stCardResult.code = nCode;
		snprintf(stCardResult.ID,sizeof(stCardResult.ID),"%s",pCardData->ID);
		sprintf(stCardResult.userId,"%s", pCardData->userId);
		sprintf(stCardResult.cardId,"%s", pCardData->card);
		sprintf(stCardResult.operation, "%s", pCardData->operation);
		//PRT_PRINT(ACS_DEBUG,"acslib syncCardUpload pCardData point NULL nCode:%d",nCode);
		ACSHTTP_SyncCardResult_Upload(&stCardResult,pMsgId);
	}
	
	if((ACS_LOGVER_TWO == ACSHTTP_Get_Logver()) || (ACS_LOGVER_ZERO == ACSHTTP_Get_Logver()))
	{
		//if(ACS_ERR_SUCCESS != nCode)
		{
			ACS_MESSAGE_S stMsg = {0};
			sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCCARD);
			sprintf(stMsg.szID,"%s",pCardData->ID);
			stMsg.nCode = nCode;
			ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_SYCCARD,(ACS_CHAR *)pCardData,pMsgId);
		}
	}

	return; 
}

ACS_VOID syncCard(const ACS_CHAR *pMsgId)
{
	ACS_INT	ret = 0,getRet = ACS_ERR_FAIL;
	ACS_INT nPage = 1;
	ACS_INT k = 0;
	ACS_INT nFirstPageSum = 0;//ACS_INT nPageNum = ACSSYNCNUM;//请求的记录条数

	//ACS_INT	save = 0;
	
	PRT_PRINT(ACS_INFO,"acs e_cmd:card;pMsgId:%s",pMsgId);
	
	ACS_RECORD_S stCardInfo = {0};
	ACS_CARD_INFO_S *pSyncCardData = NULL;

	ACSCOM_Set_ifDataNum(ACS_FALSE);//不推


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_CARD,ACS_ITURN);
	PRT_PRINT(ACS_DEBUG,"card SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_CARD));
	
	while(1)
	{
		memset(&stCardInfo,0,sizeof(ACS_RECORD_S));

		if((Mqtt_Get_DataDealFlag() & ACS_CD_CARD) == ACS_CD_CARD)
		{
			getRet = ACSHTTP_Get_SyncCards(nPage,ACSSYNCNUM,&stCardInfo,pMsgId);//获取数据 入队列 获取的数据条数=队列数
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(nPage == 1)
			{
				nFirstPageSum = stCardInfo.nPageSize;
			}
			
			pSyncCardData = (ACS_CARD_INFO_S *)stCardInfo.pRecord;
			for(k=0;k<stCardInfo.nPageSize;k++)
			{
				if((Mqtt_Get_DataDealFlag() & ACS_CD_CARD) == ACS_CD_CARD)
				{
					ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYNCCARD,(ACS_CHAR *)&pSyncCardData[k]);
					syncCardResultUpload(&pSyncCardData[k],ret,pMsgId);
				}
				else
				{
					getRet = ACS_ERR_FAIL;
				}
			}
		}
		
		if((stCardInfo.nPageSize)&&(stCardInfo.pRecord))
		{
			PRT_PRINT(ACS_DEBUG,"free pSyncCardData:%p;stCardInfo.pRecord:%p",pSyncCardData,stCardInfo.pRecord);
			free(stCardInfo.pRecord);
			stCardInfo.pRecord = NULL;
			pSyncCardData = NULL;
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(stCardInfo.nPageSize >= ACS_SUCCESS)	
				ACSHTTP_Set_SyncMark(ACS_CD_CARD);
		}
		
		//平台无数据或者数据取完
		if((nFirstPageSum < 1)||(stCardInfo.nPageSize < nFirstPageSum)||(stCardInfo.nTotal <= ACSSYNCNUM))
		{
			PRT_PRINT(ACS_DEBUG,"acs card nFirstPageSize:%d;nPageSize:%d;nTotal:%d",nFirstPageSum,stCardInfo.nPageSize,stCardInfo.nTotal);
			PRT_PRINT(ACS_DEBUG,"pSyncCardData:%p;stCardInfo.pRecord:%p",pSyncCardData,stCardInfo.pRecord);
			nPage = 1;
			break;
		}
		nPage++;
	}
	
	ACSCOM_Set_ifDataNum(ACS_ITURN);//推


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_CARD,ACS_FALSE);
	PRT_PRINT(ACS_DEBUG,"card SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_CARD));
	

	PRT_PRINT(ACS_INFO,"acs e_cmd:card;pMsgId:%s end",pMsgId);
	return;
}




ACS_VOID syncPwdResultUpload(ACS_PWD_INFO_S *pPwdData,ACS_INT nCode,const ACS_CHAR *pMsgId)
{
	if(pPwdData == NULL)
	{
		PRT_PRINT(ACS_DEBUG,"acslib syncPwdResultUpload pPwdData point NULL");
		return;
	}

	PRT_PRINT(ACS_DEBUG,"acslib syncPwdResultUpload nCode:%d",nCode);

	if((ACS_LOGVER_TWO == ACSHTTP_Get_Logver()) || (ACS_LOGVER_ZERO == ACSHTTP_Get_Logver()))
	{
		//if(ACS_ERR_SUCCESS != nCode)
		{
			ACS_MESSAGE_S stMsg = {0};
			sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCPWD);
			sprintf(stMsg.szID,"%s",pPwdData->ID);
			stMsg.nCode = nCode;
			ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_SYCPWD,(ACS_CHAR *)pPwdData,pMsgId);
		}
	}

	return; 
}


ACS_VOID syncPwd(const ACS_CHAR *pMsgId)
{
	ACS_INT	ret = 0,getRet=1;
	ACS_INT nPage = 1;
	ACS_INT k = 0;
	ACS_INT nFirstPageSum = 0;
	PRT_PRINT(ACS_INFO,"acs e_cmd:pwd;pMsgId:%s",pMsgId);

	ACS_RECORD_S stPwdInfo = {0};
	ACS_PWD_INFO_S *pSyncPwdData = NULL;

	
	ACSCOM_Set_ifDataNum(ACS_FALSE);//不推


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_PWD,ACS_ITURN);
	PRT_PRINT(ACS_DEBUG,"pwd SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_PWD));

	

	while(1)//card //PRT_PRINT("card nPage:%d",nPage);
	{
		memset(&stPwdInfo,0,sizeof(ACS_RECORD_S));
		
		if((Mqtt_Get_DataDealFlag() & ACS_CD_PWD) == ACS_CD_PWD)
		{
			getRet = ACSHTTP_Get_SyncPwds(nPage,ACSSYNCNUM,&stPwdInfo,pMsgId);//获取数据 入队列 获取的数据条数=队列数
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(nPage == 1)
			{
				nFirstPageSum = stPwdInfo.nPageSize;
			}
			
			pSyncPwdData = (ACS_PWD_INFO_S *)stPwdInfo.pRecord;
			for(k=0;k<stPwdInfo.nPageSize;k++)
			{
				if((Mqtt_Get_DataDealFlag() & ACS_CD_PWD) == ACS_CD_PWD)
				{
					ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYNCPWD,(ACS_CHAR *)&pSyncPwdData[k]);
					syncPwdResultUpload(&pSyncPwdData[k],ret,pMsgId);
				}
				else
				{
					getRet = ACS_ERR_FAIL;
				}
			}
		}
		
		if((stPwdInfo.nPageSize)&&(stPwdInfo.pRecord))
		{
			PRT_PRINT(ACS_DEBUG,"free pSyncPwdData:%p;stPwdInfo.pRecord:%p",pSyncPwdData,stPwdInfo.pRecord);
			free(stPwdInfo.pRecord);
			stPwdInfo.pRecord = NULL;
			pSyncPwdData = NULL;
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(stPwdInfo.nPageSize >= ACS_SUCCESS)
				ACSHTTP_Set_SyncMark(ACS_CD_PWD);
		}
		
		//平台无数据或者数据取完
		if((nFirstPageSum < 1)||(stPwdInfo.nPageSize < nFirstPageSum)||(stPwdInfo.nTotal <= ACSSYNCNUM))
		{
			PRT_PRINT(ACS_DEBUG,"acs pwd nFirstPageSize:%d;nPageSize:%d;nTotal:%d",nFirstPageSum,stPwdInfo.nPageSize,stPwdInfo.nTotal);
			PRT_PRINT(ACS_DEBUG,"pSyncPwdData:%p;stPwdInfo.pRecord:%p",pSyncPwdData,stPwdInfo.pRecord);
			nPage = 1;
			break;
		}
		nPage++;
	}

	ACSCOM_Set_ifDataNum(ACS_ITURN);//推


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_PWD,ACS_FALSE);
	PRT_PRINT(ACS_DEBUG,"pwd SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_PWD));
	
	
	PRT_PRINT(ACS_INFO,"acs e_cmd:pwd;pMsgId:%s end",pMsgId);

	return;
}


ACS_VOID syncAdResultUpload(ACS_ADVERT_INFO_S *pAdData,ACS_INT nCode,const ACS_CHAR *pMsgId)
{
	if(pAdData == NULL)
	{
		PRT_PRINT(ACS_INFO,"acslib syncAdUpload pFaceData point NULL");
		return;
	}
	PRT_PRINT(ACS_DEBUG,"nCode:%d",nCode);

	if(ACS_LOGVER_TWO != ACSHTTP_Get_Logver())
	{
		ACS_ADVERT_RESULT_S stAdResult = {0};
		memset(&stAdResult,0,sizeof(ACS_ADVERT_RESULT_S));
		stAdResult.code = nCode;
		snprintf(stAdResult.ID,sizeof(stAdResult.ID),"%s",pAdData->ID);
		sprintf(stAdResult.adId,"%s", pAdData->adId);
		sprintf(stAdResult.operation, "%s", pAdData->operation);
		ACSHTTP_SyncAdResult_Upload(&stAdResult,pMsgId);//上传广告入库同步结果
	}
	
	if((ACS_LOGVER_TWO == ACSHTTP_Get_Logver()) || (ACS_LOGVER_ZERO == ACSHTTP_Get_Logver()))
	{
		ACS_MESSAGE_S stMsg = {0};
		//if(ACS_ERR_SUCCESS != nCode)
		{
			sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SETAD);
			sprintf(stMsg.szID,"%s",pAdData->ID);
			sprintf(stMsg.szMsg,"%s",pAdData->msg);
			stMsg.nCode = nCode;
			ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_SETAD,(ACS_CHAR *)pAdData,pMsgId);
		}
	}

	return; 
}


ACS_VOID syncAds(const ACS_CHAR *pMsgId)
{
	ACS_INT k = 0;
	ACS_INT ret = 0,getRet=1;
	ACS_RECORD_S stAdInfo = {0};
	stAdInfo.pRecord = NULL;

	ACS_ADVERT_INFO_S *pAdPicData = NULL;
	
	PRT_PRINT(ACS_DEBUG,"acs e_cmd:ad;pMsgId:%s",pMsgId);

	//不推
	ACSCOM_Set_ifDataNum(ACS_FALSE);


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_AD,ACS_ITURN);
	PRT_PRINT(ACS_DEBUG,"ad SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_AD));

	if((Mqtt_Get_DataDealFlag() & ACS_CD_AD) == ACS_CD_AD)
	{
		getRet = ACSHTTP_Get_SyncAds(0,0,&stAdInfo,pMsgId);
	}
	
	if(ACS_ERR_SUCCESS == getRet)
	{
		pAdPicData = (ACS_ADVERT_INFO_S *)stAdInfo.pRecord;
		for(k = 0;k < stAdInfo.nPageSize; k++)
		{
			if((Mqtt_Get_DataDealFlag() & ACS_CD_AD) == ACS_CD_AD)
			{
				ret = ACSCOM_SyncAdPic(&pAdPicData[k],pMsgId);
				if(ACS_ERR_SUCCESS == ret)
				{
					//ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYNCAD,(ACS_CHAR *)&stAdInfo.pRecord[k]);
					ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYNCAD,(ACS_CHAR *)&pAdPicData[k]);
					syncAdResultUpload(&pAdPicData[k],ret,pMsgId);
				}
					
				if(&pAdPicData[k] && (pAdPicData[k].pPicData))
				{
					PRT_PRINT(ACS_DEBUG,"acs ads free pPicData:%p;nPicDataLen:%d",pAdPicData[k].pPicData,pAdPicData[k].nPicDataLen);
					free(pAdPicData[k].pPicData);
					pAdPicData[k].pPicData = NULL;
				}
			}
			else
			{
				getRet = ACS_ERR_FAIL;
			}
		}
	}
	
	PRT_PRINT(ACS_DEBUG,"acs ads nPageSize:%d;nTotal:%d;ret:%d",stAdInfo.nPageSize,stAdInfo.nTotal,ret);
	
	if(ACS_ERR_SUCCESS == getRet)
	{
		if(stAdInfo.nPageSize >= ACS_SUCCESS)	
			ACSHTTP_Set_SyncMark(ACS_CD_AD);
	}
	
	if(stAdInfo.pRecord)
	{
		PRT_PRINT(ACS_DEBUG,"free stAdInfo.pRecord:%p",stAdInfo.pRecord);
		free(stAdInfo.pRecord);
		stAdInfo.pRecord = NULL;
	}
	
	ACSCOM_Set_ifDataNum(ACS_ITURN);//推


	ACSCOM_Set_SyncDataFlag(ACS_CDSYNC_AD,ACS_FALSE);
	PRT_PRINT(ACS_DEBUG,"ad SyncData:%d",ACSCOM_Get_SyncDataFlag(ACS_CDSYNC_AD));
	
	
	PRT_PRINT(ACS_INFO,"acs e_cmd:ads;pMsgId:%s end",pMsgId);
	return;
}


ACS_VOID syncReissueRecords(const ACS_CHAR *pMsgId)
{
	ACS_INT nPage = 1;
	ACS_INT k = 0;
	//ACS_INT ret = 0;
	ACS_INT nFirstPageSum = 0;//ACS_INT nPageNum = ACSSYNCNUM;//请求的记录条数
	
	ACS_RECORD_S stReissueInfo = {0};
	stReissueInfo.pRecord = ACSCOM_MallocReissueAddress();
	PRT_PRINT(ACS_DEBUG,"e_cmd:reissue;pMsgId:%s",pMsgId);
	if(NULL == stReissueInfo.pRecord)
	{
		PRT_PRINT(ACS_DEBUG,"ACSCOM_MallocReissueAddress:%p NULL;",stReissueInfo.pRecord);
		return;
	}
	
	ACS_REISSUE_RECORD_S *pReissueData = (ACS_REISSUE_RECORD_S *)stReissueInfo.pRecord;
	if(NULL == pReissueData)
	{
		PRT_PRINT(ACS_DEBUG,"pReissueData:%p NULL;",pReissueData);
		return;
	}

	
	//不推
	ACSCOM_Set_ifDataNum(ACS_FALSE);
	

	while(1)
	{	
		ACSHTTP_Get_ReissueRecords(nPage,REISSUENUM,&stReissueInfo,pMsgId);
		//printf("[%s %d] ===== ret=%d =====\n", __func__, __LINE__, ret);
		if(nPage == 1)
		{
			nFirstPageSum = stReissueInfo.nPageSize;					
		}

		for(k=0;k<stReissueInfo.nPageSize;k++)
		{
			if((Mqtt_Get_DataDealFlag() & ACS_CD_REISSUES) == ACS_CD_REISSUES)
			{
				ACSCOM_HandleCmdCallback(ACS_CMD_REISSUE_RECORD,(ACS_CHAR *)&pReissueData[k]);
			}
			else
			{
				stReissueInfo.nPageSize = -1;//break;
			}
		}
		
		if(stReissueInfo.nPageSize >= ACS_SUCCESS)	
			ACSHTTP_Set_SyncMark(ACS_CD_REISSUES);

		//平台无数据 数据取完 停止入库
		if((nFirstPageSum < 1)||(stReissueInfo.nPageSize < nFirstPageSum)||(stReissueInfo.nTotal <= REISSUENUM))
		{
			PRT_PRINT(ACS_DEBUG,"acs face nFirstPageSize:%d;nPageSize:%d;nTotal:%d",nFirstPageSum,stReissueInfo.nPageSize,stReissueInfo.nTotal);
			ACSCOM_CleanReissueAddress();
			PRT_PRINT(ACS_DEBUG,"pReissueData:%p;pReissueData.pRecord:%p",pReissueData,stReissueInfo.pRecord);
			nPage = 1;
			break;
		}
		nPage++;
	}

	ACSCOM_Set_ifDataNum(ACS_ITURN);//推

	ACSCOM_FreeReissueAddress();
	return;
}


ACS_VOID syncDevParam(const ACS_CHAR *pMsgId)
{
	ACSHTTP_SyncParam(pMsgId);
	return;
}
ACS_INT PostDevParam(const ACS_CHAR *pMsgId)
{
	return ACSHTTP_PostDevParam(pMsgId);
}

ACS_VOID PostDevDcitTalkList(const ACS_CHAR *pMsgId)
{
	ACSHTTP_PostDevDcitTalkList(pMsgId);
	return;
}


ACS_VOID syncEleRulesResultUpload(ACS_ELERULES_INFO_S *pEleRulesData,ACS_INT nCode,const ACS_CHAR *pMsgId)
{
	if(pEleRulesData == NULL)
	{
		PRT_PRINT(ACS_ERROR,"acslib syncEleRules pEleRulesData point NULL");
		return;
	}
	PRT_PRINT(ACS_DEBUG,"nCode:%d",nCode);

	if((ACS_LOGVER_TWO == ACSHTTP_Get_Logver()) || (ACS_LOGVER_ZERO == ACSHTTP_Get_Logver()))
	{
		ACS_MESSAGE_S stMsg = {0};//if(ACS_ERR_SUCCESS != nCode)
		{
			sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_ELERULES);
			sprintf(stMsg.szID,"%s",pEleRulesData->ID);
			//sprintf(stMsg.szMsg,"%s",pEleRulesData->msg);
			stMsg.nCode = nCode;
			ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_ELEVATORRULES,(ACS_CHAR *)pEleRulesData,pMsgId);
		}
	}

	return; 
}

ACS_VOID syncEleRules(const ACS_CHAR *pMsgId)
{
	ACS_INT k = 0;
	ACS_INT j = 0;
	ACS_INT ret = ACS_ERR_SUCCESS,getRet=1;
	ACS_INT nPage = 1;
	ACS_INT nFirstPageSum = 0;
	ACS_RECORD_S stERulesInfo = {0};
	stERulesInfo.pRecord = NULL;
	ACS_FLOORS_INFO_S *pFloorData = NULL;
	ACS_ELERULES_INFO_S *peleRules = NULL;
	PRT_PRINT(ACS_DEBUG,"acs e_cmd:eleRules;pMsgId:%s",pMsgId);

	while(1)
	{
		if(ACS_CD_ELERULES == (Mqtt_Get_DataDealFlag() & ACS_CD_ELERULES))
		{
			getRet = ACSHTTP_Get_SyncElevatorRules(nPage,0,&stERulesInfo,pMsgId);
		}
		else
		{
			stERulesInfo.nPageSize = 0;
		}
		
		if(ACS_ERR_SUCCESS == getRet)
		{
			if(nPage == 1)
			{
				nFirstPageSum = stERulesInfo.nPageSize;					
			}
		}
		
		for(k=0;k<stERulesInfo.nPageSize;k++)
		{
			peleRules = (ACS_ELERULES_INFO_S *)stERulesInfo.pRecord;
			if(peleRules)
			{
				pFloorData = (ACS_FLOORS_INFO_S *)peleRules[k].pFloorData;
				if(pFloorData)
				{
					if(ACS_ERR_SUCCESS == getRet)
					{
						if(ACS_CD_ELERULES == (Mqtt_Get_DataDealFlag() & ACS_CD_ELERULES))
						{
							for(j=0;j<peleRules[k].floorTotal;j++)
							{
								memset(&peleRules[k].stFloors,0,sizeof(ACS_FLOORS_INFO_S));
								memcpy(&peleRules[k].stFloors,&pFloorData[j],sizeof(ACS_FLOORS_INFO_S));
								ret = ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYNCELERULE,(ACS_CHAR *)&peleRules[k]);
								syncEleRulesResultUpload((ACS_ELERULES_INFO_S *)&stERulesInfo.pRecord[k],ret,pMsgId);						
							}
						}
						else
						{
							getRet = ACS_ERR_FAIL;
						}	
						
						/**
						else
						{
							stERulesInfo.nPageSize = -1;//break;
						}**/
					}
							
					if(pFloorData)
					{
						PRT_PRINT(ACS_DEBUG,"free pstFloorss:%p;stERulesInfo.pRecord:%p",pFloorData,peleRules[k].pFloorData);
						free(pFloorData);
						pFloorData = NULL;
						peleRules[k].pFloorData = NULL;
					}
				}
			}
		}

		if(ACS_ERR_SUCCESS == getRet)
		{
			if(stERulesInfo.nPageSize >= ACS_SUCCESS)	
				ACSHTTP_Set_SyncMark(ACS_CD_ELERULES);
		}
		
		//平台无数据 数据取完 停止入库
		if((nFirstPageSum < 1)||(stERulesInfo.nPageSize < nFirstPageSum)||(stERulesInfo.nTotal <= REISSUENUM))
		{
			PRT_PRINT(ACS_DEBUG,"acs eleRules nFirstPageSize:%d;nPageSize:%d;nTotal:%d",nFirstPageSum,stERulesInfo.nPageSize,stERulesInfo.nTotal);
			PRT_PRINT(ACS_DEBUG,"stERulesInfo:%p;stERulesInfo.pRecord:%p",stERulesInfo,stERulesInfo.pRecord);
			nPage = 1;
			break;
		}
		nPage++;
	}
	
	PRT_PRINT(ACS_DEBUG,"acs eleRules nPageSize:%d;nTotal:%d;ret:%d",stERulesInfo.nPageSize,stERulesInfo.nTotal,ret);
	
	if(stERulesInfo.pRecord)
	{
		PRT_PRINT(ACS_DEBUG,"free pstEleRules.pRecord:%p",stERulesInfo.pRecord);
		free(stERulesInfo.pRecord);
		stERulesInfo.pRecord = NULL;
	}
	return;
}


ACS_VOID  SyncMark(ACS_BYTE byOpe,const ACS_ULONG   utimestamp)
{
	ACS_SYNC_MARK_S stSysctime = {0};
	ACS_UINT64 ulltimestamp = (ACS_UINT64)utimestamp;
	if(utimestamp)
	{
		ACSCOM_Get_SyncTimeCfg(&stSysctime);
		switch (byOpe)
		{
			case ACS_CD_FACE:
			{
				stSysctime.stTime.face = ulltimestamp;
				break;
			}
			case ACS_CD_CARD:
			{
				stSysctime.stTime.card = ulltimestamp;
				break;
			}
			case ACS_CD_AD:
			{
				stSysctime.stTime.ad = ulltimestamp;
				break;
			}
			case ACS_CD_PWD:
			{
				stSysctime.stTime.pwd = ulltimestamp;
				break;
			}
			case ACS_CD_REISSUES:
			{
				stSysctime.stTime.reissue = ulltimestamp;
				break;
			}
			default:
			{
				break;
			}
		}
		ACSCOM_Set_SyncTimeCfg(&stSysctime);
		PRT_PRINT(ACS_DEBUG,"byOpe:%d;stSysctime.stTime:%lld;ltimestamp:%ld",byOpe,ulltimestamp,utimestamp);
	}
	return;
}


//非MQTT阻塞 (费时间)
ACS_VOID *DataDeal_Task(ACS_VOID *arg)
{
	PRT_PRINT(ACS_DEBUG,"DataDeal_Task begin");
	ACS_CMD_S stCmdInfo;// = {0};
	while(1)
	{
		memset(&stCmdInfo,0,sizeof(ACS_CMD_S));
		ACSCOM_DeDataQueue(&stCmdInfo);
		if(stCmdInfo.e_cmd == ACS_CD_FACE)		
		{
			SyncMark(stCmdInfo.e_cmd,stCmdInfo.utimestamp);
			syncFace(stCmdInfo.szMsgID);
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)ACS_FACES);
		}
		else if(stCmdInfo.e_cmd == ACS_CD_CARD)		
		{
			SyncMark(stCmdInfo.e_cmd,stCmdInfo.utimestamp);
			syncCard(stCmdInfo.szMsgID);
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)ACS_CARDS);
		}
		else if(stCmdInfo.e_cmd == ACS_CD_PWD)		
		{	
			SyncMark(stCmdInfo.e_cmd,stCmdInfo.utimestamp);
			syncPwd(stCmdInfo.szMsgID);
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)ACS_PWDS);
		}
		else if(stCmdInfo.e_cmd == ACS_CD_AD)		
		{
			SyncMark(stCmdInfo.e_cmd,stCmdInfo.utimestamp);
			syncAds(stCmdInfo.szMsgID);
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)ACS_ADS);
		}
		else if(stCmdInfo.e_cmd == ACS_CD_UPACK)
		{
			upgradePack(stCmdInfo.szMsgID);			
		}
		else if(stCmdInfo.e_cmd == ACS_CD_REISSUES)		
		{
			SyncMark(stCmdInfo.e_cmd,stCmdInfo.utimestamp);
			syncReissueRecords(stCmdInfo.szMsgID);
			ACSCOM_HandleCmdCallback(ACS_CMD_GET_STATISTICS,(ACS_CHAR *)ACS_REISSUES);
		}
		else if(stCmdInfo.e_cmd == ACS_CD_PARAM)		
		{
			syncDevParam(stCmdInfo.szMsgID);			
		}
		else if(stCmdInfo.e_cmd == ACS_CD_GETPARAM)
		{
			PostDevParam(stCmdInfo.szMsgID);
		}
		else if(stCmdInfo.e_cmd == ACS_CD_GETDCITTALKLIST)
		{
			PostDevDcitTalkList(stCmdInfo.szMsgID);			
		}
		else if(stCmdInfo.e_cmd == ACS_CD_ELERULES)		
		{
			SyncMark(stCmdInfo.e_cmd,stCmdInfo.utimestamp);
			syncEleRules(stCmdInfo.szMsgID);			
		}
		sleep(2);
	}

	ACSCOM_DestroyDataSequeue();
	return NULL;
}

