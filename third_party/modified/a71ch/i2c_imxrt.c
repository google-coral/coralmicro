/*
 *
 * Copyright 2018,2020 NXP
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * I2C implmentation for ICs related to i.MX RT Family
 */

#include "libs/base/main_freertos_m7.h"
#include <stdio.h>
#include "board.h"
#if defined(FSL_FEATURE_SOC_LPI2C_COUNT) && FSL_FEATURE_SOC_LPI2C_COUNT > 0
#include "i2c_a7.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"

#include "sm_timer.h"
#include "fsl_gpio.h"
#include "sci2c_cfg.h"

#if defined(SCI2C)
#define I2C_BAUDRATE (400u * 1000u) // 400K
#elif defined(T1oI2C)
//#define I2C_BAUDRATE (3400u * 1000u) // 3.4. Not used by default
#define I2C_BAUDRATE (400u * 1000u) // 400K
#else
#error "Invalid combination"
#endif


#if defined(__GNUC__)
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

#define DELAY_LPI2C_US            (0)

#define LPI2C_LOG_PRINTF(...) do { printf("\r\n[%04d] ",__LINE__); printf(__VA_ARGS__); } while (0)

#if defined(CPU_MIMXRT1062DVL6A) /* TODO: Should be board specific */
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#   define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#   define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#   define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#   define AX_I2CM              LPI2C1
#   define AX_LPI2C_CLK_SRC       LPI2C_CLOCK_FREQUENCY
#   define AX_I2CM_IRQN         LPI2C1_IRQn
#   define USE_LIP2C            1
#elif defined(CPU_MIMXRT1176DVMAA_cm7) || defined(CPU_MIMXRT1176CVM8A_cm7) /* TODO: Should be board specific */
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#   define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#   define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#   define LPI2C_CLOCK_FREQUENCY (CLOCK_GetFreq(kCLOCK_OscRc48MDiv2))
// #   define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#   define AX_I2CM              (LPI2C_Type *)LPI2C5_BASE
#   define AX_LPI2C_CLK_SRC       LPI2C_CLOCK_FREQUENCY
#   define AX_I2CM_IRQN         LPI2C5_IRQn
#   define USE_LIP2C            1
#endif

// #define LPI2C_DEBUG

#if USE_LIP2C
#   include "fsl_lpi2c.h"
#   if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
#       include "fsl_lpi2c_freertos.h"
#   endif

static status_t LPI2C_MasterTransferBlocking_Send_MODIFIED(LPI2C_Type *base, uint16_t slaveAddress, void *data, size_t dataSize);
static status_t LPI2C_MasterTransferBlocking_Receive_MODIFIED(LPI2C_Type *base, uint16_t slaveAddress, void *data);
static status_t LPI2C_MasterReceive_MODIFIED(LPI2C_Type *base, void *rxBuff);

static status_t LPI2C_MasterWaitForTxReady(LPI2C_Type *base);
#else /* FSL_FEATURE_SOC_LPI2C_COUNT */
#   include "fsl_i2c.h"
#   if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
#       include "fsl_i2c_freertos.h"
#   endif
#endif  /* FSL_FEATURE_SOC_LPI2C_COUNT */


uint32_t LPI2C1_GetFreq(void)
{
    return LPI2C_CLOCK_FREQUENCY;
}

#if 0
#   define baudRate_Bps                   baudRate_Hz

#endif  /* FSL_FEATURE_SOC_LPI2C_COUNT */

#if defined(I2C_DEBUG) || 1
#define DEBUG_PRINT_KINETIS_I2C(Operation, status)                                                       \
    if(result == kStatus_Success) { /* LPI2C_LOG_PRINTF(Operation " OK\r\n");                   */ }       \
    else if(result == kStatus_LPI2C_Nak) { /* LPI2C_LOG_PRINTF(Operation " A-Nak\r\n");  		*/ }       \
    else if(result == kStatus_LPI2C_Busy) LPI2C_LOG_PRINTF(Operation " Busy\r\n");                           \
    else if(result == kStatus_LPI2C_Idle) LPI2C_LOG_PRINTF(Operation " Idle\r\n");                           \
    else if(result == kStatus_LPI2C_Timeout) LPI2C_LOG_PRINTF(Operation " T/O\r\n");                         \
    else if(result == kStatus_LPI2C_ArbitrationLost) LPI2C_LOG_PRINTF(Operation " ArbtnLost\r\n");           \
    else LPI2C_LOG_PRINTF (Operation " ERROR  : 0x%02lX\r\n", status);
#else
#   define DEBUG_PRINT_KINETIS_I2C(Operation, status)
#endif

/* Handle NAK from the A71CH */
static volatile int gBackoffDelay;

static int address_nack = 0;



void axI2CResetBackoffDelay() {
    gBackoffDelay = 0;
}

static void BackOffDelay_Wait() {
    if (gBackoffDelay < 200 ) {
        gBackoffDelay += 1;
    }
    sm_sleep(gBackoffDelay);
}

static i2c_error_t kinetisI2cStatusToAxStatus(
    status_t kinetis_i2c_status)
{
    i2c_error_t retStatus;
    switch (kinetis_i2c_status)
    {
    case kStatus_Success:
        axI2CResetBackoffDelay();
        retStatus = I2C_OK;
        break;
    case kStatus_LPI2C_Busy:
        retStatus = I2C_BUSY;
        break;
    case kStatus_LPI2C_Idle:
        retStatus = I2C_BUSY;
        break;
    case kStatus_LPI2C_Nak:
        if(address_nack)
        {
            BackOffDelay_Wait();
            retStatus = I2C_NACK_ON_ADDRESS;
            address_nack = 0;
        }
        else
            retStatus = I2C_NACK_ON_DATA;
        break;
    case kStatus_LPI2C_ArbitrationLost:
        retStatus = I2C_ARBITRATION_LOST;
        break;
    case kStatus_LPI2C_Timeout:
        retStatus = I2C_TIME_OUT;
        break;
    default:
        retStatus = I2C_FAILED;
        break;
    }
    return retStatus;
}

#define RETURN_ON_BAD_kinetisI2cStatus(kinetis_i2c_status)                      \
    {                                                                           \
        /* LPI2C_LOG_PRINTF("I2C Status = %d\r\n", kinetis_i2c_status);*/         \
        i2c_error_t ax_status = kinetisI2cStatusToAxStatus(kinetis_i2c_status); \
        if ( ax_status != I2C_OK ) {                                            \
            LPI2C_RTOS_Unlock(I2C5Handle());                                    \
            return ax_status;                                                   \
        }                                                                       \
    }

i2c_error_t axI2CInit(void **conn_ctx, const char *pDevName)
{
#if 0
    lpi2c_master_config_t masterConfig;
    LPI2C_MasterGetDefaultConfig(&masterConfig);
/*    masterConfig.enableDoze = false;
    masterConfig.debugEnable = true;
    masterConfig.baudRate_Hz = I2C_BAUDRATE;
    masterConfig.sdaGlitchFilterWidth_ns = 150;
    masterConfig.sclGlitchFilterWidth_ns = 150;*/
    uint32_t sourceClock = LPI2C_CLOCK_FREQUENCY;//CLOCK_GetFreq(AX_LPI2C_CLK_SRC);
#if defined(SDK_OS_FREE_RTOS) && (SDK_OS_FREE_RTOS == 1)
    NVIC_SetPriority(AX_I2CM_IRQN, 3);
    LPI2C_RTOS_Init(I2C5Handle(), AX_I2CM, &masterConfig, sourceClock);
#else
    LPI2C_MasterInit(AX_I2CM, &masterConfig, sourceClock);

    LPI2C_MasterCheckAndClearError(AX_I2CM, LPI2C_MasterGetStatusFlags(AX_I2CM));
#endif
#endif
    return I2C_OK;
}

void axI2CTerm(
    void* conn_ctx,
    int mode)
{
}

#if defined(SDK_OS_FREE_RTOS) && (SDK_OS_FREE_RTOS == 1)
#    define    I2CM_TX() \
        result = LPI2C_RTOS_Transfer(I2C5Handle(), &masterXfer)
#else
#    define    I2CM_TX() \
        result = LPI2C_MasterTransferBlocking(AX_I2CM, &masterXfer)
#endif

#if defined (__ICCARM__) || (__CC_ARM__)
_Pragma ("optimize=none")
#endif
unsigned int axI2CWrite(
    void* conn_ctx, unsigned char bus_unused_param, unsigned char addr, unsigned char * pTx, unsigned short txLen)
{
    status_t result;
    lpi2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer)); //clear values

    if(pTx == NULL || txLen > MAX_DATA_LEN)
    {
        return I2C_FAILED;
    }

    LPI2C_RTOS_Lock(I2C5Handle());

#ifdef LPI2C_DEBUG
    LPI2C_LOG_PRINTF("I2C Write \r\n");
#endif
    masterXfer.slaveAddress = addr >> 1; // the address of the A70CM
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data = pTx;
    masterXfer.dataSize = txLen;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;
    I2CM_TX();

/*TODO: LPI2C_MasterTransferBlocking doent not handle A71CH post reset properly. Need to be further analysed*/
//    if (result)
//    {
//        return result;
//   }


/*The below loop is getting infinite in case of Reset A71ch. Need to further analyse*/
    // block until the end, this will not differentiate between NACK on data and NACK on address
//    do {
//        status = LPI2C_MasterGetStatusFlags(AX_I2CM);
//    } while(status & kLPI2C_MasterBusyFlag);

    sm_sleep(2);
    result = LPI2C_MasterCheckAndClearError(AX_I2CM, LPI2C_MasterGetStatusFlags(AX_I2CM));

    DEBUG_PRINT_KINETIS_I2C("WR",result);
    RETURN_ON_BAD_kinetisI2cStatus(result);

    LPI2C_RTOS_Unlock(I2C5Handle());

    return I2C_OK;
}

#if defined (__ICCARM__) || (__CC_ARM__)
_Pragma ("optimize=none")
#endif
unsigned int axI2CWriteRead(
    void* conn_ctx, unsigned char bus_unused_param, unsigned char addr, unsigned char * pTx, unsigned short txLen, unsigned char * pRx,
    unsigned short * pRxLen)
{
    lpi2c_master_transfer_t masterXfer;
    status_t result;
    memset(&masterXfer, 0, sizeof(masterXfer)); //clear values

    if(pTx == NULL || txLen > MAX_DATA_LEN)
    {
        return I2C_FAILED;
    }

    if(pRx == NULL || *pRxLen > MAX_DATA_LEN)
    {
        return I2C_FAILED;
    }

    LPI2C_RTOS_Lock(I2C5Handle());

    *pRxLen = 0;
    memset(pRx, 0, 2);

    //Write as part of WriteRead ( special made function )
    result = LPI2C_MasterTransferBlocking_Send_MODIFIED(AX_I2CM, addr >> 1, pTx, txLen);

    DEBUG_PRINT_KINETIS_I2C("WR",result);
    RETURN_ON_BAD_kinetisI2cStatus(result);

    //Read as part of WriteRead ( heavily modified LPI2C_MasterTransferBlocking() )
    result = LPI2C_MasterTransferBlocking_Receive_MODIFIED(AX_I2CM, addr >> 1, pRx);
    *pRxLen = pRx[0] + 1;

    DEBUG_PRINT_KINETIS_I2C("RD",result);
    RETURN_ON_BAD_kinetisI2cStatus(result);


    LPI2C_RTOS_Unlock(I2C5Handle());

    return I2C_OK;
}

#if defined (__ICCARM__) || (__CC_ARM__)
_Pragma ("optimize=none")
#endif
unsigned int axI2CRead(void* conn_ctx, unsigned char bus, unsigned char addr, unsigned char * pRx, unsigned short rxLen)
{
	lpi2c_master_transfer_t masterXfer;
    status_t result;
    memset(&masterXfer, 0, sizeof(masterXfer)); //clear values

    if(pRx == NULL || rxLen > MAX_DATA_LEN)
    {
        return I2C_FAILED;
    }

#if defined(SCI2C_DEBUG)
    I2C_LOG_PRINTF("\r\n I2C Read \r\n");
#endif

    LPI2C_RTOS_Lock(I2C5Handle());

    masterXfer.slaveAddress = addr >> 1; // the address of the A70CM
    //masterXfer.slaveAddress = addr;
    masterXfer.direction = kLPI2C_Read;
    masterXfer.subaddress = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data = pRx;
    masterXfer.dataSize = rxLen;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    I2CM_TX();

#if defined(CPU_MIMXRT1176DVMAA_cm7) || defined(CPU_MIMXRT1176CVM8A_cm7) /* TODO: Should be board specific */
    uint32_t status = LPI2C_MasterGetStatusFlags(AX_I2CM);
    status &= kLPI2C_MasterBusyFlag | kLPI2C_MasterBusBusyFlag;
    if (status) {
        address_nack = 1;
    }
#endif

    DEBUG_PRINT_KINETIS_I2C("RD",result);
    RETURN_ON_BAD_kinetisI2cStatus(result);

    LPI2C_RTOS_Unlock(I2C5Handle());

    return I2C_OK;
}


#if defined (__ICCARM__) || (__CC_ARM__)
_Pragma ("optimize=none")
#endif
// this function calls LPI2C_MasterTransferBlocking() for writing then actually blocks waiting for the address word to be sent
static status_t LPI2C_MasterTransferBlocking_Send_MODIFIED(LPI2C_Type *base, uint16_t slaveAddress, void *data, size_t dataSize)
{
    lpi2c_master_transfer_t masterXfer = {0};
    status_t result = kStatus_Success;
    size_t txCount;
    volatile uint32_t status;

    masterXfer.slaveAddress = slaveAddress; // the address of the A70CM
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data = data;
    masterXfer.dataSize = dataSize;
    masterXfer.flags = kLPI2C_TransferNoStopFlag;

    result = LPI2C_MasterTransferBlocking(base, &masterXfer);

    if (result)
    {
        return result;
    }

    // make sure the address word is sent,
    //    the loop exits either after an error flag (e.g. NACK)
    //    or after the first and only data word start sending
    do {
        LPI2C_MasterGetFifoCounts(base, NULL, &txCount);
        status = LPI2C_MasterGetStatusFlags(base);
        status &= kLPI2C_MasterNackDetectFlag | kLPI2C_MasterArbitrationLostFlag | kLPI2C_MasterFifoErrFlag | kLPI2C_MasterPinLowTimeoutFlag;
    } while ((!status) && (txCount >= dataSize));

    // if the error was indeed a NACK, then it is a I2C_NACK_ON_ADDRESS
    if (status & kLPI2C_MasterNackDetectFlag)
    {
        address_nack = 1; // check to return the right error code to upper layer
    }

    // if there was an error, wait for the master to issue a stop bit
    if (status)
    {
        do {
            status = LPI2C_MasterGetStatusFlags(base);
        } while(status & kLPI2C_MasterBusyFlag);
    }

    result = LPI2C_MasterCheckAndClearError(base, LPI2C_MasterGetStatusFlags(base));

    return result;
}

// modified version of LPI2C_MasterTransferBlocking() for reading
static status_t LPI2C_MasterTransferBlocking_Receive_MODIFIED(LPI2C_Type *base, uint16_t slaveAddress, void *data)
{
    status_t result = kStatus_Success;
    uint16_t commandBuffer;
    int kMasterClearFlags = kLPI2C_MasterEndOfPacketFlag | kLPI2C_MasterStopDetectFlag | kLPI2C_MasterNackDetectFlag |
            kLPI2C_MasterArbitrationLostFlag | kLPI2C_MasterFifoErrFlag | kLPI2C_MasterPinLowTimeoutFlag |
            kLPI2C_MasterDataMatchFlag;
    int kStartCmd = LPI2C_MTDR_CMD(0x4U);

    /* Return an error if the bus is already in use not by us. */
    result = LPI2C_CheckForBusyBus(base);
    if (result)
    {
        return result;
    }

    /* Clear all flags. */
    LPI2C_MasterClearStatusFlags(base, kMasterClearFlags);

    /* Turn off auto-stop option. */
    base->MCFGR1 &= ~LPI2C_MCFGR1_AUTOSTOP_MASK;

    // repeated start
    commandBuffer = (uint16_t)kStartCmd | (uint16_t)((uint16_t)((uint16_t)slaveAddress << 1U) | (uint16_t)kLPI2C_Read);

    /* Send command buffer */
    /* Wait until there is room in the fifo. This also checks for errors. */
    result = LPI2C_MasterWaitForTxReady(base);

    if (result)
    {
        return result;
    }


    /* Write byte into LPI2C master data register. */
    base->MTDR = commandBuffer;

    /* Receive Data. */
    result = LPI2C_MasterReceive_MODIFIED(base, data);

    if (result)
    {
        return result;
    }

    result = LPI2C_MasterStop(base);

    if (result)
    {
        return result;
    }

    return kStatus_Success;
}

#if defined (__ICCARM__) || (__CC_ARM__)
_Pragma ("optimize=none")
#endif
// modified version of LPI2C_MasterReceive() to allow smbus block read
static status_t LPI2C_MasterReceive_MODIFIED(LPI2C_Type *base, void *rxBuff)
{
    status_t result;
    uint8_t *buf;
    buf = (uint8_t *)rxBuff;
    volatile size_t rxSize;
    uint16_t kRxDataCmd = LPI2C_MTDR_CMD(0X1U);

    if (rxBuff == NULL) {

        return kStatus_Fail;
    }

    /* Wait until there is room in the command fifo. */
    result = LPI2C_MasterWaitForTxReady(base);
    if (result)
    {
        return result;
    }
    /* Clear Receive FIFO */
    base->MCR |= LPI2C_MCR_RRF_MASK;
    /* Issue command to receive data. */
    base->MTDR = kRxDataCmd; // read first byte (count byte)



    base->MTDR = kRxDataCmd; // read second byte
    result = LPI2C_MasterWaitForTxReady(base);
#if LPI2C_WAIT_TIMEOUT
    volatile uint32_t waitTimes = LPI2C_WAIT_TIMEOUT;
#endif

    /* Receive count byte */

    /* Read LPI2C receive fifo register. The register includes a flag to indicate whether */
    /* the FIFO is empty, so we can both get the data and check if we need to keep reading */
    /* using a single register read. */
    volatile uint32_t value;

    do
    {
        /* Check for errors. */
        result = LPI2C_MasterCheckAndClearError(base, LPI2C_MasterGetStatusFlags(base));
        if (result)
        {
            return result;
        }

value = base->MRDR;
#if LPI2C_WAIT_TIMEOUT
    } while ((value & LPI2C_MRDR_RXEMPTY_MASK) && (--waitTimes));
    if (waitTimes == 0)
    {
        return kStatus_LPI2C_Timeout;
    }
#else
    } while (value & LPI2C_MRDR_RXEMPTY_MASK);
#endif
    rxSize = value & LPI2C_MRDR_DATA_MASK;

    rxSize = value & LPI2C_MRDR_DATA_MASK;
    *buf++ = rxSize;

    if (rxSize > 1)
    {
        result = LPI2C_MasterWaitForTxReady(base);
        if (result)
        {
            return result;
        }
        base->MTDR = kRxDataCmd | LPI2C_MTDR_DATA(rxSize - 2); // read the rest
    }

#if LPI2C_WAIT_TIMEOUT
    waitTimes = LPI2C_WAIT_TIMEOUT;
#endif
    /* Receive rest of data */
    while (rxSize--)
    {
        /* Read LPI2C receive fifo register. The register includes a flag to indicate whether */
        /* the FIFO is empty, so we can both get the data and check if we need to keep reading */
        /* using a single register read. */
        volatile uint32_t value;
        do
        {
            /* Check for errors. */
            result = LPI2C_MasterCheckAndClearError(base, LPI2C_MasterGetStatusFlags(base));
            if (result)
            {
                return result;
            }

            value = base->MRDR;
#if LPI2C_WAIT_TIMEOUT
        } while ((value & LPI2C_MRDR_RXEMPTY_MASK) && (--waitTimes));
        if (waitTimes == 0)
        {
            return kStatus_LPI2C_Timeout;
        }
#else
        } while (value & LPI2C_MRDR_RXEMPTY_MASK);
#endif

        *buf++ = value & LPI2C_MRDR_DATA_MASK;
    }
    return kStatus_Success;
}



/*!
 * @brief Wait until there is room in the tx fifo.
 * @param base The LPI2C peripheral base address.
 * @retval #kStatus_Success
 * @retval #kStatus_LPI2C_PinLowTimeout
 * @retval #kStatus_LPI2C_ArbitrationLost
 * @retval #kStatus_LPI2C_Nak
 * @retval #kStatus_LPI2C_FifoError
 */
#if defined (__ICCARM__) || (__CC_ARM__)
_Pragma ("optimize=none")
#endif
static status_t LPI2C_MasterWaitForTxReady(LPI2C_Type *base)
{
    volatile uint32_t status;
    size_t txCount;
    size_t txFifoSize = FSL_FEATURE_LPI2C_FIFO_SIZEn(base);

#if LPI2C_WAIT_TIMEOUT
    volatile uint32_t waitTimes = LPI2C_WAIT_TIMEOUT;
#endif
    do
    {
        status_t result;

        /* Get the number of words in the tx fifo and compute empty slots. */
        LPI2C_MasterGetFifoCounts(base, NULL, &txCount);
        txCount = txFifoSize - txCount;

        /* Check for error flags. */
        status = LPI2C_MasterGetStatusFlags(base);
        result = LPI2C_MasterCheckAndClearError(base, status);
        if (result)
        {
            return result;
        }
#if LPI2C_WAIT_TIMEOUT
    } while ((!txCount) && (--waitTimes));

    if (waitTimes == 0)
    {
        return kStatus_LPI2C_Timeout;
    }
#else
    } while (!txCount);
#endif

    return kStatus_Success;
}

#if defined(__GNUC__)
#pragma GCC pop_options
#endif

#endif /* FSL_FEATURE_SOC_LPI2C_COUNT */
