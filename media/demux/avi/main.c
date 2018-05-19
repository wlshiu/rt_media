/*
 * Copyright (c) 2018-, Wei-Lun Hsu. All Rights Reserved.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the
 * License.
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See the GNU General Public License version 3 for more details.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/03/22
 * @license GNU GENERAL PUBLIC LICENSE Version 3
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avi.h"

#include "riff.h"

#include <windows.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define MAX_VFRAME_NUM              30

#define BUF_ENC_VFRAME_MAX          (50 << 10)

#define FILENAME_MOVI_PAYLAOD       "movi_payload.tmp"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, args...)           do{printf("%s[%u] " str, __func__, __LINE__, ##args);}while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct chunk
{
    uint32_t    fcc;
    uint32_t    length;
} chunk_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t      g_bs_vbuf[BUF_ENC_VFRAME_MAX] = {0};
static uint8_t      g_bs_abuf[BUF_ENC_VFRAME_MAX] = {0};
static uint8_t      g_buf[1024] = {0};
static bool         g_avi_braking = true;

static uint32_t     g_media_data_offset = 0;

static FILE         *g_fout = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

static int
_test_reload()
{
    FILE        *fin = 0;
    uint32_t    file_size = 0;
    uint8_t     buf[1 << 20] = {0};

    if( !(fin = fopen("./test.avi", "rb")) )
    {
        err("open %s fail \n", "./test.avi");
        return -1;
    }
    fseek(fin, 0l, SEEK_END);
    file_size = ftell(fin);
    fseek(fin, 0l, SEEK_SET);

    fread(buf, 1, file_size, fin);
    fclose(fin);

    avi_parse_header((uint32_t*)buf, file_size, 0);
    return 0;
}

static int
_test_gen_header()
{
    FILE        *fout = 0;
    uint8_t     buf[512] = {0};
    uint32_t    hdr_size = 512;

    memset(buf, 0xFF, hdr_size);
    avi_mux_gen_header(buf, &hdr_size);

    if( !(fout = fopen("./head.avi", "wb")) )
    {
        err("open %s fail\n", "./head.avi");
        return -1;
    }

    fwrite(buf, 1, hdr_size, fout);
    fclose(fout);
    fout = 0;

    #if 0
    avi_parse_header((uint32_t*)buf, hdr_size, 0);
    memset(buf, 0xFF, hdr_size);
    avi_mux_gen_header(buf, &hdr_size);
    if( !(fout = fopen("./head_reload.avi", "wb")) )
    {
        err("open %s fail\n", "./head.avi");
        break;
    }

    fwrite(buf, 1, hdr_size, fout);
    fclose(fout);
    fout = 0;
    #endif

    return 0;
}


static int
_get_video_source(
    uint8_t             **ppBuf,
    uint32_t            *pBuf_len,
    avi_frame_state_t   *pFrm_state)
{
    *ppBuf = (uint8_t*)0x878787;
    *pBuf_len = 1111;
    *pFrm_state = AVI_FRAME_START | AVI_FRAME_END;
    return 0;
}

static int
_get_audio_source(
    uint8_t             **ppBuf,
    uint32_t            *pBuf_len,
    avi_frame_state_t   *pFrm_state)
{
    *ppBuf = (uint8_t*)0x787878;
    *pBuf_len = 0;
    return 0;
}


static int
_encode_one_vframe(
    avi_mux_ctrl_info_t *pCtrl_info,
    uint8_t             *pFrm_buf,
    uint32_t            frm_buf_len,
    uint8_t             *pBS_buf,
    uint32_t            *pBS_len)
{
    static uint32_t     frame_cnt = 11;

    int         rval = 0;
    FILE        *fin = 0;

    do{
        char        jpg_name[128] = {0};
        uint32_t    jpg_len = 0;

        // printf("src_buf= x%x, len= %u\n", (uint32_t)pSrc_buf, src_buf_len);

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

    return 0;
}

static int
_encode_one_aframe(
    avi_mux_ctrl_info_t *pCtrl_info,
    uint8_t             *pFrm_buf,
    uint32_t            frm_buf_len,
    uint8_t             *pBS_buf,
    uint32_t            *pBS_len)
{
    *pBS_len = 0;
    return 0;
}

static int
_empty_buf(
    avi_mux_ctrl_info_t     *pCtrl_info,
    uint8_t                 *pBS_buf,
    uint32_t                len)
{
    int         rval = 0;
    FILE        *fout = *(FILE**)pCtrl_info->pPrivate_data;

    fwrite(pBS_buf, 1, len, fout);
    return rval;
}

static void
_test_mux(
    char    *pOut_path)
{
    avi_video_cfg_t     vid_cfg = {0};

    vid_cfg.vcodec   = AVI_CODEC_MJPG;
    vid_cfg.width    = 800;
    vid_cfg.height   = 600;
    vid_cfg.fps      = 10;
    avi_mux_reset_header(&vid_cfg, 0, 9);

    {
        uint32_t    length = 512;
        memset(g_buf, 0x0, length);
        avi_mux_gen_header(g_buf, &length);

        if( !(g_fout = fopen(pOut_path, "wb")) )
        {
            err("open %s fail\n", pOut_path);
            return;
        }

        fwrite(g_buf, 1, length, g_fout);
    }

    do {
        uint8_t                 *pFrm_buf = 0;
        uint32_t                frm_buf_len = 0;
        uint32_t                vframe_cnt = 0;
        uint32_t                media_total_size = 0;
        avi_mux_ctrl_info_t     vctrl_info = {0};
        avi_mux_ctrl_info_t     actrl_info = {0};

        do {
            avi_frame_state_t       vfrm_state = AVI_FRAME_NONE;
            avi_frame_state_t       afrm_state = AVI_FRAME_NONE;
            avi_update_info_t       info = {0};

            _get_video_source(&pFrm_buf, &frm_buf_len, &vfrm_state);
            if( frm_buf_len )
            {
                vctrl_info.pPrivate_data     = &g_fout;
                vctrl_info.frm_type          = AVI_FRM_VIDEO;
                vctrl_info.cb_enc_one_frame  = _encode_one_vframe;
                vctrl_info.cb_empty_buf      = _empty_buf;
                vctrl_info.pBS_buf           = g_bs_vbuf;
                avi_mux_one_frame(&vctrl_info, pFrm_buf, frm_buf_len, vfrm_state);
            }

            if( vfrm_state & AVI_FRAME_END )
            {
                vframe_cnt++;
                media_total_size += vctrl_info.bs_len;
            }

            _get_audio_source(&pFrm_buf, &frm_buf_len, &afrm_state);
            if( frm_buf_len )
            {
                actrl_info.pPrivate_data     = &g_fout;
                actrl_info.frm_type          = AVI_FRM_AUDIO;
                actrl_info.cb_enc_one_frame  = _encode_one_aframe;
                actrl_info.cb_empty_buf      = _empty_buf;
                actrl_info.pBS_buf           = g_bs_abuf;
                avi_mux_one_frame(&actrl_info, pFrm_buf, frm_buf_len, AVI_FRAME_END);
            }

            if( afrm_state == AVI_FRAME_END )
                media_total_size += actrl_info.bs_len;

            if( vfrm_state == AVI_FRAME_END )
            {
                uint32_t    cur_pos = 0;
                uint32_t    length = 512;

                info.media_data_size = media_total_size;
                /**
                 *  TODO: need to check the total size is right or not
                 */
                info.total_file_size = media_total_size + avi_mux_get_header_size();
                info.total_frames    = vframe_cnt;
                avi_mux_update_info(&info);

                cur_pos = ftell(g_fout);
                fseek(g_fout, 0, SEEK_SET);

                memset(g_buf, 0x0, length);
                avi_mux_gen_header(g_buf, &length);

                fwrite(g_buf, 1, length, g_fout);
                fseek(g_fout, cur_pos, SEEK_SET);
            }
        } while( vframe_cnt < MAX_VFRAME_NUM );

    } while(0);

    if( g_fout )      fclose(g_fout);
    g_fout = 0;

    return;
}


static int
_misc_proc(
    avi_demux_ctrl_info_t     *pCtrl_info)
{
    return 0;
}


static int
_fill_buf(
    avi_demux_ctrl_info_t   *pCtrl_info,
    uint8_t                 *pBuf,
    uint32_t                *pLen)
{
    int         rval = 0;
    FILE        *fin = *(FILE**)pCtrl_info->pPrivate_data;

    rval = fread(pBuf, 1, *pLen, fin);
    *pLen = rval;

    return (rval) ? 0 : AVI_END_OF_STREAM;
}


static int
_frame_state(
    avi_demux_ctrl_info_t   *pCtrl_info,
    avi_media_info_t        *pMedia_info,
    avi_frame_info_t        *pFrm_info)
{
    static int      total_size = 0;
    static FILE     *fdump = 0;

    if( pFrm_info->frm_state & AVI_FRAME_START )
    {
        static int      i = 0;

        if( !fdump )
        {
            char    name[64] = {0};
            snprintf(name, 64, "%02d.jpg", i++);
            fdump = fopen(name, "wb");
        }

        printf("real frm offset= x%04x\n", g_media_data_offset + pFrm_info->frame_offset);
    }

#if 0
    if( pFrm_info->frame_len & 0x3 )
        printf("len align: %d\n", pFrm_info->frame_len & 0x3);

    if( (uint32_t)pFrm_info->pFrame_addr & 0x3 )
        printf("addr align: %d\n", (uint32_t)pFrm_info->pFrame_addr & 0x3);
#endif // 0

    if( pFrm_info->frm_state & AVI_FRAME_PARTIAL )
    {
        if( fdump )
            fwrite(pFrm_info->pFrame_addr, 1, pFrm_info->frame_len, fdump);

//        printf("offset = x%04X: %02X %02X %02X %02X\n",
//               total_size,
//               pFrm_info->pFrame_addr[0],
//               pFrm_info->pFrame_addr[1],
//               pFrm_info->pFrame_addr[2],
//               pFrm_info->pFrame_addr[3]);

        total_size += pFrm_info->frame_len;
    }

    if( pFrm_info->frm_state & AVI_FRAME_END )
    {
        if( fdump )
        {
            fwrite(pFrm_info->pFrame_addr, 1, pFrm_info->frame_len, fdump);
            fclose(fdump);
            fdump = 0;
        }

        total_size += pFrm_info->frame_len;
//        printf("end = %d, total = x%x\n", pFrm_info->frame_len, total_size);

        g_avi_braking = false;
    }

    if( pFrm_info->frm_state & AVI_FRAME_EOS )
    {
        g_avi_braking = false;
        return AVI_FRAME_EOS;
    }


    return 0;
}

static void
_test_demux(
    char    *pFile_path)
{
#define AVI_CACHE_SIZE      512
    FILE    *fin = 0;

    do {
        int         cnt = 0;
        uint32_t    section_len = AVI_CACHE_SIZE;
        bool        is_reset = true;

        if( !(fin = fopen(pFile_path, "rb")) )
        {
            err("open %s fail \n", pFile_path);
            break;
        }

        #if 1 // for test
        section_len = 10 << 10;
        #endif // 1

        fread(g_bs_vbuf, 1, section_len, fin);

        avi_parse_header((uint32_t*)g_bs_vbuf, section_len, &g_media_data_offset);
        printf("media offset= x%x\n", g_media_data_offset);

        // fill bs buffer
        fseek(fin, g_media_data_offset, SEEK_SET);

        while(1)
        {
            int                       rval = 0;
            avi_demux_ctrl_info_t     ctrl_info = {0};

            ctrl_info.pPrivate_data     = &fin;
            ctrl_info.cb_frame_state    = _frame_state;
            ctrl_info.cb_fill_buf       = _fill_buf;
            ctrl_info.cb_misc_proc      = _misc_proc;
            ctrl_info.pRing_buf         = g_bs_vbuf;
            ctrl_info.ring_buf_size     = 1 << 10; //BUF_ENC_VFRAME_MAX;
            rval = avi_demux_media_data(&ctrl_info, (uint32_t)is_reset, (uint32_t*)&g_avi_braking);
            if( rval )      break;

            is_reset = false;

            printf("decode a frame end (%d)\n", cnt++);
            Sleep(1);
            g_avi_braking = true;
        }

    } while(0);

    if( fin )   fclose(fin);

    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    char    *name = "./out.avi";
    _test_mux(name);
    _test_demux(name);

    return 0;
}

