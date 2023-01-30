#include <stdarg.h>
#include <sys/file.h>

#include "acscloudcommon.h"
#include "acscloudmqtt.h"
#include "acscloudhttp.h"
#include "acscloudqueue.h"
#include "acscloudshare.h"
#include "acscloud_intellect.h"
#include "acsclouddefine.h"


#define ACSCLOUDPATH 	"/acscloud"
#define ACSPARAMFILE 	"/param.dat"
#define ACSADPICFILE 	"/adPic.jpg"
#define ACSFACEPICFILE 	"/acscloud.jpg"


ACS_QUEUE_S *g_pAcsCmdDbqueue  = NULL;					// 入库等花时间的命令队列

static ACS_FACE_INFO_S *g_pSyncFaceData = NULL;			//同步人脸数据的缓存
static ACS_REISSUE_RECORD_S *g_pReissueData = NULL; 	//补发考勤数据的缓存
static ACS_HandleCmd_Callback g_Acs_CmdHandle = NULL;	//消息处理通知回调函数
static ACS_SERVER_STATE_E ACS_State = ACS_SERVER_INIT;	//设备状态

static ACS_BYTE g_bifSetDataNum = ACS_ITURN;			//是否设置推送设备数据库数量标志 1-是 0-否 默认1
static ACS_DATANUM_S g_stDataNum = {-1};				//设备数据库数量

ACS_SYNC_MARK_S g_stSyncTime = {0};						//同步数据的时间
ACS_CONFIG_S g_stDoor_Info = {0};						//门禁云平台的所有信息


static ACS_DWORD g_syncData = ACS_FALSE;


static ACS_CHAR g_szAcsPath[64];						//门禁云平台存储文件的目录路径    			/xxx/acscloud
static ACS_CHAR g_szAcsUPackPath[64];					//门禁云平台存储升级包的文件

//static ACS_BYTE g_bHttpConnectTime = 5;					//mqtt连接超时时间秒s 登录 人脸 刷卡 密码 广告 升级 补发考勤 派梯 参数



#if 1
static pthread_mutex_t gSysParamMutex_AcscloudInfo = PTHREAD_MUTEX_INITIALIZER;
static ACS_INT AcsCom_Param_WriteJason(const ACS_CHAR *pFilePath,const ACS_CHAR *pParam,ACS_INT nLen)
{	
	ACS_INT ret = ACS_ERR_SUCCESS;
	FILE *pFile  = NULL;
	
	if(pFilePath==NULL || pParam==NULL || nLen <=0)
	{
		return ACS_ERR_PARAMERR;
	}
	pFile = fopen(pFilePath, "wb");
	if(pFile == NULL)
	{
		ACS_WLOG("open %s err",pFilePath);
		return ACS_ERR_FILE_OPENERR;
	}

	ret = fwrite(pParam, nLen, 1, pFile);
	if(ret != 1)
	{
		fclose(pFile);
		pFile=NULL;
		ACS_WLOG("ACS_sha1_write %s err",pFilePath);
		return ACS_ERR_FILE_WRITEERR;
	}
	fflush(pFile);
	fsync(fileno(pFile));
	fclose(pFile);
	pFile = NULL;
	return ACS_ERR_SUCCESS;
}

/******************************************************************************
函数名称: CORED_Param_ReadJason

功能描述: 读取json值 (函数内部调用)

入口参数: 	参数1:pFilePath json文件路径
		 参数2:pParam json数据
	
出口参数:成功返回json数据长度,失败返回-1

使用方法:
******************************************************************************/
static ACS_INT AcsCom_Param_GetFileSize(const ACS_CHAR* filename)
{

    struct stat statbuf;
	ACS_INT size = 0;

	if(access(filename,F_OK)==-1)
	{
		ACS_WLOG("%s not find err",filename);
		return -1;
	}
	
    if(stat(filename,&statbuf)==0)
	{
		size = statbuf.st_size;
    }
    return size;
}
static ACS_INT AcsCom_Param_ReadJason(const ACS_CHAR *pFilePath,ACS_CHAR **pParam)
{
	ACS_INT ret = ACS_ERR_FAIL;
	ACS_INT nJasonLen = 0;
	FILE *pFile  = NULL;
	
	if(pFilePath == NULL)
	{
		return -1;
	}
	if(access(pFilePath, F_OK) == -1)
	{
		ACS_WLOG("%s not find err",pFilePath);
		return -1;
	}
	
	nJasonLen= AcsCom_Param_GetFileSize(pFilePath);
	if(nJasonLen > 0)
	{
		if(*pParam!=NULL)
		{
			*pParam=(char *)realloc(*pParam, nJasonLen);
		}
		else
		{
			*pParam=(ACS_CHAR *)malloc(nJasonLen);
		}
		
		if(*pParam==NULL)
		{
			ACS_WLOG("*pParam is NULL err");
			return -1;
		}
	}
	else
	{
		ACS_WLOG("bufszie is %d err",nJasonLen);
		return -1;
	}

	pFile = fopen(pFilePath, "rb");
	if(pFile==NULL)
	{
		free(*pParam);
		ACS_WLOG("fopen %s err",pFilePath);
		return -1;
	}

	
	memset(*pParam, 0, nJasonLen);
	ret = fread(*pParam, nJasonLen, 1, pFile);
	if(ret!=1)
	{
		fclose(pFile);
		pFile=NULL;
		free(*pParam);
		ACS_WLOG("read %s err",pFilePath);
		return -1;
	}

	fclose(pFile);
	pFile=NULL;
	return nJasonLen;
}
static ACS_INT AcsCom_Param_GetJsonValue(cJSON *pObjectJson,const ACS_CHAR*pKey,ACS_VOID *pDst,ACS_DWORD nMaxStrLen,ACS_INT DataType)
{
	if(pObjectJson==NULL || pKey==NULL || pDst==NULL)
	{
		return -1; 
	}

	cJSON *pItem = cJSON_GetObjectItem(pObjectJson, pKey);
	if(pItem!=NULL)
	{
		switch(DataType)
		{
			case ACS_JSON_DATA_CHAR:
				*((char *)pDst) = pItem->valueint;
				return 0;
			
			case ACS_JSON_DATA_SHORT:
				*((short *)pDst) = pItem->valueint;
				return 0;

			case ACS_JSON_DATA_DOUBLE:
				*((double *)pDst) = pItem->valuedouble;
				return 0;

			case ACS_JSON_DATA_INT:
				*((int *)pDst) = pItem->valueint;
				return 0;
			
			case ACS_JSON_DATA_FLOAT:
				*((float *)pDst) = pItem->valuedouble;
				return 0;
			
			case ACS_JSON_DATA_USHORT:
				*((unsigned short *)pDst) = (unsigned short)pItem->valueint;
				return 0;
			
			case ACS_JSON_DATA_LONG:
				*((long *)pDst) = (long)pItem->valuedouble;
				return 0;
			
			case ACS_JSON_DATA_ULONG:
				*((unsigned long *)pDst) = (unsigned long)pItem->valuedouble;
				return 0;
			
			case ACS_JSON_DATA_STRING:
			{
				if(pItem->valuestring != NULL)
				{
					if(strlen(pItem->valuestring) < nMaxStrLen)
					{
						sprintf((ACS_CHAR *)pDst, "%s", pItem->valuestring); 
					}
					else
					{
						memcpy(pDst, pItem->valuestring, nMaxStrLen);
					}
					
					return 0;
				}
				else
				{
					return -1;
				}
			
			}
		}
	}

	return -1;
}
static ACS_INT AcsCom_Param_GetSyncTime(ACS_SYNC_MARK_S *pstTimeInfo,const ACS_CHAR *pParamfile)
{
	ACS_INT ret = ACS_ERR_SUCCESS;
	ACS_CHAR *pJsonBuf = NULL;
	if(NULL == pstTimeInfo)
	{
		return ACS_ERR_PARAMNULL;
	}
	
	pthread_mutex_lock(&gSysParamMutex_AcscloudInfo);

	if(AcsCom_Param_ReadJason(pParamfile,&pJsonBuf) > 0)
	{
		cJSON *pJsonRoot = cJSON_Parse(pJsonBuf);
		if(pJsonRoot)
		{
			cJSON *pAtteInfo = cJSON_GetObjectItem(pJsonRoot,"syncTime");
			if(pAtteInfo)
			{
				AcsCom_Param_GetJsonValue(pAtteInfo, "face",  	&pstTimeInfo->stTime.face,0,ACS_JSON_DATA_INT);
				AcsCom_Param_GetJsonValue(pAtteInfo, "card", 	&pstTimeInfo->stTime.card,0,ACS_JSON_DATA_INT);
				AcsCom_Param_GetJsonValue(pAtteInfo, "ad", 		&pstTimeInfo->stTime.ad,0,ACS_JSON_DATA_INT);
				AcsCom_Param_GetJsonValue(pAtteInfo, "reissue", &pstTimeInfo->stTime.reissue,0,ACS_JSON_DATA_INT);
			}

			cJSON *pField = cJSON_GetObjectItem(pJsonRoot,"syncField");
			if(pField)
			{
				AcsCom_Param_GetJsonValue(pField, "face",  	&pstTimeInfo->stField.face,NCHAR_A128,ACS_JSON_DATA_STRING);
				AcsCom_Param_GetJsonValue(pField, "card", 	&pstTimeInfo->stField.card,NCHAR_A128,ACS_JSON_DATA_STRING);
				AcsCom_Param_GetJsonValue(pField, "ad", 		&pstTimeInfo->stField.ad,NCHAR_A128,ACS_JSON_DATA_STRING);
				AcsCom_Param_GetJsonValue(pField, "reissue", &pstTimeInfo->stField.reissue,NCHAR_A128,ACS_JSON_DATA_STRING);
			}

			cJSON *plogLevel = cJSON_GetObjectItem(pJsonRoot,"logLevel");
			if(plogLevel)
			{
				AcsCom_Param_GetJsonValue(plogLevel, "level",  	&pstTimeInfo->logLevel,0,ACS_JSON_DATA_CHAR);
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
	pthread_mutex_unlock(&gSysParamMutex_AcscloudInfo);
	return  ret;
}

static ACS_INT AcsCom_Param_SetSyncTime(ACS_SYNC_MARK_S *pstTimeInfo,const ACS_CHAR *pParamfile)
{
	ACS_INT ret = ACS_ERR_FAIL;
	cJSON* pJsonRoot  = NULL;
	cJSON* pAtteInfo = NULL;
	cJSON* pstField = NULL;
	cJSON* pLogLevel = NULL;
	ACS_CHAR*  pJsonBuf=NULL;
	pthread_mutex_lock(&gSysParamMutex_AcscloudInfo);
	pJsonRoot  = cJSON_CreateObject();
	if(pJsonRoot)
	{
		pAtteInfo = cJSON_CreateObject();
		if(pAtteInfo)
		{
			cJSON_AddNumberToObject(pAtteInfo, "face",	 pstTimeInfo->stTime.face);
			cJSON_AddNumberToObject(pAtteInfo, "card",	 pstTimeInfo->stTime.card);
			cJSON_AddNumberToObject(pAtteInfo, "ad",	 pstTimeInfo->stTime.ad);
			cJSON_AddNumberToObject(pAtteInfo, "reissue",	 pstTimeInfo->stTime.reissue);
			cJSON_AddNumberToObject(pAtteInfo, "erules",pstTimeInfo->stTime.erules);
			cJSON_AddNumberToObject(pAtteInfo, "pwd",	 pstTimeInfo->stTime.pwd);
			cJSON_AddItemToObject(pJsonRoot, "syncTime", pAtteInfo);
		}

		pstField = cJSON_CreateObject();
		if(pstField)
		{
			cJSON_AddStringToObject(pstField, "face",	 pstTimeInfo->stField.face);
			cJSON_AddStringToObject(pstField, "card",	 pstTimeInfo->stField.card);
			cJSON_AddStringToObject(pstField, "ad",	 	 pstTimeInfo->stField.ad);
			cJSON_AddStringToObject(pstField, "reissue", pstTimeInfo->stField.reissue);
			cJSON_AddStringToObject(pstField, "erules", pstTimeInfo->stField.erules);
			cJSON_AddStringToObject(pstField, "pwd", pstTimeInfo->stField.pwd);
			cJSON_AddItemToObject(pJsonRoot, "syncField",pstField);
		}
		
		pLogLevel = cJSON_CreateObject();
		if(pLogLevel)
		{
			cJSON_AddNumberToObject(pLogLevel, "level",	 	pstTimeInfo->logLevel);
			cJSON_AddStringToObject(pLogLevel, "level-tik", "eg:debug:10;info:20;error:30");
			cJSON_AddItemToObject(pJsonRoot, "logLevel",pLogLevel);
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
	pthread_mutex_unlock(&gSysParamMutex_AcscloudInfo);
	return ret; 
}
#endif




ACS_BYTE ACSCom_GetPrintfLevel(ACS_BYTE logLevel)
{
	if(logLevel >= g_stSyncTime.logLevel)//10/20/30 < 10 ACS_DEBUG
		return ACS_ITURN;
	return ACS_FALSE;
}

ACS_BYTE ACSCom_SetPrintfLevel(ACS_VOID)
{
	ACS_CHAR szParam[128] = {0};
	ACS_SYNC_MARK_S stSysctime = {0};
	snprintf(szParam,sizeof(szParam),"%s%s",g_szAcsPath,ACSPARAMFILE);
	AcsCom_Param_GetSyncTime(&stSysctime,szParam);
	g_stSyncTime.logLevel = stSysctime.logLevel;
	return 0;
}


ACS_BYTE ACSCOM_GetLogLevel(ACS_VOID)
{
	if(access(ACS_LOGPATH,F_OK) == 0)
	{
		return 1;
	}	
	else
	{
		return 0;
	}
	return 0;
}


/**
* fun:打印同步的人脸信息
* @pszFaceInfo-in:平台下发的执行指令
* @return:ACS_VOID;
**/
ACS_VOID ACSCOM_PrintfPicDatac(ACS_FACE_INFO_S *pszFaceInfo)
{
	printf("zzzzzzzzzzzzzzz\n");
	printf("begin userId    :%s\n",pszFaceInfo->userId);
	printf("userName  :%s\n",pszFaceInfo->userName);
	printf("userIdCard:%s\n",pszFaceInfo->userIdCard);
	printf("userType  :%s\n",pszFaceInfo->userType);
	printf("url       :%s\n",pszFaceInfo->url);
	printf("end operation :%s\n",pszFaceInfo->operation);
	return;
}





/*******************************************************公共的********************************************************/
ACS_INT ACS_WriteLOG(const ACS_CHAR *pfunc,ACS_INT nline,const ACS_CHAR *format, ...)
{
	if(ACSCOM_GetLogLevel() < 1)
		return ACS_ERR_SUCCESS;
		
	va_list arg;
    ACS_INT len = 0;
    ACS_INT file = 0;

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;

   // ACS_CHAR szPath[NCHAR_A128] = {0};
   // sprintf (szPath,"%s",ACS_LOGPATH);

    ACS_INT m ;
    ACS_INT mode = O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND | O_NONBLOCK;
    m = mode | O_NONBLOCK;
    file = open(ACS_LOGPATH, m, 0600);
    if (file == 0)
		return ACS_ERR_FAIL;

    fl.l_type = F_WRLCK;
    fcntl(file, F_SETLKW, &fl);

    len  = lseek(file, 0, SEEK_END);
    if (len > 2*1024*1024)
    {
		ftruncate(file,0);
		lseek(file,0,SEEK_SET);
    }

    ACS_CHAR szBuff[NCHAR_A512] = {0};
    ACS_CHAR szMsg[NCHAR_A512] = {0};

    va_start(arg, format);
    vsnprintf(szMsg, 450, format, arg);
    va_end (arg);

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    sprintf(szBuff,"%s[%s:%d]%02d-%02d %02d:%02d:%02d\n",szMsg,pfunc,nline,tm_log->tm_mon + 1,tm_log->tm_mday,tm_log->tm_hour,tm_log->tm_min,tm_log->tm_sec);
	strcat(szBuff, "</br>");
    len = strlen(szBuff);
    write(file, szBuff, len);

    fl.l_type = F_UNLCK;
    fcntl(file, F_SETLKW, &fl);

    close(file);
    return ACS_ERR_SUCCESS;
}



/**********设备操作码***********平台操作码        ***平台操作消息***/
DBOPRA_MAP_ACS  DbopraresultACS[] =
{
	{ACS_ERR_SUCCESS,				0,		"操作成功"},
	{ACS_ERR_FAIL,					1000,	"操作失败"},
	{ACS_ERR_PICFACE_COLLECTERR,	1001,	"提取特征值失败"},
	{ACS_ERR_FILEREPEAT,			1002,	"文件名字编号重复"},
	{ACS_ERR_DBFULL,				1003,	"库满"},
	{ACS_ERR_TIMEOUT,				1004,	"添加超时"},
	{ACS_ERR_PARAMERR,				1005,	"参数错误"},
	{ACS_ERR_FILEBIG,				1006,	"文件太大"},
//	{ACS_ERR_SPACEERR,				1007,	"存储空间不足"},
	{ACS_ERR_FILE_OPENERR,			1008,	"文件打开失败"},
	{ACS_ERR_MALLOCFAIL,			1009,	"内存不足"},
	{ACS_ERR_FILE_READERR,			1010,	"文件读取失败"},
	{ACS_ERR_DB_READERR,			1011,	"数据库损坏"},
	{ACS_ERR_PIC_QUALITYERR,		1012,	"图片质量差"},
	{ACS_ERR_PIC_SIZEERR,			1013,	"图片尺寸错误"},
	{ACS_ERR_PIC_NOFACE,			1014,	"检测人脸失败"},
	{ACS_ERR_PIC_FORMATERR,			1015,	"图片格式错误"},
	{ACS_ERR_PICFACE_AREAERR,		1016,	"人脸区域错误"},
	{ACS_ERR_FILE_WRITEERR,			1017,	"文件写入失败"},
	{ACS_ERR_DEVBUSY,				1018,	"设备忙"},
	{ACS_ERR_FILE_DOWNERR,			1019,	"下载文件出错"},
	{ACS_ERR_FILENOT,				1020,	"文件不存在"},
	{ACS_ERR_DB_NOTID,				1021,	"数据库没有ID"}, 
	{ACS_ERR_CARDREPEAT,			1022,	"IC卡号重复"}, 
	{ACS_ERR_IDCARDREPEAT,			1023,	"身份证号重复"},
	{ACS_ERR_IDREPEAT,				1024,	"ID重复"},
	{ACS_ERR_CHECKFAIL,				1025,	"校验失败"},
	{ACS_ERR_NOTCERTIFICATION,		1025,	"无权限"},
	{ACS_ERR_PWDREPEAT,				1026,	"密码重复"}, 
	{ACS_ERR_DEVNOTREGISTER,		1200,	"平台未登记此设备"},//务器错误码
	{ACS_ERR_CURLFIAL,				2010,	"数据请求失败"},//服务器错误码
	{ACS_ERR_DATANULL,				2010,	"数据请求失败"},//服务器错误码
	{ACS_ERR_DATAERR,				2011,	"数据错误"},//服务器错误码
	{ACS_ERR_DATAMAX,				2011,	"数据太长"},//服务器错误码
	{ACS_ERR_NODATA,				2012,	"空数据"},//服务器错误码
	{ACS_ERR_PARAMNULL,				1000,	"NOTTHEERR"},
};
	
/**********设备操作码***********平台操作码		***平台操作消息***/

DBOPRA_MAP_ACS *FindResultACS(ACS_INT eMDorerror)//DORERR_CODE
{
    for(size_t i=0;i<sizeof(DbopraresultACS)/sizeof(DBOPRA_MAP_ACS);i++)
    {
		if(DbopraresultACS[i].eMDorerror == eMDorerror)
		{
		    return &DbopraresultACS[i];
		}
    }
    return NULL;
}

ACS_INT ACSCOM_GetClouldEnable(ACS_VOID)
{
	return g_stDoor_Info.stClouldInfo.nEnable;
}

/**
* fun:由设备错误码转为平台错误码
* @pnError-in:操作错误码
* @pszMsg-in:操作错误消息 如为NULL返回pnError对应的错误消息
* @return:success-平台错误码;失败返回 -1;
**/
ACS_INT ACSCOM_OperateResult(ACS_INT *pnError,ACS_CHAR *pszMsg)
{
	if(pnError)
	{
		DBOPRA_MAP_ACS *perrMsg = FindResultACS(*pnError);
		if(perrMsg)
		{	
			*pnError = perrMsg->nCode;
			if((pszMsg) && (strlen(pszMsg)<1))
				snprintf(pszMsg,sizeof(perrMsg->szMsg),"%s",perrMsg->szMsg);
			return perrMsg->nCode;
		}
	}
	return -1;
}

/*******************************************************公共的********************************************************/

ACS_INT ACSCOM_EnDataQueue(ACS_CLEARDATA_E e_cmd,const ACS_CHAR *pMsgId,const ACS_ULONG utimestamp)
{
	ACS_CMD_S stcmdInfo;
	stcmdInfo.e_cmd = e_cmd;
	stcmdInfo.utimestamp = utimestamp;
	snprintf(stcmdInfo.szMsgID,33,pMsgId);
	return Acs_EnDataQueue(g_pAcsCmdDbqueue,(ACS_CHAR *)&stcmdInfo,sizeof(ACS_CMD_S));
}

ACS_INT ACSCOM_DeDataQueue(ACS_CMD_S *pstCmdInfo)
{
	return Acs_DeDataQueue(g_pAcsCmdDbqueue,(ACS_CHAR *)pstCmdInfo,sizeof(ACS_CMD_S));
}

ACS_INT ACSCOM_DeAllDataQueue(ACS_VOID)
{
	return Acs_DeAllDataQueue(g_pAcsCmdDbqueue);
}

ACS_VOID ACSCOM_DestroyDataSequeue(ACS_VOID)
{
	Acs_DestroyDataSequeue(g_pAcsCmdDbqueue);
	return;
}



/**
* fun:系统调用
* @cmdstring-in:命令
* @return:success-0 ACS_SUCCESS;fail:-1 ACS_FAILED;
**/
//extern ACS_INT errno;
ACS_INT ACSCOM_COMMON_System(const ACS_CHAR * cmdstring)
{
	int status, ret = 0;
	pid_t pid;
	pid = vfork();
	if (pid < 0)
	{
		PRT_PRINT(ACS_ERROR,"vfork failed");
		return ACS_FAILED;
	}
	else if (pid == 0) 
	{
		ret = execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
		if (ret < 0)
		{
			PRT_PRINT(ACS_ERROR,"execl failed, errno = %d\n",ret);
			return ACS_FAILED;
		}
		return ret;
	} 
	else 
	{
		ret = waitpid(pid, &status, 0);
		if (ret != pid)
		{
			PRT_PRINT(ACS_ERROR,"waitpid failed, ret = %d", ret);
			return ACS_FAILED;
		}

		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status))
			{
				PRT_PRINT(ACS_ERROR,"exit status = %d", WEXITSTATUS(status));
				return ACS_FAILED;
			}
		}
		else 
		{
			PRT_PRINT(ACS_ERROR,"abort exceptly");
			return ACS_FAILED;
		}
	}
	return status;
}


/**
* fun:申请Malloc内存
* @return:ACS_ERR_E;
**/
ACS_INT  ACSCOM_Malloc(ACS_VOID)
{
	g_pAcsCmdDbqueue = Acs_CreateEmptyDataSequeue(sizeof(ACS_CMD_S),ACSSYNCCMDNUM+1); 
	if(g_pAcsCmdDbqueue == NULL)
	{
		PRT_PRINT(ACS_ERROR,"AdData CreateEmpty Sequeue malloc error");
		ACS_WLOG("acs AdData CreateEmpty Sequeue malloc error !!!");
		return ACS_ERR_MALLOCFAIL;
	}
	return ACS_ERR_SUCCESS;
}


/**
* fun:获取人脸的malloc地址
* @return:success-返回地址char *;fail-NULL
**/
ACS_CHAR *ACSCOM_MallocFaceAddress(ACS_VOID)
{
	g_pSyncFaceData = (ACS_FACE_INFO_S *)malloc(sizeof(ACS_FACE_INFO_S)*ACSSYNCNUM);
	if(g_pSyncFaceData == NULL)
	{
		ACS_WLOG("g_pSyncFaceData malloc %d error",sizeof(ACS_FACE_INFO_S)*ACSSYNCNUM);
		PRT_PRINT(ACS_ERROR,"g_pSyncFaceData malloc %d error",sizeof(ACS_FACE_INFO_S)*ACSSYNCNUM);
		return NULL;
	}
	memset(g_pSyncFaceData,0,(sizeof(ACS_FACE_INFO_S)*ACSSYNCNUM));
	//PRT_PRINT(ACS_DEBUG,"g_pSyncFaceData:%p",g_pSyncFaceData);
	return (ACS_CHAR *)g_pSyncFaceData;
}

/**
* fun:清空人脸的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_CleanFaceAddress(ACS_VOID)
{
	memset(g_pSyncFaceData,0,(sizeof(ACS_FACE_INFO_S)*ACSSYNCNUM));
}

/**
* fun:释放人脸的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_FreeFaceAddress(ACS_VOID)
{
	if(NULL != g_pSyncFaceData)
	{
		free(g_pSyncFaceData);
		g_pSyncFaceData = NULL;
	}
}


/**
* fun:获取补发考勤的malloc地址
* @return:success-返回地址char *;fail-NULL
**/
ACS_CHAR *ACSCOM_MallocReissueAddress(ACS_VOID)
{
	g_pReissueData = (ACS_REISSUE_RECORD_S *)malloc(sizeof(ACS_REISSUE_RECORD_S)*REISSUENUM);
	if(g_pReissueData == NULL)
	{
		ACS_WLOG("g_pReissueData malloc %d error",sizeof(ACS_REISSUE_RECORD_S)*REISSUENUM);
		PRT_PRINT(ACS_ERROR,"g_pReissueData malloc %d error",sizeof(ACS_REISSUE_RECORD_S)*REISSUENUM);
		return NULL;
	}
	memset(g_pReissueData,0,(sizeof(ACS_REISSUE_RECORD_S)*REISSUENUM));
	PRT_PRINT(ACS_INFO,"g_pReissueData:%p",g_pReissueData);
	return (ACS_CHAR *)g_pReissueData;
}

/**
* fun:清空考勤补发的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_CleanReissueAddress(ACS_VOID)
{
	memset(g_pReissueData,0,(sizeof(ACS_REISSUE_RECORD_S)*REISSUENUM));
}

/**
* fun:释放考勤补发的malloc内存
* @return:void
**/
ACS_VOID ACSCOM_FreeReissueAddress(ACS_VOID)
{
	if(NULL != g_pReissueData)
	{
		free(g_pReissueData);
		g_pReissueData = NULL;
	}
}



/**
* fun:通知设备处理数据
* @eCmd-in:操作命令 ACS_CMD_E
* @pstSyncDb-in:待设备操作的数据
* @return:ACS_VOID;
**/
ACS_INT ACSCOM_HandleCmdCallback(ACS_CMD_E eCmd,ACS_CHAR *pstSyncDb)
{
	if(g_Acs_CmdHandle && pstSyncDb)
	{
		return g_Acs_CmdHandle(eCmd,pstSyncDb);
	}
	else
	{
		PRT_PRINT(ACS_ERROR,"g_Acs_CmdHandle point NULL to %d!!!",eCmd);
		ACS_WLOG("g_Acs_CmdHandle point NULL to %d!!!",eCmd);
	}
	return -1;
}



ACS_CHAR* ACSCOM_GetAcsPath(ACS_VOID)
{
	return g_szAcsPath;
}


ACS_CHAR* ACSCOM_GetUploadPath(ACS_VOID)
{
	return g_szAcsUPackPath;
}


/**
* fun:设置门禁云平台操作路径
* @pPath-in:路径 /database/acscloud
* @pUPackPath-in:升级包路径
* @return:ACS_ERR_E;
**/
ACS_INT  ACSCOM_DirPath(const ACS_CHAR *pPath,const ACS_CHAR *pUPackPath)
{
	//PRT_PRINT("pTimePath:%s;pUPackPath:%s",pPath,pUPackPath);
	ACS_INT ret = 0;
	ACS_CHAR szMsg[128] = {0};
	ACS_CHAR szCmd[128] = {0};
	
	if((NULL == pPath) || (strlen(pPath) < 1))
	{
		PRT_PRINT(ACS_ERROR,"pFilePath:%s error!",pPath);
		return ACS_ERR_PARAMERR;
	}
	else if(ACS_SUCCESS != access(pPath,F_OK))
	{
		sprintf(szCmd,"mkdir -p %s",pPath);
		PRT_PRINT(ACS_INFO,"szCmd:%s",szCmd);
		ACSCOM_COMMON_System(szCmd);
	}
	
	usleep(100);
	ACSCOM_COMMON_System("sync");
	
	if(ACS_SUCCESS != access(pPath,F_OK))
	{
		PRT_PRINT(ACS_ERROR,"%s not access!",pPath);
		return ACS_ERR_FAIL;
	}
	

	memset(g_szAcsUPackPath,0,sizeof(g_szAcsUPackPath));
	if(pUPackPath && (strlen(pUPackPath) > 0) && (access(pUPackPath,F_OK) == ACS_SUCCESS))
	{
		snprintf(g_szAcsUPackPath,sizeof(g_szAcsUPackPath),"%s",pUPackPath);
	}
	else
	{
		PRT_PRINT(ACS_ERROR,"pUPackPath:%s error",pUPackPath);
		return ACS_ERR_PARAMERR;
	}

	memset(szMsg,0,sizeof(szMsg));
	sprintf(szMsg,"%s%s",pPath,ACSFACEPICFILE);
	if(ACS_SUCCESS != access(szMsg,F_OK))
	{
		memset(szCmd,0,sizeof(szCmd));
		sprintf(szCmd,"touch %s",szMsg);
		PRT_PRINT(ACS_INFO,"szCmd:%s",szCmd);
		ACSCOM_COMMON_System(szCmd);
	}

	memset(szMsg,0,sizeof(szMsg));
	sprintf(szMsg,"%s%s",pPath,ACSADPICFILE);
	if(ACS_SUCCESS != access(szMsg,F_OK))
	{
		memset(szCmd,0,sizeof(szCmd));
		sprintf(szCmd,"touch %s",szMsg);
		PRT_PRINT(ACS_INFO,"szCmd:%s",szCmd);
		ACSCOM_COMMON_System(szCmd);
	}

	memset(szMsg,0,sizeof(szMsg));
	snprintf(szMsg,sizeof(szMsg),"%s%s",pPath,ACSPARAMFILE);
	if(ACS_SUCCESS != access(szMsg,F_OK))
	{
		memset(szCmd,0,sizeof(szCmd));
		sprintf(szCmd,"touch %s",szMsg);
		PRT_PRINT(ACS_INFO,"szCmd:%s",szCmd);
		ACSCOM_COMMON_System(szCmd);
	}
	
	usleep(100);
	ACSCOM_COMMON_System("sync");

	ret = ACSCOM_Init_SyncTimeCfg(szMsg);
	PRT_PRINT(ACS_INFO,"acsfile path:%s",pPath);
	PRT_PRINT(ACS_INFO,"param filepath:%s",szMsg);
	
	if(ret < 0)
	{
		ACS_WLOG("ACSCOM_Init_SyncTimeCfg param.dat fail");
		PRT_PRINT(ACS_ERROR,"ACSCOM_Init_SyncTimeCfg param.dat fail");
	}

	memset(g_szAcsPath,0,sizeof(g_szAcsPath));
	snprintf(g_szAcsPath,sizeof(g_szAcsPath),"%s",pPath);
	
	
	
	return ACS_ERR_SUCCESS;
}

/**
* fun:通知函数
* @pCallbackFunc-in:回调处理函数
* @return:ACS_VOID;
**/
ACS_VOID ACSCOM_SetCallback_HandleCmd(ACS_HandleCmd_Callback pCallbackFunc)
{
	g_Acs_CmdHandle = pCallbackFunc;
}


/**
* fun:保存同步时间 以文件形式保存到设备
* @pPath-in:存储路径
* @stConfig-in:保存的同步时间
* @return:ACS_ERR_SUCCESS-success;ACS_ERR_FAIL-fail
**/
ACS_INT ACSCOM_Save_SyncTimeCfg(const ACS_CHAR *pPath, ACS_SYNC_MARK_S *stConfig)
{
	ACS_INT ret = ACS_ERR_FAIL;
	ret = AcsCom_Param_SetSyncTime(stConfig,pPath);
	return ret;
}


/**
* fun:初始化人脸同步信息
* @pPath:输入时间同步的文件路径
* @return:ACS_ERR_E
**/
ACS_INT ACSCOM_Init_SyncTimeCfg(const ACS_CHAR *pPath)
{
	pthread_mutex_init(&gSysParamMutex_AcscloudInfo, NULL);
	AcsCom_Param_GetSyncTime(&g_stSyncTime,pPath);
    return ACS_ERR_SUCCESS;
}

/**
* fun:清理由平台入库的数据
* @return:ACS_VOID
**/
ACS_VOID ACSCOM_CleanData(ACS_CHAR *pOperation)
{
	ACS_SYNC_MARK_S stSysctime;//重置同步时间
	ACSCOM_Get_SyncTimeCfg(&stSysctime);
	ACS_BYTE byLog = stSysctime.logLevel;
	memset(&stSysctime,0,sizeof(ACS_SYNC_MARK_S));
	stSysctime.logLevel = byLog;
	ACSCOM_Set_SyncTimeCfg(&stSysctime);
	ACSCOM_HandleCmdCallback(ACS_CMD_SET_CLEANDATA,(ACS_CHAR *)CMD_MQTTSERVER_SYCCLEANDA);//通知设备清理人脸卡号
	return;
}


/**
* fun:重新启动门禁云平台 重新登入门禁云平台 同步平台数据用户门卡广告
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_VOID;
ACS_VOID ACSCOM_RestartLogin(const ACS_CHAR *pMsgId,const ACS_ULONG ltimestamp)
{
	//ACSCOM_Set_ServiceState(ACS_SERVER_INIT);
	//重新登录

	g_szUrl
	#if 0
	ACS_INT ret = ACSHTTP_DeviceLogin(pMsgId);
	if(ret == 0)
	{
		ACSCOM_Set_ServiceState(ACS_SERVER_OK);
		ACS_WLOG("RestartApp success !!!");
		
		Mqtt_Set_DataDealFlag(ACS_CD_FACE,1);//同步用户
		Mqtt_Set_DataDealFlag(ACS_CD_CARD,1);//同步门卡		
		Mqtt_Set_DataDealFlag(ACS_CD_PWD,1);  //同步密码
		Mqtt_Set_DataDealFlag(ACS_CD_AD,1);  //同步广告

		ACSCOM_EnDataQueue(ACS_CD_FACE,pMsgId,ltimestamp);
		ACSCOM_EnDataQueue(ACS_CD_CARD,pMsgId,ltimestamp);
		ACSCOM_EnDataQueue(ACS_CD_PWD,pMsgId,ltimestamp);
		ACSCOM_EnDataQueue(ACS_CD_AD,pMsgId,ltimestamp);
		ACSCOM_EnDataQueue(ACS_CD_PARAM,pMsgId,ltimestamp);
	}
	else
	{
		ACS_WLOG("ACSHTTP_DeviceLogin ret:%d failed!!!",ret);
		ACSCOM_Set_ServiceState(ACS_SERVER_FAIL);
	}
	#endif

	return;
}
**/



/**
* fun:是否在同步数据 1-是 0-否
* @nCmd-in:输入 nCmd 
* @bData-in:输入 bData 1/0 
* @return:ACS_ERR_E
**/
ACS_VOID  ACSCOM_Set_SyncDataFlag(ACS_SYNCDATA_E nCmd,ACS_BYTE bData)
{
	if(bData)
		ACSBIT_SET(g_syncData,nCmd);
	else
		ACSBIT_CLR(g_syncData,nCmd);
	return;
}


/**
* fun:获取是否在同步数据 1-是 0-否
* @nCmd-in:输入 nCmd 
* @return:ACS_ERR_E
**/
ACS_DWORD  ACSCOM_Get_SyncDataFlag(ACS_SYNCDATA_E nCmd)
{
	if(nCmd)
		return ACSBIT_GET(g_syncData,nCmd);
	else
	{
		PRT_PRINT(ACS_DEBUG,"SyncData nCmd:%02x;%d",nCmd,nCmd);
		return g_syncData;
	}
}




/**
* fun:获取mqtt的登入信息
* @pstSynctime-out:输出 ACS_MQTT_CONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_MQTTCfg(ACS_MQTT_CONFIG_S *pstMqttConfig)
{
	if(pstMqttConfig == NULL)
		return ACS_ERR_PARAMNULL;
	memcpy(pstMqttConfig, &g_stDoor_Info.stMqttConfig, sizeof(ACS_MQTT_CONFIG_S));
	return ACS_ERR_SUCCESS;
}


/**
* fun:获取同步数据的时间
* @pstSynctime-out:输出 ACS_SYNCTIME_CONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_SyncTimeCfg(ACS_SYNC_MARK_S *pstSynctime)
{
	if(pstSynctime == NULL)
		return ACS_ERR_PARAMNULL;
	memcpy(pstSynctime, &g_stSyncTime, sizeof(ACS_SYNC_MARK_S));
	return ACS_ERR_SUCCESS;
}


/**
* fun:设置同步数据的时间
* @pstSynctime-in:输入 ACS_SYNCTIME_CONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Set_SyncTimeCfg(ACS_SYNC_MARK_S        *pstSynctime)
{
	ACS_CHAR szMsg[128] = {0};
	if(pstSynctime == NULL)
		return ACS_ERR_PARAMNULL;
	if(memcmp(&g_stSyncTime, pstSynctime, sizeof(ACS_SYNC_MARK_S)) != 0)
	{
		memcpy(&g_stSyncTime, pstSynctime, sizeof(ACS_SYNC_MARK_S));
		snprintf(szMsg,sizeof(szMsg),"%s%s",g_szAcsPath,ACSPARAMFILE);
		ACSCOM_Save_SyncTimeCfg(szMsg, &g_stSyncTime);
		//PRT_PRINT("g_szAcsTimePathName:%s;",g_szAcsTimePathName);
	}
	return ACS_ERR_SUCCESS;
}


/**
* fun:获取提示数据库数量的标志位 0-不推送 1-推送 默认1
* @ACS_VOID-in:
* @return:0-不推送 1-推送
**/
ACS_VOID ACSCOM_Set_ifDataNum(ACS_BYTE bIfPush)
{
	g_bifSetDataNum = bIfPush;
	return;
}

/**
* fun:获取提示数据库数量的标志位 0-不推送 1-推送
* @ACS_VOID-in:
* @return:0-不推送 1-推送
**/
ACS_BYTE ACSCOM_Get_ifDataNum(ACS_VOID)
{
	//PRT_PRINT(ACS_DEBUG,"ACSCOM_Get_ifDataNum:%d;",g_bifPushDataNum);
	return g_bifSetDataNum;
}

/**
* fun:获取所有门禁云平台参数信息
* @pstMjapiconfig-out:输出 ACS_CONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_Config(ACS_CONFIG_S *pstMjapiconfig)
{
	if(pstMjapiconfig == NULL)
		return ACS_ERR_PARAMNULL;
	memcpy(pstMjapiconfig, &g_stDoor_Info, sizeof(ACS_CONFIG_S));
	return ACS_ERR_SUCCESS;
}

/**
* fun:获取设备信息
* @pstDevInfo-in:输入 ACS_DEVCONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_GetDevInfo_Config(ACS_DEVCONFIG_S *pstDevInfo)
{
	if(pstDevInfo == NULL)
		  return ACS_ERR_PARAMNULL;
	memcpy(pstDevInfo, &g_stDoor_Info.stDevInfo, sizeof(ACS_DEVCONFIG_S));
	return ACS_ERR_SUCCESS;
}

/**
* fun:设置设备信息
* @pstDevInfo-in:输入 ACS_DEVCONFIG_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_SetDevInfo_Config(ACS_DEVCONFIG_S *pstDevInfo)
{
	if(pstDevInfo == NULL)
		return ACS_ERR_PARAMNULL;
	if(memcmp(pstDevInfo, &g_stDoor_Info.stDevInfo, sizeof(ACS_DEVCONFIG_S)) != 0)
	{
		memcpy(&g_stDoor_Info.stDevInfo, pstDevInfo, sizeof(ACS_DEVCONFIG_S));
	}
	return ACS_ERR_SUCCESS;
}

/**
* fun:获取设备数据数量
* @pstDevInfo-in:输入 ACS_DATANUM_S *
* @return:ACS_ERR_E
**/
ACS_VOID  ACSCOM_GetDataNum_Config(ACS_DATANUM_S *pstDataNum)
{
	memcpy(pstDataNum, &g_stDataNum, sizeof(ACS_DATANUM_S));
	return;
}

/**
* fun:清空设备数据数量
* @pstDevInfo-in:输入 ACS_DATANUM_S *
* @return:ACS_ERR_E
**/
ACS_VOID  ACSCOM_ClearDataNum_Config(ACS_VOID)
{
	memset(&g_stDataNum,0, sizeof(ACS_DATANUM_S));
	return;
}



/**
* fun:设置设备数据数量
* @pstDevInfo-in:输入 ACS_DATANUM_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_SetDataNum_Config(ACS_DATANUM_S *pstDataNum)
{
	if(memcmp(pstDataNum, &g_stDataNum, sizeof(ACS_DATANUM_S)) != 0)
	{
		memcpy(&g_stDataNum, pstDataNum, sizeof(ACS_DATANUM_S));
		ACS_BYTE bFlag = 0;
		ACS_SYNC_MARK_S stSysctime;
		ACSCOM_Get_SyncTimeCfg(&stSysctime);
		if(!pstDataNum->person_count)
		{
			memset(stSysctime.stField.face,0,sizeof(stSysctime.stField.face));
			stSysctime.stTime.face = 0;
			bFlag ++;
		}
		if(!pstDataNum->card_cont)
		{
			memset(stSysctime.stField.card,0,sizeof(stSysctime.stField.card));
			stSysctime.stTime.card = 0;
			bFlag ++;
		}
		if(!pstDataNum->pwd_cont)
		{
			memset(stSysctime.stField.pwd,0,sizeof(stSysctime.stField.pwd));
			stSysctime.stTime.pwd = 0;
			bFlag ++;
		}
		if(!pstDataNum->ad_cont)
		{
			memset(stSysctime.stField.ad,0,sizeof(stSysctime.stField.ad));
			stSysctime.stTime.ad = 0;
			bFlag ++;
		}		
		if(bFlag > 3)
		{
			stSysctime.logLevel = ACS_ERROR;
		}
		
		ACSCOM_Set_SyncTimeCfg(&stSysctime);
		
		ACSHTTP_Set_DataNumIfPush(ACS_ITURN);
		
		return ACS_ERR_SUCCESS;
	}
	return ACS_ERR_FAIL;
}
 	





/**
* fun:设置门禁云平台登入信息
* @ACS_CLOUDLOGIN_S-in:输入 ACS_CLOUDLOGIN_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Set_DoorClouldConfig(ACS_CLOUDLOGIN_S *pstClouldInfo)
{
	if(pstClouldInfo == NULL)
		return ACS_ERR_PARAMNULL;
	if(memcmp(pstClouldInfo, &g_stDoor_Info.stClouldInfo, sizeof(ACS_CLOUDLOGIN_S)) != 0)
	{
		memcpy(&g_stDoor_Info.stClouldInfo, pstClouldInfo, sizeof(ACS_CLOUDLOGIN_S));
	}
	return ACS_ERR_SUCCESS;
}

/**
* fun:获取门禁云平台登入信息
* @ACS_CLOUDLOGIN_S-out:输出 ACS_CLOUDLOGIN_S *
* @return:ACS_ERR_E
**/
ACS_INT  ACSCOM_Get_DoorClouldConfig(ACS_CLOUDLOGIN_S *pstCloudCfg)
{
	if(pstCloudCfg == NULL)
		return ACS_ERR_PARAMNULL;
	memcpy(pstCloudCfg, &g_stDoor_Info.stClouldInfo, sizeof(ACS_CLOUDLOGIN_S));
	return ACS_ERR_SUCCESS;
}




/**
* fun:设置平台状态并且通知设备
* @ACS_SERVER_STATE_E-in:输入状态值
* @return:ACS_SERVER_STATE_E;
**/
ACS_INT ACSCOM_Set_ServiceState(ACS_SERVER_STATE_E state)
{
	if(ACS_State != state )//|| ACS_SERVER_NETERR == state)
	{
		ACS_State = state;
		
		if(g_Acs_CmdHandle)
		{
			PRT_PRINT(ACS_INFO,"AcsCloud_handleCmd pState:%d",ACS_State);
			g_Acs_CmdHandle(ACS_CMD_DEV_STATE,(ACS_CHAR *)&state);
		}
	}
	return ACS_State;
}

/**
* fun:获取平台状态
* @ACS_VOID-in:输入void
* @return:ACS_SERVER_STATE_E;
**/
ACS_INT ACSCOM_Get_ServiceState(ACS_VOID)
{
	return ACS_State;
}


/**
* fun:下载人脸图片  如失败上传失败信息
* @pstFaceRecord:输入人脸信息ACS_FACE_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_ERR_SUCCESS-success;ACS_ERR_FAIL-fail
**/
ACS_INT ACSCOM_SyncFacePic(ACS_FACE_INFO_S *pstFaceRecord,const ACS_CHAR *pMsgId)
{
	ACS_INT i = 0;
	ACS_INT ret = ACS_ERR_FAIL;
	ACS_SYNCFACE_RESULT_S stFaceResult = {0};
	struct AcsMemoryStruct szfileBuf = {0};

	//for(ACS_INT i = 0;i< 1;i++)
	{
		memset(&stFaceResult,0,sizeof(ACS_SYNCFACE_RESULT_S));
		if((strcmp(pstFaceRecord[i].operation,ACSDB_SAVE) == 0) || (strcmp(pstFaceRecord[i].operation,ACSDB_UPDATE) == 0))
		{
			stFaceResult.code = ACSHTTP_DownOnePicData(&szfileBuf,pstFaceRecord[i].url);
			PRT_PRINT(ACS_INFO,"downFacePic Finish;ret:%d,Datalen:%d",stFaceResult.code,szfileBuf.size);
		}
		else if((strcmp(pstFaceRecord[i].operation,ACSDB_DELETE) == 0))
		{
			stFaceResult.code = ACS_ERR_SUCCESS;
			pstFaceRecord[i].nPicDataLen = 0;
			pstFaceRecord[i].pPicData = NULL;
		}
		else
		{
			PRT_PRINT(ACS_ERROR,"ACS facepic operation:%s error;ret:%d;",pstFaceRecord[i].operation,stFaceResult.code);
			stFaceResult.code = ACS_ERR_FAIL;
		}
		
		if(ACS_ERR_SUCCESS == stFaceResult.code)//if(ACS_ERR_SUCCESS == ACSCOM_OperateResult(&stFaceResult.code,stFaceResult.msg))
		{
			if(szfileBuf.size)
			{
				pstFaceRecord[i].nPicDataLen = szfileBuf.size;
				pstFaceRecord[i].pPicData = szfileBuf.memory;
			}
			ret = ACS_ERR_SUCCESS;
		}
		else
		{
			sprintf(stFaceResult.userId,"%s", pstFaceRecord[i].userId);
			sprintf(stFaceResult.url,"%s", pstFaceRecord[i].url);
			sprintf(stFaceResult.operation, "%s", pstFaceRecord[i].operation);
		
			ret = stFaceResult.code;
			sprintf(stFaceResult.result,"%s", "fail");
			snprintf(stFaceResult.ID,sizeof(stFaceResult.ID),"%s",pstFaceRecord[i].ID);
			ACSHTTP_SyncFaceResult_Upload(&stFaceResult,pMsgId); /***上传人脸入库同步结果***/
			

			if(ACS_ERR_SUCCESS != ret)
			{
				ACS_MESSAGE_S stMsg = {0};
				sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SYCUSERS);
				sprintf(stMsg.szID,"%s",stFaceResult.ID);
				stMsg.nCode = ret;
				ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_SYCUSERS,(ACS_CHAR *)&pstFaceRecord[i],pMsgId);
			}
			
			if(NULL != szfileBuf.memory)
			{
				free(szfileBuf.memory);
				szfileBuf.memory = NULL;
			}
		}
	}
	return ret;
}


/**
* fun:下载广告图片  如失败上传失败信息
* @pstAdInfo:输入人脸信息 ACS_ADVERT_INFO_S *
* @pMsgId-in:消息ID 如果无消息ID填"",不能为NULL
* @return:ACS_ERR_E;
**/
ACS_INT ACSCOM_SyncAdPic(ACS_ADVERT_INFO_S *pstAdInfo,const ACS_CHAR *pMsgId)
{
	ACS_INT ret = ACS_ERR_FAIL;
	ACS_INT i = 0;
	//ACS_CHAR szPicName[256] = {0};
	ACS_CHAR szDetails[16] = {0};
	ACS_ADVERT_RESULT_S stAdResult = {0};
	struct AcsMemoryStruct szfileBuf = {0};

	if(NULL == pstAdInfo)
	{
		PRT_PRINT(ACS_ERROR,"SyncAdPic pstAdInfo point NULL");
		return ACS_ERR_PARAMERR;
	}
	
	/***save or update***/
	//for(ACS_INT i = 0;i< 1;i++)
	{
		PRT_PRINT(ACS_INFO,"SyncAdPic operation:%s",pstAdInfo[i].operation);
		memset(&stAdResult,0,sizeof(ACS_ADVERT_RESULT_S));
		stAdResult.code = ACS_ERR_FAIL;

		if((strcmp(pstAdInfo[i].operation,ACSDB_SAVE) == 0))
		{
			//memset(szPicName,0,sizeof(szPicName));
			//sprintf(szPicName,"%s%s",g_szAcsPath,ACSADPICFILE);
			//stAdResult.code = ACSHTTP_DownAdPic(pstAdInfo,szPicName);
			//memset(szPicName,0,sizeof(szPicName));
			//sprintf(szPicName,"%s%s.%s",g_szAcsPath,pstAdInfo[i].adId,pstAdInfo[i].format);
			stAdResult.code = ACSHTTP_DownOnePicData(&szfileBuf,pstAdInfo[i].adurl);
			PRT_PRINT(ACS_DEBUG,"downAdPic Finish stAdResult.code:%d;",stAdResult.code);
		}
		else if((strcmp(pstAdInfo[i].operation,ACSDB_UPDATE) == 0))
		{
			if(strlen(pstAdInfo->adurl) < 4)
			{
				stAdResult.code = ACS_ERR_SUCCESS;
			}
			else
			{
				//memset(szPicName,0,sizeof(szPicName));
				//sprintf(szPicName,"%s%s.%s",g_szAcsPath,pstAdInfo[i].adId,pstAdInfo[i].format);
				stAdResult.code = ACSHTTP_DownOnePicData(&szfileBuf,pstAdInfo[i].adurl);
				PRT_PRINT(ACS_DEBUG,"downAdPic Finish stAdResult.code:%d;",stAdResult.code);
			}
		}
		else if((strcmp(pstAdInfo[i].operation,ACSDB_DELETE) == 0))
		{
			stAdResult.code = ACS_ERR_SUCCESS;
		}
		else
		{
			//PRT_PRINT(ACS_ERROR,"ACS downAdPic operation:%s error;ret:%d;",pstAdInfo[i].operation,stAdResult.code);
			stAdResult.code = ACS_ERR_DATAERR;
			memcpy(szDetails,pstAdInfo[i].operation,16);
		}
	
		
		ret = stAdResult.code;
		if(ACS_ERR_SUCCESS == stAdResult.code)//if(ACS_ERR_SUCCESS == ACSCOM_OperateResult(&stAdResult.code,stAdResult.msg))
		{
			if(szfileBuf.size)
			{
				pstAdInfo[i].nPicDataLen = szfileBuf.size;
				pstAdInfo[i].pPicData = szfileBuf.memory;
			}
			ret = ACS_ERR_SUCCESS;
		}
		else
		{
			sprintf(stAdResult.adId,"%s", pstAdInfo[i].adId);
			sprintf(stAdResult.operation, "%s", pstAdInfo[i].operation);
			sprintf(stAdResult.result,"%s", "fail");
			snprintf(stAdResult.ID,sizeof(stAdResult.ID),"%s",pstAdInfo[i].ID);
			ACSHTTP_SyncAdResult_Upload(&stAdResult,pMsgId); /***上传广告入库同步结果***/

			if(ACS_ERR_SUCCESS != ret)
			{
				ACS_MESSAGE_S stMsg = {0};
				sprintf(stMsg.szCmd,"%s",CMD_MQTTSERVER_SETAD);
				sprintf(stMsg.szID,"%s",stAdResult.ID);
				sprintf(stMsg.szDetails,"%s",szDetails);
				stMsg.nCode = ret;
				ACSHTTP_Message_upload(&stMsg,ACS_HTTPURL_SETAD,(ACS_CHAR *)&pstAdInfo[i],pMsgId);
			}

			if(NULL != szfileBuf.memory)
			{
				free(szfileBuf.memory);
				szfileBuf.memory = NULL;
			}
		}
	}
	return ret;
}





