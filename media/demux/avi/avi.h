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

#ifndef __avi___H___
#define __avi___H___

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum avi_codec
{
    AVI_CODEC_NONE    = 0,
    AVI_CODEC_MJPG,
    AVI_CODEC_PCM,

} avi_codec_t;

typedef enum avi_frm_type
{
    AVI_FRM_UNKNOWN     = 0,
    AVI_FRM_VIDEO,
    AVI_FRM_AUDIO,

} avi_frm_type_t;


typedef uint32_t    avi_frame_state_t;
#define AVI_FRAME_NONE          (0)
#define AVI_FRAME_START         (0x1 << 1)
#define AVI_FRAME_PARTIAL       (0x1 << 2)
#define AVI_FRAME_END           (0x1 << 3)
#define AVI_FRAME_EOS           (0x1 << 4)  // end of stream

#define AVI_END_OF_STREAM       0x00454f53


typedef struct avi_video_cfg
{
    avi_codec_t     vcodec;
    uint32_t        width;
    uint32_t        height;
    uint32_t        fps;

} avi_video_cfg_t;


typedef struct avi_audio_cfg
{
    avi_codec_t     acodec;
    uint32_t        sample_rate;

} avi_audio_cfg_t;

/**
 *  update info
 */
typedef struct avi_update_info
{
    uint32_t    total_frames;
    uint32_t    total_file_size;

    uint32_t    media_data_size;  // media data size (audio + video data)
} avi_update_info_t;

/**
 *  media info (it MUST parse header first)
 */
typedef struct avi_media_info
{
    avi_codec_t     codec;

    union {
        struct {
            float           fps;
        } vid;

        struct {
            uint32_t        sample_rate;
        } aud;
    };

} avi_media_info_t;

/**
 *  frame info
 */
typedef struct avi_frame_info
{
    avi_frame_state_t   frm_state;
    uint8_t             *pFrame_addr;
    int                 frame_len;

    uint32_t            frame_offset;

} avi_frame_info_t;

struct avi_mux_ctrl_info;
typedef int (*CB_ENCODE_ONE_FRAME)(struct avi_mux_ctrl_info *pCtrl_info,
                                    uint8_t *pFrm_buf, uint32_t frm_buf_len,
                                    uint8_t *pBS_buf, uint32_t *pBS_len);

typedef int (*CB_EMPTY_BUF)(struct avi_mux_ctrl_info *pCtrl_info, uint8_t *pBS_buf, uint32_t len);

typedef struct avi_mux_ctrl_info
{
    avi_frm_type_t          frm_type;
    avi_frame_state_t       frm_state;

    CB_ENCODE_ONE_FRAME     cb_enc_one_frame;
    CB_EMPTY_BUF            cb_empty_buf;

    // record bit stream length after encoding
    uint32_t        bs_len;

    // Need to estimate bitstream size of a frame
    uint8_t         *pBS_buf;


    void    *pPrivate_data;

} avi_mux_ctrl_info_t;

struct avi_demux_ctrl_info;

/**
 *  \brief  CB_MISC_PROC
 *
 *  \param [in] pCtrl_info      pass control info from avi_demux_media_data()
 *  \return                     0: ok, other: leave braking in avi_demux_media_data() and return from avi_demux_media_data()
 *
 *  \details
 *      Because avi_demux_media_data() will brake flow, use callback to do something which user wants.
 */
typedef int (*CB_MISC_PROC)(struct avi_demux_ctrl_info *pCtrl_info);

/**
 *  \brief  CB_FILL_BUF
 *
 *  \param [in] pCtrl_info      pass control info from avi_demux_media_data()
 *  \param [in] pBuf            the buffer address which need to fill
 *  \param [in] pLen            the length of buffer and user need to re-assign the real length of filling
 *  \return                     0: ok, other: leave braking in avi_demux_media_data() and return from avi_demux_media_data()
 *
 *  \details
 */
typedef int (*CB_FILL_BUF)(struct avi_demux_ctrl_info *pCtrl_info, uint8_t *pBuf, uint32_t *pLen);

/**
 *  \brief  CB_FRAME_STATE
 *
 *  \param [in] pCtrl_info      pass control info from avi_demux_media_data()
 *  \param [in] pMedia_info     the media info of current avi file
 *  \param [in] frm_info        current frame parsing info
 *                                  frame state:
 *                                      AVI_FRAME_NONE      => no frame
 *                                      AVI_FRAME_PARTIAL   => get parts of a frame
 *                                      AVI_FRAME_END       => the last part of a frame
 *
 *  \return                     0: ok, other: leave braking in avi_demux_media_data() and return from avi_demux_media_data()
 *
 *  \details
 */
typedef int (*CB_FRAME_STATE)(struct avi_demux_ctrl_info *pCtrl_info, avi_media_info_t *pMedia_info, avi_frame_info_t *pFrm_info);

typedef struct avi_demux_ctrl_info
{
    CB_MISC_PROC        cb_misc_proc;
    CB_FRAME_STATE      cb_frame_state;
    CB_FILL_BUF         cb_fill_buf;

    uint32_t            movi_data_len;

    // TODO: Need to give a suggest value (more than 1 frame size)
    uint8_t             *pRing_buf;
    int                 ring_buf_size;

    void    *pPrivate_data;

} avi_demux_ctrl_info_t;



/**
 *  I/O handler
 */
typedef struct avi_io_desc
{
    int     (*cb_open)(struct avi_io_desc *pDesc, const char *pFilename, const char *pMode);
    int     (*cb_close)(struct avi_io_desc *pDesc);
    int     (*cb_read)(struct avi_io_desc *pDesc, uint8_t *pBuf, uint32_t length);
    int     (*cb_write)(struct avi_io_desc *pDesc, uint8_t *pBuf, uint32_t length);

    void    *pFile;
    void    *pPrivate;

} avi_io_desc_t;


int
avi_mux_reset_header(
    avi_video_cfg_t   *pVid_cfg,
    avi_audio_cfg_t   *pAud_cfg,
    uint32_t          align_pow2_num);

/**
 *  \brief  update info to avi header
 *
 *  \param [in] pUpdate_info    info for updating
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
avi_mux_update_info(
    avi_update_info_t   *pUpdate_info);


int
avi_mux_one_frame(
    avi_mux_ctrl_info_t     *pCtrl_info,
    uint8_t                 *pFrm_buf,
    uint32_t                frm_buf_len,
    avi_frame_state_t       frm_state);


/**
 *  \brief  get avi header length
 *
 *  \return     length
 *
 *  \details
 */
uint32_t
avi_mux_get_header_size(void);


/**
 *  \brief  generate binary of avi header
 *
 *  \param [in] pHeader_buf         output buffer for binary
 *  \param [in] pHeader_buf_len     valid length in pHeader_buf
 *  \return                         0: ok, other: fail
 *
 *  \details
 */
int
avi_mux_gen_header(
    uint8_t     *pHeader_buf,
    uint32_t    *pHeader_buf_len);


/**
 *  \brief  parsing avi header
 *
 *  \param [in] pHeader_buf         binary data of avi
 *                                      ps. all header MUST in this buffer besides movi data
 *  \param [in] header_buf_len      valid length in pHeader_buf
 *  \param [in] pMovi_offset        report movi data offset
 *  \return                         0: ok, other: fail
 *
 *  \details
 */
int
avi_parse_header(
    uint32_t    *pHeader_buf,
    uint32_t    header_buf_len,
    uint32_t    *pTotal_size,
    uint32_t    *pMovi_offset);


/**
 *  \brief  demux media data
 *
 *  \param [in] pCtrl_info      demux control info
 *  \param [in] is_reset        reset process
 *  \param [in] pIs_braking     braking this function or not
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
avi_demux_media_data(
    avi_demux_ctrl_info_t   *pCtrl_info,
    uint32_t                is_reset,
    uint32_t                *pIs_braking);


#ifdef __cplusplus
}
#endif

#endif
