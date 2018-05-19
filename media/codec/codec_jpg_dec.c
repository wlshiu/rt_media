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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
static uint32_t                     g_jpg_dec_initialized = false;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
jpg_dec_init(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    media_assert(pCodec_setting != 0);

    return rval;
}

static int
jpg_dec_proc(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    media_assert(pCodec_setting != 0);

    return rval;
}

static int
jpg_dec_deinit(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    g_jpg_dec_initialized = false;

    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
media_codec_desc_t      media_codec_desc_jpg_dec =
{
    .codec_type = MEDIA_CODEC_JPG_DEC,
    .cb_init    = jpg_dec_init,
    .cb_proc    = jpg_dec_proc,
    .cb_deinit  = jpg_dec_deinit,
};


