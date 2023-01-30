#ifndef _ACSCLOUDQUEUE_H_
#define _ACSCLOUDQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "acscloudcommon.h"
#include "acscloud_intellect.h"




typedef struct _ACS_QUEUE_S
{
	ACS_INT nQueneSize;//队列长度 
	ACS_INT front,rear;
	ACS_CHAR *data;
}ACS_QUEUE_S;

ACS_QUEUE_S *Acs_CreateEmptyDataSequeue(ACS_INT oneDataSize,ACS_INT nQueneSize);
ACS_VOID Acs_DestroyDataSequeue(ACS_QUEUE_S *queue);
ACS_INT Acs_EmptyDataSequeue(ACS_QUEUE_S *queue);
ACS_INT Acs_FullDataSequeue(ACS_QUEUE_S *queue);
ACS_INT Acs_EnDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * inData,ACS_INT oneDataSize);
ACS_INT Acs_DeDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * outData,ACS_INT oneDataSize);
ACS_INT Acs_DeAllDataQueue(ACS_QUEUE_S *queue);

#if 0
#define ACSMAXNUM_MSG 	55
#define PICNUMMAX_MSG 	1010
#define CARDNUMMAX_MSG  1010

ACS_QUEUE_S *AcsPic_CreateEmptyDataSequeue(ACS_INT oneDataSize);
ACS_VOID AcsPic_DestroyDataSequeue(ACS_QUEUE_S *queue);
ACS_INT AcsPic_EmptyDataSequeue(ACS_QUEUE_S *queue);
ACS_INT AcsPic_FullDataSequeue(ACS_QUEUE_S *queue);
ACS_INT AcsPic_EnDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * inData,ACS_INT oneDataSize);
ACS_INT AcsPic_DeDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * outData,ACS_INT oneDataSize);
ACS_INT AcsPic_DeAllDataQueue(ACS_QUEUE_S *queue);
#endif


#ifdef __cplusplus
}
#endif

#endif


