/**
  ******************************************************************************
  * @file    describe.c
  * @author  SMD application team
  * @version V1.1.0
  * @brief   Implementation of describe application that display STSAFE-A config
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
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
//#include <unistd.h>

#include "common_app.h"
#include "common_mbedtls.h"
#include "term_colors.h"

#define xstr(s) str(s)
#define str(s) #s

char trace_buff[500];

int printf_tfm(char const *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int len;

  len = vsnprintf(trace_buff, sizeof(trace_buff), fmt, args);

  tfm_hal_output_spm_log(trace_buff, len);
}


#define ERROR_PRINT(_mod, _val) { printf_tfm("%sError while " # _mod " : "UINT32"\n", RED, _val); printf(NC);}
#define WARN_PRINT(_mod, _val)  { printf_tfm("%sError while " # _mod " : "UINT32"\n", PURPLE, _val);printf(NC);}
#define LOG_PRINT                 printf_tfm

#define UINT32 "%u"


char *perm2str(uint8_t val)
{
  switch (val)
  {
    case 0:
      return GREEN "Always      "NC;
    case 1:
      return YELLOW"Pairing Key "NC;
    case 2:
      return YELLOW"Admin Auth  "NC;
    case 3:
      return YELLOW"Admin Auth 2"NC;
    case 5:
    case 6:
    case 7:
    case 4:
      return RED   "Never       "NC;
    default :
      return RED "Unknown     "NC;
  }
}

char *oneway2str(SE_Region_t *reg)
{
  char *value;
  if (reg->type == 0)
  {
    return YELLOW"None"NC;
  }
  if (reg->type == 1)
  {
    value = malloc(40);
    snprintf(value, 40, "%s"UINT32"%s", YELLOW, reg->OneWay, NC);
    return value;
  }
  return "";
}

char *state2str(SE_State_t state)
{
  switch (state)
  {
    case SE_STATE_OPERATIONNAL:
      return GREEN "Operationnal" NC;
    case SE_STATE_LOCKED:
      return YELLOW "Locked" NC;
    case SE_STATE_TERMINATED:
      return RED "Terminated" NC;
    default :
      return RED "Unknown" NC;
  }
  return RED "Unknown" NC;
}

char *cmd2str(uint8_t val)
{
  switch (val)
  {
    case 0x1C :
      return CYAN "Decrypt            (1C)"NC;
    case 0x08 :
      return CYAN "Derive Key         (08)"NC;
    case 0x1B :
      return CYAN "Encrypt            (1B)"NC;
    case 0x18 :
      return CYAN "Establish Key      (18)"NC;
    case 0x09 :
      return CYAN "Generate MAC       (09)"NC;
    case 0x16 :
      return CYAN "Generate Signature (16)"NC;
    case 0x0f :
      return CYAN "Unwrap Local       (0F)"NC;
    case 0x0E :
      return CYAN "Wrap Local         (0E)"NC;
    case 0x0A :
      return CYAN "Verify MAC         (0A)"NC;
  }
  return RED "Unknown CMD            "NC;
}

char *auth2str(uint8_t val)
{
  switch (val)
  {
    case 0:
      return RED    "Never               " NC;
    case 1:
      return GREEN  "Free                " NC;
    case 2:
      return YELLOW "Admin               " NC;
    case 3:
      return YELLOW "Pairing Key         " NC;
    case 4:
      return YELLOW "Admin or Pwd        " NC;
    case 5:
      return YELLOW "Admin or Pairing Key" NC;
    case 7:
      return YELLOW "Admin               " NC;
  }
  return RED "Invalid Auth        " NC;
}

char *enc2str(uint8_t val)
{
  switch (val)
  {
    case 0 :
      return GREEN "None                    "NC;
    case 1 :
      return YELLOW"Response enc            "NC;
    case 2 :
      return YELLOW"Command enc             "NC;
    case 3 :
      return YELLOW"Command and Response enc"NC;
  }
  return RED "Invalid Params          "NC;
}

void get_pairing_status()
{
  int32_t ret;
  uint8_t presence;
  uint32_t seq_count;
  ret = SE_GetPairingStatus(&presence, &seq_count);
  if (ret == SE_OK)
  {
    if (presence == 1)
    {
      LOG_PRINT("Pairing Keys are set Sequence Counter = "UINT32"\n", seq_count);
    }
    else
    {
      LOG_PRINT("Pairing Keys are not set\n");
    }
  }
}

int32_t stsafe_describe()
{
  int32_t ret;
  uint8_t se_serial[9];
  SE_Region_t *regions;
  uint32_t nb_regions = 20;
  SE_State_t state;
  uint32_t init_val = SE_INIT_TEST_ECHO | SE_INIT_TEST_PAIRING;

  SE_SetDebug(LOG_PRINT);
  ret = SE_Init(init_val, SE_SECURITY_LEVEL_NONE, 0x20);
  if (ret != SE_OK)
  {
    ERROR_PRINT(SE_Init, ret);
  }

  ret = SE_GetSerial(se_serial, SE_SERIAL_FORMAT_RAW);
  if (ret != SE_OK)
  {
    ERROR_PRINT(SE_GetSerial, ret);
    goto describe_end;
  }

  LOG_PRINT("SE Serial Number " YELLOW " %02x%02x%02x%02x%02x%02x%02x%02x%02x\n" NC, se_serial[0], se_serial[1],
            se_serial[2], se_serial[3], se_serial[4], se_serial[5], se_serial[6], se_serial[7], se_serial[8]);

  get_pairing_status();

  ret = SE_GetState(&state);
  if (ret != SE_OK)
  {
    ERROR_PRINT(SE_GetState, ret);
    goto describe_end;
  }

  LOG_PRINT("SE State %s\n", state2str(state));

  if (state == SE_STATE_LOCKED)
  {
    LOG_PRINT("StSafe is locked \n");
    goto describe_end;
  }


  ret = SE_GetKeySize(0);

  if (ret <= 0)
  {
    ERROR_PRINT(SE_GetKeySize, ret);
    goto describe_end;
  }

  LOG_PRINT("SE Slot O key size = %s"UINT32"%s\n", YELLOW, ret, NC);

  ret = SE_GetKeySize(1);

  if (ret <= 0)
  {
    WARN_PRINT(SE_GetKeySize, ret);
  }

  LOG_PRINT("SE Slot 1 key size = %s"UINT32"%s\n", YELLOW, ret, NC);

  regions = malloc(sizeof(SE_Region_t) * nb_regions);
  if (regions != NULL)
  {
    ret = SE_GetMapping(regions, &nb_regions);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetMapping, ret);
      goto describe_end;
    }
    for (int i = 0; i < nb_regions; i++)
    {
      LOG_PRINT("Region " GREEN "% 2d" NC" : Size = "YELLOW"% 6d"
                NC" Read Update %s, Read Access %s, Write Update %s, Write Access %s, OneWay %s\n", regions[i].index, regions[i].size,
                (regions[i].read_update == 1 ? "True " : "False"), perm2str(regions[i].read_ac), (regions[i].write_update == 1 ? "True " : "False"), perm2str(regions[i].write_ac), oneway2str(&regions[i]));
    }
  }

  {
    SE_Cmd_t cmds[9];
    uint32_t cmd_size = sizeof(cmds);
    ret = SE_GetCommandStatus(cmds, &cmd_size);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetCommandStatus, ret);
      goto describe_end;
    }
    for (int i = 0; i < 9; i++)
    {
      LOG_PRINT("Command %s authentication %s encryption %s\n", cmd2str(cmds[i].id), auth2str(cmds[i].auth),
                enc2str(cmds[i].enc));
    }
  }

  {
    uint8_t cert[1024];
    char buff[4096];
    uint32_t sizes[2];
    uint8_t *data[2];
    uint8_t hash[32];
    data[1] = NULL;
    sizes[1] = 0;

    size_t cert_size = sizeof(cert);
    ret = SE_GetCertificate(0, cert, &cert_size);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetCertificate, ret);
      goto describe_end;
    }
    data[0] = cert;
    sizes[0] = cert_size;
    ret = display_cert(cert, cert_size, buff, 4096, "STSAFE Certificate  ");
    LOG_PRINT("Certificate :\n%s", buff);
    hash256(data, sizes, hash);
    LOG_PRINT("certificate hash = ");
    for (int j = 0; j < 32; j++)
    {
      LOG_PRINT("%02x", hash[j]);
    }
    LOG_PRINT("\n\n");
    for (int i = 1; i < nb_regions; i++)
    {
      cert_size = sizeof(cert);
      ret = SE_GetCertificate(i, cert, &cert_size);
      if (ret != SE_OK)
      {
        LOG_PRINT(PURPLE "No certificate in region %d\n\n" NC, i);
      }
      else
      {
        data[0] = cert;
        sizes[0] = cert_size;
        ret = display_cert(cert, cert_size, buff, 4096, "STSAFE Certificate  ");
        LOG_PRINT("Certificate in region %d:\n%s", i, buff);
        hash256(data, sizes, hash);
        LOG_PRINT("certificate hash = ");
        for (int j = 0; j < 32; j++)
        {
          LOG_PRINT("%02x", hash[j]);
        }
        LOG_PRINT("\n");
        cert[0] = 0x00;
        cert[1] = 0x00;
        SE_SetSecurity(SE_SECURITY_LEVEL_C_PAIRING);
        ret = SE_Update(i, 0, 2, cert);
        if (ret != SE_OK)
        {
          WARN_PRINT(SE_Update, ret);
        }
        SE_SetSecurity(SE_SECURITY_LEVEL_NONE);
        LOG_PRINT("\n");
      }
    }
  }
  SE_Info_t se_info;
  ret = SE_GetInfo(&se_info);
  if (ret == SE_OK)
  {
    LOG_PRINT("Envelop Key 0 is %s \n\r", (se_info.envelop.presence0 == 1 ? "present": "not present"));
    if (se_info.envelop.presence0 == 1)
      LOG_PRINT("Envelop Key 0 size %d \n\r", se_info.envelop.size0);
      
    LOG_PRINT("Envelop Key 1 is %s \n\r", (se_info.envelop.presence1 == 1 ? "present": "not present"));
    if (se_info.envelop.presence1 == 1)
      LOG_PRINT("Envelop Key 1 size %d \n\r", se_info.envelop.size1);
  }
  
describe_end:
  return (0);
}
