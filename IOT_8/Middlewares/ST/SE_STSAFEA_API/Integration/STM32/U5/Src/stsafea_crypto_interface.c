/**
  ******************************************************************************
  * @file    stsafea_crypto_interface.c
  * @author  SMD application team
  * @version V3.1.0
  * @brief   Crypto Interface file to support the crypto services required by the
  *          STSAFE-A Middleware and offered by the mbedTLS crypto library:
  *           + Key Management
  *           + SHA
  *           + AES
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

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "stsafea_crypto.h"
#include "stsafea_interface_conf.h"
#include MCU_PLATFORM_INCLUDE

/** Intermediate testing pairing solution
  * Store Host Keys in ITS
  * Use MbedTLS crypto interface.
  * not test : hash interface (not use for STSAFE-A110)
  */

#include "psa/internal_trusted_storage.h"

#define SE_EXAMPLE_PAIRING

#ifdef SE_EXAMPLE_PAIRING
#include "se_stsafea_api.h"
#endif /* SE_EXAMPLE_PAIRING */

#if (USE_SIGNATURE_SESSION)
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#endif /* USE_SIGNATURE_SESSION */

#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"


#ifndef MBEDTLS_CMAC_C
#error Please activate MBEDTLS_CMAC_C in your mbedtls config file
#endif /* MBEDTLS_CMAC_C */
#ifndef MBEDTLS_AES_C
#error Please activate MBEDTLS_AES_C in your mbedtls config file
#endif /* MBEDTLS_AES_C */

#if (USE_SIGNATURE_SESSION)
#ifndef MBEDTLS_SHA256_C
#error Please activate MBEDTLS_SHA256_C in your mbedtls config file
#endif /* MBEDTLS_SHA256_C */
#ifndef MBEDTLS_SHA512_C
#error Please activate MBEDTLS_SHA512_C in your mbedtls config file
#endif /* MBEDTLS_SHA512_C */
#endif /* USE_SIGNATURE_SESSION */
/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
#if (USE_SIGNATURE_SESSION)
#ifdef MBEDTLS_SHA256_C
static mbedtls_sha256_context         sha256_ctx;
#endif /* MBEDTLS_SHA256_C */
#ifdef MBEDTLS_SHA512_C
static mbedtls_sha512_context         sha512_ctx;
#endif /* MBEDTLS_SHA512_C */
#endif /* USE_SIGNATURE_SESSION */

#if defined MBEDTLS_AES_C & defined MBEDTLS_CIPHER_MODE_CBC
static mbedtls_cipher_context_t       cipher_ctx;
#endif /* MBEDTLS_AES_C - MBEDTLS_CIPHER_MODE_CBC */

#define PSA_PS_PK_UID   0x53455f504b

#define ST_SE_DEFAULT_KEY_MAC    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF
#define ST_SE_DEFAULT_KEY_CIPHER 0x11,0x11,0x22,0x22,0x33,0x33,0x44,0x44,0x55,0x55,0x66,0x66,0x77,0x77,0x88,0x88

uint8_t cmac_key[16] = {0};
uint8_t cipher_key[16] = {0};

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/** @addtogroup CRYPTO_IF_Exported_Functions_Group1 Host MAC and Cipher keys Initialization
  *  @brief    Crypto Interface APIs to be implemented at application level. Templates are provided.
  *
@verbatim
 ===============================================================================
           ##### Host MAC and Cipher keys Initialization functions #####
 ===============================================================================

@endverbatim
  * @{
  */

/**
  * @brief   StSafeA_HostKeys_Init
  *          Initialize STSAFE-Axxx Host MAC and Cipher Keys that will be used by the crypto interface layer
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   None
  * @retval  0 if success. An error code otherwise
  */
int32_t StSafeA_HostKeys_Init()
{
  psa_status_t status;
  const psa_storage_uid_t uid = PSA_PS_PK_UID;
  size_t data_len;
  uint8_t read_data[32];


  /* read UID */
  status = psa_its_get(uid, 0, sizeof(read_data), read_data, &data_len);
  if (status == PSA_SUCCESS)
  {
    memcpy(cmac_key, read_data, 16);
    memcpy(cipher_key, &(read_data[16]), 16);
    memset(read_data, 0, 32);
  }
  else
  {
    if (status == PSA_ERROR_DOES_NOT_EXIST)
    {
      uint8_t defaults_key[32] = { ST_SE_DEFAULT_KEY_MAC, ST_SE_DEFAULT_KEY_CIPHER };
#ifdef SE_EXAMPLE_PAIRING
      SE_API_Status_t ret = SE_COMMAND_ERROR;
      printf_tfm("SE_SetPairaingKeys = ");
      ret = SE_SetPairingKeys(defaults_key);
      if (ret != SE_OK)
      {
        printf_tfm("KO\n\rSet pairing key error = %d\n\r", ret);
      }
      printf_tfm("OK\n\r");
      printf_tfm("SE_GenerateEnvelop 0 = ");
      ret = SE_GenerateEnvelop(0, 32);
      if (ret != SE_OK)
      {
        printf_tfm("KO\n\rSet pairing key error = %d\n\r", ret);
      }
      printf_tfm("OK\n\r");
      printf_tfm("SE_GenerateEnvelop 1 = ");
      ret = SE_GenerateEnvelop(1, 32);
      if (ret != SE_OK)
      {
        printf_tfm("KO\n\rSet pairing key error = %d\n\r", ret);
      }
      printf_tfm("OK\n\r");
#endif /* SE_EXAMPLE_PAIRING */

      /* for testing only, if key does not exist, create them with default keys */
      status = psa_its_set(uid, 32, defaults_key, PSA_STORAGE_FLAG_WRITE_ONCE);
      if (status == PSA_SUCCESS)
      {

        /* call again with key store updated */
        memset(defaults_key, 0, 32);
        return (StSafeA_HostKeys_Init());
      }
      memset(defaults_key, 0, 32);
      return -1;
    }
    return -1;
  }
  return 0;
}

/**
  * @}
  */

StSafeA_ResponseCode_t StSafeA_Crypto_GetKeys(uint8_t *serial, uint32_t size)
{
  /* pairing keys to be loaded */
  /* if needed SE serial number is provided */
  /* in this example, no need for SE key derivation */
  return STSAFEA_OK;
}
#if (USE_SIGNATURE_SESSION)
/** @addtogroup CRYPTO_IF_Exported_Functions_Group2 HASH Functions
  *  @brief    Crypto Interface APIs to be implemented at application level. Templates are provided.
  *
@verbatim
 ===============================================================================
                          ##### HASH functions #####
 ===============================================================================

@endverbatim
  * @{
  */

/**
  * @brief   StSafeA_SHA_Init
  *          SHA initialization function to initialize the SHA context
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   InHashType : type of SHA
  *          This parameter can be one of the StSafeA_HashTypes_t enum values:
  *            @arg STSAFEA_SHA_256: 256-bits
  *            @arg STSAFEA_SHA_384: 384-bits
  * @param   ppShaCtx : SHA context to be initialized
  * @retval  None
  */
void StSafeA_SHA_Init(StSafeA_HashTypes_t InHashType, void **ppShaCtx)
{
  switch (InHashType)
  {
#ifdef MBEDTLS_SHA256_C
    case STSAFEA_SHA_256:
      *ppShaCtx = &sha256_ctx;
      mbedtls_sha256_init(*ppShaCtx);
      mbedtls_sha256_starts(*ppShaCtx, 0);
      break;
#endif /* MBEDTLS_SHA256_C */

#ifdef MBEDTLS_SHA512_C
    case STSAFEA_SHA_384:
      *ppShaCtx = &sha512_ctx;
      mbedtls_sha512_init(*ppShaCtx);
      mbedtls_sha512_starts(*ppShaCtx, 1);
      break;
#endif /* MBEDTLS_SHA512_C */

    default:
      break;
  }
}

/**
  * @brief   StSafeA_SHA_Update
  *          SHA update function to process SHA over a message data buffer.
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   InHashType : type of SHA
  *          This parameter can be one of the StSafeA_HashTypes_t enum values:
  *            @arg STSAFEA_SHA_256: 256-bits
  *            @arg STSAFEA_SHA_384: 384-bits
  * @param   pShaCtx : SHA context
  * @param   pInMessage : message data buffer
  * @param   InMessageLength : message data buffer length
  * @retval  None
  */
void StSafeA_SHA_Update(StSafeA_HashTypes_t InHashType, void *pShaCtx, uint8_t *pInMessage, uint32_t InMessageLength)
{
  switch (InHashType)
  {
#ifdef MBEDTLS_SHA256_C
    case STSAFEA_SHA_256:
      if ((pShaCtx != NULL) && (pInMessage != NULL))
      {
        mbedtls_sha256_update(pShaCtx, pInMessage, InMessageLength);
      }
      break;
#endif /* MBEDTLS_SHA256_C */

#ifdef MBEDTLS_SHA512_C
    case STSAFEA_SHA_384:
      if ((pShaCtx != NULL) && (pInMessage != NULL))
      {
        mbedtls_sha512_update(pShaCtx, pInMessage, InMessageLength);
      }
      break;
#endif /* MBEDTLS_SHA512_C */

    default:
      break;
  }
}

/**
  * @brief   StSafeA_SHA_Final
  *          SHA final function to finalize the SHA Digest
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   InHashType : type of SHA
  *          This parameter can be one of the StSafeA_HashTypes_t enum values:
  *            @arg STSAFEA_SHA_256: 256-bits
  *            @arg STSAFEA_SHA_384: 384-bits
  * @param   ppShaCtx : SHA context to be finalized
  * @param   pMessageDigest : message digest data buffer
  * @retval  None
  */
void StSafeA_SHA_Final(StSafeA_HashTypes_t InHashType, void **ppShaCtx, uint8_t *pMessageDigest)
{
  switch (InHashType)
  {
#ifdef MBEDTLS_SHA256_C
    case STSAFEA_SHA_256:
      if (*ppShaCtx != NULL)
      {
        if (pMessageDigest != NULL)
        {
          mbedtls_sha256_finish(*ppShaCtx, pMessageDigest);
        }
        mbedtls_sha256_free(*ppShaCtx);
        *ppShaCtx = NULL;
      }
      break;
#endif /* MBEDTLS_SHA256_C */

#ifdef MBEDTLS_SHA512_C
    case STSAFEA_SHA_384:
      if (*ppShaCtx != NULL)
      {
        if (pMessageDigest != NULL)
        {
          mbedtls_sha512_finish(*ppShaCtx, pMessageDigest);
        }
        mbedtls_sha512_free(*ppShaCtx);
        *ppShaCtx = NULL;
      }
      break;
#endif /* MBEDTLS_SHA512_C */

    default:
      break;
  }
}
#endif /* USE_SIGNATURE_SESSION */

/**
  * @}
  */


/** @addtogroup CRYPTO_IF_Exported_Functions_Group3 AES Functions
  *  @brief    Crypto Interface APIs to be implemented at application level. Templates are provided.
  *
@verbatim
 ===============================================================================
                          ##### AES functions #####
 ===============================================================================

@endverbatim
  * @{
  */

/**
  * @brief   StSafeA_AES_MAC_Start
  *          Start AES MAC computation
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   ppAesMacCtx : AES MAC context
  * @retval  None
  */
void StSafeA_AES_MAC_Start(void **ppAesMacCtx)
{
#if defined MBEDTLS_AES_C & defined MBEDTLS_CIPHER_MODE_CBC
  *ppAesMacCtx = &cipher_ctx;

  mbedtls_cipher_init(*ppAesMacCtx);
  (void)mbedtls_cipher_setup(*ppAesMacCtx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB));
  (void)mbedtls_cipher_cmac_starts(*ppAesMacCtx, cmac_key, STSAFEA_HOST_KEY_LENGTH * 8);
#endif /* MBEDTLS_AES_C - MBEDTLS_CIPHER_MODE_CBC */
}

/**
  * @brief   StSafeA_AES_MAC_Update
  *          Update / Add data to MAC computation
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   pInData : data buffer
  * @param   InDataLength : data buffer length
  * @param   pAesMacCtx : AES MAC context
  * @retval  None
  */
void StSafeA_AES_MAC_Update(uint8_t *pInData, uint16_t InDataLength, void *pAesMacCtx)
{
#if defined MBEDTLS_AES_C & defined MBEDTLS_CIPHER_MODE_CBC
  (void)mbedtls_cipher_cmac_update(pAesMacCtx, pInData, InDataLength);
#endif /* MBEDTLS_AES_C - MBEDTLS_CIPHER_MODE_CBC */
}

/**
  * @brief   StSafeA_AES_MAC_LastUpdate
  *          Update / Add data to MAC computation
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   pInData : data buffer
  * @param   InDataLength : data buffer length
  * @param   pAesMacCtx : AES MAC context
  * @retval  None
  */
void StSafeA_AES_MAC_LastUpdate(uint8_t *pInData, uint16_t InDataLength, void *pAesMacCtx)
{
  StSafeA_AES_MAC_Update(pInData, InDataLength, pAesMacCtx);
}

/**
  * @brief   StSafeA_AES_MAC_Final
  *          Finalize AES MAC computation
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   pOutMac : calculated MAC
  * @param   ppAesMacCtx : AES MAC context
  * @retval  None
  */
void StSafeA_AES_MAC_Final(uint8_t *pOutMac, void **ppAesMacCtx)
{
#if defined MBEDTLS_AES_C & defined MBEDTLS_CIPHER_MODE_CBC
  (void)mbedtls_cipher_cmac_finish(*ppAesMacCtx, pOutMac);
  mbedtls_cipher_free(*ppAesMacCtx);
  *ppAesMacCtx = NULL;
#endif /* MBEDTLS_AES_C - MBEDTLS_CIPHER_MODE_CBC */
}

/**
  * @brief   StSafeA_AES_ECB_Encrypt
  *          AES ECB Encryption
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   pInData : plain data buffer
  * @param   pOutData : encrypted output data buffer
  * @param   InAesType : type of AES. Can be one of the following values:
  *            @arg STSAFEA_KEY_TYPE_AES_128: AES 128-bits
  *            @arg STSAFEA_KEY_TYPE_AES_256: AES 256-bits
  * @retval  0 if success, an error code otherwise
  */
int32_t StSafeA_AES_ECB_Encrypt(uint8_t *pInData, uint8_t *pOutData, uint8_t InAesType)
{
#ifdef MBEDTLS_AES_C
  int32_t status_code;
  mbedtls_aes_context aes;

  switch (InAesType)
  {
    case STSAFEA_KEY_TYPE_AES_128:
    case STSAFEA_KEY_TYPE_AES_256:
      mbedtls_aes_init(&aes);
      status_code = mbedtls_aes_setkey_enc(&aes, cipher_key, STSAFEA_AES_KEY_BITSIZE((uint32_t)InAesType));
      if ((status_code == 0) && (pInData != NULL) && (pOutData != NULL))
      {
        status_code = mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, pInData, pOutData);
      }
      mbedtls_aes_free(&aes);
      break;

    default:
      status_code = 1;
      break;
  }

  return status_code;
#else
  return 1;
#endif /* MBEDTLS_AES_C */
}

/**
  * @brief   StSafeA_AES_CBC_Encrypt
  *          AES CBC Encryption
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   pInData : plain data buffer
  * @param   InDataLength : plain data buffer length
  * @param   pOutData : encrypted output data buffer
  * @param   InInitialValue : initial value
  * @param   InAesType : type of AES. Can be one of the following values:
  *            @arg STSAFEA_KEY_TYPE_AES_128: AES 128-bits
  *            @arg STSAFEA_KEY_TYPE_AES_256: AES 256-bits
  * @retval  0 if success, an error code otherwise
  */
int32_t StSafeA_AES_CBC_Encrypt(uint8_t *pInData, uint16_t InDataLength, uint8_t *pOutData,
                                uint8_t *InInitialValue, uint8_t InAesType)
{
#if defined MBEDTLS_AES_C & defined MBEDTLS_CIPHER_MODE_CBC
  int32_t status_code;
  mbedtls_aes_context aes;

  switch (InAesType)
  {
    case STSAFEA_KEY_TYPE_AES_128:
    case STSAFEA_KEY_TYPE_AES_256:
      mbedtls_aes_init(&aes);
      status_code = mbedtls_aes_setkey_enc(&aes, cipher_key, STSAFEA_AES_KEY_BITSIZE((uint32_t)InAesType));
      if ((status_code == 0) && (pInData != NULL) && (pOutData != NULL) && (InInitialValue != NULL))
      {
        status_code = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, InDataLength, InInitialValue, pInData, pOutData);
      }
      mbedtls_aes_free(&aes);
      break;

    default:
      status_code = 1;
      break;
  }

  return status_code;
#else
  return 1;
#endif /* MBEDTLS_AES_C - MBEDTLS_CIPHER_MODE_CBC */
}

/**
  * @brief   StSafeA_AES_CBC_Decrypt
  *          AES CBC Decryption
  * @note    This is a weak function that MUST be implemented at application interface level.
  *          A specific example template stsafea_crypto_xxx_interface_template.c is provided with this Middleware
  *
  * @param   pInData : encrypted data buffer
  * @param   InDataLength : encrypted data buffer length
  * @param   pOutData : plain output data buffer
  * @param   InInitialValue : initial value
  * @param   InAesType : type of AES. Can be one of the following values:
  *            @arg STSAFEA_KEY_TYPE_AES_128: AES 128-bits
  *            @arg STSAFEA_KEY_TYPE_AES_256: AES 256-bits
  * @retval  0 if success, an error code otherwise
  */
int32_t StSafeA_AES_CBC_Decrypt(uint8_t *pInData, uint16_t InDataLength, uint8_t *pOutData,
                                uint8_t *InInitialValue, uint8_t InAesType)
{
#if defined MBEDTLS_AES_C & defined MBEDTLS_CIPHER_MODE_CBC
  int32_t status_code;
  mbedtls_aes_context aes;

  switch (InAesType)
  {
    case STSAFEA_KEY_TYPE_AES_128:
    case STSAFEA_KEY_TYPE_AES_256:
      mbedtls_aes_init(&aes);
      status_code = mbedtls_aes_setkey_dec(&aes, cipher_key, STSAFEA_AES_KEY_BITSIZE((uint32_t)InAesType));
      if ((status_code == 0) && (pInData != NULL) && (pOutData != NULL) && (InInitialValue != NULL))
      {
        status_code = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, InDataLength, InInitialValue, pInData, pOutData);
      }
      mbedtls_aes_free(&aes);
      break;

    default:
      status_code = 1;
      break;
  }

  return status_code;
#else
  return 1;
#endif /* MBEDTLS_AES_C - MBEDTLS_CIPHER_MODE_CBC */
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
