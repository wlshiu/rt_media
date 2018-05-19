/*
 * Copyright (c) 2016-, Shiu. All Rights Reserved.
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


#ifndef __riff___H___
#define __riff___H___

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define _FOURCC(a, b, c, d)         (((d) << 24) | ((c) <<16) | ((b) << 8) | (a))

#define AVI_FCC_RIFF        _FOURCC('R', 'I', 'F', 'F') // "FFIR"
#define AVI_FCC_AVI         _FOURCC('A', 'V', 'I', ' ') // " IVA"
#define AVI_FCC_LIST        _FOURCC('L', 'I', 'S', 'T') // "TSIL"
#define AVI_FCC_HDRL        _FOURCC('h', 'd', 'r', 'l') // "lrdh"
#define AVI_FCC_AVIH        _FOURCC('a', 'v', 'i', 'h') // "hiva"

#define AVI_FCC_STRL        _FOURCC('s', 't', 'r', 'l') //"lrts"
#define AVI_FCC_STRH        _FOURCC('s', 't', 'r', 'h') //"hrts"
#define AVI_FCC_STRF        _FOURCC('s', 't', 'r', 'f') //"frts"
#define AVI_FCC_STRD        _FOURCC('s', 't', 'r', 'd') //"drts"
#define AVI_FCC_STRN        _FOURCC('s', 't', 'r', 'n') //"nrts"

#define AVI_FCC_FMT         _FOURCC('f', 'm', 't', ' ') //" tmf"

#define AVI_FCC_MOVI        _FOURCC('m', 'o', 'v', 'i') //"ivom"

#define AVI_FCC_VIDS        _FOURCC('v', 'i', 'd', 's') //"sdiv"
#define AVI_FCC_MJPG        _FOURCC('M', 'J', 'P', 'G') //"GPJM"

#define AVI_FCC_AUDS        _FOURCC('a', 'u', 'd', 's') //"sdua"
#define AVI_FCC_WAVE        _FOURCC('W', 'A', 'V', 'E') //"EVAW"

#define AVI_FCC_MIDS        _FOURCC('m', 'i', 'd', 's') //"sdim"

#define AVI_FCC_TXTS        _FOURCC('t', 'x', 't', 's') //"stxt"

#define AVI_FCC_JUNK        _FOURCC('J', 'U', 'N', 'K') //"KNUJ"
#define AVI_FCC_FACT        _FOURCC('f', 'a', 'c', 't') //"tcaf"
#define AVI_FCC_DATA        _FOURCC('d', 'a', 't', 'a') //"atad"

#define AVI_FCC_IDX1        _FOURCC('i', 'd', 'x', '1') //"1xdi"
#define AVI_FCC_00DC        _FOURCC('0', '0', 'd', 'c') //"cd00"
#define AVI_FCC_00DB        _FOURCC('0', '0', 'd', 'b') //"bd00"
#define AVI_FCC_00WB        _FOURCC('0', '0', 'w', 'b') //"bw00"
#define AVI_FCC_01DC        _FOURCC('0', '1', 'd', 'c') //"cd10"
#define AVI_FCC_01WB        _FOURCC('0', '1', 'w', 'b') //"bw10"

#define AVI_FCC_INDX        _FOURCC('i', 'n', 'd', 'x') //"xdni"
#define AVI_FCC_IX00        _FOURCC('i', 'x', '0', '0') //"00xi"
#define AVI_FCC_IX01        _FOURCC('i', 'x', '0', '1') //"10xi"


#define AVI_FCC_ODML        _FOURCC('o', 'd', 'm', 'l') //"lmdo"
#define AVI_FCC_DMLH        _FOURCC('d', 'm', 'l', 'h') //"hlmd"


typedef enum avi_bmp_compression
{
    BI_RGB       = 0x0000,
    BI_RLE8      = 0x0001,
    BI_RLE4      = 0x0002,
    BI_BITFIELDS = 0x0003,
    BI_JPEG      = 0x0004,
    BI_PNG       = 0x0005,
    BI_CMYK      = 0x000B,
    BI_CMYKRLE8  = 0x000C,
    BI_CMYKRLE4  = 0x000D
} avi_bmp_compression_t;


typedef struct avi_riff
{
    uint32_t        tag_riff;
    uint32_t        size;
    uint32_t        fcc;

} avi_riff_t;

typedef struct avi_chunk
{
    uint32_t    fcc;
    uint32_t    size;

} avi_chunk_t;

typedef struct avi_list
{
    uint32_t    tag_list;
    uint32_t    size;
    uint32_t    fcc;

} avi_list_t;


typedef struct avi_main_hdr
{
    uint32_t    dwMicroSecPerFrame;
    uint32_t    dwMaxBytesPerSec;
    uint32_t    dwPaddingGranularity;
    uint32_t    dwFlags;
    uint32_t    dwTotalFrames;
    uint32_t    dwInitialFrames;
    uint32_t    dwStreams;
    uint32_t    dwSuggestedBufferSize;
    uint32_t    dwWidth;
    uint32_t    dwHeight;
    uint32_t    dwReserved[4];

} avi_main_hdr_t;

typedef struct avi_avih
{
    uint32_t    tag_avih;
    uint32_t    size;

    avi_main_hdr_t  main_hdr;

} avi_avih_t;

typedef struct rect
{
    uint16_t    left;
    uint16_t    top;
    uint16_t    right;
    uint16_t    bottom;
} rect_t;

typedef struct avi_stream_hdr
{
    uint32_t      fccType;
    uint32_t      fccHandler;
    uint32_t      dwFlags;
    uint16_t      wPriority;
    uint16_t      wLanguage;
    uint32_t      dwInitialFrames;
    uint32_t      dwScale;
    uint32_t      dwRate;
    uint32_t      dwStart;
    uint32_t      dwLength;
    uint32_t      dwSuggestedBufferSize;
    uint32_t      dwQuality;
    uint32_t      dwSampleSize;
    rect_t        rcFrame;

} avi_stream_hdr_t;

typedef struct avi_stream_hdr_box
{
    uint32_t      tag_strh;
    uint32_t      size;

    avi_stream_hdr_t    stream_hdr;

} avi_stream_hdr_box_t;

/**
 *  Video
 *  https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
 */
typedef struct avi_bmp_info_hdr
{
    uint32_t    biSize;
    uint32_t    biWidth;
    uint32_t    biHeight;
    uint16_t    biPlanes;
    uint16_t    biBitCount;
    uint32_t    biCompression;
    uint32_t    biSizeImage;
    uint32_t    biXPelsPerMeter;
    uint32_t    biYPelsPerMeter;
    uint32_t    biClrUsed;
    uint32_t    biClrImportant;

} avi_bmp_info_hdr_t;

typedef struct avi_bmp_info_hdr_box
{
    uint32_t      tag_strf;
    uint32_t      size;

    avi_bmp_info_hdr_t  bmp_hdr;

} avi_bmp_info_hdr_box_t;

/**
 *  Audio
 */
typedef struct avi_wave_fmt_ex
{
    uint16_t      wFormatTag;
    uint16_t      nChannels;
    uint32_t      nSamplesPerSec;
    uint32_t      nAvgBytesPerSec;
    uint16_t      nBlockAlign;
    uint16_t      wBitsPerSample;
    uint16_t      cbSize;

} avi_wave_fmt_ex_t;

typedef struct avi_mp3_wave_fmt
{
    avi_wave_fmt_ex_t   wfx;
    uint16_t            wID;
    uint32_t            fdwFlags;
    uint16_t            nBlockSize;
    uint16_t            nFramesPerBlock;
    uint16_t            nCodecDelay;
} avi_mp3_wave_fmt_t;

typedef struct avi_wave_fmt_ex_box
{
    uint32_t      tag_strf;
    uint32_t      size;

    avi_wave_fmt_ex_t   wave_hdr;

} avi_wave_fmt_ex_box_t;

/**
 *  Index
 */
typedef struct avi_idx_entry
{
    uint32_t    ckid;
    uint32_t    dwFlags;
    uint32_t    dwChunkOffset;
    uint32_t    dwChunkLength;

} avi_idx_entry_t;

typedef struct avi_super_idx_entry
{
    uint32_t        wOffset;
    uint32_t        dwSize;
    uint32_t        dwDuration;
} avi_super_idx_entry_t;

typedef struct avi_super_idx
{
    uint32_t      fcc;
    uint32_t      size;

    uint16_t      wLongsPerEntry;
    uint8_t       bIndexSubType;
    uint8_t       bIndexType;
    uint32_t      nEntriesInUse;
    uint32_t      dwChunkId;
    uint32_t      dwReserved[3];

    avi_super_idx_entry_t      *idx_entry;

} avi_super_idx_t;



#ifdef __cplusplus
}
#endif

#endif
