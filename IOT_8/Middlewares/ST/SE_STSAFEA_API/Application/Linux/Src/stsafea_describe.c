/**
  ******************************************************************************
  * @file    stsafea_describe.c
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
#include "unistd.h"

#include "common_app.h"
#include "common_mbedtls.h"

#define xstr(s) str(s)
#define str(s) #s

#define BLACK       "\033[0;30m"
#define RED       "\033[0;31m"
#define GREEN     "\033[0;32m"
#define ORANGE      "\033[0;33m"
#define BLUE      "\033[0;34m"
#define PURPLE      "\033[0;35m"
#define CYAN      "\033[0;36m"
#define LIGTH_GRAY    "\033[0;37m"

#define DARK_GRAY   "\033[1;30m"
#define LIGHT_RED   "\033[1;31m"
#define LIGHT_GREEN   "\033[1;32m"
#define YELLOW      "\033[1;33m"
#define LIGHT_BLUE    "\033[1;34m"
#define LIGHT_PURPLE  "\033[1;35m"
#define LIGHT_CYAN    "\033[1;36m"
#define WHITE     "\033[1;37m"

#define NC "\033[0m"

#define ERROR_PRINT(_mod, _val) { printf(RED "Error while " # _mod " : %d\n" NC, _val);}
#define WARN_PRINT(_mod, _val) { printf(PURPLE "Error while " # _mod " : %d\n" NC, _val);}

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
    snprintf(value, 40, YELLOW"%d"NC, reg->OneWay);
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
  uint32_t presence;
  uint32_t seq_count;
  ret = SE_GetPairingStatus(&presence, &seq_count);
  if (ret == SE_OK)
  {
    if (presence == 1)
    {
      printf("Pairing Keys are set Sequence Counter = %d\n", seq_count);
    }
    else
    {
      printf("Pairing Keys are not set\n");
    }
  }
}


const char *argp_program_version = "stsafe_desribe 1.0.0";
const char *argp_program_bug_address = "<benjamin.baratte@st.com>";
static char doc[] = "Get general info on the STSAFE-A";
static char args_doc[] = "";
static struct argp_option options[] =
{
  { "auto", 'a', "AUTO", 0, "Select STSAFE-A auto management region. Default value is hard coded in the lib." },
  { "addr", 'i', "ADDR", 0, "Select STSAFE-A I2C addr (on 7bits). Default " xstr(STSAFEA_DEFAULT_I2C_ADDR) },
  { "pairing", 'p', 0, 0, "Activate secure channel testing.", 1 },
  { "loop", 'l', "LOOP", 0, "pairing channel loop test.", 1 },
  { "verbose", 'v', 0, 0, "Activate SE_API trace.", 1 },
  { 0 }
};

struct arguments
{
  int32_t auto_region;
  uint8_t addr;
  uint32_t loop;
  bool pairing;
  bool verbose;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
  {
    case 'p':
      arguments->pairing = true;
      break;
    case 'i':
      arguments->addr = arg ? strtol(arg, NULL, 0) : STSAFEA_DEFAULT_I2C_ADDR;
      break;
    case 'l':
      arguments->loop = arg ? atoi(arg) : 0;
      break;
    case 'a':
      arguments->auto_region = arg ? atoi(arg) : -1;
      break;
    case 'v':
      arguments->verbose = true;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
      {
        argp_usage(state);
      }
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 0)
        /* Not enough arguments. */
      {
        argp_usage(state);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };



int main(int ac, char **argv)
{
  int32_t ret;
  uint8_t se_serial[9];
  SE_Region_t *regions;
  uint32_t nb_regions = 20;
  SE_State_t state;
  uint32_t init_val = SE_INIT_TEST_ECHO;

  struct arguments arguments;

  arguments.auto_region = -1;
  arguments.addr = STSAFEA_DEFAULT_I2C_ADDR;
  arguments.pairing = false;
  arguments.loop = 0;
  arguments.verbose = false;

  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse(&argp, ac, argv, 0, 0, &arguments);

  if (arguments.verbose)
  {
    SE_SetDebug(printf);
  }
  if (arguments.pairing)
  {
    ret = SE_Init(init_val | SE_INIT_TEST_PAIRING, SE_SECURITY_LEVEL_NONE, arguments.addr);
  }
  else
    ret = SE_Init(init_val, SE_SECURITY_LEVEL_NONE, arguments.addr);

  if (ret != SE_OK)
  {
    ERROR_PRINT(SE_Init, ret);
  }

  if (arguments.auto_region != -1)
  {
    SE_app_set_auto_region(arguments.auto_region);
  }

  ret = SE_GetSerial(se_serial, SE_SERIAL_FORMAT_RAW);
  if (ret != SE_OK)
  {
    ERROR_PRINT(SE_GetSerial, ret);
  }

  printf("SE Serial Number " YELLOW " %02x%02x%02x%02x%02x%02x%02x%02x%02x\n" NC, se_serial[0], se_serial[1],
         se_serial[2], se_serial[3], se_serial[4], se_serial[5], se_serial[6], se_serial[7], se_serial[8]);

  get_pairing_status();

  ret = SE_GetState(&state);
  if (ret != SE_OK)
  {
    ERROR_PRINT(SE_GetState, ret);
  }

  printf("SE State %s\n", state2str(state));

  if (state == SE_STATE_LOCKED)
  {
    char buff[80];
    uint8_t derived = 0;
    printf("StSafe is locked do you want to unlock it ?\n");
    ret = scanf("%[ynYN]", buff);
    if (ret == 1)
    {
      int c;
      while ((c = getchar()) != EOF && c != '\n');
      if (*buff == 'y' || *buff == 'Y')
      {
        printf("StSafe is locked is password diversified ?\n");
        ret = scanf("%[ynYN]", buff);
        if (ret == 1)
        {
          while ((c = getchar()) != EOF && c != '\n');
          printf("Password is %s\n", (derived == 1 ? "derived" : "not derived"));
          uint8_t pwd[16];
          if (*buff == 'y' || *buff == 'Y')
          {
            derived = 1;
          }
          while (ret != 16)
          {
            printf("Enter password (16 bytes in hex without 0x): \n");
            ret = scanf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", &pwd[0], &pwd[1], &pwd[2], &pwd[3], &pwd[4], &pwd[5], &pwd[6], &pwd[7], &pwd[8], &pwd[9], &pwd[10], &pwd[11], &pwd[12], &pwd[13], &pwd[14], &pwd[15]);
            if (ret != 16)
            {
              printf("Password shall be 16 bytes long\n");
            }
          }
          if (derived == 1)
          {
            uint8_t hash_buf[32];
            uint8_t *data[3];
            uint32_t sizes[3];
            data[0] = se_serial;
            sizes[0] = 9;
            data[1] = pwd;
            sizes[1] = 16;
            data[2] = NULL;
            sizes[2] = 0;
            ret = hash256(data, sizes, hash_buf);
            memcpy(pwd, &hash_buf[16], 16);
          }
          printf("Password : ");
          for (int i = 0; i < 16; i++)
          {
            printf("%02X", pwd[i]);
          }
          printf("\n");
          ret = SE_Unlock(pwd);
          if (ret != SE_OK)
          {
            ERROR_PRINT(SE_Unlock, ret);
          }
          printf("STSAFE-A unlocked!!\n");
        }
      }
    }
  }


  ret = SE_GetKeySize(0);

  if (ret <= 0)
  {
    ERROR_PRINT(SE_GetKeySize, ret);
  }

  printf("SE Slot O key size = " YELLOW " %d\n" NC, ret);

  ret = SE_GetKeySize(1);

  if (ret <= 0)
  {
    WARN_PRINT(SE_GetKeySize, ret);
  }

  printf("SE Slot 1 key size = " YELLOW " %d\n" NC, ret);

  regions = malloc(sizeof(SE_Region_t) * nb_regions);
  if (regions != NULL)
  {
    ret = SE_GetMapping(regions, &nb_regions);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetMapping, ret);
    }
    for (int i = 0; i < nb_regions; i++)
    {
      printf("Region " GREEN
             "% 2d"NC" : Size = "YELLOW"% 6d"NC" Read Update %s, Read Access %s, Write Update %s, Write Access %s, OneWay %s\n",
             regions[i].index, regions[i].size, (regions[i].read_update == 1 ? "True " : "False"), perm2str(regions[i].read_ac), (regions[i].write_update == 1 ? "True " : "False"), perm2str(regions[i].write_ac), oneway2str(&regions[i]));
    }
  }

  {
    SE_Cmd_t cmds[9];
    ret = SE_GetCommandStatus(cmds);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetCommandStatus, ret);
    }
    for (int i = 0; i < 9; i++)
    {
      printf("Command %s authentication %s encryption %s\n", cmd2str(cmds[i].id), auth2str(cmds[i].auth),
             enc2str(cmds[i].enc));
    }
  }

  if (arguments.pairing)
  {
    get_pairing_status();
    ret = SE_SetSecurity(SE_SECURITY_LEVEL_R_PAIRING);
  }
  {
    uint8_t cert[1024];
    char buff[4096];
    size_t cert_size = sizeof(cert);
    ret = SE_GetCertificate(0, cert, &cert_size);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetCertificate, ret);
    }
    ret = display_cert(cert, cert_size, buff, 4096, "STSAFE Certificate  ");
    printf("Certificate :\n%s", buff);
    cert_size = sizeof(cert);
    ret = SE_GetActiveCertificate(cert, &cert_size);
    if (ret != SE_OK)
    {
      ERROR_PRINT(SE_GetActiveCertificate, ret);
    }
    else
    {
      ret = display_cert(cert, cert_size, buff, 4096, "STSAFE Certificate  ");
      printf("Auto Managed Certificate :\n%s", buff);
    }
  }

  if (arguments.pairing)
  {
    ret = SE_SetSecurity(SE_SECURITY_LEVEL_NONE);
    get_pairing_status();
  }

  if (arguments.loop != 0)
  {
    uint8_t  r_data[1];

    ret = SE_SetSecurity(SE_SECURITY_LEVEL_R_PAIRING);
    for (int i = 0; i < arguments.loop; i++)
    {
      ret = SE_Read(0, 0, 1, r_data);
      if (ret != SE_OK)
      {
        WARN_PRINT(SE_Read, ret);
        break;
      }
      if ((i % 10) == 0)
      {
        printf("loop % 010d/%d\r", i, arguments.loop);
        fflush(stdout);
      }
    }
    ret = SE_SetSecurity(SE_SECURITY_LEVEL_NONE);
    get_pairing_status();
  }
  return (0);
}
