/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file media_comm.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/03
 * @license
 * @description
 */

#ifndef __media_comm_H_wL8dRTV5_liPg_H1TD_sRXH_u0Nkow0bsr1F__
#define __media_comm_H_wL8dRTV5_liPg_H1TD_sRXH_u0Nkow0bsr1F__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 *  Simulation
 */
#define SIMULATE_ENABLE
//=============================================================================
//                  Constant Definition
//=============================================================================
#define FS_SD_VOLUME                 "0:"

#define MEDIA_RAW_BUF_SIZE            512

/**
 *  supporting type of media container
 */
typedef enum media_fmt
{
    MEDIA_FMT_UNKOWN    = 0,
    MEDIA_FMT_AVI,
    MEDIA_FMT_AVI_MUX,

} media_fmt_t;

/**
 *  supporting type of media codec
 */
typedef enum media_codec
{
    MEDIA_CODEC_UNKNOWN     = 0,
    MEDIA_CODEC_JPG_DEC,
    MEDIA_CODEC_JPG_ENC,
    MEDIA_CODEC_WAV_DEC,

} media_codec_t;

typedef enum media_stream_type
{
    MEDIA_STREAM_UNKNOWN        = 0,
    MEDIA_STREAM_VIDEO,
    MEDIA_STREAM_AUDIO,

} media_stream_type_t;

/**
 *  media source type
 */
typedef enum media_io
{
    MEDIA_IO_UNKNOWN       = 0,
    MEDIA_IO_PATH,
    MEDIA_IO_FILE_HANDLE,
    MEDIA_IO_NOR_FLASH, // not ready
    MEDIA_IO_MEM,   // all in memory
} media_io_t;

/**
 *  media frame state
 */
#define MEDIA_FRM_STATE_UNKNOWN         0
#define MEDIA_FRM_STATE_START           (0x1 << 1)
#define MEDIA_FRM_STATE_PARTIAL         (0x1 << 2)
#define MEDIA_FRM_STATE_END             (0x1 << 3)
//=============================================================================
//                  Macro Definition
//=============================================================================
#define media_assert(expression)                                        \
        do{ if(expression) break;                                       \
            printf("%s: %s[#%u]\n", #expression, __FILE__, __LINE__);   \
            while(1);                                                   \
        }while(0)

#if defined(SIMULATE_ENABLE)
    #include "pthread.h"
    extern pthread_mutex_t          g_log_mtx;
    #define msg(str, argv...)       do{ pthread_mutex_lock(&g_log_mtx); \
                                        printf("%s[%u] " str, __func__, __LINE__, ##argv); \
                                        pthread_mutex_unlock(&g_log_mtx); \
                                    }while(0)

    #define trace()                 msg("%s", "\n");
#else
    #define msg(str, argv...)       printf("%s[%u] " str, __func__, __LINE__, ##argv)
#endif

#define err(str, argv...)                           do{ msg(str, ##argv); while(1); }while(0)

#define MEDIA_CHK(rval, act_func, err_operator)     do{ if((rval = act_func)) {                                 \
                                                            msg("call %s: err (%d) !!\n", #act_func, rval);     \
                                                            err_operator;                                       \
                                                        }                                                       \
                                                    }while(0)

#define MEDIA_4CC(a, b, c, d)       (((d) << 24) | ((c) <<16) | ((b) << 8) | (a))
//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct media_render_info
{
    uint32_t    fps;
    uint32_t    frm_index;

} media_render_info_t;

/**
 *  media initial info
 */
typedef struct media_init
{
    media_fmt_t     media_fmt;

    union {
        media_io_t      in_type;
        media_io_t      out_type;
    };

    union {
        char        *pPath;
        void        *pFile;

        struct {
            uint8_t     *pStream_buf;
            uint32_t    stream_size;
        } mem;
    };

#define MEDIA_TRACK_VIDEO       (0x1)
#define MEDIA_TRACK_AUDIO       (0x1 << 1)

    uint32_t        tracks;

    int (*cb_vid_render)(media_render_info_t *pInfo);

} media_init_t;

/**
 *  media codec info
 */
typedef struct media_codec_set
{
    media_codec_t       codec_type;

    uint32_t            frm_state;

    uint8_t             *pStream_buf;
    uint32_t            stream_buf_len;

} media_codec_set_t;

/**
 *  media operator description of media container
 */
typedef struct media_fmt_desc
{
    media_fmt_t     media_fmt;

    int (*cb_init)(media_init_t *pMedia_init);
    int (*cb_proc)(media_init_t *pMedia_init);
    int (*cb_deinit)(media_init_t *pMedia_init);

} media_fmt_desc_t;

/**
 *  media operator description of media codec
 */
typedef struct media_codec_desc
{
    media_codec_t       codec_type;

    int (*cb_init)(media_codec_set_t *pCodec_setting);
    int (*cb_proc)(media_codec_set_t *pCodec_setting);
    int (*cb_deinit)(media_codec_set_t *pCodec_setting);

} media_codec_desc_t;


typedef struct media_stream_info
{
    media_stream_type_t     stream_type;
    uint32_t                frm_state;

    union {
        struct {
            uint32_t    width;
            uint32_t    height;
            uint32_t    fps;
        } vid;

        struct {
            uint32_t    sample_rate;
        } aud;
    };

} media_stream_info_t;

/**
 *  descriptor of media stream device
 */
typedef struct media_stream_dev
{
#define MEIDA_STREAM_DEF    MEDIA_4CC('S', 'D', 'E', 'F')

    uint32_t    dev_type;

    int     (*cb_init)(media_stream_info_t *pInfo);
    int     (*cb_deinit)(media_stream_info_t *pInfo);

    int     (*cb_read)(media_stream_info_t *pInfo, uint8_t **ppBuf, int request_len, int *pReal_len, void *pExtra_info);
    int     (*cb_write)(media_stream_info_t *pInfo, uint8_t *pBuf, int request_len, int *pReal_len, void *pExtra_info);

} media_stream_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================




#ifdef __cplusplus
}
#endif

#endif
