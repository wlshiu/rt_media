#include <stdio.h>
#include <stdlib.h>

#include "freertos_warp.h"


#include "ff.h"
#include "ff_macro.h"

#include "media_comm.h"
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
//=============================================================================
uint32_t                g_meida_raw_buf[32 + (MEDIA_RAW_BUF_SIZE >> 2)] = {0};
FATFS                   g_sd_fatfs = {0};
FIL                     g_ff_file = {0};

extern QueueHandle_t     g_mediaQ;

#if defined(SIMULATE_ENABLE)
#include <windows.h>
#include "pthread.h"
pthread_mutex_t     g_log_mtx;
#endif // defined
//=============================================================================
//                  Private Function Definition
//=============================================================================
#define TEST_AVI_MUX        1

#define FF_PATH_AVI         "ttt.avi" // "test_mj.avi"
#define FF_PATH_AVI_MUX     "mj_mux.avi"

static uint8_t      *g_media_raw = 0;
static uint32_t     g_media_filesize = 0;

static int
_send_media_msg(void)
{
    uint32_t        cnt = 10;

    while( cnt--)   Sleep(10);

    {
        media_init_t    media_init[] =
        {
            [0] = {
                .media_fmt = MEDIA_FMT_AVI,
                .in_type   = MEDIA_IO_PATH,
                .pPath     = FS_SD_VOLUME FF_PATH_AVI,
            },
            [1] = {
                .media_fmt = MEDIA_FMT_AVI,
                .in_type   = MEDIA_IO_MEM,
                .mem.pStream_buf  = g_media_raw,
                .mem.stream_size  = g_media_filesize,
            },
            [2] = {
                .media_fmt = MEDIA_FMT_AVI_MUX,
                .out_type  = MEDIA_IO_PATH,
                .pPath     = FS_SD_VOLUME FF_PATH_AVI_MUX,
                .tracks    = MEDIA_TRACK_VIDEO,
            },
            [3] = {
                .media_fmt = MEDIA_FMT_AVI_MUX,
                .out_type  = MEDIA_IO_MEM,
                .mem.pStream_buf  = g_media_raw,
                .mem.stream_size  = g_media_filesize,
                .tracks    = MEDIA_TRACK_VIDEO,
            },
        };

        #if (TEST_AVI_MUX)
        xQueueSend(g_mediaQ, &media_init[2], portMAX_DELAY);
        #else
        xQueueSend(g_mediaQ, &media_init[0], portMAX_DELAY);
        #endif
    }
    return 0;
}

static FRESULT
_fat_simulation_test(
    char        *pPath,
    uint8_t     *pBuf,
    uint32_t    buf_size)
{
    FRESULT         rst = FR_OK;
    UINT            len = 0;
    uint8_t         *pBuf_cmp = 0;
    char            full_path[64] = {0};

    snprintf(full_path, 64, "%s%s", FS_SD_VOLUME, pPath);

    FCHK(rst, f_open(&g_ff_file, full_path, FA_CREATE_NEW | FA_WRITE), while(1));

    printf("create '%s'\n", full_path);
    do{
        if( !(pBuf_cmp = malloc(buf_size)) )
        {
            err("malloc %u fail \n", buf_size);
            break;
        }

        FCHK(rst, f_write(&g_ff_file, pBuf, buf_size, &len), while(1));
        FCHK(rst, f_sync(&g_ff_file), while(1));
        FCHK(rst, f_close(&g_ff_file), while(1));

        #if 0
        FCHK(rst, f_open(&g_ff_file, full_path, FA_OPEN_EXISTING | FA_READ), while(1));
        FCHK(rst, f_read(&g_ff_file, pBuf_cmp, buf_size, &len), while(1));
        FCHK(rst, f_lseek(&g_ff_file, 0), while(1));

        if( memcmp(pBuf, pBuf_cmp, buf_size) )
        {
            err("write data '%s' fail \n", pPath);
            break;
        }
        #endif
    }while(0);

    if( pBuf_cmp )      free(pBuf_cmp);

#if 0
    FCHK(rst, f_close(&g_ff_file), while(1));
#endif // 0

    return rst;
}

static int
_init_fat(void)
{
    static uint8_t      g_wrok_buf[1024] = {0};
    FRESULT             rst = FR_OK;

    do {
        rst = f_mount(&g_sd_fatfs, _T(FS_SD_VOLUME), 1);
        if( rst != FR_OK )
        {
            if( rst != FR_NO_FILESYSTEM )
            {
                err("%s", "fat mount err !\n");
                break;
            }

            FCHK(rst, f_mkfs(_T(FS_SD_VOLUME), FM_ANY, 0, g_wrok_buf, sizeof(g_wrok_buf)), while(1));
            FCHK(rst, f_mount(&g_sd_fatfs, _T(FS_SD_VOLUME), 1), while(1));
        }

        #if defined(SIMULATE_ENABLE) // test
        do{
        #if (TEST_AVI_MUX)
            g_media_filesize = (2 << 20);
            if( !(g_media_raw = malloc(g_media_filesize)) )
            {
                err("mallco %u fail\n", g_media_filesize);
                break;
            }
        #else
            FILE            *fin = 0;
            char            *pPath = FF_PATH_AVI; //"test_mj.avi";
            char            *pPath_ff = FF_PATH_AVI;


            if( !(fin = fopen(pPath, "rb")) )
            {
                err("open %s fail \n", pPath);
                break;
            }

            fseek(fin, 0, SEEK_END);
            g_media_filesize = ftell(fin);
            fseek(fin, 0, SEEK_SET);

            if( g_media_raw )   free(g_media_raw);

            if( !(g_media_raw = malloc(g_media_filesize)) )
            {
                err("mallco %u fail\n", g_media_filesize);
                break;
            }
            fread(g_media_raw, 1, g_media_filesize, fin);
            fclose(fin);

            rst = _fat_simulation_test(pPath_ff, g_media_raw, g_media_filesize);
            if( rst != FR_OK )
            {
                err("%s", "err !!!!");
                break;
            }
        #endif // #if (TEST_AVI_MUX)

        }while(0);

        if( rst )       break;
        #endif // #if defined(SIMULATE_ENABLE)

    } while(0);

    return (rst) ? -1 : 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
extern void meida_init(void);
int main()
{
#if defined(SIMULATE_ENABLE)
    init_rtos_wrap();
#endif

    if( _init_fat() )
    {
        err("%s", "init fat fail !\n");
        return 1;
    }

    meida_init();

    #if defined(SIMULATE_ENABLE)
    pthread_mutex_init(&g_log_mtx, 0);

    _send_media_msg();
    #endif // #if defined(SIMULATE_ENABLE)

    while(1)    Sleep(0xFFFFFFFF);

    return 0;
}
