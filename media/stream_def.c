/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file stream_def.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/19
 * @license
 * @description
 */


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

//=============================================================================
//                  Private Function Definition
//=============================================================================

static int
def_init(
    media_stream_info_t    *pInfo)
{
    int     rval = 0;

    return rval;
}

static int
def_deinit(
    media_stream_info_t    *pInfo)
{
    int     rval = 0;
    return rval;
}

static int
def_read(
    media_stream_info_t     *pInfo,
    uint8_t                 **ppBuf,
    int                     request_len,
    int                     *pReal_len,
    void                    *pExtra_info)
{
    int         rval = 0;

    return rval;
}

static int
def_write(
    media_stream_info_t     *pInfo,
    uint8_t                 *pBuf,
    int                     request_len,
    int                     *pReal_len,
    void                    *pExtra_info)
{
    int     rval = 0;

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
media_stream_dev_t      media_stream_dev_def =
{
    .dev_type   = MEIDA_STREAM_DEF,
    .cb_init    = def_init,
    .cb_deinit  = def_deinit,
    .cb_read    = def_read,
    .cb_write   = def_write,
};
