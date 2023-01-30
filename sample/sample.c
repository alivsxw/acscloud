#include "acscloudshare.h"

#include "sample.h"


#define 	ACS_INITPATH   				"/home/acscloud/test"
#define 	ACS_INITPATH_ENABLE   		"/home/acscloud/test/1"
#define 	ACS_INITPATH_TEST   		"/home/acscloud/test/test.txt"

#define 	ACS_INITPATH_DISENABLE   	"/home/acscloud/test/0"


#if 0
#define PRT_MAIN(...)
#else
#ifndef PRT_MAIN
#define PRT_MAIN(msg,arg...)		printf("[acsMain]:[%s-%d]---- " msg " ----\n",__func__,__LINE__,##arg)
#endif
#endif

ACS_CLOUDLOGIN_S g_stLogIn = {0};



int Sample_Health_sample()
{
	ACS_HEALTH_QRCODE_S QRCodeinfo = {36.86, 	"{\"label\":\"yss\",\"cid\":\"44**************51\",\"cidtype\":\"1000\",\"name\":\"陈**\",\"phone\":\"134****0574\",\"encode\":\"5de2e04284210907ba273e925d9eabc10de4707990df2feeefcaea153e0ff38975ae8498463719b49cacdef622dcc275bcb90e07beaf3130a658620ea8206e2fa9912ab8a95bd782439bcff5aecf762e\",\"e1\":\"d632a25ecbd49ce5d68a552cc4a993ba\"}"};
	//ACS_HEALTH_QRCODE_S QRCodeinfo = {36.86, 	"ABj+AV18wMbARLQEbELTNsGMwMoxyuSWAGkCAUpLTTExMTAwMDAAR0JHU6472WA3r67Rh1MXmF89jM+EdetkgG1JzYGuqj23mBcnzpSvKjJzScaOnf/LY1kWLqdBqTVBunfboAvfhZNB/Nsj4+ntpnABdoSzBdTodQfPyMBLqDQAR0JHUyAGDAEpWlzEA7oVgNv4wY6Yh0RaRyKqxbtBJpP3OgTe2c6YaouYTe2jw9BgKP2laFNG/KXA6+xzgWDHxE2bZWf5NVkz"};
	ACS_HEALTHCODE_INFO_S Healthinfo = {0};
	ACS_HealthCode_QRCode_Check(&QRCodeinfo, &Healthinfo,"");
	printf("RetCode:%d\n", Healthinfo.RetCode);
	printf("message:%s\n", Healthinfo.message);
	printf("Timestamp:%lld\n", Healthinfo.Timestamp);
	printf("name:%s\n", Healthinfo.name);
	printf("id:%s\n", Healthinfo.id);
	printf("status:%s\n", Healthinfo.status);
	printf("date:%s\n", Healthinfo.date);
	printf("reason:%s\n", Healthinfo.reason);
	printf("stopOverCity:%s\n", Healthinfo.stopOverCity);
	printf("type:%d\n", Healthinfo.type);
	printf("stNucleicAcid[0].checkTime:%s\n", Healthinfo.stNucleicAcid[0].checkTime);
	printf("stNucleicAcid[0].checkResult:%s\n", Healthinfo.stNucleicAcid[0].checkResult);
	printf("stNucleicAcid[1].checkTime:%s\n", Healthinfo.stNucleicAcid[1].checkTime);
	printf("stNucleicAcid[1].checkResult:%s\n", Healthinfo.stNucleicAcid[1].checkResult);
	printf("stNucleicAcid[2].checkTime:%s\n", Healthinfo.stNucleicAcid[2].checkTime);
	printf("stNucleicAcid[2].checkResult:%s\n", Healthinfo.stNucleicAcid[2].checkResult);
	printf("=============================================\n");
	memset(&Healthinfo, 0, sizeof(Healthinfo));
	ACS_HEALTH_IDCARD_S IdCardinfo = {36.88, "9jUVtuzFBUdoWqoEovA2Qe/zKxGgTHOMT3ijx9SHwhg=", "brofdfhzW706lA3aPjCMJQ=="};
	ACS_HealthCode_IDCard_Check(&IdCardinfo, &Healthinfo,"");
	printf("RetCode:%d\n", Healthinfo.RetCode);
	printf("message:%s\n", Healthinfo.message);
	printf("Timestamp:%lld\n", Healthinfo.Timestamp);
	printf("name:%s\n", Healthinfo.name);
	printf("id:%s\n", Healthinfo.id);
	printf("status:%s\n", Healthinfo.status);
	printf("date:%s\n", Healthinfo.date);
	printf("reason:%s\n", Healthinfo.reason);
	printf("stopOverCity:%s\n", Healthinfo.stopOverCity);
	printf("type:%d\n", Healthinfo.type);
	printf("stNucleicAcid[0].checkTime:%s\n", Healthinfo.stNucleicAcid[0].checkTime);
	printf("stNucleicAcid[0].checkResult:%s\n", Healthinfo.stNucleicAcid[0].checkResult);
	printf("stNucleicAcid[1].checkTime:%s\n", Healthinfo.stNucleicAcid[1].checkTime);
	printf("stNucleicAcid[1].checkResult:%s\n", Healthinfo.stNucleicAcid[1].checkResult);
	printf("stNucleicAcid[2].checkTime:%s\n", Healthinfo.stNucleicAcid[2].checkTime);
	printf("stNucleicAcid[2].checkResult:%s\n", Healthinfo.stNucleicAcid[2].checkResult);
	printf("=============================================\n");
	
	return 0;
}

/**
* fun:打印同步的人脸信息
* @pszFaceInfo-in:平台下发的执行指令
* @return:void;
**/
void Sample_PrintfPicData(ACS_FACE_INFO_S *pszFaceInfo)
{
	ACS_ELEDISRULES_S *peleRuleDatas = (ACS_ELEDISRULES_S *)pszFaceInfo->peDisRulesData;
	printf("printf face begin\nuserId    :%s\n",pszFaceInfo->userId);
	printf("end operation :%s\n",pszFaceInfo->operation);
	printf("userName  :%s\n",pszFaceInfo->userName);
	printf("userIdCard:%s\n",pszFaceInfo->userIdCard);
	printf("userType  :%s\n",pszFaceInfo->userType);
	printf("url       :%s\n",pszFaceInfo->url);
	printf("eRulesNum :%d\n",pszFaceInfo->neDisRules);
	for(int i = 0;i< pszFaceInfo->neDisRules;i++)
	{
		printf("eleType[%d]   :%d\n",i,peleRuleDatas[i].eleFloor);
	}
	return;
}

/**
* fun:打印同步的卡号信息
* @pszFaceInfo-in:平台下发的执行指令
* @return:void;
**/
void Sample_PrintfCardData(ACS_CARD_INFO_S *pszCardInfo)
{
	printf("userId   :%s\n",pszCardInfo->userId);
	printf("type	 :%s\n",pszCardInfo->type);
	printf("card	 :%s\n",pszCardInfo->card);
	printf("expired  :%s\n",pszCardInfo->expired);
	printf("operation:%s\n",pszCardInfo->operation);
	return;
}

/**
* fun:打印同步的密码信息
* @pszFaceInfo-in:平台下发的执行指令
* @return:void;
**/
void Sample_PrintfPwdData(ACS_PWD_INFO_S *pszPwdInfo)
{
	printf("ID		 :%s\n",pszPwdInfo->ID);
	printf("userId   :%s\n",pszPwdInfo->userId);
	printf("pwd		 :%s\n",pszPwdInfo->pwd);
	printf("expired  :%s\n",pszPwdInfo->expired);
	printf("limitnum :%d\n",pszPwdInfo->limitnum);
	printf("operation:%s\n",pszPwdInfo->operation);
	return;
}


/**
* fun:打印同步的广告信息
* @pszAdInfo-in:平台下发的执行指令
* @return:void;
**/
void Sample_PrintfAdData(ACS_ADVERT_INFO_S *pszAdInfo)
{
	printf("begen operation:%s\n",pszAdInfo->operation);
	printf("ID   	 :%s\n",pszAdInfo->ID);
	printf("adId	 :%s\n",pszAdInfo->adId);
	printf("adurl	 :%s\n",pszAdInfo->adurl);
	printf("adPicName	 :%s\n",pszAdInfo->adPicName);
	return;
}


void Sample_OpenDoorResultUpload(char *pUserId,int nIDCard,int nifOpenDoor)
{
	ACS_OPENDOOR_UPLOAD_S stOpenDoor = {0};
	if(nIDCard)
	{
		sprintf(stOpenDoor.type, "FACE");
		sprintf(stOpenDoor.userId,"%s","1408691564697788418");
		sprintf(stOpenDoor.data, "110708OE");
	}
	else 
	{
		sprintf(stOpenDoor.type,"%s","APP");
		sprintf(stOpenDoor.userId,"%s",pUserId);
	}

	stOpenDoor.byOpenDoorEn = nifOpenDoor;
	sprintf(stOpenDoor.screenTime,"%ld",time(NULL));
	if(1)
	{
		snprintf(stOpenDoor.snapfacepath,sizeof(stOpenDoor.snapfacepath),"%s","/home/mjdoor/55.jpg");
	}
	else
	{
		snprintf(stOpenDoor.snapfacepath,sizeof(stOpenDoor.snapfacepath),"%s","No.jpg");
	}
	/**身份证信息**/
	snprintf(stOpenDoor.stIDCard.szName,sizeof(stOpenDoor.stIDCard.szName),"%s","李陆兵");
	snprintf(stOpenDoor.stIDCard.szIDCardImg,sizeof(stOpenDoor.stIDCard.szIDCardImg),"%s","/home/mjdoor/55.jpg");
	snprintf(stOpenDoor.stIDCard.szGender,sizeof(stOpenDoor.stIDCard.szGender),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szNational,sizeof(stOpenDoor.stIDCard.szNational),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szBirthday,sizeof(stOpenDoor.stIDCard.szBirthday),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szAddress,sizeof(stOpenDoor.stIDCard.szAddress),"%s","湖南省涿州市荔枝区富民路11号");
	snprintf(stOpenDoor.stIDCard.szId,sizeof(stOpenDoor.stIDCard.szId),"%s","4405xxxxxxxxxx0987");
	snprintf(stOpenDoor.stIDCard.szMaker,sizeof(stOpenDoor.stIDCard.szMaker),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szStartDate,sizeof(stOpenDoor.stIDCard.szStartDate),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szEndDate,sizeof(stOpenDoor.stIDCard.szEndDate),"%s","N/A");
	stOpenDoor.temperature = 36.6;

	//上传开门记录
	ACS_OpenDoorRecord_Upload(&stOpenDoor);
	
	return;
}



/**
* fun:上传设备数据库的人脸ID或者卡号用于平台验证数据
* @pstRecordOut-out:输出设备数据信息
* @return:0-success;-1-FAIL;
**/
int Sample_CheakDataUplaod(ACS_CHECKDATA_INFO_S *pstRecordOut)
{
	int i = 0;
	int nList = -1;
	int nTotal = 0;

	if(pstRecordOut == NULL)
		return -1;

	
	if(strcmp(pstRecordOut->operation,ACS_FACES) == 0)
	{
		if(0)//判断人脸是否正在入库
		{
			printf("acs i:%d[%s:%d]\n",i,__func__,__LINE__);
			
			pstRecordOut->code = ACS_ERR_DEVBUSY;
			sprintf(pstRecordOut->msg,"%s", "设备忙");
			return -1;
		}

		ACS_CHECKDATA_FACE_S *pFaceDataWhite = NULL;
		ACS_CHECKDATA_FACE_S *pFaceDataBlack = NULL;
		ACS_CHECKDATA_FACE_S *pFaceDataVip = NULL;
		//char *pListData[3] = NULL;
		//char szUserName[64] = {0};
		//sdk_fileindex_info_t WbFileIndexCfg[FACE_FOLDER_ALLPIC_NUM];
		//memset(WbFileIndexCfg,0,sizeof(sdk_fileindex_info_t)*FACE_FOLDER_ALLPIC_NUM);

		nTotal = 6;//BwlistGetFileIndexFromBuff(FACE_WHITELIST,(CHAR *)WbFileIndexCfg);
		if(nTotal > 0)
		{
			pFaceDataWhite = (ACS_CHECKDATA_FACE_S *)malloc(sizeof(ACS_CHECKDATA_FACE_S)*nTotal);
			if(pFaceDataWhite)
			{
				memset((char *)pFaceDataWhite, 0, (sizeof(ACS_CHECKDATA_FACE_S)*nTotal));
				for(i=0;i<nTotal;i++)
				{
					sprintf(pFaceDataWhite[i].userId, "%d", i);
				}
				nList += 1;
				printf("acs Whites nList:%d;malloc pFaceDataWhite:%p [%s:%d]\n",nList,pFaceDataWhite,__func__,__LINE__);
				pstRecordOut->stList[nList].pstData = (char *)pFaceDataWhite;
				pstRecordOut->stList[nList].list = 2;
				pstRecordOut->stList[nList].total = nTotal;
				pstRecordOut->size += 1;
				pstRecordOut->code = ACS_ERR_SUCCESS;
			}
			else
			{
				printf("acs malloc Whites fail pFaceDatas NULL[%s:%d]\n",__func__,__LINE__);
			}
		}

		nTotal = 7;//BwlistGetFileIndexFromBuff(FACE_WHITELIST,(CHAR *)WbFileIndexCfg);
		if(nTotal > 0)
		{
			pFaceDataBlack = (ACS_CHECKDATA_FACE_S *)malloc(sizeof(ACS_CHECKDATA_FACE_S)*nTotal);
			if(pFaceDataBlack)
			{
				memset((char *)pFaceDataBlack, 0, (sizeof(ACS_CHECKDATA_FACE_S)*nTotal));
				for(i=0;i<nTotal;i++)
				{
					sprintf(pFaceDataBlack[i].userId, "ab%d", i);
				}
				nList += 1;
				printf("acs Blacks nList:%d;malloc pFaceDataBlack:%p [%s:%d]\n",nList,pFaceDataBlack,__func__,__LINE__);
				pstRecordOut->stList[nList].pstData = (char *)pFaceDataBlack;
				pstRecordOut->stList[nList].list = 1;
				pstRecordOut->stList[nList].total = nTotal;
				pstRecordOut->size += 1;
				pstRecordOut->code = ACS_ERR_SUCCESS;
			}
			else
			{
				printf("acs malloc Blacks fail pFaceDatas NULL[%s:%d]\n",__func__,__LINE__);
			}
		}
	}
	else if(strcmp(pstRecordOut->operation,ACS_CARDS) == 0)
	{
		if(0)//判断卡号是否正在入库
		{
			printf("acs i:%d[%s:%d]\n",i,__func__,__LINE__);
			
			pstRecordOut->code = ACS_ERR_DEVBUSY;
			sprintf(pstRecordOut->msg,"%s", "设备忙");
			return -1;
		}
	
		ACS_CHECKDATA_CARD_S *pCardData = NULL;
		nTotal = 6;
		//sdk_fileindex_info_t WbFileIndexCfg[FACE_FOLDER_ALLPIC_NUM];
		//memset(WbFileIndexCfg,0,sizeof(sdk_fileindex_info_t)*FACE_FOLDER_ALLPIC_NUM);
		//nFaceTotal = BwlistGetFileIndexFromBuff(FACE_WHITELIST,(CHAR *)WbFileIndexCfg);
		if(nTotal > 0)
		{
			pCardData = (ACS_CHECKDATA_CARD_S *)malloc(sizeof(ACS_CHECKDATA_CARD_S)*nTotal);
			if(pCardData)
			{
				memset(pCardData, 0, (sizeof(ACS_CHECKDATA_CARD_S)*nTotal));
				for(i=0; i<nTotal; i++)
				{
					//sprintf(pCardData[i].userId, "%d", i);				
					sprintf(pCardData[i].cardId, "abc%d", i);			
				}
				nList += 1;
				printf("acs Whites nList:%d;malloc pCardData:%p [%s:%d]\n",nList,pCardData,__func__,__LINE__);
				pstRecordOut->stList[nList].pstData = (char *)pCardData;
				pstRecordOut->stList[nList].list = 2;
				pstRecordOut->stList[nList].total = nTotal;
				pstRecordOut->size += 1;
				pstRecordOut->code = ACS_ERR_SUCCESS;
			}
			else
			{
				printf("acs malloc Whites fail pCardData NULL[%s:%d]\n",__func__,__LINE__);
			}
		}
	}
	else
	{
		pstRecordOut->code = ACS_ERR_FAIL;
		sprintf(pstRecordOut->msg,"%s", "操作失败");
		printf("acs operation[%s] [%s:%d]\n",pstRecordOut->operation,__func__,__LINE__);
	}

	return 0;
}

int Http_testupload(char *pUserId,int nIDCard,int nifOpenDoor)
{
	ACS_OPENDOOR_UPLOAD_S stOpenDoor = {0};
	if(nIDCard)
	{
		sprintf(stOpenDoor.type, "IDCARD");
		sprintf(stOpenDoor.userId,"%s","123");
		sprintf(stOpenDoor.data, "110708OE");
	}
	else 
	{
		sprintf(stOpenDoor.type,"%s","APP");
		sprintf(stOpenDoor.userId,"%s",pUserId);
	}

	stOpenDoor.byOpenDoorEn = nifOpenDoor;
	sprintf(stOpenDoor.screenTime,"%ld",time(NULL));
	if(1)
	{
		snprintf(stOpenDoor.snapfacepath,sizeof(stOpenDoor.snapfacepath),"%s","/home/acscloud/test/test.jpg");
	}
	else
	{
		snprintf(stOpenDoor.snapfacepath,sizeof(stOpenDoor.snapfacepath),"%s","No.jpg");
	}
	/**身份证信息**/
	snprintf(stOpenDoor.stIDCard.szName,sizeof(stOpenDoor.stIDCard.szName),"%s","蔡伦");
	//snprintf(stOpenDoor.stIDCard.szIDCardImg,sizeof(stOpenDoor.stIDCard.szIDCardImg),"%s","/home/acscloud/test.jpg");///home/mjdoor/data.jpg
	snprintf(stOpenDoor.stIDCard.szGender,sizeof(stOpenDoor.stIDCard.szGender),"%s","男");
	snprintf(stOpenDoor.stIDCard.szNational,sizeof(stOpenDoor.stIDCard.szNational),"%s","汉");
	snprintf(stOpenDoor.stIDCard.szBirthday,sizeof(stOpenDoor.stIDCard.szBirthday),"%s","");
	snprintf(stOpenDoor.stIDCard.szAddress,sizeof(stOpenDoor.stIDCard.szAddress),"%s","湖南省涿州市荔枝区富民路11号");
	snprintf(stOpenDoor.stIDCard.szId,sizeof(stOpenDoor.stIDCard.szId),"%s","4405xxxxxxxxxx0987");
	snprintf(stOpenDoor.stIDCard.szMaker,sizeof(stOpenDoor.stIDCard.szMaker),"%s","涿州市公安局");
	snprintf(stOpenDoor.stIDCard.szStartDate,sizeof(stOpenDoor.stIDCard.szStartDate),"%s","");
	snprintf(stOpenDoor.stIDCard.szEndDate,sizeof(stOpenDoor.stIDCard.szEndDate),"%s","");
	stOpenDoor.temperature = 36.6;

	//上传开门记录
	
	
	return ACS_OpenDoorRecord_Upload(&stOpenDoor);
}

void Sample_CheckData(char *pOutData)
{
	ACS_CHECKDATA_INFO_S *pstCheackData = (ACS_CHECKDATA_INFO_S *)pOutData;
	ACS_CHECKDATA_FACE_S *pFace = NULL;
	ACS_CHECKDATA_CARD_S *pCard = NULL;

	
	PRT_MAIN("operation:%s;ID:%s;code:%d;size:%d",\
		pstCheackData->operation,pstCheackData->ID,pstCheackData->code,pstCheackData->size);
	
	Sample_CheakDataUplaod(pstCheackData);
	
	PRT_MAIN("operation:%s;ID:%s;code:%d;size:%d",\
		pstCheackData->operation,pstCheackData->ID,pstCheackData->code,pstCheackData->size);
	
	if(strcmp(pstCheackData->operation,ACS_FACES) == 0)
	{
		for(int j=0;j<pstCheackData->size;j++)
		{
			PRT_MAIN("face list:%d;total:%d;pstData:%p",\
				pstCheackData->stList[j].list,pstCheackData->stList[j].total,pstCheackData->stList[j].pstData);
			
			pFace = (ACS_CHECKDATA_FACE_S *)pstCheackData->stList[j].pstData;
			if(pFace)
			{
				PRT_MAIN("pFace:%p;",pFace);
				for(int i=0;i<pstCheackData->stList[j].total;i++)
				{
					PRT_MAIN("face userId[%d]:%s;",i,pFace[i].userId);
				}
			}
		}
	}
	else if(strcmp(pstCheackData->operation,ACS_CARDS) == 0)
	{
		for(int j=0;j<pstCheackData->size;j++)
		{
			PRT_MAIN("card list:%d;total:%d;pstData:%p",\
				pstCheackData->stList[j].list,pstCheackData->stList[j].total,pstCheackData->stList[j].pstData);
			
			pCard = (ACS_CHECKDATA_CARD_S *)pstCheackData->stList[0].pstData;
			if(pCard)
			{
				PRT_MAIN("pCard:%p;",pCard);
				for(int i=0;i<pstCheackData->stList[j].total;i++)
				{
					PRT_MAIN("card [%d] userId:%s;cardId:%s;",i,pCard[i].cardId,pCard[i].cardId);
				}
			}
		}				
	}
	//ACS_PostCheckDataResult_Upload(pstCheackData);
	
	return;
}


int Sample_SysFace(char *pOutData)
{
	ACS_FACE_INFO_S *pFaceData = (ACS_FACE_INFO_S *)pOutData;

	Sample_PrintfPicData(pFaceData);
	
	srand((unsigned)time(NULL));
	ACS_SYNCFACE_RESULT_S stFaceResult = {0};
	memset(&stFaceResult,0,sizeof(ACS_SYNCFACE_RESULT_S));
	snprintf(stFaceResult.ID,sizeof(stFaceResult.ID),"%s",pFaceData->ID);
	sprintf(stFaceResult.userId,"%s", pFaceData->userId);
	sprintf(stFaceResult.url,"%s", pFaceData->url);
	sprintf(stFaceResult.operation, "%s", pFaceData->operation);

	if(rand()%2)
		stFaceResult.code = ACS_ERR_SUCCESS;
	else
		stFaceResult.code = ACS_ERR_FAIL;
	
	PRT_MAIN("Dev sysface code %d",stFaceResult.code);

	
	char szPicPath[128] = {0};
	sprintf(szPicPath,"/home/mjdoor/%s.jpg",stFaceResult.userId);
    FILE *fpWrite=fopen(szPicPath,"w");  
    if(fpWrite)  
    {
	    fwrite(pFaceData->pPicData,pFaceData->nPicDataLen, 1, fpWrite);  
	  	fclose(fpWrite);
    }

	//上传人脸入库同步结果
	//ACS_SyncFace_Result_Upload(&stFaceResult); 
	return stFaceResult.code;
}

int Sample_SysCard(char *pOutData)
{
	ACS_CARD_INFO_S *pCardData = (ACS_CARD_INFO_S *)pOutData;
	Sample_PrintfCardData(pCardData);

	srand((unsigned)time(NULL));
	ACS_SYNCCARD_RESULT_S stCardResult = {0};
	memset(&stCardResult,0,sizeof(ACS_SYNCCARD_RESULT_S));
	snprintf(stCardResult.ID,sizeof(stCardResult.ID),"%s",pCardData->ID);
	sprintf(stCardResult.userId,"%s", pCardData->userId);
	sprintf(stCardResult.cardId,"%s", pCardData->card);
	sprintf(stCardResult.operation, "%s", pCardData->operation);

	if(rand()%2)
		stCardResult.code = ACS_ERR_SUCCESS;
	else
		stCardResult.code = ACS_ERR_FAIL;
	
	PRT_MAIN("Dev syscard code %d",stCardResult.code);

	//上传卡号入库同步结果
	//ACS_SyncCard_Result_Upload(&stCardResult);
	
	//return ACS_ERR_FAIL;
	return stCardResult.code;
}


int Sample_SysPwd(char *pOutData)
{
	int code = 0;
	ACS_PWD_INFO_S *pPwdData = (ACS_PWD_INFO_S *)pOutData;
	Sample_PrintfPwdData(pPwdData);

	if(rand()%2)
		code = ACS_ERR_SUCCESS;
	else
		code = ACS_ERR_FAIL;
	
	PRT_MAIN("Dev SysPwd code %d",code);

	//上传卡号入库同步结果
	//ACS_SyncCard_Result_Upload(&stCardResult);
	
	//return ACS_ERR_FAIL;
	return code;
}


int Sample_SysAd(char *pOutData)
{
	ACS_ADVERT_INFO_S *pAdData = (ACS_ADVERT_INFO_S *)pOutData;
	Sample_PrintfAdData(pAdData);
	
	srand((unsigned)time(NULL));
	ACS_ADVERT_RESULT_S stAdResult = {0};
	memset(&stAdResult,0,sizeof(ACS_ADVERT_RESULT_S));
	
	snprintf(stAdResult.ID,sizeof(stAdResult.ID),"%s",pAdData->ID);
	sprintf(stAdResult.adId,"%s", pAdData->adId);
	
	sprintf(stAdResult.operation, "%s", pAdData->operation);

	if(rand()%2)
		stAdResult.code = ACS_ERR_SUCCESS;
	else
		stAdResult.code = ACS_ERR_FAIL;
	
	PRT_MAIN("Dev sysad code %d",stAdResult.code);

	//上传广告入库同步结果
	//ACS_SyncAd_Result_Upload(&stAdResult);
	return stAdResult.code;
}
int CORED_ACSloud_UpParam(char *pDataIn)
{
	if(NULL == pDataIn)
		return -1;
	
	ACS_DEVCONFIG_S *pDevCfg = (ACS_DEVCONFIG_S *)pDataIn;
	
	PRT_MAIN("UpParam szAddress:%s",pDevCfg->szAddress);


	return 0;
}


int CORED_ACSloud_SyncParam(char *pDataIn)
{
	if(NULL == pDataIn)
		return 0;

	ACS_DEVCFG_INFO_S *stAcsDevinfo = (ACS_DEVCFG_INFO_S *)pDataIn;

	
	switch (stAcsDevinfo->nType)
	{
		case ACS_DEV_PARAM_HCODECLOUD:
		{
			ACS_HCODECLOUD_S stAcsHcodeCloud = {0};
			memcpy(stAcsHcodeCloud.szServerIP, "192.168.1.100", 256);
			memcpy(stAcsHcodeCloud.szAuthIV, "123456", 64);
			memcpy(stAcsHcodeCloud.szAuthKey, "123456", 64);
			memcpy(stAcsHcodeCloud.szSceneCategory, "123456", 64);
			memcpy(stAcsHcodeCloud.szSceneSerialID, "123456", 64);
			memcpy(stAcsHcodeCloud.szCityCode, "123456", 64);
			
			memcpy((char*)stAcsDevinfo->szData, &stAcsHcodeCloud,sizeof(ACS_HCODECLOUD_S));
			break;
		}
		case ACS_DEV_PARAM_HCODECLOUD+ACS_TYPE_SET_OFFSET:
		{
			ACS_HCODECLOUD_S stAcsHcodeCloud = {0};
			memcpy(&stAcsHcodeCloud, (char*)stAcsDevinfo->szData, sizeof(ACS_HCODECLOUD_S));
			PRT_MAIN("stAcsHcodeCloud.szServerIP:%s", stAcsHcodeCloud.szServerIP);
			PRT_MAIN("stAcsHcodeCloud.szCityCode:%s", stAcsHcodeCloud.szCityCode);
			break;
		}
		case ACS_DEV_PARAM_CLOUDCFG:
		{
			ACS_CLOUDCFG_S stAcsCloud = {0};
			stAcsCloud.byEnable =  1;
			memcpy(stAcsCloud.szCloudIPAddr, "192.168.1.100", 64);
			memcpy(stAcsCloud.szCloudID, "2661", 64);
			memcpy((char*)stAcsDevinfo->szData, &stAcsCloud, sizeof(ACS_CLOUDCFG_S));
			break;
		}
		case ACS_DEV_PARAM_CLOUDCFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_CLOUDCFG_S stAcsCloud = {0};
			memcpy(&stAcsCloud, (char*)stAcsDevinfo->szData, sizeof(ACS_CLOUDCFG_S));
			break;
		}
		case ACS_DEV_PARAM_ACSCFG:
		{
			ACS_ACSCFG_S stAcsCfg_s = {0};
			stAcsCfg_s.dwAcsState = 1;
			stAcsCfg_s.dwStrangeRecEnable = 1;
			stAcsCfg_s.dwRecoredPicEnable = 2;
			stAcsCfg_s.dwStrangeTik = 2;
			stAcsCfg_s.dwDelay1Time = 3;
			stAcsCfg_s.dwIo1Type = 3;
			
			memcpy((char*)stAcsDevinfo->szData, &stAcsCfg_s, sizeof(ACS_ACSCFG_S));
			break;
		}
		case ACS_DEV_PARAM_ACSCFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_ACSCFG_S stAcsCfg_s = {0};		
			memcpy(&stAcsCfg_s, (char*)stAcsDevinfo->szData, sizeof(ACS_ACSCFG_S));
			PRT_MAIN("dwAcsState:%d", stAcsCfg_s.dwAcsState);
			PRT_MAIN("dwIo1Type:%d", stAcsCfg_s.dwIo1Type);
			break;
		}
		case ACS_DEV_PARAM_SFACECFG:
		{
			ACS_SFACECFG_S stsFaceCfg = {0};
			stsFaceCfg.dwAcsScene = 1;
			stsFaceCfg.dwCmpThreshold = 2;
			stsFaceCfg.dwCmpSingleThreshold = 2;
			stsFaceCfg.dwFaceMinSize = 2;
			stsFaceCfg.dwFaceMaxSize = 2;
			stsFaceCfg.dwHacknessenble = 2;
			stsFaceCfg.dwHacknessthreshold = 33;
			stsFaceCfg.dwHacknessRank = 44;
			stsFaceCfg.dwMaskEnable = 55;
			stsFaceCfg.dwFaceSensitivity = 6;
			memcpy((char*)stAcsDevinfo->szData, &stsFaceCfg, sizeof(ACS_SFACECFG_S));
			break;
		}
		case ACS_DEV_PARAM_SFACECFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_SFACECFG_S stsFaceCfg = {0};
			memcpy(&stsFaceCfg, (char*)stAcsDevinfo->szData, sizeof(ACS_SFACECFG_S));
			break;
		}
		case ACS_DEV_PARAM_CAMCFG:
		{
			ACS_CAMCFG_S stCameraCfg = {0};
			memcpy(stCameraCfg.szDevName, "asd", sizeof(stCameraCfg.szDevName));
			memcpy((char*)stAcsDevinfo->szData, &stCameraCfg, sizeof(ACS_CAMCFG_S));
			break;
		}
		case ACS_DEV_PARAM_CAMCFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_CAMCFG_S stCameraCfg = {0};
			memcpy(&stCameraCfg, (char*)stAcsDevinfo->szData, sizeof(ACS_CAMCFG_S));
			break;
		}
		case ACS_DEV_PARAM_AUDIOCFG:
		{
			ACS_AUDIOSTREAM_S stAudioCfg = {0};
			stAudioCfg.byEnable = 1;
			stAudioCfg.byVolumeIn = 12;
			stAudioCfg.byVolumeOut = 12;
			
			memcpy((char*)stAcsDevinfo->szData, &stAudioCfg, sizeof(ACS_AUDIOSTREAM_S));
			break;
		}
		case ACS_DEV_PARAM_AUDIOCFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_AUDIOSTREAM_S stAudioCfg = {0};
			memcpy(&stAudioCfg, (char*)stAcsDevinfo->szData, sizeof(ACS_AUDIOSTREAM_S));
			PRT_MAIN("UpParam byEnable:%d;%d;%d",stAudioCfg.byEnable,stAudioCfg.byVolumeIn,stAudioCfg.byVolumeOut);
			break;
		}
		case ACS_DEV_PARAM_WIREDNETCFG:
		{
			ACS_WIREDNET_S wirednet = {0};
			wirednet.bDHCP = 1;
			memcpy(wirednet.ipAddress, "qq", sizeof(wirednet.ipAddress));
			memcpy(wirednet.subnetMask, "ss", sizeof(wirednet.subnetMask));
			memcpy(wirednet.DefaultGateway, "xx", sizeof(wirednet.DefaultGateway));
			memcpy(wirednet.PrimaryDNS, "xdx", sizeof(wirednet.PrimaryDNS));
			memcpy(wirednet.SecondaryDNS, "vvv", sizeof(wirednet.SecondaryDNS));
			memcpy(wirednet.MACAddress, "vvv", sizeof(wirednet.MACAddress));
			memcpy((char*)stAcsDevinfo->szData, &wirednet, sizeof(ACS_WIREDNET_S));

			PRT_MAIN("UpParam Get ipAddress:%s",wirednet.ipAddress);
			
			break;
		}
		case ACS_DEV_PARAM_WIREDNETCFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_WIREDNET_S wirednet = {0};
			memcpy(&wirednet, (char*)stAcsDevinfo->szData, sizeof(ACS_WIREDNET_S));
			
			break;
		}
		case ACS_DEV_PARAM_SYNTEMVERCFG:
		{
			ACS_SYSTEMVERSION_S syntemVer = {0};
			memcpy(syntemVer.DeviceAlgoVersion, "www", sizeof(syntemVer.DeviceAlgoVersion));
			memcpy(syntemVer.DeviceOnvifVersion, 
"www", sizeof(syntemVer.DeviceOnvifVersion));
			memcpy(syntemVer.DeviceSoftVersion,"vvv",sizeof(syntemVer.DeviceSoftVersion));
			memcpy(syntemVer.DeviceUiVersion, "nn", sizeof(syntemVer.DeviceUiVersion));
			memcpy(syntemVer.DeviceWebVersion,"mmm", sizeof(syntemVer.DeviceWebVersion));
			memcpy((char*)stAcsDevinfo->szData, &syntemVer, sizeof(ACS_SYSTEMVERSION_S));

			PRT_MAIN("UpParam Get cpuid:%s;DeviceUiVersion:%s",syntemVer.cpuid,syntemVer.DeviceUiVersion);
			
			break;
		}
		case ACS_DEV_PARAM_BUILDINGCFG:
		{
			ACS_BUILDINGINFO_S building = {0};
			memcpy(building.AControlIp,"nnn",sizeof(building.AControlIp));
			memcpy(building.AreaID,"nnn",sizeof(building.AreaID));
			memcpy(building.BuildingID,"mmm",sizeof(building.BuildingID));
			memcpy(building.BusDevName,"mmm",sizeof(building.BusDevName));
			building.bBusDevType = 2;
			memcpy(building.CenterIP,"nnnn",sizeof(building.CenterIP));
			memcpy(building.DevNumber,"hhhg",sizeof(building.DevNumber));			
			memcpy(building.RoomID,"hhhh",sizeof(building.RoomID));
			building.bSlaveType = 2;
			memcpy(building.UnitID,
"hhh",sizeof(building.UnitID));
			memcpy((char*)stAcsDevinfo->szData, &building, sizeof(ACS_BUILDINGINFO_S));
			
			PRT_MAIN("UpParam Get RoomID:%s,BuildingID:%s,UnitID:%s",\
				building.RoomID,building.BuildingID,building.UnitID);
			
			break;
		}
		case ACS_DEV_PARAM_BUILDINGCFG+ACS_TYPE_SET_OFFSET:
		{
			ACS_BUILDINGINFO_S building = {0};
			memcpy(&building, (char*)stAcsDevinfo->szData, sizeof(ACS_BUILDINGINFO_S));
			
			
			PRT_MAIN("UpParam Set RoomID:%s,BuildingID:%s,UnitID:%s",\
				building.RoomID,building.BuildingID,building.UnitID);
			break;
		}
		default:
		{
			return 1;
			break;
		}
	}

	return 0;
}

/**
* fun:发起呼叫 房间号/手机号/管理机
* @cmd-in:平台下发的执行指令 ACS_CALL_PHONE
* @pOutData-in:平台下发的待处理的数据
* @return:0 eg:ACS_ERR_E;
**/
int Sample_MakingCall(const int nCalType,const char *pCallUUID)
{
	int ret = 0;
	ACS_CALL_INFO_S stPostData = {0};
	ACS_CALL_RESULT_S stDataOut = {0};
	stPostData.nCallType = nCalType;
	sprintf(stPostData.cScreenTime,"%ld",time(NULL));
	memcpy(stPostData.cDataUUID,pCallUUID,32);
	sprintf(stPostData.cPhoto,"%s","/home/acscloud/test/adPic.jpg");

	

	ret = ACS_Call(&stPostData,"",&stDataOut);
	PRT_MAIN("ret:%d;nResult:%d;retCode:%d;cMsg:%s;message:%s;bRingtime:%d;nDataLen:%d\n",\
		ret,stDataOut.nResult,stDataOut.retCode,\
		stDataOut.cMsg,stDataOut.message,stDataOut.bRingtime,stDataOut.nDataLen);


		
	return ret;
}

/**
* fun:呼叫状态发送给服务器
* @byCallState-in:输入呼叫的状态        ACS_CALLTYPE_E eg:ACS_CALL_HANGUP 
* @return:CORED_SUCCESS/CORED_FAILED;
**/
int Sample_PushCallState(const int byCallState)
{
	int ret = 0;
	
	ret = ACS_CallPushState(ACS_CALL_PHONE,"123456",byCallState);
	
	PRT_MAIN("PushCallState ret:%d;byCallType:%d;szCallObject:%s;nCallState:%d\n",\
		ret,byCallState,"xx",byCallState);

	return ret;
}

/**
* fun:处理mqtt接收的操作指令函数
* @cmd-in:平台下发的执行指令
* @pOutData-in:平台下发的待处理的数据
* @return:0;
**/
int Sample_Cloud_handleCmd(int      nCmds,char *pOutData)
{
	printf("AcsCloud_handleCmd nCmds:%d\n",nCmds);
	int nRet = 0;
	switch(nCmds)
	{
		case ACS_CMD_DEV_SYNCSTOP:
		{
			PRT_MAIN("sycstop pOutData:%s",pOutData);
			break;
		}
		case ACS_CMD_GET_SYNCUSER:
		{
			PRT_MAIN("sysface");

			nRet = Sample_SysFace(pOutData);			
			sleep(1);
			
			break;
		}
		case ACS_CMD_GET_SYNCCARD:
		{
			PRT_MAIN("syscard");
			nRet = Sample_SysCard(pOutData);
			sleep(1);
			break;
		}
		case ACS_CMD_GET_SYNCPWD:
		{
			PRT_MAIN("sysPwd");
			nRet = Sample_SysPwd(pOutData);
			sleep(1);
			break;
		}
		case ACS_CMD_GET_SYNCAD:
		{
			PRT_MAIN("sysad");
			
			nRet = Sample_SysAd(pOutData);
			sleep(1);
			
			break;
		}
		case ACS_CMD_DEV_REBOOT:
		{
			PRT_MAIN("reboot %d",*(int *)pOutData);
			break;
		}
		case ACS_CMD_SET_OPENDOOR:
		{
			PRT_MAIN("openDoor");
			
			Sample_OpenDoorResultUpload(pOutData,0,0);
			
			break;
		}
		case ACS_CMD_GET_CHECKDATA:
		{
			PRT_MAIN("AcsCloud_handleCmd checkData");
			Sample_CheckData(pOutData);
			break;
		}
		case ACS_CMD_SET_UPGRADEFW:
		{
			ACS_UPGADEFW_INFO_S *pstUpgadeFW = (ACS_UPGADEFW_INFO_S *)pOutData;
			PRT_MAIN("AcsCloud_handleCmd nSize:%d;szMd5:%s;szVersion:%s;szUrl:%s;szFileName:%s;",\
				pstUpgadeFW->nSize,pstUpgadeFW->szMd5,pstUpgadeFW->szVersion,pstUpgadeFW->szUrl,pstUpgadeFW->szFileName);

			
			break;
		}
		case ACS_CMD_SET_CLEANDATA:
		{
			//清理人脸卡号广告等平台下发的数据
			PRT_MAIN("AcsCloud_handleCmd cleanData");
			break;
		}
		case ACS_CMD_DEV_STATE:
		{
			// ACS_SERVER_INIT
			ACS_SERVER_STATE_E *pState = (ACS_SERVER_STATE_E *)pOutData;
			PRT_MAIN("AcsCloud_handleCmd pState:%d",*pState);
			break;
		}
		case ACS_CMD_DEV_SYNCTIME:
		{
			ACS_UINT64 *plltimestamp = (ACS_UINT64 *)pOutData;
			PRT_MAIN("AcsCloud_handleCmd PlatformTime:%lld",*plltimestamp);
			break;
		}
		case ACS_CMD_SET_UPPARAM:
		{
			CORED_ACSloud_UpParam(pOutData);
			break;
		}
		case ACS_CMD_SYNCPARAM:
		{
			CORED_ACSloud_SyncParam(pOutData);	
			break;
		}
		default:
		{
			break;
		}
	}
	
	return nRet;
}

#if 0

int testCurl_DealSyncFace_callback()
{
	
ACS_CHAR EncodeBuf[ACSPICBASEMAXLEND] = {0};

	ACSHTTP_PicBase64("/database/image_storage/img29/2DA0F804A90F45BBB3C15A78EC754BDB.jpg",EncodeBuf);
		return 0;
}





int testCurl_DealSyncFace_callback(ACS_CHAR *CallData)
{
	ACS_RECORD_S CallDataOut;
	ACS_MESSAGE_S stMsgOut;
	//Curl_DealSyncFace_callback(CallData, (ACS_CHAR *)&CallDataOut, &stMsgOut);
	return 0;
}

int Http_testupload(char *pUserId,int nIDCard,int nifOpenDoor)
{
	ACS_OPENDOOR_UPLOAD_S stOpenDoor = {0};
	if(nIDCard)
	{
		sprintf(stOpenDoor.type, "IDCARD");
		sprintf(stOpenDoor.userId,"%s","ABC123");
		sprintf(stOpenDoor.data, "110708OE");
	}
	else 
	{
		sprintf(stOpenDoor.type,"%s","APP");
		sprintf(stOpenDoor.userId,"%s",pUserId);
	}

	stOpenDoor.byOpenDoorEn = nifOpenDoor;
	sprintf(stOpenDoor.screenTime,"%ld",time(NULL));
	if(1)
	{
		snprintf(stOpenDoor.snapfacepath,sizeof(stOpenDoor.snapfacepath),"%s","/home/acscloud/test.jpg");
	}
	else
	{
		snprintf(stOpenDoor.snapfacepath,sizeof(stOpenDoor.snapfacepath),"%s","No.jpg");
	}
	/**身份证信息**/
	snprintf(stOpenDoor.stIDCard.szName,sizeof(stOpenDoor.stIDCard.szName),"%s","蔡伦");
	//snprintf(stOpenDoor.stIDCard.szIDCardImg,sizeof(stOpenDoor.stIDCard.szIDCardImg),"%s","/home/acscloud/test.jpg");///home/mjdoor/data.jpg
	snprintf(stOpenDoor.stIDCard.szGender,sizeof(stOpenDoor.stIDCard.szGender),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szNational,sizeof(stOpenDoor.stIDCard.szNational),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szBirthday,sizeof(stOpenDoor.stIDCard.szBirthday),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szAddress,sizeof(stOpenDoor.stIDCard.szAddress),"%s","湖南省涿州市荔枝区富民路11号");
	snprintf(stOpenDoor.stIDCard.szId,sizeof(stOpenDoor.stIDCard.szId),"%s","4405xxxxxxxxxx0987");
	snprintf(stOpenDoor.stIDCard.szMaker,sizeof(stOpenDoor.stIDCard.szMaker),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szStartDate,sizeof(stOpenDoor.stIDCard.szStartDate),"%s","N/A");
	snprintf(stOpenDoor.stIDCard.szEndDate,sizeof(stOpenDoor.stIDCard.szEndDate),"%s","N/A");
	stOpenDoor.temperature = 36.6;

	//上传开门记录
	
	
	return ACS_OpenDoorRecord_Upload(&stOpenDoor);
}

void base64Lentest()
{
	ACS_CHAR picbuf[1024*1000] = {0};
	ACS_CHAR EncodeBuf[1024*1000] = {0};
	int nPiclen = 0, nEncodepiclen = 0;
	memset(picbuf, 0, sizeof(picbuf));
	memset(EncodeBuf, 0, sizeof(EncodeBuf));
	nPiclen = ACSHTTP_FileToData(picbuf,"/home/mjdoor/11.jpg");
	ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
	printf("Pic:%s strlenlen:%d;len:%d; EncodeBuflen:%d;%d\n",picbuf,strlen(picbuf),nPiclen,strlen(EncodeBuf),nEncodepiclen);

	memset(picbuf, 0, sizeof(picbuf));
	memset(EncodeBuf, 0, sizeof(EncodeBuf));
	nPiclen = ACSHTTP_FileToData(picbuf,"/home/mjdoor/22.jpg");
	ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
	printf("Pic strlenlen:%d;len:%d; EncodeBuflen:%d;%d\n",strlen(picbuf),nPiclen,strlen(EncodeBuf),nEncodepiclen);

	memset(picbuf, 0, sizeof(picbuf));
	memset(EncodeBuf, 0, sizeof(EncodeBuf));
	nPiclen = ACSHTTP_FileToData(picbuf,"/home/mjdoor/33.jpg");
	ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
	printf("Pic strlenlen:%d;len:%d; EncodeBuflen:%d;%d\n",strlen(picbuf),nPiclen,strlen(EncodeBuf),nEncodepiclen);

	memset(picbuf, 0, sizeof(picbuf));
	memset(EncodeBuf, 0, sizeof(EncodeBuf));
	nPiclen = ACSHTTP_FileToData(picbuf,"/home/mjdoor/44.jpg");
	ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
	printf("Pic strlenlen:%d;len:%d; EncodeBuflen:%d;%d\n",strlen(picbuf),nPiclen,strlen(EncodeBuf),nEncodepiclen);

	memset(picbuf, 0, sizeof(picbuf));
	memset(EncodeBuf, 0, sizeof(EncodeBuf));
	nPiclen = ACSHTTP_FileToData(picbuf,"/home/mjdoor/55.jpg");
	ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
	printf("Pic strlenlen:%d;len:%d; EncodeBuflen:%d;%d\n",strlen(picbuf),nPiclen,strlen(EncodeBuf),nEncodepiclen);

	
	memset(picbuf, 0, sizeof(picbuf));
	memset(EncodeBuf, 0, sizeof(EncodeBuf));
	nPiclen = ACSHTTP_FileToData(picbuf,"/home/mjdoor/acscloud.jpg");
	ACS_base64_encode(picbuf, nPiclen,EncodeBuf,&nEncodepiclen);
	printf("Pic strlenlen:%d;len:%d; EncodeBuflen:%d;%d\n",strlen(picbuf),nPiclen,strlen(EncodeBuf),nEncodepiclen);

}




void test_DownFile(void)
{
	printf("=====test_RamDownFile============================\n");
	curl_global_init(CURL_GLOBAL_ALL);
	struct AcsMemoryStruct fileBuf = {0};
	ACSHTTP_RamDownFile((ACS_CHAR *)"http://192.168.1.73:8086/database/image_storage/img24/240196B689FE4B42857791DD4F1FC051.jpg",&fileBuf,10);
	//ACS_Http_RamDownFile((ACS_CHAR *)"https://file.deepthen.com//temp/wx12_20220127220644_1643292603609.jpg",&fileBuf,10);
	ACSHTTP_RamDownFile((ACS_CHAR *)"https://ss0.bdstatic.com/5aV1bjqh_Q23odCf/static/superman/img/logo/bd_logo1_31bdc765.png",&fileBuf,10);

	printf("=====test_DiskDownFile============================\n");

	printf("=====test_DiskDownFile===http=========================\n");

	ACSHTTP_DiskDownFile((char *)"http://192.168.200.212:8086/database/image_storage/img90/46246BDB23A444788448030AFD7DBD4D.jpg","./11.jpg",10);
	printf("=====test_DiskDownFile===https=========================\n");
	ACSHTTP_DiskDownFile((char *)"https://ss0.bdstatic.com/5aV1bjqh_Q23odCf/static/superman/img/logo/bd_logo1_31bdc765.png","./22.jpg",10);
	
}



void Http_printftest()
{
	printf("=====printf=========\n");

	ACS_WLOG("======log======");
}


void Http_test()
{
	while(1)
	{
		ACS_UINT64 lltimestamp = 0;
	//	ACSHTTP_Get_PlatformTime(&lltimestamp);
		PRT_PRINT("lltimestamp:%lld",lltimestamp);
		//sleep(1);
	}
}

void Http_Facetest()
{
	ACS_RECORD_S stFaceInfo = {0};
	stFaceInfo.pRecord = ACSCOM_MallocFaceAddress();
	while(1)
	{
		ACSHTTP_Get_SyncFaces(1,ACSSYNCNUM,&stFaceInfo);
		PRT_PRINT("stFaceInfo.nPageSize:%d",stFaceInfo.nPageSize);
	}
}



void Http_Adtest()
{
	ACS_RECORD_S stAdInfo = {0};
	stAdInfo.pRecord = NULL;
	while(1)
	{
		ACSHTTP_Get_SyncAds(0,0,&stAdInfo);
		PRT_PRINT("stAdInfo.nPageSize:%d",stAdInfo.nPageSize);
		//for(k=0;k<stAdInfo.nPageSize;k++)
		{
			//ret = ACSCOM_SyncAdPic((ACS_ADVERT_INFO_S *)&stAdInfo.pRecord[k]);
			//if(ACS_ERR_SUCCESS == ret)
			{
				//if((Mqtt_Get_DataDealFlag() & ACS_CD_ADS) == ACS_CD_ADS)
				{
				//	ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYCADS,(ACS_CHAR *)&stAdInfo.pRecord[k]);
				}
			}
		}
	}
}
void Http_Cardtest()
{
	ACS_RECORD_S stCardInfo = {0};
	stCardInfo.pRecord = ACSCOM_MallocCardAddress();
	
	while(1)//card
	{
		//获取数据 入队列 获取的数据条数=队列数
		ACSHTTP_Get_SyncCards(1,ACSSYNCNUM,&stCardInfo);
		PRT_PRINT("stCardInfo.nPageSize:%d",stCardInfo.nPageSize);
		//for(k=0;k<stCardInfo.nPageSize;k++)
		{
			//if((Mqtt_Get_DataDealFlag() & ACS_CD_CARDS) == ACS_CD_CARDS)
			{
				//ACSCOM_HandleCmdCallback(ACS_CMD_GET_SYCCARD,(ACS_CHAR *)&pSyncCardData[k]);
			}
		}
	}
}

void AcsSyncTestTask()
{
	//curl_global_init(CURL_GLOBAL_ALL);
	int ret = 0;
	pthread_t pthread_ad;
	ret = pthread_create(&pthread_ad, NULL, (void *)&Http_Adtest, NULL);
	if(ret==0)
	{
		printf("ad Create success=%d\n",ret);
		pthread_detach(pthread_ad);
		sleep(1);
	}
	
	pthread_t pthread_card;
	ret = pthread_create(&pthread_card, NULL, (void *)&Http_Cardtest, NULL);
	if(ret==0)
	{
		printf("card Create success=%d\n",ret);
		pthread_detach(pthread_card);
		sleep(1);
	}

	pthread_t pthread_face;
	ret = pthread_create(&pthread_face, NULL, (void *)&Http_Facetest, NULL);
	if(ret==0)
	{
		printf("face Create success=%d\n",ret);
		pthread_detach(pthread_face);
		sleep(1);
	}

	pthread_t pthread_upload;
	ret = pthread_create(&pthread_upload, NULL, (void *)&Http_uploadtest, NULL);
	if(ret==0)
	{
		printf("upload Create success=%d\n",ret);
		pthread_detach(pthread_face);
		sleep(1);
	}
	//curl_global_cleanup();
}

#endif

void Http_uploadtest(void)
{
	int ret = 0;
	while(1)
	{
		ret = Http_testupload(NULL,1,1);
		PRT_MAIN("Http_testupload ret:%d",ret);
	}
}

void AcsUploadTestTask(void)
{
	int ret = 0;

	pthread_t pthread_upload;
	ret = pthread_create(&pthread_upload, NULL, (void *)&Http_uploadtest, NULL);
	if(ret==0)
	{
		printf("upload Create success=%d\n",ret);
		pthread_detach(pthread_upload);
		sleep(1);
	}
	//curl_global_cleanup();
}

#if 1
#if 0
ACS_INT AcsCom_Param_GetTest(ACS_CLOUDLOGIN_S *pstTimeInfo,const ACS_CHAR *pParamfile)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_CHAR *pJsonBuf = NULL;
	if(NULL == pstTimeInfo)
	{
		return ACS_ERR_PARAMNULL;
	}
	//pthread_mutex_lock(&gSysParamMutex_AcscloudInfo);
	if(AcsCom_Param_ReadJason(pParamfile,&pJsonBuf) > 0)
	{
		cJSON *pJsonRoot = cJSON_Parse(pJsonBuf);
		if(pJsonRoot)
		{
			cJSON *pAtteInfo = cJSON_GetObjectItem(pJsonRoot,"acscloud");
			if(pAtteInfo)
			{
				AcsCom_Param_GetJsonValue(pAtteInfo, "nEnable",  	&pstTimeInfo->nEnable,0,ACS_JSON_DATA_INT);
				AcsCom_Param_GetJsonValue(pAtteInfo, "szHttpUrl", 	&pstTimeInfo->szHttpUrl,128,ACS_JSON_DATA_STRING);
				AcsCom_Param_GetJsonValue(pAtteInfo, "szUuid", 		&pstTimeInfo->szUuid,32,ACS_JSON_DATA_STRING);
			}

			cJSON_Delete(pJsonRoot);
		}
		else
		{
			ACS_WLOG("Parse json %s is error",pParamfile);
		}
		free(pJsonBuf);
	}
	else
	{
		ACS_WLOG("read %s is error",pParamfile);
		ret =  ACS_ERR_FILE_READERR;
	}
	//pthread_mutex_unlock(&gSysParamMutex_AcscloudInfo);
	return  ret;
}


ACS_INT AcsCom_Param_SetTest(ACS_CLOUDLOGIN_S *pstTimeInfo,const ACS_CHAR *pParamfile)
{
	ACS_INT ret = ACS_ERR_FAIL;
	cJSON* pJsonRoot  = NULL;
	cJSON* pAtteInfo = NULL;
	ACS_CHAR*  pJsonBuf=NULL;
	//pthread_mutex_lock(&gSysParamMutex_AcscloudInfo);
	pJsonRoot  = cJSON_CreateObject();
	if(pJsonRoot)
	{
		pAtteInfo = cJSON_CreateObject();
		if(pAtteInfo)
		{
			cJSON_AddNumberToObject(pAtteInfo, "nEnable",	 pstTimeInfo->nEnable);
			cJSON_AddStringToObject(pAtteInfo, "szHttpUrl",	 pstTimeInfo->szHttpUrl);
			cJSON_AddStringToObject(pAtteInfo, "szUuid",	 pstTimeInfo->szUuid);
			cJSON_AddItemToObject(pJsonRoot, "acscloud", pAtteInfo);
		}
		
		pJsonBuf = cJSON_Print(pJsonRoot);
		if(pJsonBuf)
		{
			if(AcsCom_Param_WriteJason(pParamfile, pJsonBuf, strlen(pJsonBuf)) != ACS_ERR_SUCCESS)
			{
				ret = ACS_ERR_FILE_WRITEERR;
			}
			else
			{
				ret = ACS_ERR_SUCCESS;
			}
			free(pJsonBuf);
		}
		else
		{
			ret = ACS_ERR_MALLOCFAIL;
		}
		cJSON_Delete(pJsonRoot);
	}
	else
	{
		ret = ACS_ERR_MALLOCFAIL;
	}
	//pthread_mutex_unlock(&gSysParamMutex_AcscloudInfo);
	return ret; 
}

ACS_INT AcsCom_Param_SetTest(ACS_CLOUDLOGIN_S *pstTimeInfo,const ACS_CHAR *pParamfile);

ACS_INT AcsCom_Param_GetTest(ACS_CLOUDLOGIN_S *pstTimeInfo,const ACS_CHAR *pParamfile);


extern int ACSCOM_Get_ServiceState();
void *Sample_DynamicSetParam_Task(void *arg)
{
	ACS_CLOUDLOGIN_S *pstLogIn = (ACS_CLOUDLOGIN_S *)arg;
	PRT_MAIN("AcsCloud_handleCmd nEnable:%d;szHttpUrl:%s",pstLogIn->nEnable,pstLogIn->szHttpUrl);

	char szPath[128] = {0};
	ACS_CLOUDLOGIN_S szcloud = {0};
	AcsCom_Param_SetTest(pstLogIn,ACS_INITPATH_TEST);

	while(1)
	{
		sleep(5);
		if(access(ACS_INITPATH_ENABLE,F_OK) == 0)
		{
			//AcsCom_Param_GetTest(&szcloud,ACS_INITPATH_TEST);
			PRT_MAIN("AcsCloud_handleCmd nEnable:%d;szHttpUrl:%s;szUuid:%s",szcloud.nEnable,szcloud.szHttpUrl,szcloud.szUuid);

			
			g_stLogIn.nEnable = szcloud.nEnable;
			memcpy(g_stLogIn.szHttpUrl,szcloud.szHttpUrl,sizeof(g_stLogIn.szHttpUrl));//memcpy(g_stLogIn.szUuid,szcloud.szUuid,sizeof(g_stLogIn.szUuid));
			
			sprintf(szPath,"rm %s",ACS_INITPATH_ENABLE);
			ACSCOM_COMMON_System(szPath);
			ACS_SetServicePlatformInfo(&g_stLogIn);
		}
		else if(access(ACS_INITPATH_DISENABLE,F_OK) == 0)
		{
			memset(szPath,0,sizeof(szPath));
			sprintf(szPath,"rm %s",ACS_INITPATH_DISENABLE);
			g_stLogIn.nEnable = 0;
			ACS_SetServicePlatformInfo(&g_stLogIn);
			ACSCOM_COMMON_System(szPath);
			sleep(1);
		}

		
	}
	return NULL;
}

int Sample_DynamicSetParam_init(ACS_CLOUDLOGIN_S *pstLogIn)
{
	int ret = 0;
	pthread_t DynamicSetParam;
	//for(int i = 0;i<3;i++)
	{
		ret = pthread_create(&DynamicSetParam, NULL, Sample_DynamicSetParam_Task, (void *)pstLogIn);
		if(ret == 0)
		{
			pthread_detach(DynamicSetParam);
			PRT_MAIN("Create p_thread MqttState ok i:%d",ret);
		}
	}
	return 0;
}
#endif


#endif


void test(char **p)
{

	if(NULL == p)
		printf("==11==\n");
	if(*p)
		printf("==22==\n");

	*p = malloc(5);
	sprintf(*p,"%s","xxxx");

	
	printf("%p;%p;%s;%s\n",p,*p,p,*p);
}



int testCurl_Deal_callback()
{
	char *p = "{\"retCode\":200,\"message\":\"success\",\"data\":{\"healthcloudCfgV6\":null,\"audioStream\":null,\"cameraConfig\":null,\"sfaceCfg\":null,\"tempCfg\":null,\"healthCodeCfg\":null,\"CameraConfig\":null,\"AcsCfg\":{\"Relay1DelayTime\":5},\"BuildingCfg\":{\"BuildingID\":\"8\",\"BusDevName\":\"停车物联网关(研发测试)\",\"UnitID\":\"9\"}}}";
	ACS_MESSAGE_S stMsgOut = {0};
	PRT_MAIN("===Curl_DealSyncParam_Back===");
	//Curl_DealSyncParam_Back(p,NULL,&stMsgOut);

	PRT_MAIN("====Curl_DealSyncParam_Back===end");
	return 0;
}


int main(int argc,char* argv[])
{
	PRT_MAIN("====hallo acscloud world===");
char *p = "{\"retCode\":200,\"message\":\"ok\",\"timestamp\":\"1658124953\",\"data\":{\"ringtime\":45,\"result\":1204,\"msg\":\"占线中，请稍候再呼叫\",\"callid\":[\"000xf30be72a2951\"],\"webrtc\":[{\"realName\":\"ligt\",\"phone\":\"18124664324\",\"userType\":\"F\"},{\"realName\":\"user1\",\"phone\":\"181xxxxxxxx\",\"userType\":\"F\"}]}}";
//char *p = "{\"data\":{\"msg\":\"未找到可呼的对象\",\"webrtc\":[],\"ringtime\":1203},\"retCode\":200,\"message\":\"ok\",\"timestamp\":\"1670553163\"}";
	

	//PostDevParam("ccc");
//	ACS_CALL_CHECK_RESULT_S stCallRes = {0};
//	Curl_DealCallCheck_Back(p,&stCallRes);

	int ret = 0;
//	testCurl_DealSyncFace_callback();

	//return 0;

#if 0
	ACS_RECORD_S stCallDataOut = {0};
	ACS_MESSAGE_S stMsgOut = {0};
	ACS_FACE_INFO_S stFace[100] = {0};
	stCallDataOut.pRecord = (char *)&stFace;
	
	Curl_DealSyncFace_Back(p,(char *)&stCallDataOut,(char *)&stMsgOut);
	
	//外网平台
	char ip[64] = "http://aidcit.cloud/access";
	char uuid[32] = "8cun26658";
	char pCode[32] = "113666614626947072";
	//内网平台
	//char ip[64] = "http://192.168.0.10:8799";
	//char pCode[32] = "93738586426392576";
	//char uuid[32] = "12395";


	//湖南中智
	//char ip[64] = "http://120.26.65.115:8091/dcit";
	//char uuid[32] = "26658";
	//char pCode[32] = "26658";
	//外网平台
	//char ip[64] = "http://aidcit.cloud/access";
	//char uuid[32] = "8cun26658";
	//char pCode[32] = "113666614626947072";

	char ip[64] = "http://aidcit.cloud/access";
	char uuid[32] = "8cun26606";
	char pCode[32] = "113666614626947072";

	char ip[64] = "https://apids.deliyun.cn/apis/tacs";

	

	
#endif

	//char ip[64] = "https://mock.apifox.cn/m1/1533213-0-default";
	char ip[64] = "http://192.168.1.70:4523/m1/1533213-0-default";

	char uuid[32] = "26661";
#if 0//宏视
	char ip[64] = "www.eutrocity.com:9183";
#endif

#if 0//心眼
	char ip[64] = "http://47.97.103.88/parkServer";
	sprintf(g_stLogIn.szPcode,"%s","111");
#endif
	
	g_stLogIn.nEnable = 1;
	
	sprintf(g_stLogIn.szHttpUrl,"%s",ip);
	sprintf(g_stLogIn.szUuid,"%s",uuid);
	

	


	ret = ACS_ServiceInit(NULL,ACS_INITPATH,ACS_INITPATH);		//门禁云平台服务初始化
	if(ACS_ERR_SUCCESS != ret)
	{
		PRT_MAIN("ACS_ServiceInit ret fail:%d",ret);
		return ACS_ERR_FAIL;
	}
	
	//Sample_DynamicSetParam_init(&g_stLogIn);

	ACS_SetServicePlatformInfo(&g_stLogIn);				//设置门禁云平台参数
	ACS_SetCallback_HandleCmd(Sample_Cloud_handleCmd);	//注册通知回调函数

	PRT_MAIN("====hallo acscloud world===");

	
//	testCurl_Deal_callback();
	PRT_MAIN("===AcsUploadTestTask===");

	AcsUploadTestTask();
	

	sleep(2);
	//PRT_MAIN("upload ret fail:%d",ret);

	//Http_testupload("123",1,1);

	//sleep(2);

	//Http_testupload("456",0,0);
	
	for(int i = 1; i>0; i++)
	{
		sleep(10);
	}
	return 0;
}
