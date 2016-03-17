/*
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
 * WICED Configuration Mode Application
 *
 * This application demonstrates how to use WICED Configuration Mode
 * to automatically configure application parameters and Wi-Fi settings
 * via a softAP and webserver
 *
 * Features demonstrated
 *  - WICED Configuration Mode
 *
 * Application Instructions
 *   1. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. After the download completes, the terminal displays WICED startup
 *      information and starts WICED configuration mode.
 *
 * In configuration mode, application and Wi-Fi configuration information
 * is entered via webpages using a Wi-Fi client (eg. your computer)
 *
 * Use your computer to step through device configuration using WICED Config Mode
 *   - Connect the computer using Wi-Fi to the config softAP "WICED Config"
 *     The config AP name & passphrase is defined in the file <WICED-SDK>/include/default_wifi_config_dct.h
 *     The AP name/passphrase is : Wiced Config / 12345678
 *   - Open a web browser and type wiced.com in the URL
 *     (or enter 192.168.0.1 which is the IP address of the softAP interface)
 *   - The Application configuration webpage appears. This page enables
 *     users to enter application specific information such as contact
 *     name and address details for device registration
 *   - Change one of more of the fields in the form and then click 'Save settings'
 *   - Click the Wi-Fi Setup button
 *   - The Wi-Fi configuration page appears. This page provides several options
 *     for configuring the device to connect to a Wi-Fi network.
 *   - Click 'Scan and select network'. The device scans for Wi-Fi networks in
 *     range and provides a webpage with a list.
 *   - Enter the password for your Wi-Fi AP in the Password box (top left)
 *   - Find your Wi-Fi AP in the list, and click the 'Join' button next to it
 *
 * Configuration mode is complete. The device stops the softAP and webserver,
 * and attempts to join the Wi-Fi AP specified during configuration. Once the
 * device completes association, application configuration information is
 * printed to the terminal
 *
 * The wiced.com URL reference in the above text is configured in the DNS
 * redirect server. To change the URL, edit the list in
 * <WICED-SDK>/Library/daemons/dns_redirect.c
 * URLs currently configured are:
 *      # http://www.broadcom.com , http://broadcom.com ,
 *      # http://www.facebook.com , http://facebook.com ,
 *      # http://www.google.com   , http://google.com   ,
 *      # http://www.bing.com     , http://bing.com     ,
 *      # http://www.apple.com    , http://apple.com    ,
 *      # http://www.wiced.com    , http://wiced.com    ,
 *
 *  *** IMPORTANT NOTE ***
 *   The config mode API will be integrated into Wi-Fi Easy Setup when
 *   WICED-SDK-3.0.0 is released.
 *
 */

#include "wiced.h"
#include "smart_home.h"
#include "smart_home_dct.h"
//#include "uart_master_ctrl.h"
#include "uart_interface.h"
#include "net_interface.h"
#include "uart_keypad.h"
#include "light_dev.h"
#include "curtain_dev.h"
#include "device_config.h"
#include "cJSON.h"

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
static void keypad_handler( uart_key_code_t key_code, uart_key_event_t event );
static wiced_result_t device_init();
static void report_curtain_pos(void *arg);
static void report_light_status(void *arg);

/******************************************************
 *               Variable Definitions
 ******************************************************/
static uart_keypad_t device_keypad;
glob_info_t this_dev;
//static wiced_semaphore_t link_up_semaphore;
static const msg_type_t msg_bst_control = {0x42, 0x53, 0x54};

/******************************************************
 *               Function Definitions
 ******************************************************/
/* Parse text to JSON, then render back to text, and print! */
void doit(char *text)
{
	char *out;cJSON *json;
	
	json=cJSON_Parse(text);
	if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		out=cJSON_Print(json);
		cJSON_Delete(json);
		printf("%s\n",out);
		free(out);
	}
}

/* Read a file, parse, render back, etc. */
void dofile(char *filename)
{
	FILE *f;long len;char *data;
	
	f=fopen(filename,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
	doit(data);
	free(data);
}

/* Used by some code below as an example datatype. */
struct record {const char *precision;double lat,lon;const char *address,*city,*state,*zip,*country; };

/* Create a bunch of objects as demonstration. */
void create_objects()
{
	cJSON *root,*fmt,*img,*thm,*fld;char *out;int i;	/* declare a few. */
	/* Our "days of the week" array: */
	const char *strings[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
	/* Our matrix: */
	int numbers[3][3]={{0,-1,0},{1,0,0},{0,0,1}};
	/* Our "gallery" item: */
	int ids[4]={116,943,234,38793};
	/* Our array of "records": */
	struct record fields[2]={
		{"zip",37.7668,-1.223959e+2,"","SAN FRANCISCO","CA","94107","US"},
		{"zip",37.371991,-1.22026e+2,"","SUNNYVALE","CA","94085","US"}};

	/* Here we construct some JSON standards, from the JSON site. */
	
	/* Our "Video" datatype: */
	root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON_AddItemToObject(root, "format", fmt=cJSON_CreateObject());
	cJSON_AddStringToObject(fmt,"type",		"rect");
	cJSON_AddNumberToObject(fmt,"width",		1920);
	cJSON_AddNumberToObject(fmt,"height",		1080);
	cJSON_AddFalseToObject (fmt,"interlace");
	cJSON_AddNumberToObject(fmt,"frame rate",	24);
	
	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);	/* Print to text, Delete the cJSON, print it, release the string. */

	/* Our "days of the week" array: */
	root=cJSON_CreateStringArray(strings,7);

	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);

	/* Our matrix: */
	root=cJSON_CreateArray();
	for (i=0;i<3;i++) cJSON_AddItemToArray(root,cJSON_CreateIntArray(numbers[i],3));

/*	cJSON_ReplaceItemInArray(root,1,cJSON_CreateString("Replacement")); */
	
	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);


	/* Our "gallery" item: */
	root=cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Image", img=cJSON_CreateObject());
	cJSON_AddNumberToObject(img,"Width",800);
	cJSON_AddNumberToObject(img,"Height",600);
	cJSON_AddStringToObject(img,"Title","View from 15th Floor");
	cJSON_AddItemToObject(img, "Thumbnail", thm=cJSON_CreateObject());
	cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
	cJSON_AddNumberToObject(thm,"Height",125);
	cJSON_AddStringToObject(thm,"Width","100");
	cJSON_AddItemToObject(img,"IDs", cJSON_CreateIntArray(ids,4));

	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);

	/* Our array of "records": */

	root=cJSON_CreateArray();
	for (i=0;i<2;i++)
	{
		cJSON_AddItemToArray(root,fld=cJSON_CreateObject());
		cJSON_AddStringToObject(fld, "precision", fields[i].precision);
		cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
		cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
		cJSON_AddStringToObject(fld, "Address", fields[i].address);
		cJSON_AddStringToObject(fld, "City", fields[i].city);
		cJSON_AddStringToObject(fld, "State", fields[i].state);
		cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
		cJSON_AddStringToObject(fld, "Country", fields[i].country);
	}
	
/*	cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root,1),"City",cJSON_CreateIntArray(ids,4)); */
	
	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);

}

void application_start_bak( )
{
	/* Initialise the device */
    wiced_init( );
	
	/* a bunch of json: */
	char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";	
	char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
	char text3[]="[\n    [0, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
	char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n			\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
	char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";

	/* Process each json textblock by parsing, then rebuilding: */
	//doit(text1);
	//doit(text2);	
	//doit(text3);
	//doit(text4);
	//doit(text5);

	/* Parse standard testfiles: */
/*	dofile("../../tests/test1"); */
/*	dofile("../../tests/test2"); */
/*	dofile("../../tests/test3"); */
/*	dofile("../../tests/test4"); */
/*	dofile("../../tests/test5"); */

	/* Now some samplecode for building objects concisely: */
	create_objects();
	
	return 0;
}

void application_start( )
{
    /* Initialise the device */
    wiced_init( );

	//WPRINT_APP_INFO(("Version data & time 2015/09/06 10:00\n"));

	/* Configure the device */
    configure_device();

	if(device_init() != WICED_SUCCESS){
		WPRINT_APP_INFO(("device_init failed\n"));
		return;
	}
	
	WPRINT_APP_INFO(("end ...\n"));
}

static wiced_result_t device_init()
{
	smart_home_app_dct_t* dct_app;
    platform_dct_wifi_config_t* dct_wifi_config          = NULL;
	wiced_result_t res;

	/* get the wi-fi config section for modifying, any memory allocation required would be done inside wiced_dct_read_lock() */
    wiced_dct_read_lock( (void**) &dct_wifi_config, WICED_TRUE, DCT_WIFI_CONFIG_SECTION, 0, sizeof( *dct_wifi_config ) );

    /* Print original MAC addresses */
    this_dev.mac_addr = dct_wifi_config->mac_address;

    /* release the read lock */
    wiced_dct_read_unlock( dct_wifi_config, WICED_TRUE );
	
	if(	wiced_dct_read_lock( (void**) &dct_app, WICED_TRUE, DCT_APP_SECTION, 0, sizeof( *dct_app ) ) != WICED_SUCCESS)
	{
        return WICED_ERROR;
    }
	this_dev.configured = dct_app->device_configured;
	this_dev.dev_type = dct_app->dev_type;
	this_dev.light_dev_enable = dct_app->light_dev_enable;
	this_dev.curtain_dev_enable = dct_app->curtain_dev_enable;
	strncpy(this_dev.dev_name, dct_app->dev_name, sizeof(this_dev.dev_name));
	WPRINT_APP_INFO(("this_dev.dev_name is %s\n", this_dev.dev_name));
	wiced_dct_read_unlock( dct_app, WICED_TRUE );

	if(this_dev.dev_type == DEV_TYPE_MASTER) {
		uart_receive_enable(master_process_uart_msg);
		master_receive_enable();
	}else {
		if(this_dev.light_dev_enable == WICED_TRUE) {
			res = light_dev_init(&this_dev.light_dev, WICED_HARDWARE_IO_WORKER_THREAD, report_light_status);
			if(res != WICED_SUCCESS) {
				return WICED_ERROR;
			}
		}
		if(this_dev.curtain_dev_enable == WICED_TRUE) {
			res = curtain_dev_init(&this_dev.curtain_dev, WICED_HARDWARE_IO_WORKER_THREAD, report_curtain_pos);
			if( res != WICED_SUCCESS) {
				return WICED_ERROR;
			}
		}
		uart_keypad_enable( &device_keypad, WICED_HARDWARE_IO_WORKER_THREAD, keypad_handler, 3000);
		slave_receive_enable();
	}
	return WICED_SUCCESS;
}

static void keypad_handler( uart_key_code_t key_code, uart_key_event_t event )
{
	int i;
	light_dev_t *light_dev = this_dev.light_dev;
	curtain_dev_t *curtain_dev = this_dev.curtain_dev;
	//WPRINT_APP_INFO(("light_keypad_handler: key_code = 0x%.2x, key_event = %d\n", code, event));

    if ( event == KEY_EVENT_RELEASED ) {
		if(light_dev != NULL) {
			for(i = 0; i < light_dev->light_count; i++) {
				if(key_code == light_dev->light_list[i].key_code) {
					switch_light_status(&light_dev->light_list[i]);
					break;
				}
			}
		}
		if(curtain_dev != NULL) {
			for(i = 0; i < curtain_dev->curtain_count; i++) {
				if(key_code == curtain_dev->curtain_list[i].key_open) {
					curtain_open(&curtain_dev->curtain_list[i]);
				} else if(key_code == curtain_dev->curtain_list[i].key_close) {
					curtain_close(&curtain_dev->curtain_list[i]);
				} else if(key_code == curtain_dev->curtain_list[i].key_stop) {
					curtain_stop(&curtain_dev->curtain_list[i]);
				}
			}
		}
    } else if( event == KEY_EVENT_LONG_LONG_PRESSED) {
		WPRINT_APP_INFO(("KEY_EVENT_LONG_LONG_PRESSED\n"));
		if(curtain_dev != NULL) {
			for(i = 0; i < curtain_dev->curtain_count; i++) {
				if(key_code == curtain_dev->curtain_list[i].key_open) {
					curtain_cali_start(&curtain_dev->curtain_list[i]);
					break;
				}
			}
		}
	}
}

static void report_curtain_pos(void *arg)
{
	msg_t msg;
	curtain_t *curtain = (curtain_t *)arg;

	WPRINT_APP_INFO(("report_curtain_pos\n"));
	
	memset(&msg, 0, sizeof(msg_t));
	memcpy(msg.msg_type, msg_bst_control, sizeof(msg.msg_type));
	msg.fun_type = CURTAIN_FUN_REPORT_POS;
	msg.fun_data = get_curtain_pos(curtain);
	msg.data_len = 0;
	send_to_pre_dev((char *)&msg, MSG_HEAD_LEN + msg.data_len);
	return;
}

static void report_light_status(void *arg)
{
	msg_t msg;
	light_t *light = (light_t *)arg;

	WPRINT_APP_INFO(("report_light_status\n"));
	memset(&msg, 0, sizeof(msg_t));
	memcpy(msg.msg_type, msg_bst_control, sizeof(msg.msg_type));
	msg.fun_type = LIGHT_FUN_REPORT_STATE;
	msg.data_len = 0;
	msg.fun_data = get_light_status(light->light_no);
	send_to_pre_dev((char *)&msg, MSG_HEAD_LEN + msg.data_len);
	return;
}

