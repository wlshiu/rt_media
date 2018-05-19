/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file fatfs_test.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/05/04
 * @license
 * @description
 */

#ifndef __fatfs_test_H_w2xofnRT_lspD_HJpc_s0Kn_uxeTRr6ecrNE__
#define __fatfs_test_H_w2xofnRT_lspD_HJpc_s0Kn_uxeTRr6ecrNE__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
#if 1
    #define ff_msg(str, argv...)        printf("%s[%u] " str, __func__, __LINE__, ##argv)
#else
    #define ff_msg(str, argv...)
#endif // 1

#define FCHK(res, act_func, err_operator) \
    do{ res = act_func; \
        if(res) {  \
            ff_msg("call %s: res %d %s\n", #act_func, res, fatfs_error_to_string(res));\
            err_operator; \
        } \
    }while(0)

static const char *fatfs_error_to_string(FRESULT err)
{
    switch (err)
    {
        case FR_OK:
            return "Succeeded";
        case FR_DISK_ERR:
            return "FATFS: A hard error occurred in the low level disk I/O layer";
        case FR_INT_ERR:
            return "FATFS: Assertion failed (check for corruption)";
        case FR_NOT_READY:
            return "FATFS: The physical drive cannot work";
        case FR_NO_FILE:
            return "FATFS: Could not find the file";
        case FR_NO_PATH:
            return "FATFS: Could not find the path";
        case FR_INVALID_NAME:
            return "FATFS: The path name format is invalid";
        case FR_DENIED:
            return "FATFS: Access denied due to prohibited access or directory full";
        case FR_EXIST:
            return "FATFS: Destination file already exists";
        case FR_INVALID_OBJECT:
            return "FATFS: The file/directory object is invalid";
        case FR_WRITE_PROTECTED:
            return "FATFS: The physical drive is write protected";
        case FR_INVALID_DRIVE:
            return "FATFS: The logical drive number is invalid";
        case FR_NOT_ENABLED:
            return "FATFS: The volume has no work area";
        case FR_NO_FILESYSTEM:
            return "FATFS: There is no valid FAT volume";
        case FR_MKFS_ABORTED:
            return "FATFS: f_mkfs() aborted due to a parameter error. Try adjusting the partition size.";
        case FR_TIMEOUT:
            return "FATFS: Could not get a grant to access the volume within defined period";
        case FR_LOCKED:
            return "FATFS: The operation is rejected according to the file sharing policy";
        case FR_NOT_ENOUGH_CORE:
            return "FATFS: LFN working buffer could not be allocated";
        case FR_TOO_MANY_OPEN_FILES:
            return "FATFS: Number of open files > _FS_SHARE";
        default:
        case FR_INVALID_PARAMETER:
            return "FATFS: Invalid parameter";
    }
}
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

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
