/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights rsterved.
 */
/** @file media_proc.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/02
 * @license
 * @description
 */


#include <stdint.h>
#include <stdio.h>
#include "media_comm.h"

/**
 *  Simulation
 */
#if defined(SIMULATE_ENABLE)
    #include "pthread.h"
    #include "freertos_warp.h"
    pthread_mutex_t     g_log_mtx;
#else
    /* OS */
    #include "FreeRTOS.h"
    #include "task.h"
    #include "semphr.h"
#endif // #if defined(SIMULATE_ENABLE)
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern media_fmt_desc_t        media_fmt_desc_avi_demux;
extern media_fmt_desc_t        media_fmt_desc_avi_mux;

static media_fmt_desc_t     *g_fmt_table[] =
{
    0,
    &media_fmt_desc_avi_demux,
    &media_fmt_desc_avi_mux,
};

static int                      g_is_media_init = 0;
static TaskHandle_t             g_htask_meida;
static SemaphoreHandle_t        g_meida_mtx;

QueueHandle_t           g_mediaQ;
media_codec_desc_t      *g_vcodec_desc = 0;
media_codec_desc_t      *g_acodec_desc = 0;

/**
 *  flag for stopping event happen
 */
uint32_t                        g_is_exit_mplay = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_media_def_vid_render(media_render_info_t *pInfo)
{
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
static void
_task_media(void *argv)
{
    do {
        g_meida_mtx = xSemaphoreCreateMutex();
        if( !g_meida_mtx )
        {
            err("%s", "create mutex fail !\n");
            break;
        }

        while(1)
        {
            media_init_t    media_init = {0};

            if( xSemaphoreTake(g_meida_mtx, portMAX_DELAY) == pdFALSE )
            {
                vTaskDelay(5);
                continue;
            }

            if( xQueueReceive(g_mediaQ, &media_init, portMAX_DELAY) )
            {
                int                 rval = 0;
                media_fmt_desc_t    *pFmt_desc_cur = g_fmt_table[media_init.media_fmt];

                if( pFmt_desc_cur && pFmt_desc_cur->media_fmt == media_init.media_fmt )
                {
                    media_init.cb_vid_render = _media_def_vid_render;

                    rval = pFmt_desc_cur->cb_init(&media_init);

                    while( rval == 0 )
                    {
                        rval = pFmt_desc_cur->cb_proc(&media_init);
                    }

                    pFmt_desc_cur->cb_deinit(&media_init);
                }
                else
                {
                    err("unknown format type %d or wrong fmt_desc !\n", media_init.media_fmt);
                }
            }

            xSemaphoreGive(g_meida_mtx);
        }

    } while(0);

    /**
     *  Delete self (NULL means self)
     */
    vTaskDelete(NULL);
}

void meida_init(void)
{
    do {
        if( g_is_media_init )
        {
            vTaskResume(g_htask_meida);
            break;
        }

        g_mediaQ = xQueueCreate(1, sizeof(media_init_t));

        xTaskCreate(_task_media, "MPly",
                    (512 >> 2), // stack size = sizeof(unsigned int) * (512 >> 2)
                    NULL,
                    0,
                    &g_htask_meida);

        g_is_media_init = 1;
    } while(0);

    return;
}

void meida_deinit(void)
{
    if( g_is_media_init )
        vTaskSuspend(g_htask_meida);

    return;
}

