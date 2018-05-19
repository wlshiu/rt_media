/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file codec_jpg.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/09
 * @license
 * @description
 */



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
//                  Private Function Definition
//=============================================================================
static int
wav_dec_init(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    return rval;
}

static int
wav_dec_proc(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    return rval;
}

static int
wav_dec_deinit(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
media_codec_desc_t      media_codec_desc_wav_dec =
{
    .codec_type = MEDIA_CODEC_WAV_DEC,
    .cb_init    = wav_dec_init,
    .cb_proc    = wav_dec_proc,
    .cb_deinit  = wav_dec_deinit,
};


