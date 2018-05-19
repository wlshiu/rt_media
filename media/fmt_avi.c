/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file fmt_avi.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/03
 * @license
 * @description
 */


#include <stdint.h>
#include <stdbool.h>

#include "ff.h"
#include "ff_macro.h"
#include "media_comm.h"
#include "demux/avi/avi.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define FF_ERR_OPERATOR         (return rst)
#define MEDIA_ERR_OPERATOR      (return rval)
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct avi_io
{
    media_io_t     in_type;

    union {
        FIL         *pFil;  // file handle of FatFS
        uint8_t     *pStream_buf;
    };
} avi_io_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static avi_io_t     g_avi_io = {0};
static bool         g_avi_braking = true;
static bool         g_avi_is_reset = true;

static uint32_t     g_media_data_offset = 0;
static uint32_t     g_media_data_pos = 0;
static uint32_t     g_media_filesize = 0;
static uint32_t     g_meida_fps = 0;
static uint32_t     g_meida_sample_rate = 0;
static uint32_t     g_meida_frm_cnt = 0;


extern FIL              g_ff_file;
extern uint32_t         g_meida_raw_buf[32 + (MEDIA_RAW_BUF_SIZE >> 2)];

extern media_codec_desc_t       media_codec_desc_jpg_dec;
extern media_codec_desc_t       media_codec_desc_wav_dec;

#if defined(SIMULATE_ENABLE)
    extern media_codec_desc_t      media_codec_desc_jpg_sim_dec;

    #undef FF_ERR_OPERATOR
    #define FF_ERR_OPERATOR         while(1)

    #undef MEDIA_ERR_OPERATOR
    #define MEDIA_ERR_OPERATOR      while(1)
#endif // #if defined(SIMULATE_ENABLE)
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_avi_misc_proc(
    avi_demux_ctrl_info_t     *pCtrl_info)
{
    /* something to break demux*/
    return 0;
}

static int
_avi_fill_buf(
    avi_demux_ctrl_info_t   *pCtrl_info,
    uint8_t                 *pBuf,
    uint32_t                *pLen)
{
    int         rval = 0;
    avi_io_t    *pAvi_io = (avi_io_t*)pCtrl_info->pPrivate_data;

    do {
        if( pAvi_io->in_type == MEDIA_IO_FILE_HANDLE ||
            pAvi_io->in_type == MEDIA_IO_PATH)
        {
            FRESULT         rst = FR_OK;

            if( f_eof(pAvi_io->pFil) )
            {
                rval = 1;
                break;
            }

            FCHK(rst, f_read(pAvi_io->pFil, pBuf, *pLen, (UINT*)pLen), FF_ERR_OPERATOR);
        }
        else if( pAvi_io->in_type == MEDIA_IO_MEM )
        {
            if( g_media_filesize <= g_media_data_pos )
            {
                rval = 1;
                break;
            }
            memcpy(pBuf, &pAvi_io->pStream_buf[g_media_data_pos], *pLen);
            g_media_data_pos += (*pLen);
        }
    } while(0);

    return (rval) ? AVI_END_OF_STREAM : 0;
}

static int
_avi_frame_state(
    avi_demux_ctrl_info_t   *pCtrl_info,
    avi_media_info_t        *pMedia_info,
    avi_frame_info_t        *pFrm_info)
{
    int     rval = 0;

    do {
        media_codec_desc_t      *pCodec_desc = 0;
        media_codec_set_t       codec_setting = {0};

        //----------------------------------
        // check CODEC
        if( pMedia_info->codec == AVI_CODEC_PCM )
        {
            pCodec_desc = &media_codec_desc_wav_dec;

            codec_setting.codec_type = MEDIA_CODEC_WAV_DEC;
            g_meida_sample_rate = pMedia_info->aud.sample_rate;
        }
        else if( pMedia_info->codec == AVI_CODEC_MJPG )
        {
            pCodec_desc = &media_codec_desc_jpg_dec;

            codec_setting.codec_type = MEDIA_CODEC_JPG_DEC;
            g_meida_fps = (uint32_t)pMedia_info->vid.fps;
        }
        else
        {
            err("unknown codec (x%x) type !\n", pMedia_info->codec);
            rval = -1;
            break;
        }

        codec_setting.pStream_buf    = pFrm_info->pFrame_addr;
        codec_setting.stream_buf_len = pFrm_info->frame_len;
        codec_setting.frm_state      = pFrm_info->frm_state;

        //----------------------------------
        // init CODEC
        if( pFrm_info->frm_state & AVI_FRAME_START )
        {
            MEDIA_CHK(rval, pCodec_desc->cb_init(&codec_setting), MEDIA_ERR_OPERATOR);
        }

    #if 0 // check alignment
        if( pFrm_info->frame_len & 0x3 )
            msg("len align: %d\n", pFrm_info->frame_len & 0x3);

        if( (uint32_t)pFrm_info->pFrame_addr & 0x3 )
            msg("addr align: %d\n", (uint32_t)pFrm_info->pFrame_addr & 0x3);
    #endif // 0

        //----------------------------------
        // process bit stream
        if( pFrm_info->frm_state & AVI_FRAME_PARTIAL )
        {
            MEDIA_CHK(rval, pCodec_desc->cb_proc(&codec_setting), MEDIA_ERR_OPERATOR);
        }

        if( pFrm_info->frm_state & AVI_FRAME_END )
        {
            MEDIA_CHK(rval, pCodec_desc->cb_proc(&codec_setting), MEDIA_ERR_OPERATOR);
            MEDIA_CHK(rval, pCodec_desc->cb_deinit(&codec_setting), MEDIA_ERR_OPERATOR);

            g_avi_braking = false;

            g_meida_frm_cnt++;
        }

        //--------------------------------
        // check EOS
        if( pFrm_info->frm_state & AVI_FRAME_EOS )
        {
            g_avi_braking = false;
            rval = AVI_FRAME_EOS;
            break;
        }
    } while(0);

    return rval;
}

int
meida_fmt_avi_init(
    media_init_t    *pMedia_init)
{
    int         rval = -1;

    media_assert(pMedia_init != 0);

    do {
        FRESULT         rst = FR_OK;
        UINT            len = MEDIA_RAW_BUF_SIZE;

        g_media_filesize  = 0;
        g_media_data_pos  = 0;
        g_avi_io.in_type = pMedia_init->in_type;

        if( pMedia_init->in_type == MEDIA_IO_PATH )
        {
            /**
             *  file system access
             */
            media_assert(pMedia_init->pPath != 0);

            msg("open file '%s'\n", pMedia_init->pPath);
            FCHK(rst, f_open(&g_ff_file, pMedia_init->pPath, FA_OPEN_EXISTING | FA_READ), FF_ERR_OPERATOR);

            FCHK(rst, f_read(&g_ff_file, (void*)g_meida_raw_buf, MEDIA_RAW_BUF_SIZE, &len), FF_ERR_OPERATOR);

            g_avi_io.pFil = &g_ff_file;
        }
        else if( pMedia_init->in_type == MEDIA_IO_FILE_HANDLE )
        {
            /**
             *  file handle
             */
            media_assert(pMedia_init->pFile != 0);

            g_avi_io.pFil = (FIL*)pMedia_init->pFile;
            FCHK(rst, f_read(g_avi_io.pFil, (void*)g_meida_raw_buf, MEDIA_RAW_BUF_SIZE, &len), FF_ERR_OPERATOR);
        }
        else if( pMedia_init->in_type == MEDIA_IO_MEM )
        {
            /**
             *  memory access
             */
            media_assert(pMedia_init->mem.pStream_buf != 0);

            g_avi_io.pStream_buf = pMedia_init->mem.pStream_buf;
            memcpy(g_meida_raw_buf, g_avi_io.pStream_buf, MEDIA_RAW_BUF_SIZE);

            g_media_data_pos += MEDIA_RAW_BUF_SIZE;
        }

        MEDIA_CHK(rval, avi_parse_header((uint32_t*)g_meida_raw_buf, MEDIA_RAW_BUF_SIZE, &g_media_filesize, &g_media_data_offset), MEDIA_ERR_OPERATOR);


        if( pMedia_init->in_type == MEDIA_IO_PATH ||
            pMedia_init->in_type == MEDIA_IO_FILE_HANDLE )
        {
            FCHK(rst, f_lseek(g_avi_io.pFil, g_media_data_offset), FF_ERR_OPERATOR);
        }
        else if( pMedia_init->in_type == MEDIA_IO_MEM )
        {
            if( g_media_filesize != pMedia_init->mem.stream_size )
            {
                err("avi file size not match (%u, %u)!\n",
                    g_media_filesize, pMedia_init->mem.stream_size);
                rval = -1;
                break;
            }
        }

    } while(0);

    return rval;
}

int
meida_fmt_avi_proc(
    media_init_t    *pMedia_init)
{
    int                       rval = 0;
    avi_demux_ctrl_info_t     ctrl_info = {0};

    g_avi_braking = true;

    ctrl_info.pPrivate_data     = &g_avi_io;
    ctrl_info.cb_frame_state    = _avi_frame_state;
    ctrl_info.cb_fill_buf       = _avi_fill_buf;
    ctrl_info.cb_misc_proc      = _avi_misc_proc;
    ctrl_info.pRing_buf         = (uint8_t*)g_meida_raw_buf;
    ctrl_info.ring_buf_size     = MEDIA_RAW_BUF_SIZE;
    MEDIA_CHK(rval, avi_demux_media_data(&ctrl_info, (uint32_t)g_avi_is_reset, (uint32_t*)&g_avi_braking), (void)0);

    do {    // video render
        media_render_info_t     info = {0};
        info.fps        = g_meida_fps;
        info.frm_index  = g_meida_frm_cnt;
        pMedia_init->cb_vid_render(&info);
    } while(0);

    g_avi_is_reset = false;
    return rval;
}

int
meida_fmt_avi_deinit(
    media_init_t    *pMedia_init)
{
    int         rval = 0;
    do {
        if( g_avi_io.in_type == MEDIA_IO_PATH )
        {
            FRESULT         rst = FR_OK;

            FCHK(rst, f_close(&g_ff_file), (void)0);
            memset(&g_ff_file, 0x0, sizeof(g_ff_file));
        }

        g_avi_is_reset      = true;
        g_meida_fps         = 0;
        g_meida_sample_rate = 0;
        g_meida_frm_cnt     = 0;

        memset(&g_avi_io, 0x0, sizeof(g_avi_io));
    } while(0);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
media_fmt_desc_t        media_fmt_desc_avi_demux =
{
    .media_fmt = MEDIA_FMT_AVI,
    .cb_init   = meida_fmt_avi_init,
    .cb_proc   = meida_fmt_avi_proc,
    .cb_deinit = meida_fmt_avi_deinit,
};
