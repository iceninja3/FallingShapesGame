/**
  ******************************************************************************
  * @file    common_app.c
  * @author  SMD application team
  * @version V1.0.0
  * @brief   Implementation file of common application function
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
#include "common_app.h"

int32_t GetKeyFromStr(char *key, SE_Key_Type_t *key_type)
{
  if ((key == NULL) || (key_type == NULL))
  {
    return -1;
  }

  if (strncmp(key, "NIST_256", 9) == 0)
  {
    *key_type = SE_KEY_TYPE_NIST_256;
    return 0;
  }

  if (strncmp(key, "NIST_384", 9) == 0)
  {
    *key_type = SE_KEY_TYPE_NIST_384;
    return 0;
  }

  if (strncmp(key, "BP_256", 9) == 0)
  {
    *key_type = SE_KEY_TYPE_BP_256;
    return 0;
  }

  if (strncmp(key, "BP_384", 9) == 0)
  {
    *key_type = SE_KEY_TYPE_BP_384;
    return 0;
  }
  return -1;
}


int32_t SE_app_set_auto_region(int32_t auto_region)
{
  int32_t ret;
  SE_Region_t *regions;
  uint32_t nb_regions = 20;

  if (auto_region < 0)
  {
    return -1;
  }
  regions = malloc(sizeof(SE_Region_t) * nb_regions);
  if (regions == NULL)
  {
    return -1;
  }

  ret = SE_GetMapping(regions, &nb_regions);

  if (ret != SE_OK)
  {
    printf("Unable to get SE mapping\n");
  }
  else
  {
    if (nb_regions >= auto_region)
    {
      ret = SE_SetAutoManagement(auto_region, regions[auto_region].size - 2, regions[auto_region].size - 1);
      if (ret != SE_OK)
      {
        printf("Unalbe to set auto managed region\n");
      }
    }
  }
  if (regions != NULL)
  {
    free(regions);
  }
  return ret;
}
