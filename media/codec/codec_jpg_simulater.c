/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file codec_jpg_simulater.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/09
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>

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
static FILE         *g_fout = 0;
static uint32_t     g_cnt = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
jpg_dec_init(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    char    name[64] = {0};

    snprintf(name, 64, "./dump_%02d.jpg", g_cnt++);

    if( !g_fout )
    {
        if( !(g_fout = fopen(name, "wb")) )
        {
            printf("open %s fail \n", name);
            rval = -1;
        }
    }

    return rval;
}

static int
jpg_dec_proc(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    if( g_fout )
    {
        fwrite(pCodec_setting->pStream_buf,
               1, pCodec_setting->stream_buf_len, g_fout);
    }

    if( pCodec_setting->frm_state & MEDIA_FRM_STATE_END )
    {
        if( g_fout )        fclose(g_fout);
        g_fout = 0;
    }
    return rval;
}

static int
jpg_dec_deinit(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    if( g_fout )        fclose(g_fout);
    g_fout = 0;

    return rval;
}

static int
jpg_enc_init(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    return rval;
}

static int
jpg_enc_proc(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    return rval;
}

static int
jpg_enc_deinit(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
media_codec_desc_t      media_codec_desc_jpg_sim_dec =
{
    .codec_type = MEDIA_CODEC_JPG_DEC,
    .cb_init    = jpg_dec_init,
    .cb_proc    = jpg_dec_proc,
    .cb_deinit  = jpg_dec_deinit,
};

media_codec_desc_t      media_codec_desc_jpg_sim_enc =
{
    .codec_type = MEDIA_CODEC_JPG_ENC,
    .cb_init    = jpg_enc_init,
    .cb_proc    = jpg_enc_proc,
    .cb_deinit  = jpg_enc_deinit,
};

