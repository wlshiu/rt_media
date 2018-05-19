/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "ffconf.h"     /* FatFs configuration options */
#include "diskio.h"		/* FatFs lower layer API */

#ifdef _MSC_VER
#include "windows.h"
#endif


//#define SECTOR_SIZE         512
#define BLOCK_SIZE         512
#define DISK_SIZE           (5 << 20) //(128 * BLOCK_SIZE)

static uint8_t      disk_space[DISK_SIZE];
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
        case DEV_RAM:
            return RES_OK;

        case DEV_SD:
            break;

        default:
            return STA_NOINIT;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
        case DEV_RAM :
            return RES_OK;

        case DEV_SD:
            break;

        default:
            return STA_NOINIT;//RES_PARERR;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    uint32_t    BufferSize = (BLOCK_SIZE * count);
    uint8_t     *pSramAddress = (uint8_t *)(disk_space + (sector * BLOCK_SIZE));

    switch (pdrv)
    {
        case DEV_RAM :
            memcpy(buff, pSramAddress, BufferSize);
            return RES_OK;

        case DEV_SD:
            break;

        default:
            return RES_PARERR;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    uint32_t    BufferSize = (BLOCK_SIZE * count);// + count;
    uint8_t     *pSramAddress = (uint8_t *)(disk_space + (sector * BLOCK_SIZE));

    switch (pdrv)
    {
        case DEV_RAM :
            memcpy(pSramAddress, buff, BufferSize);
            return RES_OK;

        case DEV_SD:
            break;

        default:
            return RES_PARERR;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_OK;

    switch (pdrv)
    {
        case DEV_RAM :
            switch (cmd)
            {
                case GET_SECTOR_COUNT:
                    *(DWORD*)buff = DISK_SIZE / BLOCK_SIZE;
                    break;
                case GET_SECTOR_SIZE:
                    *(WORD*)buff = BLOCK_SIZE;
                    break;
                case GET_BLOCK_SIZE :
                    *(DWORD*)buff = BLOCK_SIZE;
                    break;
                case CTRL_SYNC:
                    res = RES_OK;
                    break;
                default:
                    res = RES_PARERR;
                    break;
            }
            break;

        case DEV_SD:
            break;

        default:
            res = RES_PARERR;
            break;
    }

    return res;
}


DWORD get_fattime(void)
{
#ifdef _MSC_VER
    return GetTickCount();
#endif
    return 0;
}
