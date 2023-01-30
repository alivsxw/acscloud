#include "acscloudqueue.h"

pthread_mutex_t gMutexQueueDataDeal = PTHREAD_MUTEX_INITIALIZER;


ACS_QUEUE_S *Acs_CreateEmptyDataSequeue(ACS_INT oneDataSize,ACS_INT nQueneSize)
{
	
	ACS_QUEUE_S* queue;

	queue = (ACS_QUEUE_S*)malloc(sizeof(ACS_QUEUE_S));

	if(queue == NULL) return NULL;
	
	queue->data = (ACS_CHAR *)malloc(oneDataSize*nQueneSize);
	if (NULL == queue->data) return NULL;
	
	queue->front = 0;
	queue->rear = 0;
	queue->nQueneSize = nQueneSize;
	//printf("[%s:%d]####front=%d,rear=%d####\n",__func__,__LINE__,queue->front,queue->rear);
	pthread_mutex_init(&gMutexQueueDataDeal, NULL);
	return queue;
}

ACS_VOID Acs_DestroyDataSequeue(ACS_QUEUE_S *queue)
{
	if(queue == NULL)
		return ;
	
	if(queue->data)
	{
		free(queue->data);
		queue->data = NULL;
	}

	free(queue);
	queue = NULL;
}

ACS_INT Acs_EmptyDataSequeue(ACS_QUEUE_S *queue)
{
	if (NULL == queue||queue->data == NULL) 
		return -1;
	//printf("[%s:%d]####queue=%p,front=%d,rear=%d####\n",__func__,__LINE__,queue,queue->front,queue->rear);
	return (queue->front == queue->rear ? 1 : 0);
}

ACS_INT Acs_FullDataSequeue(ACS_QUEUE_S *queue)
{
	if (NULL == queue||queue->data == NULL) return -1;
	
	//printf("[%s:%d]####queue=%p,front=%d,rear=%d;queue->nQueneSize:%d####\n",__func__,__LINE__,queue,queue->front,queue->rear,queue->nQueneSize);

	return ((queue->rear + 1) % queue->nQueneSize == queue->front ? 1 : 0);
}
//进队列
ACS_INT Acs_EnDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * inData,ACS_INT oneDataSize)
{
	if (NULL == queue||queue->data == NULL) return - 1;
	
	pthread_mutex_lock(&gMutexQueueDataDeal);

	if (1 == Acs_FullDataSequeue(queue)) 
	{
		pthread_mutex_unlock(&gMutexQueueDataDeal);
		return -1; /* full */
	}
	queue->rear = (queue->rear + 1) % queue->nQueneSize;

	memset(queue->data+queue->rear*oneDataSize,0,oneDataSize);
	memcpy(queue->data+queue->rear*oneDataSize,inData,oneDataSize);
	pthread_mutex_unlock(&gMutexQueueDataDeal);
	return 0;
}
//出队列
ACS_INT Acs_DeDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * outData,ACS_INT oneDataSize)
{
	if (NULL == queue||queue->data == NULL) return -1;
	
	pthread_mutex_lock(&gMutexQueueDataDeal);

	if (1 == Acs_EmptyDataSequeue(queue)) 
	{
		pthread_mutex_unlock(&gMutexQueueDataDeal);
		return -1; /* empty */
	}
	queue->front = (queue->front + 1) % queue->nQueneSize;

	if (NULL != outData) 
	{
		memcpy(outData,queue->data+queue->front*oneDataSize,oneDataSize);
	}
	
	pthread_mutex_unlock(&gMutexQueueDataDeal);
	return 0;
}

ACS_INT Acs_DeAllDataQueue(ACS_QUEUE_S *queue)
{
	if (NULL == queue||queue->data == NULL) return -1;
	
	queue->front = 0;
	queue->rear = 0;
//	queue->nQueneSize = 0;//设置为0要注意%0的情况
	return 0;
}




#if 0
ACS_QUEUE_S *AcsPic_CreateEmptyDataSequeue(ACS_INT oneDataSize)
{
	ACS_QUEUE_S* queue;

	queue = (ACS_QUEUE_S*)malloc(sizeof(ACS_QUEUE_S));

	if(queue == NULL) return NULL;
	
	queue->data = (ACS_CHAR *)malloc(oneDataSize*PICNUMMAX_MSG);
	if (NULL == queue->data) return NULL;
	
	queue->front = 0;
	queue->rear = 0;
	printf("[%s:%d]####front=%d,rear=%d####\n",__func__,__LINE__,queue->front,queue->rear);
	return queue;
}

ACS_VOID AcsPic_DestroyDataSequeue(ACS_QUEUE_S *queue)
{
	if(queue == NULL)
		return ;
	
	if(queue->data)
	{
		free(queue->data);
		queue->data = NULL;
	}

	free(queue);
	queue = NULL;
}

ACS_INT AcsPic_EmptyDataSequeue(ACS_QUEUE_S *queue)
{
	if (NULL == queue||queue->data == NULL) 
		return -1;
	//printf("[%s:%d]####queue=%p,front=%d,rear=%d####\n",__func__,__LINE__,queue,queue->front,queue->rear);
	return (queue->front == queue->rear ? 1 : 0);
}

ACS_INT AcsPic_FullDataSequeue(ACS_QUEUE_S *queue)
{
	if (NULL == queue||queue->data == NULL) return -1;

	return ((queue->rear + 1) % PICNUMMAX_MSG == queue->front ? 1 : 0);
}

ACS_INT AcsPic_EnDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * inData,ACS_INT oneDataSize)
{
	if (NULL == queue||queue->data == NULL) return - 1;

	if (1 == AcsPic_FullDataSequeue(queue)) return -1; /* full */
	
	queue->rear = (queue->rear + 1) % PICNUMMAX_MSG;

	memset(queue->data+queue->rear*oneDataSize,0,oneDataSize);
	memcpy(queue->data+queue->rear*oneDataSize,inData,oneDataSize);
	
	return 0;
}

ACS_INT AcsPic_DeDataQueue(ACS_QUEUE_S *queue,ACS_CHAR * outData,ACS_INT oneDataSize)
{
	if (NULL == queue||queue->data == NULL) return -1;

	if (1 == AcsPic_EmptyDataSequeue(queue)) return -1; /* empty */

	queue->front = (queue->front + 1) % PICNUMMAX_MSG;

	if (NULL != outData) 
	{
		memcpy(outData,queue->data+queue->front*oneDataSize,oneDataSize);
	}

	return 0;
}

ACS_INT AcsPic_DeAllDataQueue(ACS_QUEUE_S *queue)
{
	if (NULL == queue||queue->data == NULL) return -1;
	
	queue->front = 0;
	queue->rear = 0;

	return 0;
}
#endif


