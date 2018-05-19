/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file freertos_warp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/17
 * @license
 * @description
 */


#include <stdio.h>
#include "freertos_warp.h"
#include <windows.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef void* (*routing)(void*);

#define O_ACCMODE           0003
#define O_RDONLY            00
#define O_WRONLY            01
#define O_RDWR              02
#define O_CREAT             0100 /* not fcntl */
#define O_EXCL              0200 /* not fcntl */
#define O_NOCTTY            0400 /* not fcntl */
#define O_TRUNC             01000 /* not fcntl */
#define O_APPEND            02000
#define O_NONBLOCK          04000
#define O_NDELAY            O_NONBLOCK
#define O_SYNC              010000
#define O_FSYNC             O_SYNC
#define O_ASYNC             020000
//=============================================================================
//                  Macro Definition
//=============================================================================
static pthread_mutex_t          g_log_mtx;
#define pmsg(str, argv...)      do{ pthread_mutex_lock(&g_log_mtx); \
                                    printf("%s[%u] " str, __func__, __LINE__, ##argv); \
                                    pthread_mutex_unlock(&g_log_mtx); \
                                }while(0)

#define FCHK(rval, act_func, err_operator)     do{  if((rval = act_func)) {                                 \
                                                        pmsg("call %s: err (%d) !!\n", #act_func, rval);    \
                                                        err_operator;                                       \
                                                    }                                                       \
                                                }while(0)


#define _assert(expression)                                             \
        do{ if(expression) break;                                       \
            printf("%s: %s[#%u]\n", #expression, __FILE__, __LINE__);   \
            while(1);                                                   \
        }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct queue
{
    uint8_t *buffer;
    int capacity;
    int size;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
    unsigned long uxItemSize;
    int oflag;
} queue_t;

struct mq_attr
{
   size_t mq_msgsize;
   size_t mq_maxmsg;
} mq_attr_t;

typedef uint32_t    mqd_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static int          g_is_rtos_wrap_init = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static mqd_t
mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr)
{
    if (oflag & O_CREAT)
    {
        queue_t* q = malloc(sizeof (queue_t));
        if (!q)
            return -1;

        q->buffer = malloc(attr->mq_maxmsg * attr->mq_msgsize);
        if (!q->buffer)
        {
            free(q);
            return -1;
        }

        q->capacity     = attr->mq_maxmsg;
        q->size         = 0;
        q->in           = 0;
        q->out          = 0;
        q->uxItemSize   = attr->mq_msgsize;
        q->oflag        = oflag;

        pthread_mutex_init(&q->mutex, 0);
        pthread_cond_init(&q->cond_full, 0);
        pthread_cond_init(&q->cond_empty, 0);

        return (mqd_t)q;
    }
    else
    {
        return -1;
    }
}

int mq_close(mqd_t msgid)
{
    queue_t* q = (queue_t*) msgid;
    if( q )
    {
        if(q->buffer)   free(q->buffer);
        free(q);
    }
    return 0;
}

int mq_send(mqd_t msgid, const char *msg, size_t msg_len, unsigned int msg_prio)
{
    queue_t* queue = (queue_t*) msgid;

    if ((queue->oflag & O_NONBLOCK) && queue->size == queue->capacity)
        return 0;

    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == queue->capacity)
        pthread_cond_wait(&(queue->cond_full), &(queue->mutex));

    memcpy(queue->buffer + queue->in * queue->uxItemSize, msg, queue->uxItemSize);
    ++ queue->size;
    ++ queue->in;
    queue->in %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_empty));

    return 0;
}

ssize_t mq_receive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio)
{
    queue_t* queue = (queue_t*) msgid;

    if ((queue->oflag & O_NONBLOCK) && queue->size == 0)
        return 0;

    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 0)
        pthread_cond_wait(&(queue->cond_empty), &(queue->mutex));

    memcpy(msg, queue->buffer + queue->out * queue->uxItemSize, queue->uxItemSize);
    -- queue->size;
    ++ queue->out;
    queue->out %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_full));

    return queue->uxItemSize;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
init_rtos_wrap(void)
{
    pthread_mutex_init(&g_log_mtx, 0);
    g_is_rtos_wrap_init = 1;
    return 0;
}

BaseType_t
xTaskCreate(
    TaskFunction_t  pxTaskCode,
    const char      *pcName,
    const uint16_t  usStackDepth,
    void            *pvParameters,
    UBaseType_t     uxPriority,
    TaskHandle_t    *pxCreatedTask)
{
    int             rval = 0;
    pthread_attr_t  attr;

    _assert(g_is_rtos_wrap_init == 1);

    FCHK(rval, pthread_attr_init(&attr), while(1));
    FCHK(rval, pthread_attr_setstacksize(&attr, usStackDepth*4), while(1));

    pthread_mutex_init(&pxCreatedTask->mtx, 0);
    pthread_cond_init(&pxCreatedTask->cond, 0);

    pthread_create(&pxCreatedTask->hThread, &attr, (routing)pxTaskCode, pvParameters);
    return 0;
}

void
vTaskDelete_wrap(
    TaskHandle_t    *xTaskToDelete)
{
    _assert(g_is_rtos_wrap_init == 1);
    pthread_exit(0);
}

void
vTaskSuspend_wrap(
    TaskHandle_t    *xTaskToSuspend)
{
    // event wait
    _assert(g_is_rtos_wrap_init == 1);
    return;
}

void
task_event_wait(
    TaskHandle_t    *xTaskToSuspend)
{
    pthread_mutex_lock(&xTaskToSuspend->mtx);
    pthread_cond_wait(&xTaskToSuspend->cond, &xTaskToSuspend->mtx);

    // only for warp
    pthread_mutex_unlock(&xTaskToSuspend->mtx);
    return;
}

void
vTaskResume_wrap(
    TaskHandle_t    *xTaskToResume)
{
    // event set
    _assert(g_is_rtos_wrap_init == 1);
    pthread_cond_signal(&xTaskToResume->cond);
}

void
vTaskDelay(
    const TickType_t    xTicksToDelay)
{
    _assert(g_is_rtos_wrap_init == 1);

    if( xTicksToDelay )     Sleep(xTicksToDelay);
    else                    Sleep(1);
}

QueueHandle_t
xQueueCreate(
    const UBaseType_t   uxQueueLength,
    const UBaseType_t   uxItemSize)
{
    _assert(g_is_rtos_wrap_init == 1);

    my_queue_info_t *pQ_info = 0;
    struct mq_attr  attr;

    if( !(pQ_info = malloc(sizeof(my_queue_info_t))) )
    {
        return pQ_info;
    }

    pQ_info->node_num = attr.mq_maxmsg  = uxQueueLength;
    pQ_info->node_len = attr.mq_msgsize = uxItemSize;

    // Open a queue with the attribute structure
    pQ_info->hQueue = (uint32_t)mq_open("mq", O_CREAT | O_RDWR, 0, &attr);
    if( pQ_info->hQueue == (mqd_t) -1 )
    {
        free(pQ_info);
        return 0;
    }
    return (QueueHandle_t)pQ_info;
}

void vQueueDelete(
    QueueHandle_t   xQueue)
{
    my_queue_info_t     *pQ_info = (my_queue_info_t*)xQueue;

    if( pQ_info )
    {
        mq_close((mqd_t)pQ_info->hQueue);
        free(pQ_info);
    }
    return;
}

BaseType_t
xQueueSend(
    QueueHandle_t   xQueue,
    void            *pvItemToQueue,
    TickType_t      xTicksToWait)
{
    _assert(g_is_rtos_wrap_init == 1);

    my_queue_info_t     *pQ_info = (my_queue_info_t*)xQueue;
    mq_send((mqd_t)pQ_info->hQueue, pvItemToQueue, pQ_info->node_len, 0);
    return 0;
}

BaseType_t
xQueueReceive(
    QueueHandle_t   xQueue,
    void            *pvBuffer,
    TickType_t      xTicksToWait)
{
    _assert(g_is_rtos_wrap_init == 1);

    unsigned int        prio;
    my_queue_info_t     *pQ_info = (my_queue_info_t*)xQueue;
    return mq_receive((mqd_t)pQ_info->hQueue, pvBuffer, pQ_info->node_len, &prio);
}

SemaphoreHandle_t
xSemaphoreCreateMutex(void)
{
    pthread_mutex_t     *pMtx = 0;

    _assert(g_is_rtos_wrap_init == 1);
    if( !(pMtx = malloc(sizeof(pthread_mutex_t))) )
    {
        return (SemaphoreHandle_t)pMtx;
    }

    pthread_mutex_init(pMtx, 0);
    return (SemaphoreHandle_t)pMtx;
}

void
vSemaphoreDelete(
    SemaphoreHandle_t   xSemaphore)
{
    pthread_mutex_t     *pMtx = (pthread_mutex_t*)xSemaphore;
    if( pMtx )  free(pMtx);
}

BaseType_t
xSemaphoreTake(
    SemaphoreHandle_t   xSemaphore,
    TickType_t          xBlockTime)
{
    pthread_mutex_t     *pMtx = (pthread_mutex_t*)xSemaphore;

    _assert(g_is_rtos_wrap_init == 1);

    if( pMtx )
    {
        pthread_mutex_lock(pMtx);
        return pdTRUE;
    }
    return pdFALSE;
}

BaseType_t
xSemaphoreGive(
    SemaphoreHandle_t   xSemaphore)
{
    pthread_mutex_t     *pMtx = (pthread_mutex_t*)xSemaphore;

    _assert(g_is_rtos_wrap_init == 1);

    if( pMtx )
    {
        pthread_mutex_unlock(pMtx);
        return pdTRUE;
    }
    return pdFALSE;
}














