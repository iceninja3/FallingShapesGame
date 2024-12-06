/**
  ******************************************************************************
  * @file    common_app.h
  * @author  SMD application team
  * @version V1.0.0
  * @brief   common application function headers
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
#ifndef __COMMON_APP_H_
#define __COMMON_APP_H_
#include "se_stsafea_api.h"

#include "common_mbedtls.h"

int32_t SE_app_set_auto_region(int32_t region);
int32_t GetKeyFromStr(char *key, SE_Key_Type_t *key_type);

#endif
