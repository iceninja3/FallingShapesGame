/**
  ******************************************************************************
  * @file    se_stsafea_api.h
  * @author  SMD application team
  * @version V2.0.0
  * @brief   Header file of SE STSAFEA API.
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
#ifndef __SE_STSAFEA_API_H__
#define __SE_STSAFEA_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stsafea_core.h"
#include "se_stsafea_api_struct.h"

/** @addtogroup SE_STSAFEA_API
  * @{
  */


/** @addtogroup Core
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup Core_Exported_Types Core's exported types
  * @{
  */
/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup Core_Exported_Defines Core's exported defines
  * @{
  */


/** @defgroup SE_STSAFEA_API's version
  * @{
  */

/**
  * @}
  */


/** @defgroup SE_STSAFEA_API Commands definition
  * @{
  */

#define WRAP_CMD_MAC STSAFEA_MAC_HOST_CMAC
#define WRAP_CMD_ENC STSAFEA_ENCRYPTION_COMMAND

#define UNWRAP_CMD_MAC STSAFEA_MAC_HOST_CMAC
#define UNWRAP_CMD_ENC STSAFEA_ENCRYPTION_RESPONSE

#define SHARED_SECRET_CMD_MAC STSAFEA_MAC_HOST_CMAC
#define SHARED_SECRET_CMD_ENC STSAFEA_ENCRYPTION_NONE

#define GEN_SIG_MAC STSAFEA_MAC_HOST_CMAC
#define GEN_SIG_ENC STSAFEA_ENCRYPTION_NONE


/**
  * @}
  */


#define SE_DEBUG /*! Activate SE_TRACES */

/** @defgroup SE_STSAFEA_API Debuf definition
  * @{
  */
#ifdef SE_DEBUG
#define SE_TRACE_BUFF_SIZE   512
#define SE_PRINTF(...) se_printf(0, __VA_ARGS__)
#define SE_PRINTF_RES(...) se_printf(1, __VA_ARGS__)
#ifndef ENDL
#define ENDL "\n\r"
void se_printf(uint8_t status, const char *fmt, ...);
#endif /* ENDL */
#else
#define SE_PRINTF(...)
#define SE_PRINTF_RES(...)
#endif /* SE_DEBUG */
/**
  * @}
  */


/** @defgroup SE_STSAFEA_API_INIT_OPTION Bits definition for SE_Init command
  * @{
  */

/**
  * @}
  */


StSafeA_ResponseCode_t StSafeA_Crypto_GetKeys(uint8_t *serial, size_t serial_size);

#ifdef SE_FREERTOS
#include "cmsis_os.h"
#endif /* SE_FREERTOS */


typedef struct
{
  uint8_t  region;
  uint16_t key_offset;
  uint16_t cert_offset;
} SE_Auto_Management_t;

#define SE_PSA_SUPPORT


typedef struct
{
  StSafeA_Handle_t    se_handle;
  uint8_t       init;
  uint8_t       a_rx_tx_se_data [STSAFEA_BUFFER_MAX_SIZE];
  uint16_t        slot0_key_size;
  uint16_t        slot1_key_size;
  uint16_t        slotff_key_size;
  uint8_t       active_key;
  uint8_t       active_certificate;
  uint8_t       security_level;
  uint8_t       cmd_params[SE_CMD_MAX][2];
  uint16_t        max_buffer_size;
  uint8_t               serial[STSAFEA_ST_NUMBER_LENGTH];
  uint8_t               serial_loaded;
  SE_Region_t           *mapping;
  uint32_t              mapping_size;
  SE_Auto_Management_t  auto_managed;
#ifdef SE_FREERTOS
  osMutexId(se_api_mutex);
#endif /* SE_FREERTOS */
#ifdef SE_DEBUG
  int32_t (*debug_print)(const char *fmt, ...);
  char          trace_buff[SE_TRACE_BUFF_SIZE];
#endif /* SE_DEBUG */
  SE_Public_Key_Entry_t fixed_pub_keys[SE_FIXED_PRIV_KEY_MAX];
  uint32_t        nb_renewable_pub_keys;
  SE_Public_Key_Entry_t *renewable_pub_keys[SE_RENEWABLE_PRIV_KEY_MAX];
} SE_Ctx_t;

/* Primitive API */
SE_API_Status_t SE_Init(uint32_t flag, SE_Security_Level_t sec_level, uint32_t i2c_addr);
SE_API_Status_t SE_SetSecurity(SE_Security_Level_t level);
uint32_t SE_GetKeySize(uint8_t slot);
SE_API_Status_t SE_Read(uint8_t region, uint16_t offset, uint16_t size, uint8_t *buf);
SE_API_Status_t SE_ReadRight(uint8_t region, uint16_t offset, uint32_t update, uint16_t size, uint8_t *buf);
SE_API_Status_t SE_Update(uint8_t region, uint16_t offset, uint16_t size, uint8_t *buf);
SE_API_Status_t SE_UpdateRight(uint8_t region, uint16_t offset, uint32_t update, uint16_t size, uint8_t *buf);
SE_API_Status_t SE_GenerateWrapKey(int8_t slot, SE_Wrap_Type_t type);
SE_API_Status_t SE_Wrap(uint8_t slot, uint8_t *clear_buf, uint16_t size, uint8_t *env_buf);
SE_API_Status_t SE_Unwrap(uint8_t slot, uint8_t *env_buf, uint16_t size, uint8_t *clear_buf);
SE_API_Status_t SE_GenerateSignature(SE_Slot_t slot, const uint8_t *hash, uint16_t hash_size, uint8_t *R, uint8_t *S);
SE_API_Status_t SE_VerifySignature(SE_Key_Type_t k_type, uint8_t *hash, size_t hash_size, uint8_t *pub_X,
                                   uint8_t *pub_Y,
                                   uint8_t *R, uint8_t *S);
SE_API_Status_t SE_GetSerial(uint8_t *serial, SE_SerialFormat_t format);
SE_API_Status_t SE_Pairing(uint8_t *mac, uint8_t *cipher);
SE_API_Status_t SE_Unlock(uint8_t *password);
SE_API_Status_t SE_Lock(uint8_t *password);
SE_API_Status_t SE_GetMapping(SE_Region_t *regions, uint32_t *nb_region);
SE_API_Status_t SE_GetCommandStatus(SE_Cmd_t *cmd, uint32_t *size);
SE_API_Status_t SE_GetState(SE_State_t *state);
SE_API_Status_t SE_GetPairingStatus(uint8_t *presence, uint32_t *counter);
int32_t SE_GetRandom(void *ctx, uint8_t *data, size_t size);
SE_API_Status_t SE_GetInfo(SE_Info_t *se_info);
SE_API_Status_t SE_Reset(uint32_t type);
SE_API_Status_t SE_Hibernate(uint32_t type);
SE_API_Status_t SE_Echo(uint8_t *input, size_t size, uint8_t *output);
SE_API_Status_t SE_MemoryCheck(uint8_t region, uint16_t offset, uint16_t *size);


/* Applicative API */
SE_API_Status_t SE_StoreCertificate(uint8_t id, uint8_t *certificate, uint16_t size);
SE_API_Status_t SE_GetCertificate(uint8_t region, uint8_t *buf, size_t *size);
SE_API_Status_t SE_SetActiveKey(uint8_t id);
SE_API_Status_t SE_GetActiveKey(uint8_t *id);
SE_API_Status_t SE_SetActiveCertificate(uint8_t id);
SE_API_Status_t SE_GetActiveCertificate(uint8_t *buf, size_t *size);
SE_API_Status_t SE_GenerateKeyPair(SE_Slot_t slot, SE_Key_Type_t k_type, uint8_t *pub_X, uint8_t *pub_Y);
SE_API_Status_t SE_DeriveSharedSecret(SE_Slot_t slot, uint8_t *x, uint8_t *y, uint8_t *z, uint16_t size);
SE_API_Status_t SE_SetAutoManagement(uint8_t region, uint16_t key_offset, uint16_t cert_offset);

/* Extra API */
SE_API_Status_t SE_GenerateKey(uint16_t size, uint8_t *key);
SE_API_Status_t SE_SecureKey(uint16_t size, uint8_t *key, uint8_t *enc_key);
SE_API_Status_t SE_GetKey(uint16_t *size, uint8_t *enc_key, uint8_t *key);
SE_API_Status_t SE_Raw(uint8_t *cmd, size_t cmd_size, uint8_t *rsp, size_t *rsp_size);

/* Debug API */
SE_API_Status_t SE_SetDebug(int32_t (*dbg_print)(const char *fmt, ...));
SE_API_Status_t SE_GenerateEnvelop(uint32_t slot, uint32_t ksize);
SE_API_Status_t SE_SetPairingKeys(uint8_t *keys);
SE_API_Status_t SE_Refresh(void);

/* extra API to load public key */
SE_API_Status_t SE_Init_PubKeys(SE_Public_Key_Entry_t *pub_keys, size_t nb_pub_keys);
SE_API_Status_t SE_Get_PubKey(SE_Slot_t slot, uint8_t *pub_x, uint8_t *pub_y, size_t *size, SE_Key_Type_t *type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__SE_STSAFEA_API_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

