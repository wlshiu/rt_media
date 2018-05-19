/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bmp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/07
 * @license
 * @description
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "bmp.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define msg(str, argv...)       printf("%s[%u] " str, __func__, __LINE__, ##argv)

#define _assert(expression)                                        \
        do{ if(expression) break;                                       \
            printf("%s: %s[#%u]\n", #expression, __FILE__, __LINE__);   \
            while(1);                                                   \
        }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
bmp_reader(
    FILE            **ppfin,
    bmp_hdr_t       *pBmp_hdr,
    bmp_option_t    opt,
    uint8_t         *buffer)
{
    int         rval = 0;
    FILE        *fin = *ppfin;

    _assert(fin != 0);

    switch( opt )
    {
        case BMP_OPTION_READ_HEADER:
            _assert(pBmp_hdr != 0);

            fread(pBmp_hdr, 1, sizeof(bmp_hdr_t), fin);
            break;
        case BMP_OPTION_READ_RAW:
            _assert(buffer != 0);

            fread(buffer, 1, (pBmp_hdr->width * pBmp_hdr->height * 3), fin);
            break;
        case BMP_OPTION_RESET:
            break;
        default:
            rval = -1;
            msg("unknown opion '%u'!\n", opt);
            break;
    }

    return rval;
}
