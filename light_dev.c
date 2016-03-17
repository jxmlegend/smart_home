/**
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */
#include "wiced.h"
#include "light_dev.h"
#include "smart_home_dct.h"
#include "wiced_time.h"
#include "wiced_rtos.h"
#include "wiced_utilities.h"
#include "wwd_constants.h"

/******************************************************
 *                      Macros
 ******************************************************/

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



/******************************************************
 *               Static Function Declarations
 ******************************************************/


/******************************************************
 *               Variable Definitions
 ******************************************************/
static light_t light_list[] =
{
	{
		.light_no = 1,
		.key_code = KEY_1,
		.relay_io = RELAY_GPIO_1,
	},
	{
		.light_no = 2,
		.key_code = KEY_2,
		.relay_io = RELAY_GPIO_2,
	},
	{
		.light_no = 3,
		.key_code = KEY_3,
		.relay_io = RELAY_GPIO_3,
	},
#if 0
	{
		.light_no = 4,
		.key_code = KEY_4,
		.relay_io = RELAY_GPIO_4,
	},
#endif
};

static light_dev_t *light_dev;

/******************************************************
 *               Function Definitions
 ******************************************************/
wiced_result_t light_dev_init(light_dev_t **light_dev_arg, wiced_worker_thread_t* thread, light_handler_t function)
{
	smart_home_app_dct_t*   dct_app;
	uint8_t light_status;
	int i;

	light_dev = (light_dev_t *) malloc_named("light_dev", sizeof(light_dev_t));
    if ( light_dev == NULL ) {
        return WICED_ERROR;
    }
	
	memset( light_dev, 0, sizeof(light_dev_t) );
	//light_dev->light_list = light_list;
	//light_dev->light_count = sizeof(light_list)/sizeof(light_t);
	light_dev->function   = function;
    light_dev->thread     = thread;

	wiced_dct_read_lock( (void**) &dct_app, WICED_TRUE, DCT_APP_SECTION, 0, sizeof( *dct_app ) );
	//light_status = dct_app->specific.light_config.light_status;
	light_dev->light_count = dct_app->light_dev_config.light_count;
	for(i = 0; i < light_dev->light_count; i++) {
		light_dev->light_list[i].light_no = dct_app->light_dev_config.light_config[i].light_no;
		light_dev->light_list[i].key_code = dct_app->light_dev_config.light_config[i].key_code;
		light_dev->light_list[i].relay_io = dct_app->light_dev_config.light_config[i].relay_io;
		light_dev->light_list[i].status = dct_app->light_dev_config.light_config[i].status;
		strncpy(light_dev->light_list[i].light_name, dct_app->light_dev_config.light_config[i].light_name, 32);
		set_light_status(light_dev->light_list[i].light_no, light_dev->light_list[i].status);
	}
	
	wiced_dct_read_unlock( dct_app, WICED_TRUE );
#if 0
	for(i = 0; i < sizeof(light_list)/sizeof(light_t); i++) {
		light_list[i].owner = light_dev;
		light_list[i].status = light_status & (0x01 << i);
		set_light_status_internal(&light_list[i], light_list[i].status);
	}
#endif
	*light_dev_arg = light_dev;
	
	return WICED_SUCCESS;
}

void switch_light_status(light_t *light)
{
	light_status_t status = get_light_status(light);

	if(status == LIGHT_STATUS_ON) {
		set_light_status(light, LIGHT_STATUS_OFF);
	} else if(status == LIGHT_STATUS_OFF) {
		set_light_status(light, LIGHT_STATUS_ON);
	}
	light_dev->function(light);
	return;
}

void set_light_status(light_t *light, light_status_t status)
{
	if(status == LIGHT_STATUS_ON) {
		wiced_gpio_output_high( light->relay_io);
		light->status = LIGHT_STATUS_ON;
		WPRINT_APP_INFO(("light_%d is on\n", light->light_no));
	}
	else if(status == LIGHT_STATUS_OFF) {
		wiced_gpio_output_low( light->relay_io );
		light->status = LIGHT_STATUS_OFF;
		WPRINT_APP_INFO(("light_%d is off\n", light->light_no));
	}
	return;
}

light_status_t get_light_status(light_t *light)
{
#if 0
	wiced_bool_t gpio_status;
	gpio_status = wiced_gpio_output_get( light->relay_io );
	if(gpio_status == WICED_TRUE)
		light->status = LIGHT_STATUS_ON;
	else
		light->status = LIGHT_STATUS_OFF;
#endif
	return light->status;
}
