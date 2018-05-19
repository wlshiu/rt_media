/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bmp.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/07
 * @license
 * @description
 */

#ifndef __bmp_H_wk4jgyVR_lORU_HMBr_stw4_uJrSKJvaIBzj__
#define __bmp_H_wk4jgyVR_lORU_HMBr_stw4_uJrSKJvaIBzj__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum bmp_option
{
    BMP_OPTION_READ_HEADER  = 0,
    BMP_OPTION_READ_RAW,
    BMP_OPTION_RESET,
} bmp_option_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct bmp_hdr
{
    unsigned short      identifier;      // 0x0000
    unsigned int        filesize;          // 0x0002
    unsigned int        reserved;          // 0x0006
    unsigned int        bitmap_dataoffset; // 0x000A
    unsigned int        bitmap_headersize; // 0x000E
    unsigned int        width;             // 0x0012
    unsigned int        height;            // 0x0016
    unsigned short      planes;          // 0x001A
    unsigned short      bits_perpixel;   // 0x001C
    unsigned int        compression;       // 0x001E
    unsigned int        bitmap_datasize;   // 0x0022
    unsigned int        hresolution;       // 0x0026
    unsigned int        vresolution;       // 0x002A
    unsigned int        usedcolors;        // 0x002E
    unsigned int        importantcolors;   // 0x0032
    unsigned int        palette;           // 0x0036
} __attribute__((packed, aligned(1))) bmp_hdr_t;
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
    uint8_t         *buffer);
    

#ifdef __cplusplus
}
#endif

#endif
