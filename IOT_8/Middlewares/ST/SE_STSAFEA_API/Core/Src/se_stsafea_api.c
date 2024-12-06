/**
  ******************************************************************************
  * @file    se_stsafea_api.c
  * @author  SMD application team
  * @version V2.0.0
  * @brief   Implementation file of SE STSAFEA API.
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
#include "se_stsafea_api.h"

/* Accessor helper for internal usage */
#define SE_HDL &se_ctx.se_handle
/* Get command minimal authentication */
#define SE_AUTH_CMD(X) (SE_SEC_LEVEL > se_ctx.cmd_params[X][SE_AUTH_PARAM] ? SE_SEC_LEVEL : \
                        se_ctx.cmd_params[X][SE_AUTH_PARAM])
/* Get command encryption */
#define SE_ENC_CMD(X)  se_ctx.cmd_params[X][SE_ENC_PARAM]
/* Accessor for configured security level */
#define SE_SEC_LEVEL  se_ctx.security_level

#define SE_SEC_LEVEL_NONE STSAFEA_MAC_NONE
#define SE_ENVELOPE_SLOT 0
#define SE_ENVELOPE_SIZE STSAFEA_KEY_TYPE_AES_256

/* Get region minimal read authentication */
#define SE_MAPPING_READ_LEVEL(_x, _auth) if (se_ctx.mapping != NULL) \
  { \
    if (_x < se_ctx.mapping_size) \
      _auth = (se_ctx.mapping[_x].read_ac == 0 ? SE_SEC_LEVEL : STSAFEA_MAC_HOST_CMAC | SE_SEC_LEVEL) ;\
    else\
      _auth = SE_SEC_LEVEL; \
  } \
  else \
  { \
    _auth = SE_SEC_LEVEL;\
  }

/* Get region minimal update authentication */
#define SE_MAPPING_UPDATE_LEVEL(_x, _auth) if (se_ctx.mapping != NULL) \
  { \
    if (_x < se_ctx.mapping_size) \
      _auth = (se_ctx.mapping[_x].write_ac == 0 ? SE_SEC_LEVEL : STSAFEA_MAC_HOST_CMAC | SE_SEC_LEVEL) ;\
    else\
      _auth = SE_SEC_LEVEL; \
  } \
  else \
  { \
    _auth = SE_SEC_LEVEL;\
  }


#ifdef SE_FREERTOS
#define SE_API_LOCK   while (osMutexAcquire(se_ctx.se_api_mutex, osWaitForever) != osOK)\
  { \
    osDelay(10); \
  }

#define SE_API_UNLOCK osMutexRelease(se_ctx.se_api_mutex)
#endif /* SE_FREERTOS */

#ifndef SE_API_LOCK
#define SE_API_LOCK
#endif /* ! SE_API_LOCK */

#ifndef SE_API_UNLOCK
#define SE_API_UNLOCK
#endif /* ! SE_API_UNLOCK */

/* SE API will manage active device key and active device certificate */
/* The current active key is by default 0 if new key is generated use SE_SetActiveKey */
/* the current active certificate is by default 0 if new certificate is generated use SE_SetActiveCertificate */

#ifdef STSAFEA_AUTO_MANAGED_REGION
#define SE_AUTO_MANAGEMENT_REGION_DEFAULT     STSAFEA_AUTO_MANAGED_REGION
#else
#define SE_AUTO_MANAGEMENT_REGION_DEFAULT     1
#endif /* STSAFEA_AUTO_MANAGED_REGION */
#ifdef STSAFEA_AUTO_MANAGED_CERTIFICATE_OFFSET
#define SE_AUTO_MANAGED_ACTIVE_CERTIFICATE_DEFAULT  STSAFEA_AUTO_MANAGED_CERTIFICATE_OFFSET
#else
#define SE_AUTO_MANAGED_ACTIVE_CERTIFICATE_DEFAULT  699
#endif /* STSAFEA_AUTO_MANAGED_CERTIFICATE_OFFSET */
#ifdef STSAFEA_AUTO_MANAGED_KEY_OFFSET
#define SE_AUTO_MANAGED_ACTIVE_KEY_DEFAULT    STSAFEA_AUTO_MANAGED_KEY_OFFSET
#else
#define SE_AUTO_MANAGED_ACTIVE_KEY_DEFAULT    698
#endif /* STSAFEA_AUTO_MANAGED_KEY_OFFSET */


static SE_API_Status_t se_GetAutoConfig(void);
static SE_API_Status_t se_GetSerial(void);
static SE_API_Status_t se_GetCommandConfig(void);
static SE_API_Status_t se_GetMapping(SE_Region_t *regions, uint32_t *nb_region);
SE_Ctx_t se_ctx = {0};

#define SE_TAG "[SE_STSAFEA] : "
void se_printf(uint8_t status, const char *fmt, ...)
{
#ifdef SE_DEBUG
  va_list args;
  va_start(args, fmt);
  char fmt_buf_data[100];
  const char *fmt_buf = fmt;

  if (se_ctx.debug_print == NULL)
  {
    return;
  }
  if (status == 0)
  {
    fmt_buf = fmt_buf_data;
    snprintf(fmt_buf_data, 100, SE_TAG "%s", fmt);
  }
  vsnprintf(se_ctx.trace_buff, SE_TRACE_BUFF_SIZE, fmt_buf, args);

  se_ctx.debug_print(se_ctx.trace_buff);
#endif /* SE_DEBUG */
}

SE_API_Status_t SE_SetDebug(int32_t (*dbg_print)(const char *fmt, ...))
{
#ifdef SE_DEBUG
  se_ctx.debug_print = dbg_print;
#endif /* SE_DEBUG */
  return SE_OK;
}


SE_API_Status_t SE_SetSecurity(SE_Security_Level_t level)
{
  if (level > SE_SECURITY_LEVEL_R_PAIRING)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  if (level == SE_SECURITY_LEVEL_NONE)
  {
    se_ctx.security_level = STSAFEA_MAC_NONE;
  }
  if (level == SE_SECURITY_LEVEL_C_PAIRING)
  {
    se_ctx.security_level = STSAFEA_MAC_HOST_CMAC;
  }
  if (level == SE_SECURITY_LEVEL_R_PAIRING)
  {
    se_ctx.security_level = STSAFEA_MAC_HOST_RMAC;
  }
  return SE_OK;
}

SE_API_Status_t SE_GenerateWrapKey(int8_t slot, SE_Wrap_Type_t type)
{
  StSafeA_LocalEnvelopeKeyTableBuffer_t outEnvelopTable;
  StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t outSlot0;
  StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t outSlot1;
  StSafeA_ResponseCode_t ret;
  uint8_t env_type;

  switch (type)
  {
    case SE_WRAP_KEY_256:
      env_type = STSAFEA_KEY_TYPE_AES_256;
      break;
    case SE_WRAP_KEY_128:
      env_type = STSAFEA_KEY_TYPE_AES_128;
      break;
    default :
      return SE_INVALID_PARAMS_ERROR;
  }

  ret = StSafeA_LocalEnvelopeKeySlotQuery(SE_HDL, &outEnvelopTable, &outSlot0, &outSlot1, SE_SEC_LEVEL_NONE);
  if (ret == STSAFEA_OK)
  {
    if (slot == 0)
      if ((outEnvelopTable.NumberOfSlots != 0) && (outSlot0.PresenceFlag == 0))
      {
        ret = StSafeA_GenerateLocalEnvelopeKey(SE_HDL, 0, env_type, NULL, 0, STSAFEA_MAC_NONE);
        if (ret != STSAFEA_OK)
        {
          SE_PRINTF("Unalbe to generate envelop key %x\n", ret);
        }
      }
    if (slot == 1)
    {
      if ((outEnvelopTable.NumberOfSlots != 0) && (outSlot1.PresenceFlag == 0))
      {
        ret = StSafeA_GenerateLocalEnvelopeKey(SE_HDL, 1, env_type, NULL, 0, STSAFEA_MAC_NONE);
        if (ret != STSAFEA_OK)
        {
          SE_PRINTF("Unalbe to generate envelop key %x\n", ret);
        }
      }
    }
  }
  return SE_OK;
}

SE_API_Status_t SE_SetPairingKeys(uint8_t *keys)
{
  StSafeA_HostKeySlotBuffer_t out;
  StSafeA_ResponseCode_t status_code;
  /* check pairing keys */


  status_code = StSafeA_HostKeySlotQuery(SE_HDL, &out, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    if (out.HostKeyPresenceFlag == 0)
    {
      SE_PRINTF("Setting Pairing key into STSAFE-A110 :");
      status_code = StSafeA_PutAttribute(SE_HDL, STSAFEA_TAG_HOST_KEY_SLOT, keys, 32, SE_SEC_LEVEL_NONE);
      if (status_code == STSAFEA_OK)
      {
        SE_PRINTF("OK"ENDL);
      }
      else
      {
        SE_PRINTF("KO *** 0x%x"ENDL, status_code);
      }
    }
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GenerateEnvelop(uint32_t slot, uint32_t ksize)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;
  StSafeA_LocalEnvelopeKeyTableBuffer_t out_env;
  StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t out_rec0;
  StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t out_rec1;
  uint8_t keytype = STSAFEA_KEY_TYPE_AES_128;
  if (ksize == 32)
  {
    keytype = STSAFEA_KEY_TYPE_AES_256;
  }
  else
  {
    if (ksize != 16)
    {
      return SE_INVALID_PARAMS_ERROR;
    }
  }

  status_code = StSafeA_LocalEnvelopeKeySlotQuery(SE_HDL, &out_env, &out_rec0, &out_rec1, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    if (slot == 0)
      if (out_rec0.PresenceFlag == 0)
      {
        status_code = StSafeA_GenerateLocalEnvelopeKey(SE_HDL, 0, keytype, NULL, 0, SE_SEC_LEVEL_NONE);
      }
    if (slot == 1)
      if (out_rec1.PresenceFlag == 0)
      {
        status_code = StSafeA_GenerateLocalEnvelopeKey(SE_HDL, 1, keytype, NULL, 0, SE_SEC_LEVEL_NONE);
      }
  }
  if (status_code == STSAFEA_OK)
  {
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GetPairingStatus(uint8_t *presence, uint32_t *counter)
{
  StSafeA_HostKeySlotBuffer_t out;
  StSafeA_ResponseCode_t status_code;
  /* check pairing keys */
  status_code = StSafeA_HostKeySlotQuery(SE_HDL, &out, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    *presence  = out.HostKeyPresenceFlag;
    if (*presence == 1)
    {
      *counter = out.HostCMacSequenceCounter;
    }
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

void StSafeA_Set_I2C_Address(uint8_t i2c_addr);


#define SE_AUTO se_ctx.auto_managed
static SE_API_Status_t se_GetAutoConfig(void)
{
  SE_API_Status_t ret = SE_COMMAND_ERROR;
  /* get active certificate and key */
  ret = SE_Read(SE_AUTO.region, SE_AUTO.key_offset, 1, &se_ctx.active_key);
  if (ret != SE_OK)
  {
    return ret;
  }
  ret = SE_Read(SE_AUTO.region, SE_AUTO.cert_offset, 1, &se_ctx.active_certificate);
  return ret;
}


SE_API_Status_t SE_SetAutoManagement(uint8_t region, uint16_t key_offset, uint16_t cert_offset)
{
  SE_AUTO.region = region;
  SE_AUTO.cert_offset = cert_offset;
  SE_AUTO.key_offset = key_offset;
  se_GetAutoConfig();
  return SE_OK;
}

SE_API_Status_t SE_Echo(uint8_t *input, size_t size, uint8_t *output)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_LVBuffer_t OutEcho;

  OutEcho.Data = output;
  OutEcho.Length = size;

  status_code = StSafeA_Echo(SE_HDL, input, size, &OutEcho, SE_SEC_LEVEL);
  if (status_code == STSAFEA_OK)
  {
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

static SE_API_Status_t se_GetKeySize(void)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;
  StSafeA_TLVBuffer_t inCmd;
  uint8_t data[100];
  StSafeA_TLVBuffer_t outTLV;
  outTLV.LV.Data = data;
  outTLV.LV.Length = sizeof(data);

  inCmd.Header = 0x14;
  inCmd.LV.Data = data;
  data[0] = 0x14;
  inCmd.LV.Length = 1;

  /* get slot 0 key size */
  status_code = StSafeA_RawCommand(SE_HDL, &inCmd, sizeof(data), &outTLV, 0, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    if (outTLV.Header == 0x00)
    {
      uint8_t offset = 4;
      if (outTLV.LV.Data[offset + 1] & 0x1)
      {
        switch (outTLV.LV.Data[offset + 5])
        {
          case 8 :
            se_ctx.slot0_key_size = 256;
            se_ctx.fixed_pub_keys[0].public_key_curve = SE_KEY_TYPE_NIST_256;
            break;
          case 5 :
            se_ctx.slot0_key_size = 384;
            se_ctx.fixed_pub_keys[0].public_key_curve = SE_KEY_TYPE_NIST_384;
            break;
          case 9 :
            if (outTLV.LV.Data[offset + 14] == 0x07)
            {
              se_ctx.slot0_key_size = 256;
              se_ctx.fixed_pub_keys[0].public_key_curve = SE_KEY_TYPE_BP_256;
            }
            else
            {
              se_ctx.slot0_key_size = 384;
              se_ctx.fixed_pub_keys[0].public_key_curve = SE_KEY_TYPE_BP_384;
            }
            break;
          default :
            se_ctx.slot0_key_size = 0;
            break;
        }
        offset += outTLV.LV.Data[offset + 5] + 6;
      }
      else
      {
        offset += 4;
      }
      if (outTLV.LV.Data[offset + 1] & 0x1)
      {
        switch (outTLV.LV.Data[offset + 5])
        {
          case 8 :
            se_ctx.slot1_key_size = 256;
            break;
          case 5 :
            se_ctx.slot1_key_size = 384;
            break;
          case 9 :
            se_ctx.slot1_key_size = (outTLV.LV.Data[offset + 14] == 0x07 ? 256 : 384);
            break;
          default :
            se_ctx.slot1_key_size = 0;
            break;
        }
      }
    }
    return SE_OK;
  }
  if (status_code == STSAFEA_INVALID_LIFE_CYCLE_STATE)
  {
    return SE_INVALID_STATE_ERROR;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_Init(uint32_t flag, SE_Security_Level_t sec_level, uint32_t i2c_addr)
{
  StSafeA_ResponseCode_t status_code;
  SE_API_Status_t ret = SE_COMMAND_ERROR;
  uint8_t locked = 0;

  uint8_t header[4];
  uint16_t offset = 26;
  uint8_t pub_key[96];
  if (se_ctx.init == 1)
  {
    return SE_OK;
  }

  SE_AUTO.region = SE_AUTO_MANAGEMENT_REGION_DEFAULT;
  SE_AUTO.cert_offset = SE_AUTO_MANAGED_ACTIVE_CERTIFICATE_DEFAULT;
  SE_AUTO.key_offset = SE_AUTO_MANAGED_ACTIVE_KEY_DEFAULT;

  se_ctx.cmd_params[SE_CMD_WRAP][SE_AUTH_PARAM] = WRAP_CMD_MAC;
  se_ctx.cmd_params[SE_CMD_WRAP][SE_ENC_PARAM] = WRAP_CMD_ENC;

  se_ctx.cmd_params[SE_CMD_UNWRAP][SE_AUTH_PARAM] = UNWRAP_CMD_MAC;
  se_ctx.cmd_params[SE_CMD_UNWRAP][SE_ENC_PARAM] = UNWRAP_CMD_ENC;

  se_ctx.cmd_params[SE_CMD_SHARED_SECRET][SE_AUTH_PARAM] = SHARED_SECRET_CMD_MAC;
  se_ctx.cmd_params[SE_CMD_SHARED_SECRET][SE_ENC_PARAM] = SHARED_SECRET_CMD_ENC;

  se_ctx.cmd_params[SE_CMD_GEN_KEY][SE_AUTH_PARAM] = STSAFEA_MAC_NONE;
  se_ctx.cmd_params[SE_CMD_GEN_KEY][SE_ENC_PARAM] = STSAFEA_ENCRYPTION_NONE;

  se_ctx.cmd_params[SE_CMD_GEN_KEY_EPHEMERAL][SE_AUTH_PARAM] = STSAFEA_MAC_NONE;
  se_ctx.cmd_params[SE_CMD_GEN_KEY_EPHEMERAL][SE_ENC_PARAM] = STSAFEA_ENCRYPTION_NONE;

  se_ctx.cmd_params[SE_CMD_SIG][SE_AUTH_PARAM] = STSAFEA_MAC_NONE;
  se_ctx.cmd_params[SE_CMD_SIG][SE_ENC_PARAM] = STSAFEA_ENCRYPTION_NONE;

  se_ctx.serial_loaded = 0;

  /* Create STSAFE-A's handle */
  SE_PRINTF("Starting STSAFE-A110 : ");
  status_code = StSafeA_Init(SE_HDL, se_ctx.a_rx_tx_se_data);

  if (status_code != STSAFEA_OK)
  {
    SE_PRINTF_RES("KO *** Unable to start STSAFE-A110 0x%x"ENDL, status_code);
    goto skip_stsafe;
  }
  if (i2c_addr != 0)
  {
    StSafeA_Set_I2C_Address(i2c_addr);
  }
  SE_PRINTF_RES("OK" ENDL);
  /* get Serial */
  SE_PRINTF("Get Serial :");
  ret = se_GetSerial();
  if (ret != SE_OK)
  {
    SE_PRINTF_RES("KO *** Unable to get serial 0x%x"ENDL, ret);
    goto skip_stsafe;
  }
  SE_PRINTF_RES("OK"ENDL);
  SE_PRINTF("Set Crypto Keys : ");
  /* call extra Key API */
  status_code = StSafeA_Crypto_GetKeys(se_ctx.serial, sizeof(se_ctx.serial));
  if (status_code != STSAFEA_OK)
  {
    SE_PRINTF_RES("KO *** Set pairing key 0x%x"ENDL, status_code);
    ret = SE_COMMAND_ERROR;
    goto skip_stsafe;
  }
  SE_PRINTF_RES("OK"ENDL);
  /* check if chipset is locked */
  ret = se_GetKeySize();

  if (ret != SE_OK)
  {
    if (ret == SE_INVALID_STATE_ERROR)
    {
      ret = SE_OK;
      locked = 1;
    }
    else
    {
      ret = SE_COMMAND_ERROR;
      goto skip_stsafe;
    }
  }
  else
    ret = SE_OK;

#ifdef SE_FREERTOS
  /* osMutexDef(se_api_m); */
  se_ctx.se_api_mutex = osMutexNew(NULL);
#endif /* SE_FREERTOS */
  se_ctx.max_buffer_size = (STSAFEA_BUFFER_DATA_CONTENT_SIZE == 500 ? 496 : STSAFEA_BUFFER_DATA_CONTENT_SIZE);
  se_ctx.init = 1;
  if (locked == 0)
  {
    se_GetCommandConfig();
    if (flag & SE_INIT_TEST_PAIRING)
    {
      uint8_t data[8];
      uint8_t clear_data[8];
      uint8_t enc_data[16];

      SE_PRINTF("Stsafe Testing pairing keys : ");
      ret = SE_Wrap(0, data, sizeof(data), enc_data);
      if (ret != SE_OK)
      {
        SE_PRINTF_RES("KO *** Unable to wrap envelop with STSAFE-A110 0x%x"ENDL, status_code);
        goto skip_pairing;
      }
      ret = SE_Unwrap(0, enc_data, sizeof(enc_data), clear_data);
      if (ret != SE_OK)
      {
        SE_PRINTF_RES("KO *** Unable to unwrap envelop with STSAFE-A110 0x%x"ENDL, status_code);
        goto skip_pairing;
      }
      if (memcmp(data, clear_data, sizeof(data)) == 0)
      {
        SE_PRINTF_RES("OK"ENDL);
      }
      else
      {
        SE_PRINTF_RES("KO data does not match"ENDL);
      }
    }
skip_pairing:

    ret = SE_SetSecurity(sec_level);
    if (ret != SE_OK)
    {
      goto skip_stsafe;
    }
    se_GetAutoConfig();
    ret = se_GetMapping(se_ctx.mapping, &se_ctx.mapping_size);
    if (ret == SE_OK)
    {
      se_ctx.mapping = malloc(se_ctx.mapping_size * sizeof(SE_Region_t));
      if (se_ctx.mapping != NULL)
      {
        ret = se_GetMapping(se_ctx.mapping, &se_ctx.mapping_size);
      }
    }
  }

  ret = SE_Read(0, offset, 2, header);

  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  /* skip type */
  offset += 2 + header[1];
  ret = SE_Read(0, offset, 2, header);
  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  /* skip issuer */
  offset += 2 + header[1];
  ret = SE_Read(0, offset, 2, header);
  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  /* skip duration */
  offset += 2 + header[1];
  ret = SE_Read(0, offset, 2, header);
  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  /* skip subject */
  offset += 4 + header[1];
  ret = SE_Read(0, offset, 2, header);
  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  /* skip details */
  offset += 2 + header[1];
  ret = SE_Read(0, offset, 2, header);
  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  offset += 4;
  ret = SE_Read(0, offset, header[1] - 2, pub_key);
  if (ret != SE_OK)
  {
    goto skip_stsafe;
  }
  memcpy(se_ctx.fixed_pub_keys[0].public_key_x, pub_key, se_ctx.slot0_key_size / 8);
  memcpy(se_ctx.fixed_pub_keys[0].public_key_y, &pub_key[se_ctx.slot0_key_size / 8], se_ctx.slot0_key_size / 8);

skip_stsafe:
  return ret;
}


SE_API_Status_t SE_Refresh(void)
{
  SE_API_Status_t ret = SE_COMMAND_ERROR;
  SE_PRINTF("%s", __func__);
  se_GetAutoConfig();
  se_GetCommandConfig();
  se_GetSerial();
  ret = SE_GetMapping(se_ctx.mapping, &se_ctx.mapping_size);
  if (ret == SE_OK)
  {
    se_ctx.mapping = malloc(se_ctx.mapping_size * sizeof(SE_Region_t));
    if (se_ctx.mapping != NULL)
    {
      ret = SE_GetMapping(se_ctx.mapping, &se_ctx.mapping_size);
    }
  }
  se_GetKeySize();
  return ret;
}

static SE_API_Status_t se_GetSerial(void)
{
  uint8_t old = se_ctx.init;
  SE_API_Status_t ret = SE_COMMAND_ERROR;
  se_ctx.init = 1;
  ret = SE_GetSerial(NULL, SE_SERIAL_FORMAT_RAW);
  se_ctx.init = old;
  return ret;
}
/* Provide serial number in RAW or HEX mode */
/* For RAW mode serial shall be 9 bytes     */
/* For HEX mode serial shall be 18 bytes    */
SE_API_Status_t SE_GetSerial(uint8_t *serial, SE_SerialFormat_t format)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_ProductDataBuffer_t productData;
  uint32_t i;
  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }

  SE_API_LOCK;
  if (se_ctx.serial_loaded == 0)
  {
    status_code = StSafeA_ProductDataQuery(SE_HDL, &productData, SE_SEC_LEVEL_NONE);
    if (status_code != STSAFEA_OK)
    {
      SE_API_UNLOCK;
      return SE_COMMAND_ERROR;
    }
    memcpy(se_ctx.serial, productData.STNumber, STSAFEA_ST_NUMBER_LENGTH);
    se_ctx.serial_loaded = 1;
  }
  if (serial != NULL)
  {
    if (format == SE_SERIAL_FORMAT_RAW)
    {
      /* RAW format */
      memcpy(serial, se_ctx.serial, STSAFEA_ST_NUMBER_LENGTH);
    }
    else
    {
      for (i = 0; i < STSAFEA_ST_NUMBER_LENGTH; i++)
      {
        snprintf((char *)&serial[2 * i], 3, "%02X", se_ctx.serial[i]);
      }
    }
  }
  SE_API_UNLOCK;
  return SE_OK;
}

SE_API_Status_t SE_ReadRight(uint8_t region, uint16_t offset, uint32_t update, uint16_t size, uint8_t *buf)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;
  StSafeA_LVBuffer_t sts_read;
  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }

  SE_API_LOCK;
  sts_read.Length = size;
  sts_read.Data = buf;
  uint16_t local_offset = 0;
  uint16_t chunk_size = se_ctx.max_buffer_size;
  uint32_t data_read = 0;
  uint8_t auth;
  uint8_t first = 1;

  uint8_t update_flag = STSAFEA_FLAG_TRUE;
  uint8_t access_right = STSAFEA_AC_ALWAYS;

  if (update & SE_ACCESS_RIGHT_LOCK)
  {
    update_flag = STSAFEA_FLAG_FALSE;
  }
  if ((update & 0x7) == SE_ACCESS_RIGHT_HOST)
  {
    access_right = STSAFEA_AC_HOST;
  }
  if ((update & 0x7) == SE_ACCESS_RIGHT_NEVER)
  {
    access_right = 0x7;
  }


  SE_MAPPING_READ_LEVEL(region, auth);

  SE_PRINTF("%s : region  %d, auth %x\n", __func__, region, auth);

  while ((size >= se_ctx.max_buffer_size) && (status_code == STSAFEA_OK))
  {
    if (first == 1)
    {
      status_code = StSafeA_Read(SE_HDL, STSAFEA_FLAG_TRUE, update_flag, access_right, region,
                                 (uint16_t)(local_offset + offset), chunk_size, chunk_size, &sts_read, auth);
    }
    else
      status_code = StSafeA_Read(SE_HDL, STSAFEA_FLAG_FALSE, STSAFEA_FLAG_FALSE, STSAFEA_AC_ALWAYS, region,
                                 (uint16_t)(local_offset + offset), chunk_size, chunk_size, &sts_read, auth);
    if (status_code == STSAFEA_OK)
    {
      first = 0;
      se_ctx.mapping[region].read_ac = access_right;
      SE_MAPPING_READ_LEVEL(region, auth);
      size -= chunk_size;
      data_read += chunk_size;
      sts_read.Data += chunk_size;
      sts_read.Length = size;
      local_offset += chunk_size;
    }
  }
  if ((status_code == STSAFEA_OK) && (size != 0))
  {
    if (first == 1)
    {
      status_code = StSafeA_Read(SE_HDL, STSAFEA_FLAG_TRUE, update_flag, access_right, region,
                                 local_offset + offset, size, size, &sts_read, auth);
    }
    else
      status_code = StSafeA_Read(SE_HDL, STSAFEA_FLAG_FALSE, STSAFEA_FLAG_FALSE, STSAFEA_AC_ALWAYS, region,
                                 local_offset + offset, size, size, &sts_read, auth);
  }
  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
    se_ctx.mapping[region].read_ac = access_right;
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

/* Check read access */
SE_API_Status_t SE_MemoryCheck(uint8_t region, uint16_t offset, uint16_t *size)
{
  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }
  if (region >= se_ctx.mapping_size)
  {
    return SE_INVALID_PARAMS_ERROR;
  }
  if (*size + offset > se_ctx.mapping[region].size)
  {
    *size = se_ctx.mapping[region].size - offset;
    if (se_ctx.mapping[region].type == 1)
    {
      *size += 4;
    }
    return SE_PARAMS_UPDATED_ERROR;
  }
  return SE_OK;
}

SE_API_Status_t SE_Read(uint8_t region, uint16_t offset, uint16_t size, uint8_t *buf)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;
  StSafeA_LVBuffer_t sts_read;
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  sts_read.Length = size;
  sts_read.Data = buf;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }

  SE_API_LOCK;
  uint16_t local_offset = 0;
  uint16_t chunk_size = se_ctx.max_buffer_size;
  uint32_t data_read = 0;
  uint8_t auth;

  SE_MAPPING_READ_LEVEL(region, auth);

  SE_PRINTF("%s : region  %d, auth %x\n", __func__, region, auth);

  while ((size >= se_ctx.max_buffer_size) && (status_code == STSAFEA_OK))
  {
    status_code = StSafeA_Read(SE_HDL, 0, 0, STSAFEA_AC_ALWAYS, region, (uint16_t)(local_offset + offset),
                               chunk_size, chunk_size, &sts_read, auth);
    if (status_code == STSAFEA_OK)
    {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
      memcpy(buf + local_offset, sts_read.Data, sts_read.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
      size -= chunk_size;
      data_read += chunk_size;
      sts_read.Data += chunk_size;
      sts_read.Length = size;
      local_offset += chunk_size;
    }
    else
    {
      SE_PRINTF("%s : region  %d, offset %d, size %d error = %d\n", __func__, region, local_offset,
                chunk_size, status_code);
    }
  }
  if ((status_code == STSAFEA_OK) && (size != 0))
  {
    status_code = StSafeA_Read(SE_HDL, 0, 0, 0, region, local_offset + offset, size, size, &sts_read, auth);
  }

  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(buf + local_offset, sts_read.Data, sts_read.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

    return SE_OK;
  }
  else
  {
    SE_PRINTF("%s : region  %d, offset %d, size %d error = %d\n", __func__, region, local_offset + offset, size,
              status_code);
    SE_PRINTF("%s : region  %d, input buffer = %p \n\r", __func__, region, buf);
  }
  return SE_COMMAND_ERROR;
}


SE_API_Status_t SE_UpdateRight(uint8_t region, uint16_t offset, uint32_t update, uint16_t size, uint8_t *data)
{
  StSafeA_LVBuffer_t in_data;
  uint16_t local_offset = 0;
  uint16_t chunk_size = se_ctx.max_buffer_size;
  uint32_t data_read = 0;
  int32_t status_code = STSAFEA_OK;
  uint8_t auth;

  uint8_t last = 0;

  uint8_t update_flag = STSAFEA_FLAG_TRUE;
  uint8_t access_right = STSAFEA_AC_ALWAYS;

  if (update & SE_ACCESS_RIGHT_LOCK)
  {
    update_flag = STSAFEA_FLAG_FALSE;
  }
  if ((update & 0x7) == SE_ACCESS_RIGHT_HOST)
  {
    access_right = STSAFEA_AC_HOST;
  }
  if ((update & 0x7) == SE_ACCESS_RIGHT_NEVER)
  {
    access_right = 0x7;
  }
  SE_MAPPING_UPDATE_LEVEL(region, auth);

  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }

  SE_API_LOCK;
  in_data.Data = data;
  in_data.Length = size;

  status_code = STSAFEA_OK;
  while ((size >= chunk_size) && (status_code == STSAFEA_OK))
  {
    in_data.Length = chunk_size;
    if (last == 1)
    {
      status_code = StSafeA_Update(SE_HDL, 0, STSAFEA_FLAG_TRUE, update_flag, access_right, region,
                                   local_offset + offset, &in_data, auth);
    }
    else
      status_code = StSafeA_Update(SE_HDL, 0, STSAFEA_FLAG_FALSE, STSAFEA_FLAG_FALSE, STSAFEA_AC_ALWAYS, region,
                                   local_offset + offset, &in_data, auth);

    if (status_code == STSAFEA_OK)
    {
      se_ctx.mapping[region].write_ac = access_right;
      SE_MAPPING_UPDATE_LEVEL(region, auth);
      size -= chunk_size;
      data_read += chunk_size;
      local_offset += chunk_size;
      in_data.Data += chunk_size;
      if (size <= chunk_size)
      {
        last = 1;
      }
    }
  }
  if ((status_code == STSAFEA_OK) && (size != 0))
  {
    in_data.Length = size;
    status_code = StSafeA_Update(SE_HDL, 0, STSAFEA_FLAG_TRUE, update_flag, access_right, region,
                                 local_offset + offset, &in_data, auth);
  }
  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_Update(uint8_t region, uint16_t offset, uint16_t size, uint8_t *data)
{
  StSafeA_LVBuffer_t in_data;
  uint16_t local_offset = 0;
  uint16_t chunk_size = se_ctx.max_buffer_size;
  uint32_t data_read = 0;
  int32_t status_code = STSAFEA_OK;
  uint8_t auth;
  SE_MAPPING_UPDATE_LEVEL(region, auth);

  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }

  SE_API_LOCK;
  in_data.Data = data;
  in_data.Length = size;

  status_code = STSAFEA_OK;
  while ((size >= chunk_size) && (status_code == STSAFEA_OK))
  {
    in_data.Length = chunk_size;
    status_code = StSafeA_Update(SE_HDL, 0, 0, 0, 0, region, local_offset + offset, &in_data, auth);
    if (status_code == STSAFEA_OK)
    {
      size -= chunk_size;
      data_read += chunk_size;
      local_offset += chunk_size;
      in_data.Data += chunk_size;
    }
  }
  if ((status_code == STSAFEA_OK) && (size != 0))
  {
    in_data.Length = size;
    status_code = StSafeA_Update(SE_HDL, 0, 0, 0, 0, region, local_offset + offset, &in_data, auth);
  }
  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GetCertificate(uint8_t region, uint8_t *buf, size_t *size)
{
  int32_t status_code;
  size_t CertificateSize;
  if (se_ctx.init == 0)
  {
    return SE_INVALID_STATE_ERROR;
  }

  status_code = SE_Read(region, 0, 4, buf);
  if (0 == status_code)
  {
    CertificateSize = 0;

    switch (buf[1])
    {
      case 0x81U:
        CertificateSize = (uint16_t)buf[2] + 3U;
        break;

      case 0x82U:
        CertificateSize = ((uint16_t)buf[2] << 8) + buf[3] + 4U;
        break;

      default:
        if (buf[1] < 0x81U)
        {
          CertificateSize = buf[1];
        }
        break;
    }

    if (CertificateSize > 0U)
    {
      if (CertificateSize < *size)
      {
        status_code = SE_Read(region, 0, CertificateSize, buf);
        if (status_code == STSAFEA_OK)
        {

          *size = CertificateSize;
          buf[*size] = '\0';
          return SE_OK;
        }
      }
    }
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GetActiveCertificate(uint8_t *buf, size_t *size)
{
  SE_API_Status_t ret = SE_INVALID_PARAMS_ERROR;
  /* try read certificate in alternate region */
  ret = SE_GetCertificate(se_ctx.active_certificate, buf, size);
  if (ret != SE_OK)
  {
    /* in case of no certificate in active region, fallback on region 0*/
    /* no certificate in alternate region try region 0 */
    ret = SE_GetCertificate(0, buf, size);
  }
  return ret;
}

SE_API_Status_t SE_StoreCertificate(uint8_t id, uint8_t *buf, uint16_t size)
{
  /* write certificate in alternate region */

  SE_API_Status_t ret = SE_INVALID_PARAMS_ERROR;
  ret = SE_Update(id, 0, size, buf);

  SE_PRINTF("error while writing in region %d, %d\n", id, ret);
  return ret;
}

SE_API_Status_t SE_SetActiveCertificate(uint8_t id)
{

  SE_API_Status_t ret = SE_INVALID_PARAMS_ERROR;

  ret = SE_Update(SE_AUTO.region, SE_AUTO.cert_offset, 1, &id);
  if (ret == SE_OK)
  {
    se_ctx.active_certificate = id;
  }
  return ret;
}

SE_API_Status_t SE_SetActiveKey(uint8_t id)
{
  SE_API_Status_t ret = SE_INVALID_PARAMS_ERROR;
  if (id > 1)
  {
    /* invalid key index, should be 0 or 1 */
    return ret;
  }
  ret = SE_Update(SE_AUTO.region, SE_AUTO.key_offset, 1, &id);

  if (ret == SE_OK)
  {
    se_ctx.active_key = id;
  }
  return ret;
}

SE_API_Status_t SE_GetActiveKey(uint8_t *id)
{
  *id = se_ctx.active_key;
  return SE_OK;
}

/* GenerateKeyPair is either on slot 1 or ECDH */
SE_API_Status_t SE_GenerateKeyPair(SE_Slot_t slot, SE_Key_Type_t type, uint8_t *pub_x, uint8_t *pub_y)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_COMMUNICATION_ERROR;
  StSafeA_CurveId_t curve;
  uint16_t size;
  StSafeA_LVBuffer_t x;
  StSafeA_LVBuffer_t y;
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  x.Data = pub_x;
  y.Data = pub_y;
  x.Length = size;
  y.Length = size;

#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  uint8_t reppoint;

  switch (type)
  {
    case SE_KEY_TYPE_NIST_256:
      curve = STSAFEA_NIST_P_256;
      size = 32;
      break;
    case SE_KEY_TYPE_NIST_384:
      curve = STSAFEA_NIST_P_384;
      size = 48;
      break;
    case SE_KEY_TYPE_BP_256:
      curve = STSAFEA_BRAINPOOL_P_256;
      size = 32;
      break;
    case SE_KEY_TYPE_BP_384:
      curve = STSAFEA_BRAINPOOL_P_384;
      size = 48;
      break;
    default:
      return SE_INVALID_PARAMS_ERROR;
  }

  SE_API_LOCK;
  if (slot == SE_SLOT_PREPROVISIONNED)
  {
    /* error do not generate key on pre provisionned key slot */
    return SE_INVALID_PARAMS_ERROR;
  }
  if (slot == SE_SLOT_PERMANENT)
  {
    status_code = StSafeA_GenerateKeyPair(SE_HDL, STSAFEA_KEY_SLOT_1, 0xFFFF, STSAFEA_FLAG_FALSE,
                                          STSAFEA_PRVKEY_MODOPER_AUTHFLAG_MSG_DGST_SIGNEN, curve, size, &reppoint,
                                          &x, &y, SE_AUTH_CMD(SE_CMD_GEN_KEY));
    if (status_code == STSAFEA_OK)
    {
      se_ctx.slot1_key_size = size * 8;
    }
  }
  if (slot == SE_SLOT_EPHEMERAL)
  {
    status_code = StSafeA_GenerateKeyPair(SE_HDL, STSAFEA_KEY_SLOT_EPHEMERAL, 0xFFFF, STSAFEA_FLAG_FALSE,
                                          STSAFEA_PRVKEY_MODOPER_AUTHFLAG_KEY_ESTABLISHEN, curve, size, &reppoint,
                                          &x, &y, SE_AUTH_CMD(SE_CMD_GEN_KEY_EPHEMERAL));
    if (status_code == STSAFEA_OK)
    {
      se_ctx.slotff_key_size = size * 8;
    }
  }
  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(pub_x, x.Data, x.Length);
    memcpy(pub_y, y.Data, y.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_DeriveSharedSecret(SE_Slot_t slot, uint8_t *x, uint8_t *y, uint8_t *z, uint16_t size)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;

  StSafeA_LVBuffer_t pub_x, pub_y;
  StSafeA_SharedSecretBuffer_t shared;
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  shared.SharedKey.Data = z;
  shared.SharedKey.Length = size;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  shared.Length = size;

  pub_x.Data = x;
  pub_x.Length = size;
  pub_y.Data = y;
  pub_y.Length = size;

  SE_PRINTF("%s : slot %d\n", __func__, slot);
  SE_API_LOCK;
  if (slot == SE_SLOT_EPHEMERAL)
  {
    status_code = StSafeA_EstablishKey(SE_HDL, STSAFEA_KEY_SLOT_EPHEMERAL, &pub_x, &pub_y, size, &shared,
                                       SE_AUTH_CMD(SE_CMD_SHARED_SECRET), SE_ENC_CMD(SE_CMD_SHARED_SECRET));
  }
  if (slot == SE_SLOT_PERMANENT)
  {
    status_code = StSafeA_EstablishKey(SE_HDL, STSAFEA_KEY_SLOT_1, &pub_x, &pub_y, size, &shared,
                                       SE_AUTH_CMD(SE_CMD_SHARED_SECRET), SE_ENC_CMD(SE_CMD_SHARED_SECRET));
  }
  if (slot == SE_SLOT_PREPROVISIONNED)
  {
    status_code = StSafeA_EstablishKey(SE_HDL, STSAFEA_KEY_SLOT_0, &pub_x, &pub_y, size, &shared,
                                       SE_AUTH_CMD(SE_CMD_SHARED_SECRET), SE_ENC_CMD(SE_CMD_SHARED_SECRET));
  }
  SE_API_UNLOCK;

  if (status_code != STSAFEA_OK)
  {
    SE_PRINTF("%s : error %x auth = %x, enc = %x\n", __func__, status_code, SE_AUTH_CMD(SE_CMD_SHARED_SECRET),
              SE_ENC_CMD(SE_CMD_SHARED_SECRET));
    return SE_COMMAND_ERROR;
  }
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  memcpy(z, shared.SharedKey.Data, shared.SharedKey.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  return SE_OK;
}


SE_API_Status_t SE_Pairing(uint8_t *cipher, uint8_t *mac)
{
  uint8_t presence = 0;
  uint32_t counter = 0;
  SE_API_Status_t ret = SE_COMMAND_ERROR;

  ret = SE_GetPairingStatus(&presence, &counter);
  if (ret != SE_OK)
  {
    SE_PRINTF("%s : error while getting pairing status %x\n", __func__, ret);
    return ret;
  }
  if (presence == 0)
  {
    uint8_t keys[32];
    memcpy(keys, mac, 16);
    memcpy(&(keys[16]), cipher, 16);
    ret = SE_SetPairingKeys(keys);
    memset(keys, 0, 32);
    if (ret != SE_OK)
    {
      SE_PRINTF("%s: error while setting pairing keys %x\n", __func__, ret);
    }
  }
  return ret;
}

/* password is 16 bytes long */
SE_API_Status_t SE_Unlock(uint8_t *password)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_LifeCycleStateBuffer_t state;
  StSafeA_VerifyPasswordBuffer_t verif;
  SE_API_LOCK;
  status_code = StSafeA_LifeCycleStateQuery(SE_HDL, &state, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    if (state.LifeCycleStatus == STSTAFEA_LIFECYCLE_STATE_OPERATIONAL_AND_LOCKED)
    {
      status_code = StSafeA_VerifyPassword(SE_HDL, password, &verif, SE_SEC_LEVEL_NONE);
      if (status_code == STSAFEA_OK)
      {
        if (verif.VerificationStatus == STSAFEA_FLAG_TRUE)
        {
          /* password OK */
          /* Set Life Cycle to operational */
          uint8_t set_op = 0x3;
          status_code = StSafeA_PutAttribute(SE_HDL, STSAFEA_TAG_LIFE_CYCLE_STATE, &set_op, 1, SE_SEC_LEVEL_NONE);
          if (status_code == STSAFEA_OK)
          {
            status_code = StSafeA_Reset(SE_HDL, SE_SEC_LEVEL_NONE);

            SE_API_UNLOCK;
            return SE_OK;
          }
        }
        else
        {
          SE_API_UNLOCK;
          /* Password is invalid */
          return SE_INVALID_PARAMS_ERROR;
        }
      }
    }
    else
    {
      SE_API_UNLOCK;
      return SE_OK;
    }
  }
  SE_API_UNLOCK;
  return SE_COMMAND_ERROR;
}

/* password is 16 bytes long */
SE_API_Status_t SE_Lock(uint8_t *password)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_LifeCycleStateBuffer_t state;
  StSafeA_VerifyPasswordBuffer_t verif;
  SE_API_LOCK;
  status_code = StSafeA_LifeCycleStateQuery(SE_HDL, &state, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    if (state.LifeCycleStatus == STSTAFEA_LIFECYCLE_STATE_OPERATIONAL)
    {
      status_code = StSafeA_VerifyPassword(SE_HDL, password, &verif, SE_SEC_LEVEL_NONE);
      if (status_code == STSAFEA_OK)
      {
        if (verif.VerificationStatus == STSAFEA_FLAG_TRUE)
        {
          /* password OK */
          /* Set Life Cycle to operational */
          uint8_t set_op = 0x83;
          status_code = StSafeA_PutAttribute(SE_HDL, STSAFEA_TAG_LIFE_CYCLE_STATE, &set_op, 1, SE_SEC_LEVEL_NONE);
          if (status_code == STSAFEA_OK)
          {
            status_code = StSafeA_Reset(SE_HDL, SE_SEC_LEVEL_NONE);

            SE_API_UNLOCK;
            return SE_OK;
          }
        }
        else
        {
          SE_API_UNLOCK;
          /* Password is invalid */
          return SE_INVALID_PARAMS_ERROR;
        }
      }
    }
    else
    {
      SE_API_UNLOCK;
      return SE_OK;
    }
  }
  SE_API_UNLOCK;
  return SE_COMMAND_ERROR;
}


/* reasonable max size to wrap */
#define SE_MAX_WRAP_SIZE 480
/* Use STSAFE-A to unwrap few data */
SE_API_Status_t SE_Unwrap(uint8_t slot, uint8_t *enc_data, uint16_t size, uint8_t *data)
{
  StSafeA_ResponseCode_t ret = STSAFEA_OK;
  StSafeA_LVBuffer_t out;
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  out.Data = data;
  out.Length = size;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  /* check if size is reasonable */
  if (size > (SE_MAX_WRAP_SIZE + 8))
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  /* check is size if multiple of 8 as required */
  if ((size % 8) != 0)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  /* Slot shall be 0 or 1 */
  if (slot > 1)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  SE_API_LOCK;
  ret = StSafeA_UnwrapLocalEnvelope(SE_HDL, slot, enc_data, size, &out,
                                    SE_AUTH_CMD(SE_CMD_UNWRAP), SE_ENC_CMD(SE_CMD_UNWRAP));
  SE_API_UNLOCK;

  if (ret == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(data, out.Data, out.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

/* Use STSAFE-A to wrap few data */
SE_API_Status_t SE_Wrap(uint8_t slot, uint8_t *data, uint16_t size, uint8_t *enc_data)
{
  StSafeA_ResponseCode_t ret = STSAFEA_UNEXPECTED_ERROR;
  StSafeA_LVBuffer_t out;
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  out.Data = enc_data;
  out.Length = size + 8;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

  /* check if size is reasonable */
  if (size > SE_MAX_WRAP_SIZE)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  /* check is size if multiple of 8 as required */
  if ((size % 8) != 0)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  /* Slot shall be 0 or 1 */
  if (slot > 1)
  {
    return SE_INVALID_PARAMS_ERROR;
  }
  SE_API_LOCK;
  ret = StSafeA_WrapLocalEnvelope(SE_HDL, slot, data, size, &out, SE_AUTH_CMD(SE_CMD_WRAP), SE_ENC_CMD(SE_CMD_WRAP));
  SE_API_UNLOCK;
  if (ret == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(enc_data, out.Data, out.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}


SE_API_Status_t SE_VerifySignature(SE_Key_Type_t k_type, uint8_t *hash, size_t hash_size, uint8_t *pub_X,
                                   uint8_t *pub_Y,
                                   uint8_t *R, uint8_t *S)
{
  StSafeA_LVBuffer_t pX;
  StSafeA_LVBuffer_t pY;
  StSafeA_LVBuffer_t pH;
  StSafeA_LVBuffer_t pR;
  StSafeA_LVBuffer_t pS;
  StSafeA_CurveId_t curve;
  StSafeA_VerifySignatureBuffer_t outSig;
  StSafeA_ResponseCode_t status_code;
  uint32_t k_size = 0;

  uint8_t l_hash[48] = {0};
  uint8_t *u_hash = l_hash;
  switch (k_type)
  {
    case SE_KEY_TYPE_NIST_256:
      k_size = 32;
      curve = STSAFEA_NIST_P_256;
      break;
    case SE_KEY_TYPE_NIST_384:
      k_size = 48;
      curve = STSAFEA_NIST_P_384;
      break;
    case SE_KEY_TYPE_BP_256:
      k_size = 32;
      curve = STSAFEA_BRAINPOOL_P_256;
      break;
    case SE_KEY_TYPE_BP_384:
      k_size = 48;
      curve = STSAFEA_BRAINPOOL_P_384;
      break;
    default :
      return SE_INVALID_PARAMS_ERROR;
  }
  if (k_size == 0)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  pX.Length = k_size;
  pX.Data = pub_X;
  pY.Length = k_size;
  pY.Data = pub_Y;
  pR.Length = k_size;
  pR.Data = R;
  pS.Length = k_size;
  pS.Data = S;

  if (k_size != hash_size)
  {
    if (hash_size >= k_size)
    {
      /* for truncated hash no need to recopy, only adjust the hash size as leftmost significant bytes are kept */
      hash_size = k_size;
      u_hash = hash;
    }
    if (hash_size < k_size)
    {
      /* if hash is too small, need to left pad with 0 */
      memcpy(l_hash + (k_size - hash_size), hash, hash_size);
    }
  }
  else
  {
    u_hash = hash;
  }
  pH.Length = k_size;
  pH.Data = u_hash;
  SE_API_LOCK;
  status_code = StSafeA_VerifyMessageSignature(SE_HDL, curve, &pX, &pY, &pR, &pS, &pH, &outSig, SE_SEC_LEVEL);
  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
    if (outSig.SignatureValidity == 1)
    {
      return SE_OK;
    }
    else
      return SE_INVALID_SIG_ERROR;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GenerateSignature(SE_Slot_t slot, const uint8_t *hash, uint16_t hash_size, uint8_t *R,
                                     uint8_t *S)
{
  uint16_t key_size = 0;
  uint8_t l_hash[48] = {0};
  const uint8_t *u_hash = l_hash;
  StSafeA_ResponseCode_t status_code;
  StSafeA_LVBuffer_t pR;
  StSafeA_LVBuffer_t pS;
  uint8_t sslot;

  switch (slot)
  {
    case SE_SLOT_PERMANENT:
      key_size = se_ctx.slot1_key_size / 8;
      sslot = STSAFEA_KEY_SLOT_1;
      break;
    case SE_SLOT_PREPROVISIONNED:
      key_size = se_ctx.slot0_key_size / 8;
      sslot = STSAFEA_KEY_SLOT_0;
      break;
    case SE_SLOT_EPHEMERAL:
    default:
      return SE_INVALID_PARAMS_ERROR;
  }

  if (key_size == 0)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  if (key_size != hash_size)
  {
    if (hash_size >= key_size)
    {
      /* for truncated hash no need to recopy, only adjust the hash size as leftmost significant bytes are kept */
      hash_size = key_size;
      u_hash = hash;
    }
    if (hash_size < key_size)
    {
      /* if hash is too small, need to left pad with 0 */
      memcpy(l_hash + (key_size - hash_size), hash, hash_size);
    }
  }
  else
  {
    u_hash = hash;
  }
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)

  pR.Data = R;
  pS.Data = S;
  pR.Length = key_size;
  pS.Length = key_size;

#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

  SE_API_LOCK;
  status_code = StSafeA_GenerateSignature(SE_HDL, sslot, u_hash,
                                          (key_size == 32 ? STSAFEA_SHA_256 : STSAFEA_SHA_384),
                                          key_size, &pR, &pS, SE_AUTH_CMD(SE_CMD_SIG), SE_ENC_CMD(SE_CMD_SIG));
  SE_API_UNLOCK;
  if (status_code == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(R, pR.Data, pR.Length);
    memcpy(S, pS.Data, pS.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

uint32_t SE_GetKeySize(uint8_t slot)
{
  if (slot == SE_SLOT_PREPROVISIONNED)
  {
    return se_ctx.slot0_key_size;
  }
  if (slot == SE_SLOT_PERMANENT)
  {
    return se_ctx.slot1_key_size;
  }
  if (slot == SE_SLOT_EPHEMERAL)
  {
    return se_ctx.slotff_key_size;
  }
  return 0;
}

static SE_API_Status_t se_GetMapping(SE_Region_t *regions, uint32_t *nb_region)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_DataPartitionBuffer_t outDataPartition;

  if (*nb_region != 0)
  {
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    outDataPartition.pZoneInfoRecord = malloc(*nb_region * sizeof(StSafeA_ZoneInformationRecordBuffer_t));

    outDataPartition.NumberOfZones = *nb_region;
    if (outDataPartition.pZoneInfoRecord == NULL)
    {
      return SE_COMMAND_ERROR;
    }
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    status_code = StSafeA_DataPartitionQuery(SE_HDL, *nb_region, &outDataPartition, SE_SEC_LEVEL_NONE);
    if (status_code == STSAFEA_OK)
    {
      if (outDataPartition.NumberOfZones <= *nb_region)
      {
        *nb_region = outDataPartition.NumberOfZones;
      }

      for (uint32_t i = 0; i < *nb_region; i++)
      {
        regions[i].index = outDataPartition.pZoneInfoRecord[i].Index;
        regions[i].size = outDataPartition.pZoneInfoRecord[i].DataSegmentLength;
        regions[i].read_update = outDataPartition.pZoneInfoRecord[i].ReadAcChangeRight;
        regions[i].write_update = outDataPartition.pZoneInfoRecord[i].UpdateAcChangeRight;
        regions[i].read_ac = outDataPartition.pZoneInfoRecord[i].ReadAccessCondition;
        regions[i].write_ac = outDataPartition.pZoneInfoRecord[i].UpdateAccessCondition;
        regions[i].type = outDataPartition.pZoneInfoRecord[i].ZoneType;
        if (outDataPartition.pZoneInfoRecord[i].ZoneType == 1)
        {
          regions[i].OneWay = outDataPartition.pZoneInfoRecord[i].OneWayCounter;
        }
      }
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
      free(outDataPartition.pZoneInfoRecord);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
      return SE_OK;
    }
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    free(outDataPartition.pZoneInfoRecord);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  }
  else
  {
    StSafeA_ZoneInformationRecordBuffer_t tempZinfo;
    outDataPartition.pZoneInfoRecord = &tempZinfo;
    status_code = StSafeA_DataPartitionQuery(SE_HDL, *nb_region, &outDataPartition, SE_SEC_LEVEL_NONE);
    if ((status_code == STSAFEA_INVALID_RESP_LENGTH) || (status_code == STSAFEA_OK))
    {
      *nb_region = outDataPartition.NumberOfZones;
      return SE_OK;
    }
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GetMapping(SE_Region_t *regions, uint32_t *nb_region)
{
  if (*nb_region != 0)
  {
    if (se_ctx.mapping_size <= *nb_region)
    {
      *nb_region = se_ctx.mapping_size;
    }
    memcpy(regions, se_ctx.mapping, *nb_region * sizeof(SE_Region_t));
  }
  else
  {
    *nb_region = se_ctx.mapping_size;
  }
  return SE_OK;
}

SE_API_Status_t SE_GetState(SE_State_t *state)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_LifeCycleStateBuffer_t outlf;
  if (state == NULL)
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  status_code = StSafeA_LifeCycleStateQuery(SE_HDL, &outlf, SE_SEC_LEVEL_NONE);
  if (status_code != STSAFEA_OK)
  {
    return SE_COMMAND_ERROR;
  }


  switch (outlf.LifeCycleStatus)
  {
    case 3:
      *state = SE_STATE_OPERATIONNAL;
      break;
    case 4:
      *state = SE_STATE_TERMINATED;
      break;
    case 0x83:
      *state = SE_STATE_LOCKED;
      break;
    default:
      *state = SE_STATE_UNKNOWN;
      break;
  }

  return SE_OK;
}

static uint8_t authvalue(uint8_t val)
{
  if (val == 3)
  {
    return STSAFEA_MAC_HOST_CMAC;
  }
  return STSAFEA_MAC_NONE;
}

static uint8_t encvalue(uint8_t val)
{
  switch (val)
  {
    case 1 :
      return STSAFEA_ENCRYPTION_RESPONSE;
    case 2 :
      return STSAFEA_ENCRYPTION_COMMAND;
    case 3 :
      return STSAFEA_ENCRYPTION_COMMAND_RESPONSE;
    default:
      return STSAFEA_ENCRYPTION_NONE;
  }
}

static SE_API_Status_t se_GetCommandConfig(void)
{
  StSafeA_ResponseCode_t status_code;
  StSafeA_TLVBuffer_t inCmd;
  uint8_t data[100];
  int32_t index = -1;
  int32_t i;
  StSafeA_TLVBuffer_t out;
  out.LV.Data = data;
  out.LV.Length = sizeof(data);

  inCmd.Header = 0x14;
  inCmd.LV.Data = data;
  data[0] = 0x24;
  inCmd.LV.Length = 1;

  status_code = StSafeA_RawCommand(SE_HDL, &inCmd, sizeof(data), &out, 0, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
    for (i = 0; i < 9; i++)
    {
      switch (data[2 + i * 3])
      {
        case 0x18: /* Establish Key */
          index = SE_CMD_SHARED_SECRET;
          break;
        case 0x0e: /* Wrap */
          index = SE_CMD_WRAP;
          break;
        case 0x0f: /* Unwrap */
          index = SE_CMD_UNWRAP;
          break;
        case 0x16: /* Generate signature */
          index = SE_CMD_SIG;
          break;
        default:
          index = -1;
          break;
      }
      if (index != -1)
      {
        se_ctx.cmd_params[index][SE_AUTH_PARAM] = authvalue(data[2 + i * 3 + 1]);
        se_ctx.cmd_params[index][SE_ENC_PARAM] = encvalue(data[2 + i * 3 + 2]);
      }
    }
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_GetCommandStatus(SE_Cmd_t *cmd, uint32_t *size)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_INVALID_PARAMETER;
  StSafeA_CommandAuthorizationConfigurationBuffer_t cmdAuthConfig = { 0 };
  SE_API_Status_t ret = SE_COMMAND_ERROR;
  if (*size == 0)
  {
    StSafeA_CommandAuthorizationRecordBuffer_t cmdAuthRecs = { 0 };
    cmdAuthConfig.pCommandAuthorizationRecord = &cmdAuthRecs;
    status_code = StSafeA_CommandAuthorizationConfigurationQuery(SE_HDL,
                                                                 *size,
                                                                 &cmdAuthConfig,
                                                                 STSAFEA_MAC_NONE);
    if ((status_code == STSAFEA_INVALID_RESP_LENGTH) || (status_code == STSAFEA_OK))
    {
      *size = cmdAuthConfig.CommandAuthorizationRecordNumber;
      return SE_OK;
    }
    return SE_COMMAND_ERROR;
  }
  else
  {
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    cmdAuthConfig.pCommandAuthorizationRecord = malloc(*size * sizeof(StSafeA_CommandAuthorizationRecordBuffer_t));
    if (cmdAuthConfig.pCommandAuthorizationRecord == NULL)
    {
      return SE_COMMAND_ERROR;
    }
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    status_code = StSafeA_CommandAuthorizationConfigurationQuery(SE_HDL,
                                                                 *size,
                                                                 &cmdAuthConfig,
                                                                 STSAFEA_MAC_NONE);
    SE_PRINTF("%s : error = %x\n", __func__, status_code);
    if (status_code == STSAFEA_OK)
    {
      ret = SE_OK;
      if (*size > cmdAuthConfig.CommandAuthorizationRecordNumber)
      {
        *size = cmdAuthConfig.CommandAuthorizationRecordNumber;
      }

      for (uint32_t i = 0; i < *size; i++)
      {
        cmd[i].id = cmdAuthConfig.pCommandAuthorizationRecord[i].CommandCode;
        cmd[i].auth = cmdAuthConfig.pCommandAuthorizationRecord[i].CommandAC;
        cmd[i].enc = cmdAuthConfig.pCommandAuthorizationRecord[i].HostEncryptionFlags;
      }
    }
    else
      ret = SE_COMMAND_ERROR;
    if (cmdAuthConfig.pCommandAuthorizationRecord)
    {
      free(cmdAuthConfig.pCommandAuthorizationRecord);
    }
  }
  return ret;
}

int32_t SE_GetRandom(void *ctx, uint8_t *data, size_t size)
{
  StSafeA_ResponseCode_t ret = STSAFEA_OK;
  StSafeA_LVBuffer_t out;

  (void)ctx;
  if ((data == NULL) || (size == 0))
  {
    return -1;
  }
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  out.Data = data;
  out.Length = size;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
  ret = StSafeA_GenerateRandom(SE_HDL, STSAFEA_EPHEMERAL_RND, size, &out, SE_SEC_LEVEL_NONE);
  if (ret == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(data, out.Data, out.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    return 0;
  }
  return -1;
}

SE_API_Status_t SE_GetInfo(SE_Info_t *se_info)
{
  StSafeA_ResponseCode_t ret = STSAFEA_OK;
  StSafeA_ProductDataBuffer_t OutProd;
  StSafeA_LifeCycleStateBuffer_t OutLife;

  /* Product Data Query always OK */
  ret = StSafeA_ProductDataQuery(SE_HDL, &OutProd, STSAFEA_MAC_NONE);
  if (ret != STSAFEA_OK)
  {
    return SE_COMMAND_ERROR;
  }
  se_info->productData.InOutBufferSize = OutProd.InputOutputBufferSize;
  se_info->productData.NVMSize = OutProd.NonVolatileMemorySize;
  se_info->productData.AtomicityBuffer = OutProd.AtomicityBufferSize;
  se_info->productData.serial = ((uint32_t)&OutProd.STNumber) - ((uint32_t)&OutProd);
  memcpy(se_info->productData.raw_data, &OutProd, sizeof(OutProd));
  se_info->productData.raw_size = sizeof(OutProd);
  /* Life Cycle State always OK */
  ret = StSafeA_LifeCycleStateQuery(SE_HDL, &OutLife, STSAFEA_MAC_NONE);
  if (ret != STSAFEA_OK)
  {
    return SE_COMMAND_ERROR;
  }
  se_info->lifecycle = OutLife.LifeCycleStatus;
  if (se_info->lifecycle == STSTAFEA_LIFECYCLE_STATE_OPERATIONAL)
  {
    /* if life cycle state is operationnal */
    StSafeA_LocalEnvelopeKeyTableBuffer_t OutTable;
    StSafeA_LocalEnvelopeKeyInformationRecordBuffer_t OutSlot0, OutSlot1;
    /* Local Envelop */
    ret = StSafeA_LocalEnvelopeKeySlotQuery(SE_HDL, &OutTable, &OutSlot0, &OutSlot1, STSAFEA_MAC_NONE);
    if (ret != STSAFEA_OK)
    {
      return SE_COMMAND_ERROR;
    }
    se_info->envelop.presence0 = OutSlot0.PresenceFlag;
    se_info->envelop.size0 = OutSlot0.KeyLength;
    se_info->envelop.presence1 = OutSlot1.PresenceFlag;
    se_info->envelop.size1 = OutSlot1.KeyLength;
    /* Pairing status */
    StSafeA_HostKeySlotBuffer_t OutHost;
    ret = StSafeA_HostKeySlotQuery(SE_HDL, &OutHost, STSAFEA_MAC_NONE);
    if (ret != STSAFEA_OK)
    {
      return SE_COMMAND_ERROR;
    }
    se_info->pairing.present = OutHost.HostKeyPresenceFlag;
    if (se_info->pairing.present == 1)
    {
      se_info->pairing.counter = OutHost.HostCMacSequenceCounter;
    }
    else
      se_info->pairing.counter = 0;
    /* I2C Params */
    StSafeA_I2cParameterBuffer_t OutI2c;
    ret = StSafeA_I2cParameterQuery(SE_HDL, &OutI2c, STSAFEA_MAC_NONE);
    if (ret != STSAFEA_OK)
    {
      return SE_COMMAND_ERROR;
    }
    se_info->i2cParams.I2cAddress = OutI2c.I2cAddress;
    se_info->i2cParams.LowPowerModeConfig = OutI2c.LowPowerModeConfig;
    se_info->i2cParams.LockConfig = OutI2c.LockConfig;
  }
  return SE_OK;
}

SE_API_Status_t SE_Reset(uint32_t type)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_BOUNDARY_EXCEEDED;
  SE_API_Status_t  ret = SE_INVALID_PARAMS_ERROR;
  if (type == 0)
  {
    /* Soft Reset */
    status_code = StSafeA_Reset(SE_HDL, STSAFEA_MAC_NONE);
  }
  else
  {
    /* IO Reset - Not implemented */

  }
  if (status_code == STSAFEA_OK)
  {
    ret = SE_OK;
  }
  return ret;
}


SE_API_Status_t SE_Raw(uint8_t *cmd, size_t cmd_size, uint8_t *rsp, size_t *rsp_size)
{
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;
  StSafeA_TLVBuffer_t inCmd;
  StSafeA_TLVBuffer_t outTLV;
#if ! (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  outTLV.LV.Data = rsp;
  outTLV.LV.Length = *rsp_size;
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */

  inCmd.Header = cmd[0];
  inCmd.LV.Data = &cmd[1];
  inCmd.LV.Length = cmd_size - 1;

  status_code = StSafeA_RawCommand(SE_HDL, &inCmd, *rsp_size, &outTLV, 0, SE_SEC_LEVEL_NONE);
  if (status_code == STSAFEA_OK)
  {
#if (STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
    memcpy(rsp, outTLV.LV.Data, outTLV.LV.Length);
#endif /* STSAFEA_USE_OPTIMIZATION_SHARED_RAM */
    *rsp_size = outTLV.LV.Length;
    return SE_OK;
  }
  return SE_COMMAND_ERROR;
}

SE_API_Status_t SE_Hibernate(uint32_t type)
{
  SE_API_Status_t ret = SE_COMMAND_ERROR;
  StSafeA_ResponseCode_t status_code = STSAFEA_OK;
  uint8_t wakeup = STSAFEA_WAKEUP_FROM_I2C_START_OR_RESET;
  if (type == 1)
  {
    wakeup = STSAFEA_WAKEUP_FROM_RESET;
  }

  status_code = StSafeA_Hibernate(SE_HDL, wakeup, STSAFEA_MAC_NONE);
  if (status_code == STSAFEA_OK)
  {
    ret = SE_OK;
  }
  return ret;
}

/* check if copy is needed in case of PSA, this is not needed */
SE_API_Status_t SE_Init_PubKeys(SE_Public_Key_Entry_t *pub_keys, size_t nb_pub_keys)
{
  if ((pub_keys == NULL) && (nb_pub_keys != 0))
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  if (nb_pub_keys >= 1)
  {
    se_ctx.renewable_pub_keys[0] = pub_keys;
  }
  if (nb_pub_keys >= 2)
  {
    se_ctx.renewable_pub_keys[1] = &pub_keys[1];
  }
  return SE_OK;
}

SE_API_Status_t SE_Get_PubKey(SE_Slot_t slot, uint8_t *pub_x, uint8_t *pub_y, size_t *size, SE_Key_Type_t *type)
{
  SE_Public_Key_Entry_t *key;
  size_t key_size = 0;
  uint32_t i;
  uint32_t zero_count = 0;

  if ((type == NULL) || (size == NULL))
  {
    return SE_INVALID_PARAMS_ERROR;
  }

  switch (slot)
  {
    case SE_SLOT_PREPROVISIONNED :
      key = &se_ctx.fixed_pub_keys[0];
      break;
    case SE_SLOT_PERMANENT :
      key = se_ctx.renewable_pub_keys[0];
      break;
    case SE_SLOT_EPHEMERAL :
      key = se_ctx.renewable_pub_keys[1];
      break;
    default :
      return SE_INVALID_PARAMS_ERROR;
  }

  key_size = SE_GetKeySize(slot) / 8;
  /* parameters size */
  if (key_size == 0)
  {
    *size = 0;
    *type = SE_KEY_TYPE_NONE;
    return SE_OK;
  }

  for (i = 0; i < key_size; i++)
  {
    if ((key->public_key_x[i] == 0) && (key->public_key_y[i] == 0))
    {
      zero_count++;
    }
  }

  if (zero_count == key_size)
  {
    *size = 0;
    *type = SE_KEY_TYPE_NONE;
    return SE_OK;
  }

  *type = key->public_key_curve;

  if (*size < key_size)
  {
    *size = key_size;
    return SE_BUFFER_TOO_SHORT_ERROR;
  }
  *size = key_size;
  if (pub_x != NULL)
  {
    memcpy(pub_x, key->public_key_x, key_size);
  }
  if (pub_y != NULL)
  {
    memcpy(pub_y, key->public_key_y, key_size);
  }
  return SE_OK;
}
