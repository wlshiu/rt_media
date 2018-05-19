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
#include <stdio.h>

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
typedef struct avi_muxer
{
    media_io_t     out_type;

    union {
        FIL         *pFil;  // file handle of FatFS
        uint8_t     *pStream_buf;
    };

    uint32_t        tracks;

    uint32_t        meida_total_size;
    uint32_t        vframe_cnt;

} avi_muxer_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static avi_muxer_t          g_avi_muxer = {0};
static uint32_t             g_media_data_pos = 0;
static media_codec_desc_t   *g_venc_desc = 0;
static media_codec_desc_t   *g_aenc_desc = 0;
static media_stream_dev_t   *g_vstream_dev = 0;
static media_stream_dev_t   *g_astream_dev = 0;

extern FIL                  g_ff_file;
extern uint32_t             g_meida_raw_buf[32 + (MEDIA_RAW_BUF_SIZE >> 2)];
extern uint32_t             g_is_exit_mplay;
extern media_codec_desc_t   media_codec_desc_jpg_enc;
extern media_stream_dev_t   media_stream_dev_def;

static uint8_t          *g_bs_vbuf = 0;
static uint8_t          *g_bs_abuf = 0;

#if defined(SIMULATE_ENABLE)
    #include "codec/bmp.h"
    static FILE             *g_fout = 0;
    static int              g_frm_idx = 0;

    #define BUF_ENC_VFRAME_MAX          (50 << 10)
    static uint8_t          g_tmp_vbuf[BUF_ENC_VFRAME_MAX] = {0};
    static uint8_t          g_tmp_abuf[BUF_ENC_VFRAME_MAX] = {0};

    #undef FF_ERR_OPERATOR
    #define FF_ERR_OPERATOR         while(1)

    #undef MEDIA_ERR_OPERATOR
    #define MEDIA_ERR_OPERATOR      while(1)
#endif // #if defined(SIMULATE_ENABLE)
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
_avi_mux_get_volum_free_space(
    char    *pVolume_tag)
{
    extern FATFS    g_sd_fatfs;
    FATFS           *pFatfs = &g_sd_fatfs;
    FRESULT         rst = FR_OK;
    DWORD           free_clust = 0;

    // TODO: g_sd_fatfs need be set by user...
    FCHK(rst, f_getfree(pVolume_tag, &free_clust, &pFatfs), FF_ERR_OPERATOR);

    return (uint32_t)free_clust;
}

static int
_avi_mux_out_data(
    avi_muxer_t     *pMuxer,
    uint32_t        position,
    uint8_t         *pBuf,
    uint32_t        buf_len)
{
    int     rval = 0;
    do {
        if( pMuxer->out_type == MEDIA_IO_FILE_HANDLE ||
            pMuxer->out_type == MEDIA_IO_PATH)
        {
            FRESULT         rst = FR_OK;

            // TODO: volume tag need be set by user...
            if( _avi_mux_get_volum_free_space(FS_SD_VOLUME) < buf_len )
            {
                rval = -1;
                break;
            }

            FCHK(rst, f_lseek(pMuxer->pFil, position), FF_ERR_OPERATOR);
            FCHK(rst, f_write(pMuxer->pFil, pBuf, buf_len, (UINT*)&buf_len), FF_ERR_OPERATOR);

            // FCHK(rst, f_sync(pMuxer->pFil), FF_ERR_OPERATOR);
        }
        else if( pMuxer->out_type == MEDIA_IO_MEM )
        {
            memcpy(&pMuxer->pStream_buf[position], pBuf, buf_len);
        }

        if( position == g_media_data_pos )
            g_media_data_pos += buf_len;

    } while(0);
    return rval;
}

static int
_avi_mux_init(
    avi_muxer_t     *pMuxer)
{
    int     rval = 0;

    do{
        uint32_t            length = MEDIA_RAW_BUF_SIZE;
        avi_video_cfg_t     vid_cfg = {0};
        avi_audio_cfg_t     aud_cfg = {0};

        if( pMuxer->tracks & MEDIA_TRACK_VIDEO )
        {
            media_stream_info_t     stream_info = {.stream_type = MEDIA_STREAM_VIDEO,};

            g_vstream_dev = &media_stream_dev_def;

            // stream device initialize and get width/height for avi muxer
            MEDIA_CHK(rval, g_vstream_dev->cb_init(&stream_info), MEDIA_ERR_OPERATOR);

            vid_cfg.vcodec   = AVI_CODEC_MJPG;
            vid_cfg.width    = stream_info.vid.width;
            vid_cfg.height   = stream_info.vid.height;
            vid_cfg.fps      = stream_info.vid.fps;

            g_venc_desc = &media_codec_desc_jpg_enc;

            MEDIA_CHK(rval, g_venc_desc->cb_init(0), MEDIA_ERR_OPERATOR);
        }

        if( pMuxer->tracks & MEDIA_TRACK_AUDIO )
        {
            media_stream_info_t     stream_info = {.stream_type = MEDIA_STREAM_AUDIO,};

            // TODO: mic initial
            g_astream_dev = 0;

            aud_cfg.acodec      = AVI_CODEC_PCM;
            aud_cfg.sample_rate = 44100; // TODO: it should follow real case

            // TODO: audio encode
            g_aenc_desc = 0;
        }

        avi_mux_reset_header((pMuxer->tracks & MEDIA_TRACK_VIDEO) ? &vid_cfg : 0,
                             (pMuxer->tracks & MEDIA_TRACK_AUDIO) ? &aud_cfg : 0,
                             9);

        avi_mux_gen_header((uint8_t*)g_meida_raw_buf, &length);

        _avi_mux_out_data(pMuxer, 0, (uint8_t*)g_meida_raw_buf, length);
    } while(0);

    return rval;
}

static int
_avi_mux_get_video_source(
    uint8_t             **ppFrm_fuf,
    uint32_t            *pFrm_len,
    avi_frame_state_t   *pFrm_state)
{
    int                     rval = 0;
    int                     real_frm_len = *pFrm_len;
    media_stream_info_t     stream_info = {.stream_type = MEDIA_STREAM_VIDEO,};

    MEDIA_CHK(rval, g_vstream_dev->cb_read(&stream_info, ppFrm_fuf, real_frm_len, &real_frm_len, 0), MEDIA_ERR_OPERATOR);

    if( stream_info.frm_state & MEDIA_FRM_STATE_PARTIAL )
        *pFrm_state |= AVI_FRAME_PARTIAL;

    if( stream_info.frm_state & MEDIA_FRM_STATE_END )
        *pFrm_state |= AVI_FRAME_END;

    *pFrm_len = real_frm_len;

    return rval;
}

static int
_avi_mux_get_audio_source(
    uint8_t             **ppBuf,
    uint32_t            *pBuf_len,
    avi_frame_state_t   *pFrm_state)
{
    return 0;
}

static int
_avi_mux_encode_one_vframe(
    avi_mux_ctrl_info_t *pCtrl_info,
    uint8_t             *pFrm_buf,
    uint32_t            frm_buf_len,
    uint8_t             *pBS_buf,
    uint32_t            *pBS_len)
{
    /**
     *  pBS_buf is the member of avi_mux_ctrl_info_t when call avi_mux_one_frame()
     */

    int             rval = 0;
    avi_muxer_t     *pMuxer = (avi_muxer_t*)pCtrl_info->pPrivate_data;

#if defined(SIMULATE_ENABLE)
    // simulate a JPG encoder
    static uint32_t     frame_cnt = 11;
    FILE        *fin = 0;

    do{
        char        jpg_name[128] = {0};
        uint32_t    jpg_len = 0;

        // printf("src_buf= x%x, len= %u\n", (uint32_t)pFrm_buf, frm_buf_len);

        snprintf(jpg_name, 128, "./jpgs/%u.jpeg", frame_cnt++);
        if( !(fin = fopen(jpg_name, "rb")) )
        {
            rval = -1;
            err("open %s fail \n", jpg_name);
            return rval;
        }

        fseek(fin, 0, SEEK_END);
        jpg_len = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        if( BUF_ENC_VFRAME_MAX < jpg_len )
        {
            rval = -1;
            break;
        }

        *pBS_len = fread(pBS_buf, 1, jpg_len, fin);

    } while(0);

    if( fin )       fclose(fin);

    /**
     *  simulate stopping record if g_is_exit_mplay == true
     */
    if( frame_cnt == 50 )
        g_is_exit_mplay = true;
#else
    /**
     *  TODO: get bit-stream after encoding
     */
    // wait encoding ready
    *pBS_len = ;
#endif // #if defined(SIMULATE_ENABLE)
    return rval;
}

static int
_avi_mux_encode_aframe(
    avi_mux_ctrl_info_t *pCtrl_info,
    uint8_t             *pFrm_buf,
    uint32_t            frm_buf_len,
    uint8_t             *pBS_buf,
    uint32_t            *pBS_len)
{
    /**
     *  pBS_buf is the member of avi_mux_ctrl_info_t when call avi_mux_one_frame()
     */

    avi_muxer_t     *pMuxer = (avi_muxer_t*)pCtrl_info->pPrivate_data;
    *pBS_len = 0;
    return 0;
}

static int
_avi_mux_empty_buf(
    avi_mux_ctrl_info_t     *pCtrl_info,
    uint8_t                 *pBS_buf,
    uint32_t                len)
{
    int             rval = 0;
    avi_muxer_t     *pMuxer = (avi_muxer_t*)pCtrl_info->pPrivate_data;

    rval = _avi_mux_out_data(pMuxer, g_media_data_pos, pBS_buf, len);

    return rval;
}

int
meida_fmt_avi_mux_init(
    media_init_t    *pMedia_init)
{
    int         rval = -1;

    media_assert(pMedia_init != 0);

    do {
        g_is_exit_mplay     = false;
        g_media_data_pos    = 0;
        g_avi_muxer.out_type = pMedia_init->out_type;
        g_avi_muxer.tracks   = pMedia_init->tracks;

        if( pMedia_init->out_type == MEDIA_IO_PATH )
        {
            /**
             *  file system access
             */
            FRESULT         rst = FR_OK;

            media_assert(pMedia_init->pPath != 0);

            msg("open file '%s'\n", pMedia_init->pPath);
            FCHK(rst, f_open(&g_ff_file, pMedia_init->pPath, FA_CREATE_ALWAYS | FA_WRITE), FF_ERR_OPERATOR);

            g_avi_muxer.pFil = &g_ff_file;
        }
        else if( pMedia_init->out_type == MEDIA_IO_FILE_HANDLE )
        {
            /**
             *  file handle
             */
            media_assert(pMedia_init->pFile != 0);

            g_avi_muxer.pFil = (FIL*)pMedia_init->pFile;

        }
        else if( pMedia_init->out_type == MEDIA_IO_MEM )
        {
            /**
             *  memory access
             */
            media_assert(pMedia_init->mem.pStream_buf != 0);

            g_avi_muxer.pStream_buf = pMedia_init->mem.pStream_buf;
        }

        rval = _avi_mux_init(&g_avi_muxer);
        if( rval )      break;

    #if defined(SIMULATE_ENABLE)
        g_bs_vbuf = g_tmp_vbuf;
        g_bs_abuf = g_tmp_abuf;
    #else
        g_bs_vbuf = ;
        g_bs_abuf = ;
    #endif // #if defined(SIMULATE_ENABLE)

    } while(0);

    return rval;
}

int
meida_fmt_avi_mux_proc(
    media_init_t    *pMedia_init)
{
    int                     rval = 0;
    uint8_t                 *pFrm_buf = 0;
    uint32_t                frm_buf_len = 0;
    avi_frame_state_t       vfrm_state = AVI_FRAME_NONE;
    avi_frame_state_t       afrm_state = AVI_FRAME_NONE;

    do{ // video encode
        avi_mux_ctrl_info_t     vctrl_info = {0};

        _avi_mux_get_video_source(&pFrm_buf, &frm_buf_len, &vfrm_state);
        if( frm_buf_len )
        {
            vctrl_info.pPrivate_data     = &g_avi_muxer;
            vctrl_info.frm_type          = AVI_FRM_VIDEO;
            vctrl_info.cb_enc_one_frame  = _avi_mux_encode_one_vframe;
            vctrl_info.cb_empty_buf      = _avi_mux_empty_buf;
            vctrl_info.pBS_buf           = g_bs_vbuf;
            MEDIA_CHK(rval, avi_mux_one_frame(&vctrl_info, pFrm_buf, frm_buf_len, vfrm_state), (void)0);
        }

        if( vfrm_state & AVI_FRAME_END )
        {
            g_avi_muxer.vframe_cnt++;
            g_avi_muxer.meida_total_size += vctrl_info.bs_len;
        }
    }while(0);

    do{ // audio encode
        avi_mux_ctrl_info_t     actrl_info = {0};

        _avi_mux_get_audio_source(&pFrm_buf, &frm_buf_len, &afrm_state);
        if( frm_buf_len )
        {
            actrl_info.pPrivate_data     = &g_avi_muxer;
            actrl_info.frm_type          = AVI_FRM_AUDIO;
            actrl_info.cb_enc_one_frame  = _avi_mux_encode_aframe;
            actrl_info.cb_empty_buf      = _avi_mux_empty_buf;
            actrl_info.pBS_buf           = g_bs_abuf;
            MEDIA_CHK(rval, avi_mux_one_frame(&actrl_info, pFrm_buf, frm_buf_len, AVI_FRAME_END), (void)0);
        }

        if( afrm_state & AVI_FRAME_END )
        {
            g_avi_muxer.meida_total_size += actrl_info.bs_len;
        }
    }while(0);

    do {
        if( (vfrm_state & AVI_FRAME_END) || (afrm_state & AVI_FRAME_END) )
        {
            avi_update_info_t   info = {0};
            uint32_t            length = 512;

            info.media_data_size = g_avi_muxer.meida_total_size;
            /**
             *  TODO: need to check the total size is right or not
             */
            info.total_file_size = g_avi_muxer.meida_total_size + avi_mux_get_header_size();
            info.total_frames    = g_avi_muxer.vframe_cnt;

            MEDIA_CHK(rval, avi_mux_update_info(&info), MEDIA_ERR_OPERATOR);
            MEDIA_CHK(rval, avi_mux_gen_header((uint8_t*)g_meida_raw_buf, &length), MEDIA_ERR_OPERATOR);
            MEDIA_CHK(rval, _avi_mux_out_data(&g_avi_muxer, 0, (uint8_t*)g_meida_raw_buf, length), MEDIA_ERR_OPERATOR);

            if( g_is_exit_mplay )
            {
                rval = 1;
                break;
            }
        }
    } while(0);

    return rval;
}

int
meida_fmt_avi_mux_deinit(
    media_init_t    *pMedia_init)
{
    int         rval = 0;

    if( g_avi_muxer.out_type == MEDIA_IO_PATH )
    {
        FRESULT         rst = FR_OK;

        FCHK(rst, f_close(&g_ff_file), (void)0);
        memset(&g_ff_file, 0x0, sizeof(g_ff_file));
    }

    if( g_vstream_dev && g_vstream_dev->cb_deinit )
    {
        MEDIA_CHK(rval, g_vstream_dev->cb_deinit(0), (void)0);
    }

    if( g_astream_dev && g_astream_dev->cb_deinit )
    {
        MEDIA_CHK(rval, g_astream_dev->cb_deinit(0), (void)0);
    }

    g_venc_desc   = 0;
    g_aenc_desc   = 0;
    g_vstream_dev = 0;
    g_astream_dev = 0;



#if defined(SIMULATE_ENABLE)
    do {
        char        *pDump_name = "dump.avi";

        if( !(g_fout = fopen(pDump_name, "wb")) )
        {
            err("open '%s' fail\n", pDump_name);
            break;
        }

        if( g_avi_muxer.out_type == MEDIA_IO_PATH )
        {
            FRESULT     rst = FR_OK;
            uint32_t    filesize = 0;
            uint8_t     *pTmp_buf = 0;

            FCHK(rst, f_open(&g_ff_file, pMedia_init->pPath, FA_OPEN_EXISTING | FA_READ), while(1));

            filesize = f_size(&g_ff_file);

            if( !(pTmp_buf = malloc(filesize)) )
            {
                err("malloc %u fail !\n", filesize);
                break;
            }
            FCHK(rst, f_read(&g_ff_file, pTmp_buf, filesize, (UINT*)&filesize), while(1));
            fwrite(pTmp_buf, 1, filesize, g_fout);

            free(pTmp_buf);
        }
        else if( g_avi_muxer.out_type == MEDIA_IO_MEM )
        {
            fwrite(g_avi_muxer.pStream_buf, 1, g_media_data_pos, g_fout);
        }
    } while(0);

    g_is_exit_mplay = false;

    if( g_fout )    fclose(g_fout);
    g_fout = 0;
#endif // #if defined(SIMULATE_ENABLE)

    memset(&g_avi_muxer, 0x0, sizeof(g_avi_muxer));
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
media_fmt_desc_t        media_fmt_desc_avi_mux =
{
    .media_fmt = MEDIA_FMT_AVI_MUX,
    .cb_init   = meida_fmt_avi_mux_init,
    .cb_proc   = meida_fmt_avi_mux_proc,
    .cb_deinit = meida_fmt_avi_mux_deinit,
};
