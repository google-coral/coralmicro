/**
 * @file sci2c.c
 * @author NXP Semiconductors
 * @version 1.0
 * @par License
 *
 * Copyright 2016,2020 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * @par Description
 * This file implements the SCI2C Protocol Specification.
 *
 *****************************************************************************/

#include "smCom.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "sm_printf.h"
#include "sm_timer.h"
#include "i2c_a7.h"
#include "sci2c.h"
#include "sci2c_cfg.h"

#ifdef __gnu_linux__
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#endif
#include <time.h>

#if defined(SSS_USE_FTR_FILE)
#include "fsl_sss_ftr.h"
#else
#include "fsl_sss_ftr_default.h"
#endif

#include "nxLog_smCom.h"
#include "nxEnsure.h"

// #define LOG_I2C
// #define LOG_SCI2C_PARAMETER_EXCHANGE

#define DELAY_MSEC              (1)

#define RX_LEN_SOFT_RESET       (2)

#define CDB_ATS_MAX             (31)
#define MAX_ATR_RESPONSE_LENGTH (CDB_ATS_MAX + 3) /* add length and PCB byte */

#define RX_LEN_PARAM_EXCH       (2)
#define RX_LEN_BIND_SELECT      (1)
#define RX_LEN_GET_STATUS       (2)

#define DEFAULT_FWI             (9) /* default frame waiting integer (15.1.3) */

#define APDU_GET_RESPONSE       (0x02)

#define SLAVE_STATUS            (0x7)
#define SLAVE_STATUS_BUSY       ((0x1<<4) | SLAVE_STATUS)

#define SCI2C_M2S_OK                  0x00
#define SCI2C_M2S_WRITE_BLOCK_FAILED  0x01
#define SCI2C_M2S_GET_STATUS_TIME_OUT 0x02
#define SCI2C_S2M_OK                  0x03
#define SCI2C_S2M_READ_BLOCK_FAILED   0x04

#define SCI2C_tFW_DEF_ms 600 // Waiting time extension is assumed to be 600 ms

static U8 gSeqCtr = 0;


static uint8_t rxData[MAX_DATA_LEN];
static uint8_t txData[MAX_DATA_LEN];

static uint8_t * pRx = rxData;

static U8 GetCounter(void);
static U8 sci2c_WaitForStatusOkay(void* conn_ctx, U32 msec);
static i2c_status_t sci2c_SendByte(void* conn_ctx, sci2c_Data_t * pSci2cData);
static i2c_status_t sci2c_WriteBlock(void* conn_ctx, sci2c_Data_t * pSci2cData);
static i2c_status_t sci2c_ReadBlock(void* conn_ctx, sci2c_Data_t * pSci2cData, U8 * pRead, U16 * pReadLen);
static U16 sci2c_MasterToSlaveDataTx(void* conn_ctx, tSCI2C_Data_t * pSCI2C);
static U16 sci2c_SlaveToMasterDataTx(void* conn_ctx, tSCI2C_Data_t * pSCI2C);
static eSci2c_Error_t sci2c_Wakeup(void* conn_ctx);
static eSci2c_Error_t sci2c_SoftReset(void* conn_ctx, U8 * pRx, U16 * pRxLen);
static eSci2c_Error_t sci2c_ReadAnswerToReset(void* conn_ctx, U8 * pAtrResponse, U16 * pRxLen);
static eSci2c_Error_t sci2c_ParameterExchange(void* conn_ctx, sci2c_maxDataBytesS2M_t maxData, sci2c_maxDataBytesM2S_t * pResponse);
static eSci2c_Error_t sci2c_GetStatus(void* conn_ctx, U8* pStatus);
static eSci2c_Error_t sci2c_DataExchange(void* conn_ctx, tSCI2C_Data_t * pSCI2C);

/**
 * @function sci2c_Init
 * @description Initializes the SCI2C module.
 * @param SCI2Catr IN: Pointer to the buffer that will contain ATR
 * @param SCI2CatrLen IN: Pointer to length of provided buffer; OUT: Actual length of retrieved ATR
 * @return
 */
eSci2c_Error_t sci2c_Init(void* conn_ctx, U8 *SCI2Catr, U16 *SCI2CatrLen)
{
    uint8_t atr[64];
#if SSS_HAVE_A71XX
    U8 status = SCI2C_STATUS_UNDEFINED; // SCI2C_STATUS_UNDEFINED allows to detect the case no A71CH is connected
    int nGetStatus = 0;
#endif
    uint16_t len = 0;
    eSci2c_Error_t rv = eSci2c_Error;
    uint16_t nrRead = 0;
    sci2c_maxDataBytesM2S_t maxCommandLength;

    ENSURE_OR_GO_EXIT(SCI2Catr != NULL);
    ENSURE_OR_GO_EXIT(SCI2CatrLen != NULL);

#define GET_STATUS_MAX 70

    gSeqCtr = 0; /* (re)set sequence counter */

#if SSS_HAVE_A71XX
    //   Keep on getting the status until the A7 is ready. Fetching the status
    // will implicitly wake up the A7, so no explicit wake-up command is required.
    //
    //   GET_STATUS_MAX puts an upperlimit on the amount of re-tries. This prevents
    // the sci2c_Init call not returning in case e.g. no A7 is connected.
    //
    //   Time-out for a (physically) not connected A7:
    // GET_STATUS_MAX * T_WNCMD_ACTUAL = 7 sec
    //
    while (status != SCI2C_STATUS_NORMAL_READY)
    {
        sci2c_GetStatus(conn_ctx, &status);
        if ((++nGetStatus) > GET_STATUS_MAX) {
            sm_printf(DBGOUT, "Error: Failed to retrieve ready status.\r\n");
            return eSci2c_Error;
        }

        if (status == SCI2C_STATUS_NORMAL_BUSY) {
            sm_sleep(T_WNCMD_ACTUAL);
        }
        else if (status == SCI2C_STATUS_NORMAL_READY) {
            // Will drop out of while loop
        }
        else if (status == SCI2C_STATUS_UNDEFINED) {
            sm_sleep(T_WNCMD_ACTUAL);
        }
        else {
            sm_printf(DBGOUT, "Recover from Status=0x%02X by sci2c_SoftReset.\r\n", status);
            rv = sci2c_SoftReset(conn_ctx, pRx, &nrRead);
            if (rv != eSci2c_No_Error)
            {
                sm_printf(DBGOUT, "Soft reset failed\r\n");
            }
        }
    }

    // sci2c_Wakeup();
    // sm_sleep(T_WNCMD_ACTUAL); // Maximum value is 200 ms
#endif

    rv = sci2c_SoftReset(conn_ctx, pRx, &nrRead);
    if (rv != eSci2c_No_Error)
    {
        sm_printf(DBGOUT, "Soft reset failed\r\n");
        return eSci2c_Error;
    }

    sm_sleep(5 * DELAY_MSEC);

    /* --- read ATR --- */
    // sm_printf(DBGOUT, "reading ATR\r\n");
    rv = sci2c_ReadAnswerToReset(conn_ctx, atr, &len);
    if ((rv != eSci2c_No_Error) || (len == 0))
    {
        *SCI2CatrLen = 0;
        sm_printf(DBGOUT, "Error reading ATR\r\n");
        return eSci2c_Error;
    }
    else
    {
        // Copy ATR
        *SCI2CatrLen = (len < (*SCI2CatrLen)) ? len : (*SCI2CatrLen);
        memcpy(SCI2Catr, atr, *SCI2CatrLen);
    }

    /* --- param exchange --- */
#ifdef LOG_SCI2C_PARAMETER_EXCHANGE
    printf("\r\n-----------sci2c_ParameterExchange ------------------\r\n");
#endif
    rv = sci2c_ParameterExchange(conn_ctx, SMCOM_MAX_BYTES, &maxCommandLength);
    /* next types are casted to U32 for comparison; if types do not match, this comparison needs to be changed */
    if ((rv != eSci2c_No_Error) || ((U32) maxCommandLength != (U32) SMCOM_MAX_BYTES))
    {
        sm_printf(DBGOUT, "Error setting parameter exchange\r\n");
        return eSci2c_Error;
    }
#ifdef LOG_SCI2C_PARAMETER_EXCHANGE
    printf("\r\n-----------done------------------\r\n");
#endif
exit:
    return rv;
}

void sci2c_SetSequenceCounter(U8 seqCounter)
{
    gSeqCtr = seqCounter;
}

U8 sci2c_GetSequenceCounter()
{
    return gSeqCtr;
}

/**
* Terminates I2C connection without breaking SCI2C link.
*
* @param[in] full Can be either 0 or 1. A value of 0 corresponds to a 'light-weight' terminate.
*    This parameter is basically a hack to enable the Bootloader to Host-OS SCP03 key handover scenario with the
*    Raspberry Pi's GPIO based I2C master scenario.
*    To allow for this scenario the 'full' parameter must be set to '0'.
*/
void sci2c_TerminateI2C(U8 full)
{
    axI2CTerm(NULL, full);
}

static eSci2c_Error_t sci2c_Wakeup(void* conn_ctx)
{
    i2c_status_t i2cErr;
    sci2c_Data_t sci2cData;
    sci2c_Data_t * pSci2cData = (sci2c_Data_t *) &sci2cData;

    pSci2cData->pcb = PCB_WAKEUP;
    pSci2cData->dataLen = 0;
#if AX_EMBEDDED
    axI2CResetBackoffDelay();
#endif
    i2cErr = sci2c_SendByte(conn_ctx, pSci2cData);

    // Depending on the speed of the I2C master controller
    // an extra delay may have to be inserted here to ensure
    // the secure module is awake by the next command.
    // The SCI2C specification mandates a minimum delay of 180 microsec between the
    // end of the wakeup command and the start of the next command.

#if AX_EMBEDDED
    if (i2cErr == i2c_NoAddrAck ) {
        sm_usleep(SCI2C_T_CMDG);
        return eSci2c_No_Error;
    }
    else
#endif
    if (i2cErr != i2c_Okay)
    {
        return eSci2c_Error;
    }
    else
    {
        return eSci2c_No_Error;
    }
}

/**
 * @function sci2c_SoftReset
 * @description
 * @param pRx Answer to reset command.
 * @param pRxLen  number of bytes read in response to reset command.
 * @return
 */
static eSci2c_Error_t sci2c_SoftReset(void* conn_ctx, U8 * pRx, U16 * pRxLen)
{
   U8 i = 0;
   eSci2c_Error_t rv = eSci2c_Protocol_Error;
   i2c_status_t i2cErr;

   sci2c_Data_t sci2cData;
   sci2c_Data_t * pSci2cData = (sci2c_Data_t *) &sci2cData;

   ENSURE_OR_GO_EXIT(pRx != NULL);
   ENSURE_OR_GO_EXIT(pRxLen != NULL);

   pSci2cData->pcb = PCB_SOFT_RESET;
   pSci2cData->dataLen = 0;

   for (i = 0; i < N_RETRY_SRST; i++)
   {
      i2cErr = sci2c_ReadBlock(conn_ctx, pSci2cData, pRx, pRxLen);
      if (i2cErr == i2c_Okay)
      {
         rv = eSci2c_No_Error;
         break;
      }
      // else if (i2cErr != i2c_NoAddrAck) // retry only if NAK on address
      // {
      //   rv = eSci2c_Protocol_Error;
      //   break;
      // }
   }

   /* spec: wait (at least 5 msec) */
   sm_sleep(T_RSTG);

exit:
    return rv;
}

/**
 * @function sci2c_ReadAnswerToReset
 * @description
 * @param *pAtrResponse Pointer to struct containing the ATR response.
 * @param pRxLen number of bytes read in response to reset command.
 * @return
 */
static eSci2c_Error_t sci2c_ReadAnswerToReset(void* conn_ctx, U8 * pAtrResponse, U16 * pRxLen)
{
   i2c_status_t i2cErr = i2c_Okay;
   sci2c_Data_t sci2cData;
   sci2c_Data_t * pSci2cData = (sci2c_Data_t *) &sci2cData;
   U16 nrRead = 0;
   eSci2c_Error_t rv = eSci2c_Error;

   ENSURE_OR_GO_EXIT(pAtrResponse != NULL);
   ENSURE_OR_GO_EXIT(pRxLen != NULL);

   pSci2cData->pcb = PCB_READ_ATR;
   pSci2cData->dataLen = 0;

   i2cErr = sci2c_ReadBlock(conn_ctx, pSci2cData, pAtrResponse, &nrRead);

   if ((nrRead > MAX_ATR_RESPONSE_LENGTH) || (i2cErr == i2c_Failed) || (i2cErr == i2c_Sci2cException))
   {
      return eSci2c_Error;
   }
   else if (i2cErr == i2c_NoAddrAck)
   {
      sm_printf(DBGOUT, "no addr ack\r\n");
      return eSci2c_Error;
   }

   if (nrRead > 2)
   {
      /* strip the 1st (=length) and 2nd (=PCB) byte */
      nrRead -= 2;
      memmove(pAtrResponse, pAtrResponse + 2, nrRead);
      memset(&pAtrResponse[nrRead + 1], 0x00, 2); // clear the data in memory after copying
   }



   *pRxLen = nrRead;

   rv = eSci2c_No_Error;
exit:
    return rv;
}

/**
 * @function sci2c_ParameterExchange
 * @description
 * @param maxData
 * @return
 */
static eSci2c_Error_t sci2c_ParameterExchange(void* conn_ctx, sci2c_maxDataBytesS2M_t maxData, sci2c_maxDataBytesM2S_t * pResponse)
{
    sci2c_Data_t sci2cData;
    sci2c_Data_t *pSci2cData = (sci2c_Data_t *)&sci2cData;
    eSci2c_Error_t rv = eSci2c_Error;
    U8 i = 0;
    U16 nrRead = 0;
    i2c_status_t i2cErr;

    ENSURE_OR_GO_EXIT(pResponse != NULL);
    pSci2cData->pcb = (U8)(PCB_PARAM_EXCH | (maxData << 6));
    pSci2cData->dataLen = 0;

    for (i = 0; i < N_RETRY_PE; i++) /* try N_RETRY_PE times if not okay */
    {
        i2cErr = sci2c_ReadBlock(conn_ctx, pSci2cData, pRx, &nrRead);

        if ((i2cErr == i2c_Okay) && (((pRx[1] & 0x0C) >> 2) == ((~pRx[1] & 0x30) >> 4)) &&
            (((pRx[1] & 0xC0) >> 6) == maxData)) {
            *pResponse = (sci2c_maxDataBytesM2S_t)((pRx[1] & 0xC0) >> 6);
            rv = eSci2c_No_Error;
            break;
        }
        else {
            printf("\r\nagain\r\n");
        }
    }
exit:
    return rv;
}

/**
 * @function sci2c_GetStatus
 * @description Issues a Status command and will return the status of the slave.
 * @param pStatus The status of the slave.
 * @return
 */
static eSci2c_Error_t sci2c_GetStatus(void* conn_ctx, U8* pStatus)
{
   eSci2c_Error_t rv = eSci2c_Error;
   sci2c_Data_t sci2cData;
   sci2c_Data_t * pSci2cData = (sci2c_Data_t *) &sci2cData;
   U16 nrRead = 0;
   i2c_status_t i2cErr;

   ENSURE_OR_GO_EXIT(pStatus != NULL);
   pSci2cData->pcb = PCB_STATUS;
   pSci2cData->dataLen = 0;

   i2cErr = sci2c_ReadBlock(conn_ctx, pSci2cData, pRx, &nrRead);

   if ( (i2cErr == i2c_Okay) || (i2cErr == i2c_Sci2cException) )
   {
       if ((nrRead == 2) && /* 2 bytes read: LEN and PCB */
           ((pRx[1] & 0x0F) == 0x07)) /* PCB indicates status response */
       {
          *pStatus = ((pRx[1] & 0xF0) >> 4);
          if ((*pStatus == SCI2C_STATUS_NORMAL_READY) || (*pStatus == SCI2C_STATUS_NORMAL_BUSY))
          {
             rv = eSci2c_No_Error;
          }
          else
          {
            /* the specification requires to indicate a protocol exception. */
            rv = eSci2c_Protocol_Error;
          }
       }
   }
//    NOTE-1: The underlying driver doesn't return a value of i2c_NoAddrAck
    else if (i2cErr == i2c_NoAddrAck)
    {
//       printf("sci2c_GetStatus: i2c_NoAddrAck\r\n");
        rv = eSci2c_NackOnAddr;
    }
   else
   // (Refer to NOTE-1 above) If the slave did not acknowledge the I2C address, eSci2c_Error is returned as well
   {
       rv = eSci2c_Error;
   }
exit:
    return rv;
}

static U16 sci2c_MasterToSlaveDataTx(void* conn_ctx, tSCI2C_Data_t * pSCI2C)
{
   U8 maxI2CPacketLen = 253; /* limited by the I2C driver */
   U16 remaining = 0;
   U16 alreadyParsed = 0;
   U8 nrBytesToTx = 0;
   sci2c_Data_t sci2cData;
   sci2c_Data_t * pSci2cData = (sci2c_Data_t *) &sci2cData;
   i2c_status_t i2cErr;
   U16 nStatus = SCI2C_M2S_WRITE_BLOCK_FAILED;

   ENSURE_OR_GO_EXIT(pSCI2C != NULL);

   nStatus = SCI2C_M2S_OK;

   if (pSCI2C->buflen > maxI2CPacketLen) /* chaining */
   {
      remaining = pSCI2C->buflen;

      while (remaining > 0)
      {
         pSci2cData->pcb = (U8)((GetCounter() << 4) | pSCI2C->edc | pSCI2C->dir);

         if (remaining > maxI2CPacketLen)
         {
            nrBytesToTx = maxI2CPacketLen;
            pSci2cData->pcb |= (0x1 << 7); /* set the More bit */
         }
         else
         {
            nrBytesToTx = (U8) remaining; /* cast safe: remaining is always < maxI2CPacketLen */
         }

         pSci2cData->pData = &pSCI2C->pBuf[alreadyParsed];
         pSci2cData->dataLen = nrBytesToTx;

         remaining -= nrBytesToTx;

         i2cErr = sci2c_WriteBlock(conn_ctx, pSci2cData);

         // i2c_Okay,      /* the command has been transmitted succesfully */
         // i2c_NoAddrAck, /* the slave does not acknowledge the address byte */
         // i2c_Failed,

         if (i2cErr != i2c_Okay)
         {
             sm_printf(DBGOUT, "sci2c_MasterToSlaveDataTx (line=%d): sci2c_WriteBlock failed with %d.\r\n", __LINE__, i2cErr);
             nStatus = SCI2C_M2S_WRITE_BLOCK_FAILED;
             break;
         }

         alreadyParsed += nrBytesToTx;

         if ((pSci2cData->pcb & 0x80) != 0) /* more bit is set => check status and wait for okay */
         {
            /* wait until the status is okay after a packet with the More bit set */
            sm_usleep(T_CMDG_USec);
            U8 okay = sci2c_WaitForStatusOkay(conn_ctx, APP_SCI2C_TIMEOUT_ms);
            if (!okay)
            {
               sm_printf(DBGOUT, "sci2c_MasterToSlaveDataTx: stack specific timeout expired waiting for status\r\n");
               nStatus = SCI2C_M2S_GET_STATUS_TIME_OUT;
            }
         }
      }
   }
   else
   {
      pSci2cData->pcb = (U8)((GetCounter() << 4) | pSCI2C->edc | pSCI2C->dir);
      pSci2cData->pData = pSCI2C->pBuf;
      pSci2cData->dataLen = (U8) pSCI2C->buflen; /* cast safe: pSCI2C->buflen is always < 255 */

      i2cErr = sci2c_WriteBlock(conn_ctx, pSci2cData);

      if (i2cErr != i2c_Okay)
      {
          sm_printf(DBGOUT, "sci2c_MasterToSlaveDataTx (line=%d): sci2c_WriteBlock failed with %d.\r\n", __LINE__, i2cErr);
          nStatus = SCI2C_M2S_WRITE_BLOCK_FAILED;
      }
   }
exit:
    return nStatus;
}

static U16 sci2c_SlaveToMasterDataTx(void* conn_ctx, tSCI2C_Data_t * pSCI2C)
{
   U8 reTxBit = 0;
   U16 nrRead = 0;
   U16 alreadyParsed = 0;
   sci2c_Data_t sci2cData;
   sci2c_Data_t * pSci2cData = (sci2c_Data_t *) &sci2cData;
   i2c_status_t i2cErr = i2c_Failed;
   U8 moreBitSet = 1; // Ensures we also retry after a NACK on the first Slave to Master Data transmission command
   U16 nStatus = SCI2C_S2M_READ_BLOCK_FAILED;
#define MAX_IGNORE_NACK 250
   U16 nack_count = 0;

   // here we add the more bit in order to get it running

   ENSURE_OR_GO_EXIT(pSCI2C != NULL);

#if SSS_HAVE_A71XX
   pSci2cData->pcb = (U8)((reTxBit<<7) | pSCI2C->dir | 0x80);
   /* On SE050 based IC
    * pSci2cData->pcb = (U8)((reTxBit<<7) | pSCI2C->dir); */
#endif
   pSci2cData->dataLen = 0;

   do
   {
      i2cErr = sci2c_ReadBlock(conn_ctx, pSci2cData, pRx, &nrRead);

      switch(i2cErr)
      {
          case i2c_Okay:
              if (!((nrRead == 2) && (pRx[1] == SLAVE_STATUS_BUSY)))
              {
                   /* The slave is not busy, check the more bit */
                  nack_count = 0;
                  moreBitSet = (pRx[1] & (0x1 << 7)) >> 7;
                  if (nrRead > 2)
                  {
                    memcpy(&pSCI2C->pBuf[alreadyParsed], &pRx[2], nrRead-2 );
                    alreadyParsed += (nrRead -2);
                  }
              }
              // The last data transmission command shall always have the More bit set to 0.
              pSci2cData->pcb &= 0x7F;
              break;
          case i2c_Failed:
              // Fall through on purpose
          case i2c_Sci2cException:
              break;
          case i2c_NoAddrAck:
              if (nack_count > MAX_IGNORE_NACK) {
                 i2cErr = i2c_Failed;
              }
              else {
                 nack_count++;
                 sm_sleep(2);
              }
              break;
          default:
              sm_printf(DBGOUT, "sci2c_SlaveToMasterDataTx: sci2c_ReadBlock returned %x\r\n", i2cErr);
              break;
      }
   } while ( (moreBitSet == 1) && (i2cErr != i2c_Failed) );


   pSCI2C->buflen = alreadyParsed;

   if (i2cErr == i2c_Okay)
   {
       nStatus = SCI2C_S2M_OK;
   }
   else
   {
       nStatus = SCI2C_S2M_READ_BLOCK_FAILED;
   }
exit:
    return nStatus;
}

/**
 * @function sci2c_DataExchange
 * @description Used for sending and receiving the response after the status is okay.
 * @param pSCI2C Pointer to the SCI2C data structure
 */
static eSci2c_Error_t sci2c_DataExchange(void* conn_ctx, tSCI2C_Data_t * pSCI2C)
{
   U16 nStatus = SCI2C_M2S_OK;
   U8 okay = 0;
   eSci2c_Error_t rv = eSci2c_Error;

   ENSURE_OR_GO_EXIT(pSCI2C != NULL);

   if (pSCI2C->dir == eSci2c_DirectionM2S) /* master to slave */
   {
      nStatus = sci2c_MasterToSlaveDataTx(conn_ctx, pSCI2C);

      if (nStatus != SCI2C_M2S_OK)
      {
          return eSci2c_Error;
      }

      /* wait until status is okay after sending the complete payload stream */
      sm_usleep(T_CMDG_USec);
      okay = sci2c_WaitForStatusOkay(conn_ctx, APP_SCI2C_TIMEOUT_ms);
      if (!okay)
      {
          sm_printf(DBGOUT, "timeout expired waiting for status (3)\r\n");
          return eSci2c_Error;
      }
   }
   else /* slave to master */
   {
      nStatus = sci2c_SlaveToMasterDataTx(conn_ctx, pSCI2C);
      if (nStatus != SCI2C_S2M_OK)
      {
          return eSci2c_Error;
      }
   }

   rv = eSci2c_No_Error;
exit:
    return rv;
}


/* -------------------------- local functions ----------------------------- */
static U8 GetCounter(void)
{
   if (gSeqCtr >= 8)
   {
      gSeqCtr = 0;
   }
   return gSeqCtr++;
}

/* sci2c_WaitForStatusOkay
 * Returns 1 if the status is okay before the timeout expires.
 */
static U8 sci2c_WaitForStatusOkay(void* conn_ctx, U32 msec)
{
   U8 status = SCI2C_STATUS_EXCEPTION_OTHER;
   eSci2c_Error_t rv = eSci2c_Error;
   U32 time = 0;
   U32 waitingOnStatusResponse = 0;

   // Get the status until the status indicates 'slave ready'
   // - In case the Waiting Time Extension Period has expired: Return with an error

   while (time < msec)
   {
      rv = sci2c_GetStatus(conn_ctx, &status);
      if ( (rv == eSci2c_No_Error) && (status == SCI2C_STATUS_NORMAL_READY) )
      {
         break;
      }
      else if ( (rv == eSci2c_No_Error) && (status == SCI2C_STATUS_NORMAL_BUSY) )
      {
          // Status busy resets timer for Waiting Time Extension violation.
          waitingOnStatusResponse = 0;
      }
      // else if ( (rv == eSci2c_Error) || (rv == eSci2c_Protocol_Error) )
      // {
      //     printf("sci2c_WaitForStatusOkay: sci2c_GetStatus returned %d (status == 0x%02X)\r\n", rv, status);
      //     return 0;
      // }
      waitingOnStatusResponse += SCI2C_tMD_ms;

      if ( waitingOnStatusResponse > SCI2C_tFW_DEF_ms)
      {
          // Violation on Waiting Time extension
          return 0;
      }
      time += SCI2C_tMD_ms;
      sm_sleep(SCI2C_tMD_ms);
   }

   if (time >= msec)
   {
      return 0;
   }
   else
   {
      return 1;
   }
}

/* ---------------------- */
/**
 * Packages and sends the APDU command via the SCI2C protocol.
 * Receives and unwraps the APDU response via the SCI2C protocol.
 * @param[in,out] pApdu APDU structure.
 * @retval ::SMCOM_OK Successful execution
 * @retval ::SMCOM_SND_FAILED
 * @retval ::SMCOM_RCV_FAILED
 */
U32 sci2c_Transceive(void* conn_ctx, apdu_t * pApdu)
{
   uint16_t expectedLen = 0;
   U8 tx[1] = { APDU_GET_RESPONSE };
   tSCI2C_Data_t oSCI2C;
   tSCI2C_Data_t * pSCI2C = (tSCI2C_Data_t *) &oSCI2C;
   eSci2c_Error_t sci2c_Error = eSci2c_No_Error;
   U32 rv = SMCOM_SND_FAILED;

   ENSURE_OR_GO_EXIT(pApdu != NULL);

   pSCI2C->dir = eSci2c_DirectionM2S;
   pSCI2C->edc = eEdc_NoErrorDetection;
   pSCI2C->pBuf = (U8*) pApdu->pBuf;
   pSCI2C->buflen = pApdu->buflen;


   sci2c_Wakeup(conn_ctx);


   sci2c_Error = sci2c_DataExchange(conn_ctx, pSCI2C);
   if (sci2c_Error != eSci2c_No_Error)
   {
       return SMCOM_SND_FAILED;
   }

   pSCI2C->dir = eSci2c_DirectionS2M;
   pSCI2C->edc = eEdc_NoErrorDetection;
   pSCI2C->pBuf = (U8*) &tx;
   pSCI2C->buflen = 1;

   // default Rx buffer
   if (pApdu->pBuf == NULL)
   {
      pApdu->pBuf = pSCI2C->pBuf;
   }

   pSCI2C->pBuf = pApdu->pBuf;
   pSCI2C->buflen = expectedLen;

   sci2c_Error = sci2c_DataExchange(conn_ctx, pSCI2C);
   pApdu->rxlen = pSCI2C->buflen;
   // reset offset for subsequent response parsing
   pApdu->offset = 0;
   if (sci2c_Error != eSci2c_No_Error)
   {
       return SMCOM_RCV_FAILED;
   }

   rv = SMCOM_OK;
exit:
    return rv;
}

/**
 * Packages and sends the \p pTx byte array via the SCI2C protocol.
 * Receives and unwraps the APDU response via the SCI2C protocol and stores the result in \p pRx.
 * @param[in] pTx The input buffer
 * @param[in] txLen The input buffer length.
 * @param[in,out] pRx The output buffer.
 * @param[in,out] pRxLen The output buffer length.
 * @retval ::SMCOM_OK Successful execution
 * @retval ::SMCOM_SND_FAILED
 * @retval ::SMCOM_RCV_FAILED
 */
U32 sci2c_TransceiveRaw(void* conn_ctx, U8 * pTx, U16 txLen, U8 * pRx, U32 * pRxLen)
{
   U8 tx[1] = { APDU_GET_RESPONSE };
   tSCI2C_Data_t oSCI2C;
   tSCI2C_Data_t * pSCI2C = (tSCI2C_Data_t *) &oSCI2C;
   eSci2c_Error_t sci2c_Error = eSci2c_No_Error;

   pSCI2C->dir = eSci2c_DirectionM2S;
   pSCI2C->edc = eEdc_NoErrorDetection;
   pSCI2C->pBuf = pTx;
   pSCI2C->buflen = txLen;


   sci2c_Wakeup(conn_ctx);


   sci2c_Error = sci2c_DataExchange(conn_ctx, pSCI2C);
   if (sci2c_Error != eSci2c_No_Error)
   {
       return SMCOM_SND_FAILED;
   }

   pSCI2C->dir = eSci2c_DirectionS2M;
   pSCI2C->edc = eEdc_NoErrorDetection;
   pSCI2C->pBuf = (U8*) &tx;
   pSCI2C->buflen = 1;
   pSCI2C->pBuf = pRx;

   sci2c_Error = sci2c_DataExchange(conn_ctx, pSCI2C);
   *pRxLen = pSCI2C->buflen;

   if (sci2c_Error != eSci2c_No_Error)
   {
       return SMCOM_RCV_FAILED;
   }

   return SMCOM_OK;
}

static i2c_status_t sci2c_SendByte(void* conn_ctx, sci2c_Data_t * pSci2cData)
{
    i2c_error_t status = I2C_FAILED;

    ENSURE_OR_GO_EXIT(pSci2cData != NULL);

    txData[0] = pSci2cData->pcb;

    if (pSci2cData->dataLen > 0) /* add length byte + data */
    {
        return i2c_Failed;
    }

    /* only write 1 byte (the PCB) */
#ifdef PLATFORM_IMX
    status = axI2CWriteByte(conn_ctx, I2C_BUS_0, SMCOM_I2C_ADDRESS, (U8 *) &txData);
#else
    status = axI2CWrite(conn_ctx, I2C_BUS_0, SMCOM_I2C_ADDRESS, (U8 *) &txData, 1);
#endif
    if (status == I2C_OK)
    {
        return i2c_Okay;
    }
    else if (status == I2C_NACK_ON_ADDRESS)
    {
        return i2c_NoAddrAck;
    }
    else
    {
        // NOTE: An error code of 0x0D may be the (legitimate) return code
        // for an SCI2C wakeup command.
#ifndef PLATFORM_IMX
        sm_printf(DBGOUT, "I2C status: 0x%02X\r\n", status);
#endif
        return i2c_Failed;
    }
exit:
    return i2c_Failed;
}

static i2c_status_t sci2c_WriteBlock(void* conn_ctx, sci2c_Data_t * pSci2cData)
{
   U16 write_len = 1; /* PCB byte */
#ifdef LOG_I2C
   int i = 0;
#endif
   i2c_error_t status = I2C_FAILED;

   ENSURE_OR_GO_EXIT(pSci2cData != NULL);

   txData[0] = pSci2cData->pcb;

   if (pSci2cData->dataLen > 0)
   {
      txData[1] = pSci2cData->dataLen;
      memcpy(&txData[2], pSci2cData->pData, pSci2cData->dataLen);
      write_len += pSci2cData->dataLen + 1; /* add LEN byte + data length */
   }

#ifdef LOG_I2C
   printf("\r\n/send ");
   for (i = 0; i< write_len; i++)
   {
        printf("%02x", txData[i]);
   }
   printf("\r\n");
#endif

   status = axI2CWrite(conn_ctx, I2C_BUS_0, SMCOM_I2C_ADDRESS, (U8 *) &txData, write_len);

#ifdef LOG_I2C
   printf("WRITE BLOCK done: %d\r\n", status);
#endif

   if (status == I2C_OK)
   {
      return i2c_Okay;
   }
   else if (status == I2C_STARTED)
   {
      return i2c_Okay;
   }
   else if (status == I2C_NACK_ON_ADDRESS)
   {
      return i2c_NoAddrAck;
   }
   else
   {
      sm_printf(DBGOUT, "I2C status %x\r\n", status);
      return i2c_Failed;
   }
exit:
    return i2c_Failed;
}

//
// The values returned by axI2CWriteRead depend on the driver implementation.
//
static i2c_status_t sci2c_ReadBlock(void* conn_ctx, sci2c_Data_t * pSci2cData, U8 * pRead, U16 * pReadLen)
{
   U16 readlen = 0;
   i2c_error_t status = I2C_FAILED;
   U16 write_len = 1; /* LEN byte */
   i2c_status_t rv = i2c_Failed;

   ENSURE_OR_GO_EXIT(pSci2cData != NULL);
   ENSURE_OR_GO_EXIT(pRead != NULL);
   ENSURE_OR_GO_EXIT(pReadLen != NULL);

#ifdef LOG_I2C
   int i = 0;
#endif

   sm_sleep(DELAY_MSEC);

   txData[0] = pSci2cData->pcb;

   status = axI2CWriteRead(conn_ctx, I2C_BUS_0, SMCOM_I2C_ADDRESS, (U8 *) &txData, write_len, pRead, (U16 *) &readlen);

   if (status != I2C_OK)
   {
      // (Try) not to report NAKs on address: This approach (currently) does not work on iMX Linux driver
      // so a nack ripples up as an i2c_failed!
      if (status != I2C_NACK_ON_ADDRESS)
      {
         return i2c_Failed;
      }
      else
      {
         return i2c_NoAddrAck;
      }
   }
   else if (readlen < 2)
   {
      // should not happen
      sm_printf(DBGOUT,"Error: %d bytes read\r\n", readlen);
      return i2c_Failed;
   }
   else if ((pRead[0] + 1) != readlen) /* the LEN byte does not match the number of read bytes */
   {
      sm_printf(DBGOUT,"Wrong length %02X %02X\r\n", pRead[0], pRead[1]);
      return i2c_Failed;
   }
   else if ((pRead[1] & 0x8F) == 0x87) /* the slave indicates an exception */
   {
      sm_printf(DBGOUT,"Protocol exception %02X %02X\r\n", pRead[0], pRead[1]);
      *pReadLen = readlen;
      sm_sleep(DELAY_MSEC);
      return i2c_Sci2cException;
   }

#ifdef LOG_I2C
   printf("\r\n/rcv ");
   for (i = 0; i< readlen; i++)
   {
        printf("%02x", pRead[i]);
   }
   printf("\r\n");
#endif

   sm_sleep(DELAY_MSEC);

   *pReadLen = readlen;

   rv = i2c_Okay;
exit:
    return rv;
}
