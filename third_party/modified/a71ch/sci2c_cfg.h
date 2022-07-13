/**
 * @file sci2c_cfg.h
 * @author NXP Semiconductors
 * @version 1.0
 * @par License
 *
 * Copyright 2016 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * @par Description
 * configuration file for communication between Host and Secure Module.
 */


#ifndef _SCI2C_CFG_H
#define _SCI2C_CFG_H

// requested slave to master length
// #define REQ_S2M_LENGTH               (eMax254BytesS2M)

// max retries
#define N_RETRY_PD                  (3)
#define N_RETRY_PE                  (3)
#define N_RETRY_SRST                (3)

// timeouts & delays
#define T_PDE                       (0) // 150 usec
#define T_RSTG                      (5)
#define T_WNCMD_ACTUAL              (100) // 100 ms
// tCMDG : 180 usec - minimum. Safeguard to 200us
#define T_CMDG_USec                 (200)

#define APP_SCI2C_TIMEOUT_ms        (100000) // 100 sec
#ifdef SPI
#define SCI2C_tMD_ms                (50)
#else
#define SCI2C_tMD_ms                (2)
#endif

#define SMCOM_I2C_ADDRESS           (0x92)
#define SMCOM_EDC_MODE              (eEdc_NoErrorDetection)
// Linux SMBUS blockread (part of i2c driver) limits max packet size of data read back
// Non-standard I2C master implementations may support bigger max packet sizes.
#if defined(__gnu_linux__)
    #define SMCOM_MAX_BYTES             (eMax31BytesS2M)
#else
    #define SMCOM_MAX_BYTES             (eMax254BytesS2M)
#endif
//

#endif //_SCI2C_CFG_H
