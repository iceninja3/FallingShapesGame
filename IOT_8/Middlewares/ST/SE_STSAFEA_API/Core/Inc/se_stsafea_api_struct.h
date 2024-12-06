/**
  ******************************************************************************
  * @file    se_stsafea_api_struct.h
  * @author  SMD application team
  * @version V2.0.0
  * @brief   Header file of SE STSAFEA API STRUCT.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SE_STSAFEA_API_STRUCT_H__
#define __SE_STSAFEA_API_STRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"

/** @addtogroup SE_STSAFEA_API
  * @{
  */


/** @addtogroup API
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup API_Exported_Types API's exported types
  * @{
  */
/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup API_Exported_Defines API's exported defines
  * @{
  */


/** @defgroup SE_STSAFEA_API's version
  * @{
  */

#define SE_STSAFEA_API_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define SE_STSAFEA_API_VERSION_SUB1   (0x01U) /*!< [23:16] sub1 version */
#define SE_STSAFEA_API_VERSION_SUB2   (0x01U) /*!< [15:8]  sub2 version */
#define SE_STSAFEA_API_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */
/**
  * @}
  */


/** @defgroup SE_STSAFEA_API Commands definition
  * @{
  */
/**
  * @}
  */



/** @defgroup SE_STSAFEA_API Debuf definition
  * @{
  */
/**
  * @}
  */


/** @defgroup SE_STSAFEA_API_INIT_OPTION Bits definition for SE_Init command
  * @{
  */

#define SE_INIT_TEST_ECHO      1
#define SE_INIT_TEST_PAIRING   2

#define SE_MIN_PUB_KEY_SIZE         32  /* support curve from 256 bits  */
#define SE_MAX_PUB_KEY_SIZE     48  /* support curve up to 384 bits */

#define SE_FIXED_PRIV_KEY_MAX   1   /* number of supported fixed private keys */
#define SE_RENEWABLE_PRIV_KEY_MAX   2 /* number of supported renewable private keys */
#define SE_PRIV_KEY_MAX     (SE_FIXED_PRIV_KEY_MAX + SE_RENEWABLE_PRIV_KEY_MAX)   /* number of supported private keys */

/**
  * @}
  */


typedef enum
{
  SE_WRAP_KEY_128,
  SE_WRAP_KEY_256,
} SE_Wrap_Type_t;

typedef enum
{
  SE_SECURITY_LEVEL_NONE,
  SE_SECURITY_LEVEL_C_PAIRING,
  SE_SECURITY_LEVEL_R_PAIRING,
} SE_Security_Level_t;

typedef enum
{
  SE_KEY_TYPE_NIST_256,
  SE_KEY_TYPE_NIST_384,
  SE_KEY_TYPE_BP_256,
  SE_KEY_TYPE_BP_384,
  SE_KEY_TYPE_NONE,
} SE_Key_Type_t;

typedef enum
{
  SE_HASH_SHA_256,
  SE_HASH_SHA_384
} SE_Hash_Type_t;

typedef enum
{
  SE_SLOT_PREPROVISIONNED,
  SE_SLOT_PERMANENT,
  SE_SLOT_EPHEMERAL
}  SE_Slot_t;

typedef enum
{
  SE_SERIAL_FORMAT_RAW,
  SE_SERIAL_FORMAT_HEX
} SE_SerialFormat_t;


enum
{
  SE_CMD_WRAP = 0,
  SE_CMD_UNWRAP,
  SE_CMD_SHARED_SECRET,
  SE_CMD_SIG,
  SE_CMD_GEN_KEY,
  SE_CMD_GEN_KEY_EPHEMERAL,
  SE_CMD_MAX,
};

#define SE_AUTH_PARAM 0
#define SE_ENC_PARAM  1
/**
  * @brief  SE_STSAFEA_API Status structures definition
  */
typedef enum
{
  SE_OK                      = 0x00U,
  SE_COMMAND_ERROR           = 0x01U,
  SE_INVALID_STATE_ERROR     = 0x02U,
  SE_INVALID_PARAMS_ERROR    = 0x03U,
  SE_INVALID_SIG_ERROR       = 0x04U,
  SE_BUFFER_TOO_SHORT_ERROR  = 0x05U,
  SE_PARAMS_UPDATED_ERROR    = 0x06U
} SE_API_Status_t;

#define SE_ACCESS_RIGHT_LOCK                0x10
#define SE_ACCESS_RIGHT_ALWAYS              0x0
#define SE_ACCESS_RIGHT_HOST                0x1
#define SE_ACCESS_RIGHT_NEVER               0x7

typedef struct
{
  uint8_t  index;
  uint16_t size;
  uint8_t  read_ac;
  uint8_t  read_update;
  uint8_t  write_ac;
  uint8_t  write_update;
  uint32_t OneWay;
  uint8_t  type;
} SE_Region_t;

typedef struct
{
  uint8_t id;
  uint8_t auth;
  uint8_t enc;
} SE_Cmd_t;


typedef enum
{
  SE_STATE_OPERATIONNAL,
  SE_STATE_LOCKED,
  SE_STATE_TERMINATED,
  SE_STATE_UNKNOWN,
} SE_State_t;


typedef struct
{
  uint16_t InOutBufferSize;
  uint16_t NVMSize;
  uint16_t AtomicityBuffer;
  uint8_t serial;
  uint8_t raw_data[100];
  uint8_t raw_size;
}   SE_Product_Data_t;

typedef struct
{
  uint8_t I2cAddress;
  uint8_t LowPowerModeConfig;
  uint8_t LockConfig;
}   SE_I2C_Params_t;

typedef struct
{
  uint8_t  present;
  uint32_t counter;
}   SE_Pairing_Info_t;

typedef struct
{
  uint8_t presence0;
  uint8_t size0;
  uint8_t presence1;
  uint8_t size1;
}   SE_Envelop_Info_t;

typedef struct
{
  SE_Product_Data_t productData;
  SE_I2C_Params_t   i2cParams;
  SE_Pairing_Info_t pairing;
  SE_Envelop_Info_t envelop;
  uint8_t           lifecycle;
}   SE_Info_t;

typedef struct
{
  SE_Key_Type_t public_key_curve;
  uint8_t   public_key_x[SE_MAX_PUB_KEY_SIZE];
  uint8_t   public_key_y[SE_MAX_PUB_KEY_SIZE];
} SE_Public_Key_Entry_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SE_STSAFEA_API_STRUCT_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

