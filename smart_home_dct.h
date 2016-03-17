/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#pragma once

#include <stdint.h>
#include "uart_keypad.h"
#include "light_dev.h"
#include "comm.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#define MAX_LIGHT_COUNT 4
#define MAX_CURTAIN_COUNT 2
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/


/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct {
	uint8_t light_no;
	uart_key_code_t key_code;
	relay_gpio_t relay_io;
	light_status_t status;
	uint8_t light_name[32];
	//struct light_dev *owner;
} light_config_t;

typedef struct {
	uint8_t light_count;
	light_config_t light_config[MAX_LIGHT_COUNT];
} light_dev_config_t;

typedef struct {
	uint8_t curtain_no;
	uint8_t calibrated;
	uint32_t	current_pos_ms;
	uint32_t	full_pos_ms;
	uint8_t gpio_open;
	uint8_t gpio_close;
	uint8_t key_open;
	uint8_t key_close;
	uint8_t key_stop;
	uint8_t curtain_name[32];
} curtain_config_t;

typedef struct {
	uint8_t curtain_count;
	curtain_config_t curtain_config[MAX_CURTAIN_COUNT];
} curtain_dev_config_t;

typedef struct {
	uint8_t		device_configured;
	wiced_bool_t light_dev_enable;
	wiced_bool_t curtain_dev_enable;
	uint8_t		dev_type;
	uint8_t		dev_index;
	uint8_t		dev_name[32];
	light_dev_config_t light_dev_config;
	curtain_dev_config_t curtain_dev_config;
	union {
		light_config_t light_config;
		curtain_config_t curtain_config;
	} specific;
} smart_home_app_dct_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
