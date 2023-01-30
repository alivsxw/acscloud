#ifndef _ACSCLOUDDEFINE_H_
#define _ACSCLOUDDEFINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef ACS_VOID
#define ACS_VOID 	void
#endif

#ifndef ACS_CHAR
#define ACS_CHAR 	char
#endif

#ifndef ACS_BYTE
#define ACS_BYTE 	unsigned char
#endif

#ifndef ACS_INT
#define ACS_INT 	int
#endif

#ifndef ACS_DWORD
#define ACS_DWORD 	unsigned int
#endif

#ifndef ACS_LONG
#define ACS_LONG    long
#endif

#ifndef ACS_ULONG
#define ACS_ULONG 	unsigned long
#endif

#ifndef ACS_UINT64
#define ACS_UINT64 	unsigned long long
#endif

#ifndef ACS_FLOAT
#define ACS_FLOAT   float
#endif


#ifndef NCHAR_A16
#define NCHAR_A16     (16)
#endif

#ifndef NCHAR_A32
#define NCHAR_A32     (32)
#endif

#ifndef NCHAR_A64
#define NCHAR_A64     (64)
#endif

#ifndef NCHAR_A128
#define NCHAR_A128    (128)
#endif

#ifndef NCHAR_A256
#define NCHAR_A256    (256)
#endif

#ifndef NCHAR_A512
#define NCHAR_A512    (512)
#endif

#ifndef NCHAR_A1024
#define NCHAR_A1024   (1024)
#endif


#define	ACS_TYPE_SET_OFFSET	0x1000 //设置参数命令


typedef struct _ACS_Upgrade_hdr_s {
	ACS_DWORD	dwBgnFlag;				//扩展头起始标识UPFILE_BGN_FLAG
	ACS_DWORD	dwFileType;				//升级文件类型 UPDATE_TYPE_E
	ACS_DWORD   dwDeviceType;			//设备类型 1: IPC 2: DVS 4: NVR 8: 解码器
	ACS_DWORD   dwPlatform;				//平台(平台之间升级文件不兼容)
	ACS_DWORD 	dwFileNum;				//文件个数
	ACS_DWORD	dwTotalLength;			//sizeof(UPDATE_FILE_HEADER_S) + files info + files + sizeof(PATH_FILE_TAIL_S)
	ACS_BYTE	bRebootFlag;			//0: 不需重启   1: 重启机器
	ACS_BYTE	version[32];
	ACS_BYTE	modelname[12];	
	ACS_BYTE	flash_type[12];
	ACS_BYTE	byReserved[45];
	ACS_DWORD	dwEndFlag;				//扩展头结束标识UPFILE_END_FLAG
}ACS_Upgrade_hdr;


typedef struct __ACS_UPDATE_FILE_INFO_S				//sizeof() = 256
{
	ACS_CHAR 	szFileName[128];		//文件路径及文件名
	ACS_CHAR 	srFileName[64];			//文件文件名
	ACS_DWORD 	nFileSize;				//文件长度
	ACS_DWORD 	nOffset;				//偏移（相对files）
	ACS_DWORD 	nFileAttr;				//文件属性
	ACS_BYTE 	nIndex;
	ACS_BYTE 	nReserved[51];			//备用
}ACS_UPDATE_FILE_INFO_S;


typedef struct {
	ACS_Upgrade_hdr stHdr;
	ACS_UPDATE_FILE_INFO_S stFileInfo;
	ACS_DWORD 	curubifilesize;
}ACS_Upgrade_Info_t;

typedef struct {
	FILE *fp;  //写入文件描述符
	ACS_DWORD nCheckResult; //文件校验结果
	ACS_DWORD nTotalSize; //接收文件总大小
}ACS_CURL_WRITE_CALLBACK_USER;


//错误码
typedef enum _ACS_ERR_E
{
	ACS_ERR_SUCCESS			= 	0,	//成功
	ACS_ERR_FAIL			= 	201,//失败
	ACS_ERR_DEVBUSY			=	202,//设备忙
	ACS_ERR_TIMEOUT			=	203,//超时
	ACS_ERR_NODATA			=	204,//没有可同步的数据 不推送错误消息
	
	ACS_ERR_PARAMNULL		=	400,//参数为空 (传参错误通常是指针为NULL)
	ACS_ERR_DATAERR,				//DATAjson错误 (平台下发的数据缺失关键字段)
	ACS_ERR_DATAMAX,				//DATAjson太长 (平台下发的数据太长)
	ACS_ERR_DEVNOTREGISTER,			//设备未录入平台 (平台下发)

	
	ACS_ERR_PARAMERR,				//参数错误
	ACS_ERR_CHECKFAIL,				//验证失败
	ACS_ERR_NOTCERTIFICATION,		//无权限
//	ACS_ERR_SPACEERR,				//空间不足
//	ACS_ERR_NOTEXIST,				//不存在
	
	ACS_ERR_FILEBIG			=	500,//文件大
	ACS_ERR_FILE_OPENERR,			//文件打开失败
	ACS_ERR_FILENOT,				//文件不存在
	ACS_ERR_FILEREPEAT,				//文件重复
	ACS_ERR_FILE_READERR,			//文件读取错误
	ACS_ERR_FILE_WRITEERR,			//文件写入错误
	ACS_ERR_FILE_DOWNERR,			//文件下载错误
	
	ACS_ERR_PIC_QUALITYERR 	=	510,//图片质量差
	ACS_ERR_PIC_SIZEERR,			//图片尺寸错误
	ACS_ERR_PIC_NOFACE,				//图片没有人脸
	ACS_ERR_PIC_FORMATERR,			//图片格式错误
	ACS_ERR_PICFACE_AREAERR,		//图片人脸区域错误
	ACS_ERR_PICFACE_COLLECTERR, 	//图片人脸提取特征值错误

	ACS_ERR_DBFULL 			=	520,//库满
	ACS_ERR_DB_READERR,				//库读取错误
	ACS_ERR_DB_NOTID,				//库没有此ID
	ACS_ERR_IDREPEAT,				//ID重复
	ACS_ERR_CARDREPEAT,				//CARD重复
	ACS_ERR_IDCARDREPEAT,			//IDCARD重复
	ACS_ERR_PWDREPEAT,				//密码重复

	ACS_ERR_SERVERNOT		=	600,//服务未开启
	ACS_ERR_SERVERFAIL,				//服务未初始化成功
	
	//////////////以下错误需要再次请求///////////////////////////
	ACS_ERR_DATANULL,				//DATAjson为空 (curlNUll平台下发的数据空)
	ACS_ERR_MALLOCFAIL,				//malloc fail
	ACS_ERR_CURLFIAL,				//curl出错//	ACS_ERR_CURLNULL,//curl回调数据为NULL

}ACS_ERR_E;


typedef enum _ACS_ERR_SER_E
{
	ACS_ERRSER_SUCCESS  	 = 0,		//成功
	ACS_ERRSER_NOTREGISTER   = 1200,	//设备未录入平台
	ACS_ERRSER_CALLSUCCESS   = 1201,	//呼叫成功
	ACS_ERRSER_NODATASYNC    = 2012,	//暂无需要同步的数据
	
}ACS_ERR_SER_E;


//设备状态
typedef enum _ACS_SERVER_STATE_E
{
	ACS_SERVER_INIT  		= 0,	//初始状态				 (删除设备)
	ACS_SERVER_OK	 		= 1,	//平台添加且绑定设备 (绑定设备)	发送心跳
	ACS_SERVER_UNBIND 		= 2,	//平台添加未绑定设备 (解绑设备) 发送心跳
	ACS_SERVER_FAIL	  		= 3,	//平台未添加设备
	ACS_SERVER_NETERR 		= 4,	//网络问题
	ACS_SERVER_UNBINDFAIL 	= 5,	//绑定设备失败
	ACS_SERVER_LOGIN 		= 6,	//请求登入失败
}ACS_SERVER_STATE_E;


//用于回调函数的指令 通知设备处理事件
typedef enum _ACS_CMD_E
{
	//设备操作命令重启等
	ACS_CMD_DEV_REBOOT = 10,		//设备硬软重启
	ACS_CMD_DEV_STATE  = 11,		//设备状态state
	ACS_CMD_DEV_SYNCSTOP,			//停止同步
	ACS_CMD_DEV_SYNCTIME,			//同步时间

	//获取记录人脸数据等
	ACS_CMD_GET_SYNCUSER = 30,		//用户
	ACS_CMD_GET_SYNCCARD,			//卡号
	ACS_CMD_GET_SYNCAD,				//广告
	ACS_CMD_REISSUE_RECORD,			//考勤补发
	ACS_CMD_GET_SYNCELERULE,		//派梯规则
	ACS_CMD_GET_CHECKDATA,			//检测数据
	ACS_CMD_GET_SYNCPWD,			//密码
	ACS_CMD_GET_STATISTICS,			//获取数据库数据的数量
	
	//设置记录人脸数据等
	ACS_CMD_SET_OPENDOOR,			//开门
	ACS_CMD_SET_UPGRADEFW, 			//升级
	ACS_CMD_SET_CLEANDATA, 			//清理平台下发的数据

	ACS_CMD_DCITTALKLIST,			//分机列表上传
	ACS_CMD_SNAPPHOTO,				//抓拍图上传
	ACS_CMD_UPGRADEFW_FAIL, 		//下载升级失败
	ACS_CMD_SET_UPPARAM,			//更新参数
	ACS_CMD_SYNCPARAM,				//设备参数同步
	//ACS_CMD_WEBRTC_UPPARAM,		//webrtc初始化参数同步 废指令 删除了
	ACS_CMD_CALLCMD_UPPARAM,		//webrtc手机端操作命令 
	ACS_CMD_RESETFACTORY,			//恢复出厂
}ACS_CMD_E;



//门禁云平台登入参数
typedef struct _ACS_CLOUDLOGIN_S 
{
	ACS_INT				nEnable;			//云平台开关//ACS_INT				nDeviceType;		//设备类型 1.普通门禁 2.人证健康码设备
	ACS_CHAR 			szHttpUrl[128];		//http服务器地址(域名或ip:port) http://aidcit.cloud/access:8799
	ACS_CHAR			szUuid[32];			//设备ID							10cun12396
	ACS_CHAR 			szPcode[32];		//平台代码							113666614626947072//ACS_CHAR			szMediaId[32];		//流媒体账号
	ACS_CHAR 			szToken[128];		//
	ACS_CHAR			szSoftVersion[64];	//软件版本
	ACS_CHAR			szDevType[16];		//设备类型
	ACS_CHAR			szDevWiredIP[16];	//设备ip
	ACS_CHAR			szcpuID[32];		//设备cpuID						00008cc17cad5632
}ACS_CLOUDLOGIN_S;

//门禁云平台资源路径
typedef struct _ACS_API_RESOURCEPATH_S
{
	ACS_CHAR				szLogin[128]; 			//登入
	ACS_CHAR				szSyncFace[128];		//同步人脸
	ACS_CHAR				szSyncCard[128];		//同步卡号
	ACS_CHAR				szSyncPawd[128];		//同步密码
	ACS_CHAR				szSyncAds[128];			//同步广告
	ACS_CHAR				szSyncTime[128];		//同步时间

	ACS_CHAR				szResultFace[128];		//人脸结果上传
	ACS_CHAR				szResultCard[128];		//卡号结果上传
	ACS_CHAR				szResultAds[128];		//广告结果上传

	ACS_CHAR				szCheckPawd[128];		//密码验证
	ACS_CHAR				szCheckData[128];		//数据检查
}ACS_API_RESOURCEPATH_S;
//门禁云平台资源路径
typedef struct _ACS_APICONFIG_S
{
	ACS_CHAR				szProjectName[128]; 	//工程名
	ACS_API_RESOURCEPATH_S	stResourcePath;			//资源路径
}ACS_APICONFIG_S;

//数据下发 eRules
typedef struct _ACS_ELEDISRULES_S//同步人员信息接收 派梯
{
	ACS_INT eleFloor;			//派梯权限楼层
}ACS_ELEDISRULES_S;

//数据下发
typedef struct _ACS_FACE_INFO_S//size=512 //同步人员信息接收 470
{
	ACS_CHAR ID[20];		//用于平台同步校验
	ACS_CHAR userId[32];	//用户ID
	//ACS_CHAR eleRuleid[32];	//规则ID
	ACS_CHAR userName[32];	//用户姓名//32/64
	ACS_CHAR userType[16];	//用户类型
	ACS_CHAR card[16];		//卡号
	ACS_CHAR password[16];	//密码
	ACS_CHAR url[256];		//用户图片地址
	ACS_CHAR operation[16]; //操作说明(save,delete,update)
	ACS_CHAR userIdCard[20];//用户身份证号
	ACS_INT  nValidEnable;	//有效期使能
	ACS_CHAR validtime[16]; //有效期起始时间
	ACS_CHAR validtimeend[16];//有效期结束时间
	ACS_INT  nPicDataLen;
	ACS_CHAR *pPicData;
	ACS_INT  neDisRules;		//派梯结构体数量			 ACS_ELEDISRULES_S sizeof
	ACS_CHAR *peDisRulesData;	//派梯规则结构体指针 ACS_ELEDISRULES_S*neRules
}ACS_FACE_INFO_S;

//数据下发
typedef struct _ACS_CARD_INFO_S
{
	ACS_CHAR ID[20];		//用于平台同步校验
	ACS_CHAR userId[32];	//用户ID
	ACS_CHAR type[16];		//卡类型  CARD|IDCARD|RCARD
	ACS_CHAR card[32];		//门卡号
	ACS_CHAR expired[16];	//门卡过期时间
	ACS_CHAR operation[16];	//操作说明（save, delete, update）
}ACS_CARD_INFO_S;


//数据下发
typedef struct _ACS_PWD_INFO_S
{
	ACS_CHAR ID[20];		//用于平台同步校验
	ACS_CHAR userId[32];	//用户ID
	ACS_CHAR pwd[16];		//密码
	ACS_CHAR expired[16];	//密码过期时间
	ACS_CHAR operation[16];	//操作说明（save, delete, update）
	ACS_INT	 limitnum;		//使用次数 默认999999
}ACS_PWD_INFO_S;


//数据下发
typedef struct _ACS_ADVERT_INFO_S
{
	ACS_CHAR operation[16];		//操作说明（save, delete, update）
	ACS_CHAR ID[20];			//用于平台同步校验
	ACS_CHAR adId[20];			//广告id
	ACS_CHAR adurl[256];		//广告下载地址
	ACS_CHAR adPicName[128];	//广告图片名字ad%s-%ld-%ld-%d%s ad123-.jpg
	ACS_CHAR msg[32];			//广告提示
	ACS_CHAR format[8];			//广告格式 jpg/png/bmp
	ACS_CHAR level;				//广告播放等级 广告播放等级默认0(数字越高优先0-254)
	ACS_CHAR speed;				//广告轮播切换时间默认5
	ACS_CHAR type;				//广告类型 默认1(1-大广告 2-视频广告 3-logo)
	ACS_LONG adBegin;			//广告开始播放时间戳
	ACS_LONG adEnd;				//广告过期时间戳
	ACS_INT  nPicDataLen;
	ACS_CHAR *pPicData;
	
}ACS_ADVERT_INFO_S;

//数据下发 floors
typedef struct _ACS_FLOORS_INFO_S
{
	ACS_BYTE bDir;			//开门方向	0-前; 1-后;2-前和后;
	ACS_BYTE bFloor;		//权限楼层 11
	ACS_CHAR cName[32];		//楼层名称 "智能层"
}ACS_FLOORS_INFO_S;

//数据下发 elevatorRules
typedef struct _ACS_ELERULES_INFO_S
{
	ACS_CHAR *pFloorData;   //ACS_FLOORS_INFO_S
	ACS_INT  floorTotal;
	ACS_INT  type;			//类型
	ACS_INT  defaultfloor;	//默认楼层
	ACS_INT  curfloor;		//当前楼层
	ACS_INT  visitorfloor;	//访客楼层
	ACS_INT  code;			//错误码
	ACS_CHAR ID[20];		//用于平台同步校验
	ACS_CHAR eleRulesid[32];//规则ID
	ACS_CHAR operation[16]; //操作说明(save,delete,update)
	ACS_FLOORS_INFO_S stFloors;
}ACS_ELERULES_INFO_S;

//信息统计
typedef struct _ACS_DATANUM_S
{
	ACS_INT person_count;		//用户登记数量		
	ACS_INT manage_count; 		//管理员登记数量
    ACS_INT face_count;			//人脸登记数量
    ACS_INT card_cont;			//门卡登记数量
    ACS_INT pwd_cont;			//密码登记数量
    ACS_INT ad_cont;			//广告登记数量
}ACS_DATANUM_S;




//结果上传
typedef struct _ACS_SYNCFACE_RESULT_S//同步人员结果上传
{
	ACS_CHAR  ID[20];		//同步人脸下发的ID
	ACS_CHAR  userId[32];
	ACS_CHAR  url[256];
	ACS_CHAR  result[8];
	ACS_CHAR  operation[16]; //同步人脸的操作说明（save，delete，update）
	ACS_INT   code;			//设备错误码     必填 ACS_ERR_FAIL ACS_ERR_E
	ACS_CHAR  msg[32];
	
}ACS_SYNCFACE_RESULT_S;

//结果上传
typedef struct _ACS_SYNCCARD_RESULT_S//同步卡号结果上传
{
	ACS_CHAR ID[20];		//同步卡号下发的id
	ACS_CHAR userId[32];
	ACS_CHAR cardId[32];
	ACS_CHAR result[16];
	ACS_CHAR operation[16]; //同步卡号的操作说明（save，delete，update）
	ACS_INT  code; // ACS_ERR_E
	ACS_CHAR msg[64];
}ACS_SYNCCARD_RESULT_S;

//结果上传
typedef struct _ACS_ADVERT_RESULT_S//同步广告结果上传
{
	ACS_CHAR ID[20];		//同步广告下发的操作id
	ACS_CHAR adId[20];		//广告id
	ACS_CHAR result[16];	//fail success
	ACS_CHAR operation[16]; //同步卡号的操作说明（save，delete，update）
	ACS_INT  code;
	//ACS_CHAR msg[64];
}ACS_ADVERT_RESULT_S;


//结果上传
typedef struct _ACS_IDCARD_INFO_S//身份证信息
{
	ACS_CHAR szName[32];		//姓名
	ACS_CHAR szGender[4];		//性别
	ACS_CHAR szNational[24];	//民族
	ACS_CHAR szBirthday[16];	//生日
	ACS_CHAR szAddress[140];	//地址
	ACS_CHAR szId[36];			//身份证号码
	ACS_CHAR szMaker[64];		//签证机关
	ACS_CHAR szStartDate[16];	//起始日期
	ACS_CHAR szEndDate[16];		//有效期
	ACS_CHAR szIDCardImg[128];	//身份证图片路径
}ACS_IDCARD_INFO_S;
//结果上传
typedef struct _ACS_OPENDOOR_UPLOAD_S //上传开门记录结构体 用户开门信息
{
   ACS_BYTE byOpenDoorEn;	//0:N/A; 1-开门; 2-未开门
   ACS_CHAR userId[32]; 			//住户id
   ACS_CHAR type[16]; 				//开门类型 CARD   门卡   IDCARD身份证 RCARD居住证 (PASSWORD,APP,FACE,CARD,RCARD,IDCARD)
   ACS_CHAR data[256];				//卡ID、QRCODE、pwd
   ACS_CHAR screenTime[16]; 		//开门时间戳
   ACS_CHAR recordpath[128];   		//开门录像路径
   ACS_CHAR snapfacepath[128]; 		//开门抓拍人脸图片路径
   ACS_FLOAT temperature;			//测温数据
   ACS_IDCARD_INFO_S stIDCard;	//身份证信息
}ACS_OPENDOOR_UPLOAD_S;

//呼叫记录结果上传
typedef struct _ACS_CALL_UPLOAD_S
{
	ACS_DWORD dCallduration;		//通话时间 秒s 30s
	ACS_BYTE bstate;				//呼叫状态0:N/A; 1-呼叫成功; 2-呼叫失败
//	ACS_CHAR cmsgid[32]; 			//呼叫消息id    随机数
	ACS_CHAR ctype[16]; 			//呼叫类型 phone room management
//	ACS_CHAR cchannelName[16];		//通道名称 
	ACS_CHAR ccalledid[32]; 		//被呼方id userid
	ACS_CHAR cnumber[16]; 			//呼叫的手机/房间号
	ACS_CHAR ctimebegin[16];   		//开始时间 163022533
	ACS_CHAR ctimeend[16]; 			//结束时间 163022563
}ACS_CALL_UPLOAD_S;


//////////////////////////////////////////////////////
//结果上传
typedef struct _ACS_HEALTH_QRCODE_S
{
	ACS_FLOAT temp;			//人体温度
	ACS_CHAR QrValue[512];	//二维码数据
}ACS_HEALTH_QRCODE_S;

//结果上传
typedef struct _ACS_HEALTH_IDCARD_S
{
	ACS_FLOAT temp;			//人体温度
	ACS_CHAR CardId[512];	//身份证号	需要进行 AES128 的编码
	ACS_CHAR name[512];		//姓名 需要进行 AES128 的编码
}ACS_HEALTH_IDCARD_S;

//结果上传
typedef struct 	_ACS_NUCLEIC_ACID_INFO_S
{
	ACS_CHAR checkTime[32];		//检查日期
	ACS_CHAR checkResult[16];	//结果
}ACS_NUCLEICACID_INFO_S;

//数据下发
typedef struct _ACS_HEALTHCODE_INFO_S
{
	ACS_CHAR name[32];			//识别的用户姓名
	ACS_CHAR id[20];			//识别的用户身份证信息
	ACS_CHAR status[4];			//10：红码（高风险）；01：黄码（中风险）；00：绿码（低风险）
	ACS_CHAR date[32];			//日期
	ACS_INT hours;				//最近一次核酸距离现在的小时
	ACS_INT vaccine;			//接种疫苗次数
	ACS_CHAR vaccineDate[32];	//最近一次接种疫苗时间
	ACS_CHAR reason[128];		//红码，黄码原因
	ACS_CHAR stopOverCity[128];	//14 天行程信息，通过健康码返回的结果，需要用户手动申报
	ACS_INT type;			//0国康码；1粤康码；2穗康码
	ACS_NUCLEICACID_INFO_S stNucleicAcid[3];	//核酸信息
	ACS_INT RetCode;		//错误码
	ACS_CHAR message[64];	//消息提示
	ACS_UINT64 Timestamp;//时间戳
}ACS_HEALTHCODE_INFO_S;
///////////////////////////////////////////////////

//检查设备数据-pwd
typedef struct _ACS_CHECKDATA_AD_S
{
	ACS_CHAR adworkid[32];
}ACS_CHECKDATA_AD_S;


//检查设备数据-pwd
typedef struct _ACS_CHECKDATA_PWD_S
{
	ACS_CHAR pwd[16];
}ACS_CHECKDATA_PWD_S;


//检查设备数据-card
typedef struct _ACS_CHECKDATA_CARD_S
{
//	ACS_CHAR userId[32];
	ACS_CHAR cardId[32];
}ACS_CHECKDATA_CARD_S;

//检查设备数据-face
typedef struct _ACS_CHECKDATA_FACE_S
{
	ACS_CHAR 	userId[32];
}ACS_CHECKDATA_FACE_S;



typedef struct _ACS_INFORMATION_S
{
	ACS_DWORD  source;					//来源 0-未知 1-设备后台 2-web         6-平台
	//ACS_DWORD  userType;    			//用户类型 0:普通用户, 1:管理员, 2:访客,
	//ACS_DWORD  age;						//年龄
	//ACS_DWORD  certificateType;			//证件类型		
	//ACS_DWORD  gender;					//性别 0-未填值 1-男 2-女
	ACS_DWORD  authType;				//确认方式类型//  0:FC/P/C //1:FC+P//2 FC//3:P//4:C //5:FC+C
	ACS_DWORD  validtime;				//有效期开始时间 "1630000000"
	ACS_DWORD  validtime_end;			//有效期结束     时间 "1630000000"
	ACS_DWORD  validtime_enable;		//有效期使能	0 关 1开
	ACS_CHAR   workId[32];				//userid
	ACS_CHAR   cardNumber[16];			//开门卡号
	ACS_CHAR   password[16];			//开门密码
	ACS_CHAR   phone[12];				//电话
	ACS_CHAR   certificateNumber[20];	//证件号
	ACS_CHAR   name[32];				//用户名
	ACS_CHAR   facepath[128];			//人脸路径	/database/image_storage/img48/F25AC03C44074DF2A90B7BB5D0E4DC21.jpg
}ACS_INFORMATION_S;

//检查设备数据 卡号信息
typedef struct _ACS_INFORMATION_CARD_S
{
	ACS_DWORD  source;					//来源 0-未知 1-设备后台 2-web         6-平台
	ACS_DWORD  cardType;    			//开门卡类型 默认0-普通卡
	ACS_DWORD  validtime_end;			//有效期结束     时间 "1630000000"
	ACS_CHAR   workId[32];				//userid
	ACS_CHAR   cardNumber[16];			//开门卡号
}ACS_INFORMATION_CARD_S;

//检查设备数据 密码信息
typedef struct _ACS_INFORMATION_PWD_S
{
	ACS_DWORD  source;					//来源 0-未知 1-设备后台 2-web         6-平台
	ACS_DWORD  limitnum;				//使用次数 
	ACS_DWORD  validtime_end;			//有效期结束     时间 "1630000000"
	ACS_CHAR   workId[32];				//userid
	ACS_CHAR   password[16];			//开门密码
}ACS_INFORMATION_PWD_S;


//检查设备数据 广告信息
typedef struct _ACS_INFORMATION_AD_S
{
	ACS_DWORD  enable;					//广告播放使能 1-播放 0-不播放
	ACS_DWORD  source;					//广告来源 0-未知 1-设备后台 2-web           6-平台
	ACS_DWORD  level;					//广告播放等级 数字越高优先 默认0 同一等级随机播放
	ACS_DWORD  secord;					//广告播放速度 默认3s
	ACS_DWORD  type;					//广告类型 1-大广告 2-视频广告 3-logo 4-小标题 5-weblogo
	ACS_DWORD  createtime;				//广告创建时间
	ACS_DWORD  validtime;				//广告投放开始时间 
	ACS_DWORD  validtime_end;			//广告投放结束时间 "1630000000"
	ACS_CHAR   workId[32];				//userid
	ACS_CHAR   name[32];				//广告名
	ACS_CHAR   format[8];				//广告格式 jpg bmp png MP4
	ACS_CHAR   path[128];				//广告本地路径
}ACS_INFORMATION_AD_S;


//检查设备数据face/card/password/ad
typedef struct _ACS_CHECKDATA_LIST_S
{
	ACS_INT 	list;			// 1black 2white 3vip
	ACS_INT 	total;			// sum:face/card 
	ACS_CHAR	*pstData;		//ACS_CHECKDATA_FACE_S* ACS_CHECKDATA_CARD_S*
	ACS_CHAR	*pstInformation;//ACS_INFORMATION_S*
}ACS_CHECKDATA_LIST_S;

//检查设备数据
typedef struct _ACS_CHECKDATA_INFO_S//232
{
	ACS_INT  code;				//操作码    ACS_ERR_SUCCESS  ACS_ERR_E
	ACS_INT  size;				//名单数量 ACS_CHECKDATA_LIST_S 的长度
	ACS_CHAR operation[16];		//操作说明(人员,卡号,密码,所有)
	ACS_CHAR userId[32];		//用户ID
	ACS_CHAR ID[64];
	ACS_CHAR msg[64];			//平台操作消息 操作成功 DbopraresultACS[] ***/
	ACS_CHECKDATA_LIST_S stList[3];
}ACS_CHECKDATA_INFO_S;


//远程升级数据
typedef struct _ACS_UPGADEFW_INFO_S
{
	ACS_INT  nSize;
	ACS_CHAR szMd5[32];
	ACS_CHAR szFileName[32];
	ACS_CHAR szVersion[64];
	ACS_CHAR szUrl[256];
//	ACS_CHAR *pmemory;//通过szUrl下载到的数据
}ACS_UPGADEFW_INFO_S;


typedef struct _ACS_REISSUE_RECORD_S
{
	ACS_INT nWorkRole;		//员工类型（10管理员、20建筑工人、30关键岗位人员）
	ACS_CHAR szDirection[8];//通行方向（01入场02出厂）
	ACS_CHAR szSwipeTime[32];	//1603991512
	ACS_CHAR szUserId[64];		    //"工人id"
	ACS_CHAR szIdCardNumber[24];	//"证件号"
	ACS_CHAR szChannel[16];		    //通道 "入场"
	ACS_CHAR szPicUrl[128];
	ACS_CHAR szPicBase[1024*700];
}ACS_REISSUE_RECORD_S;



//设备参数更新子命令事件
typedef enum _ACS_PARAM_TYPE_E
{
	//设备操作命令重启等
	ACS_DEV_PARAM_CAMCFG = 0x20000,	//摄像机基本参数配置
	ACS_DEV_PARAM_HCODECLOUD ,		//健康码平台参数配置
	ACS_DEV_PARAM_SFACECFG ,		//人脸参数配置
	ACS_DEV_PARAM_ACSCFG ,			//门禁参数配置
	ACS_DEV_PARAM_AUDIOCFG ,		//音频参数配置
	ACS_DEV_PARAM_TEMCFG ,			//体温检测参数配置
	ACS_DEV_PARAM_CLOUDCFG ,		//门禁平台参数配置
	ACS_DEV_PARAM_WIREDNETCFG ,		//有线网络参数配置 Wirednetwork
	ACS_DEV_PARAM_SYNTEMVERCFG , 	//系统版本号参数配置 SysteminfoVersion
	ACS_DEV_PARAM_BUILDINGCFG ,		//楼宇对讲参数配置 BuildingCfg
	ACS_DEV_PARAM_CALLSETING,		//通话设置 Callseting
	ACS_DEV_PARAM_CALLPROTOCOL,		//协议设置 CallProtocol
	ACS_DEV_PARAM_TELEMAINTENANCE,	//远程维护 telemaintenance
}ACS_PARAM_TYPE_E;




//摄像机基本参数配置
typedef struct _ACS_CAMCFG_S
{
	ACS_CHAR szDevName[64];
}ACS_CAMCFG_S;



//健康码平台参数配置
typedef struct _ACS_HCODECLOUD_S
{
	ACS_CHAR szServerIP[256];			//健康码平台地址加端口
	ACS_CHAR szAuthIV[64];		//健康码平台加密偏移量 //AES/CBC/PKCS5Padding
	ACS_CHAR szAuthKey[64];		//健康码平台加密秘钥 //AES/CBC/PKCS5Padding
	ACS_CHAR szSceneCategory[64];	//健康码平台场所类型
	ACS_CHAR szSceneSerialID[64];	//健康码平台场所序列号
	ACS_CHAR szCityCode[64];		//城市代码
}ACS_HCODECLOUD_S;

//人脸参数配置
typedef struct _ACS_SFACECFG_S
{
	ACS_DWORD	dwAcsScene;			//设备使用场景0自适应 1室内 2室外
	ACS_DWORD	dwCmpThreshold;		//1:N 比对阈值 0~100
	ACS_DWORD	dwCmpSingleThreshold;		//1:1 比对阈值 0~100
	ACS_DWORD	dwFaceMinSize;		//最小抓拍像素30-300
	ACS_DWORD 	dwFaceMaxSize;		//最大抓拍像素300-500
	ACS_DWORD	dwHacknessenble; 	//活体检测开关
	ACS_DWORD	dwHacknessthreshold; //活体检测阈值0~100
	ACS_DWORD	dwHacknessRank;		//活体等级 // 0 普通  1中等2强 
	ACS_DWORD	dwMaskEnable;		//口罩检测 // 0关        1提醒		2必须
	ACS_DWORD	dwFaceSensitivity;	//灵敏度 0~100
}ACS_SFACECFG_S;

//门禁参数配置
typedef struct _ACS_ACSCFG_S
{
	ACS_DWORD	dwAcsState;				//门禁设备状态 0  进  1  出
	ACS_DWORD	dwStrangeRecEnable;		//陌生人识别记	0 关闭        1开启
	ACS_DWORD	dwRecoredPicEnable;		//存储图片记录	0 关闭        1开启
	ACS_DWORD	dwStrangeTik;			//陌生人是否提示	0 关闭        1开启	
	ACS_DWORD	dwDelay1Time;			//继电器1开锁/报警 延时
	ACS_DWORD	dwIo1Type;				//继电器10常开     1 常闭
	ACS_DWORD	dwConfirmMode;			//通行确认模式 //参考 PUB_UNLOCK_MODE 0或 1与
	ACS_DWORD	dwConfirmWayMask;		//通行确认方式	//参考 PUB_UNLOCK_TYPE 1-face 2-pwd 4-card 8-qrcode
}ACS_ACSCFG_S;


//体温检测参数配置
typedef struct _ACS_TEMCFG_S
{
	ACS_BYTE	byEnable;			//体温检测开关			0:关闭 1:开启
	ACS_BYTE	byEnvironment;		//测温环境				0:室内 1:室外
	ACS_BYTE	byUnit;				//温度单位				0:摄氏度 1:华氏度
	ACS_FLOAT	fWarnTemp;			//高温阈值				(37.0~40.0°C)/(98.6~104.0°F)
	ACS_FLOAT	fLowTemp;			//低温阈值				(32.0~35.0°C)/(89.6~95.0°F)
	ACS_BYTE	byHealthCodeTime;	//核酸时效(健康码时间段)			0:(无)阴性 1:24小时 2:48小时 3:72小时 4:7天内
}ACS_TEMCFG_S;


//声音参数配置
typedef struct _ACS_AUDIOSTREAM_S
{
   ACS_BYTE    byEnable;		//体温检测开关 		   0:关闭 1:开启
   ACS_BYTE    byVolumeIn;	    //测温环境			   0:室内 1:室外
   ACS_BYTE    byVolumeOut;		//温度单位			   0:摄氏度 1:华氏度
}ACS_AUDIOSTREAM_S;

//门禁平台参数配置
typedef struct _ACS_CLOUDCFG_S
{
	ACS_BYTE byEnable;			//门禁云平台使能开关
	ACS_CHAR szCloudIPAddr[64];	//平台地址加端口
	ACS_CHAR szCloudID[64];		//平台标识
}ACS_CLOUDCFG_S;

//有线网络参数配置
typedef struct _ACS_WIREDNET_S
{
	ACS_BYTE bDHCP;
	ACS_CHAR ipAddress[16];
	ACS_CHAR subnetMask[16];
	ACS_CHAR SecondaryDNS[16];
	ACS_CHAR PrimaryDNS[16];
	ACS_CHAR MACAddress[32];
	ACS_CHAR DefaultGateway[16];
}ACS_WIREDNET_S;

//系统版本号参数配置
typedef struct _ACS_SYSTEMVERSION_S
{
	ACS_CHAR cpuid[32];
	ACS_CHAR DeviceAlgoVersion[32];
	ACS_CHAR DeviceOnvifVersion[32];
	ACS_CHAR DeviceSoftVersion[64];
	ACS_CHAR DeviceUiVersion[32];
	ACS_CHAR DeviceWebVersion[16];
	ACS_CHAR uuid[16];
}ACS_SYSTEMVERSION_S;

//楼宇对讲参数配置
typedef struct _ACS_BUILDINGINFO_S
{
	ACS_CHAR AControlIp[16];
	ACS_CHAR AreaID[8];
	ACS_CHAR BuildingID[8];
	ACS_CHAR BusDevName[64];
	ACS_BYTE bBusDevType;
	ACS_CHAR CenterIP[16];
	ACS_CHAR DevNumber[8];
//	ACS_BYTE bDeviceType;
	ACS_CHAR RoomID[8];
	ACS_BYTE bSlaveType;
	ACS_CHAR UnitID[16];
}ACS_BUILDINGINFO_S;

//通话设置
typedef struct _ACS_TALKPARAM_S
{	
	ACS_BYTE dwRingToneVolume;				//0-10 铃声音量
	ACS_BYTE dwRingToneDuration;			//0-60 响铃时间
	ACS_BYTE dwTalkDuration;				//0-300 通话时间
	ACS_BYTE dwAutoTakeUp;					//0关1开，自动接听
	ACS_BYTE dwDNDType;					//免打扰类型 0关，1全天，2定时
	ACS_BYTE dwDNDStartHour;				//勿扰模式开始时
	ACS_BYTE dwDNDStartMinutes;			//勿扰模式开始分
	ACS_BYTE dwDNDStartTimestamp;			//勿扰模式开始时间时间戳，UI部分发时间时发开始时分和结束时分，后台需要根据当前时间计算
	ACS_BYTE dwDNDEndTimestamp;			//勿扰模式结束时间时间戳
	ACS_BYTE dwDNDEndHour;					//勿扰模式结束时
	ACS_BYTE dwDNDEndMinutes;				//勿扰模式结束分
}ACS_TALKPARAM_S;	

//远程维护设置
typedef struct _ACS_TELEMAINTENANCE_S
{	
	ACS_BYTE  dwEnable;				// 开关 默认关 0-关
	ACS_DWORD dMaintenanceDuration;	// 维护时间 单位分钟
	ACS_DWORD dPort;				// 端口    
	ACS_CHAR  cServerAddress[64];	// 服务器地址
	ACS_CHAR  cToken[64];			// token
}ACS_TELEMAINTENANCE_S;

//恢复出厂
typedef struct _ACS_RESETFACTORY_S
{	
	ACS_BYTE dwNet;				//网络重置;  1-重置,0-不重置
	ACS_BYTE dwAccount;			//账户重置;  1-重置,0-不重置
	ACS_BYTE dwFacelist;		//人脸重置;  1-重置,0-不重置
	ACS_BYTE dwRecord;			//识别记录重置;  1-重置,0-不重置
}ACS_RESETFACTORY_S;




//远程升级数据
typedef struct _ACS_DEVCFG_INFO_S
{
	ACS_DWORD  nType;
	ACS_DWORD  dwDataLen;
	ACS_CHAR *szData;
}ACS_DEVCFG_INFO_S;

//数据上传 在线验证 二维码数据/密码等 DataCheckOnline
typedef struct _ACS_DATACHECKONLINE_INFO_S
{
	ACS_DWORD	dwDataLen;			//数据长度
	ACS_BYTE	curltimeout;		//超时
	ACS_CHAR	szType[16];			//在线验证类型(PASSWORD,APP,FACE,CARD,RCARD,IDCARD,QRCODE)			
	ACS_CHAR	szScreenTime[16];	//时间戳	
	ACS_CHAR	szPhoto[128];		//图片路径
	const ACS_CHAR   *pData;		//数据指针(二维码内容/PASSWORD/CARD)
}ACS_DATACHECKONLINE_INFO_S;

//结果下发 在线验证 二维码数据/密码等 DataCheckOnline
typedef struct _ACS_DATACHECKONLINE_RESULT_S
{
	ACS_CHAR	userId[32];			//识别的用户身份证信息
	ACS_CHAR	name[32];			//识别的用户姓名
	ACS_CHAR	timestamp[16];		//时间戳
	ACS_CHAR	message[64];		//消息提示
	ACS_DWORD	opendoorEn;			//是否开门 0-不开 1-开
	ACS_DWORD	result;				//1校验通过,0校验失败
	ACS_INT		retCode;			//错误码
	ACS_CHAR	msg[128];			//消息提示
}ACS_DATACHECKONLINE_RESULT_S;


typedef enum _ACS_CALLTYPE_E
{
	ACS_CALL_NOT   			= 0,	//NOT
	ACS_CALL_PHONE,					//呼叫手机号
	ACS_CALL_ROOM,					//呼叫房间号
	ACS_CALL_MANAGEMENT,			//呼叫管理处


	ACS_CALL_START			= 5,	//发起呼叫
	ACS_CALL_ANSWER,				//接听
	ACS_CALL_HANGUP,				//挂断/拒接
	ACS_CALL_TIMEOUT,				//超时
	
	ACS_CALL_CALLOK			= 1201,//'呼叫成功',
	ACS_CALL_CALLTYPEERR	= 1202,//'呼叫类型有误',
	ACS_CALL_NOCALLID		= 1203,//'未找到可呼的对象',
	ACS_CALL_CALLBUSY		= 1204,//'占线中,请稍候再呼叫',
	ACS_CALL_NOTDISTURB		= 1205,//'对方设置免打扰,呼叫失败',
	ACS_CALL_NOBIND			= 1206,//'对方未绑定,呼叫失败',
	ACS_CALL_AUVIERR		= 1208,//'音视频账号异常',
}ACS_CALLTYPE_E;

//呼叫Post接口
typedef struct _ACS_CALL_INFO_S
{
	ACS_INT		nCallType;					//呼叫类型 ACS_CALLTYPE_E
	ACS_CHAR	cDataUUID[NCHAR_A32];		//手机ID/房间ID/	
	ACS_CHAR	cDataID[NCHAR_A32];			//peerId	
	ACS_CHAR	cScreenTime[NCHAR_A16];		//时间戳			
	ACS_CHAR	cPhoto[NCHAR_A128];			//图片路径
	//ACS_CHAR   *pData;					//数据指针(picBase64)
}ACS_CALL_INFO_S;

typedef struct _ACS_CALL_SIP_S
{
	ACS_INT		nSip;						//呼叫SIP号
	ACS_CHAR	cPhone[NCHAR_A16];			//呼叫手机号
}ACS_CALL_SIP_S;

typedef struct _ACS_CALL_RESULT_S
{
	
	ACS_BYTE	bRingtime;					//响铃时间
	ACS_INT		nDataLen;					//结果数量(ACS_CALL_SIP_S 数量)
	ACS_INT		retCode;					//错误码
	ACS_INT		nResult;					//呼叫结果
	ACS_CHAR   *pData;						//数据指针(ACS_CALL_SIP_S *)
	ACS_CHAR	cMsg[NCHAR_A32];			//呼叫结果
	ACS_CHAR	message[NCHAR_A32];			//错误结果
}ACS_CALL_RESULT_S;

//呼叫校验结构体
typedef struct _ACS_CALL_CHECK_S
{
	ACS_CHAR	type[32];			//呼叫的类型(room,phone,management)
	ACS_CHAR	callId[32];			//呼叫的数据(房间号、手机号)
}ACS_CALL_CHECK_S;

//呼叫结构体
typedef struct _ACS_CALL_WEBRTC_S
{
	ACS_CHAR    userType[16];   //住户类型,O业主、A管理、F家属、R租客、M布控人员、Y访客
	ACS_CHAR	phone[16];		//手机号
	ACS_CHAR    realName[32];   //住户名
	ACS_CHAR    perrId[128];    //设备端perrID(发起呼叫时填充)
}ACS_CALL_WEBRTC_S;


typedef struct _ACS_CALL_USERID_S
{
	ACS_CHAR	callid[64];		//呼叫ID 用于rongcloud对讲协议
}ACS_CALL_USERID_S;

//呼叫校验结果结构体
typedef struct _ACS_CALL_CHECK_RESULT_S
{
	ACS_INT     retCode;            //错误码
	ACS_INT		ringtime;           //呼叫响铃时间 默认45s
	ACS_INT		result;				//呼叫结果
	ACS_CHAR	message[128];		//消息提示
	ACS_CALL_WEBRTC_S stCall;       //呼叫信息
	ACS_DWORD  dwDataLen;			//0时pstData为空
	ACS_CHAR   *pstData;			//ACS_CALL_USERID_S * 外部需要释放
}ACS_CALL_CHECK_RESULT_S;

//呼叫操作mqtt推送结构体
typedef struct _ACS_CALL_CMD_MQTT_S
{
	ACS_CHAR	cmd[16];		//接听accept、拒接refuse、挂断hangup
	ACS_CHAR	phone[16];		//手机号
	ACS_CHAR	sessionId[128]; //手机端反向取流的webrtc会话ID
}ACS_CALL_CMD_MQTT_S;

#ifdef __cplusplus
}
#endif

#endif


