/* config.h
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_
#include "os_type.h"
#include "user_config.h"
typedef struct{
	uint32_t cfg_holder;

	uint8_t sta_ssid[64];
	uint8_t sta_pwd[64];
	uint32_t sta_type;

	uint8_t device_id[64];
	uint8_t mqtt_host[64];
	uint32_t mqtt_port;
	uint8_t mqtt_user[32];
	uint8_t mqtt_pass[32];

	uint8_t web_user[32];
	uint8_t web_pass[32];
	uint16_t enable_webpage_control;

	uint32_t mqtt_keepalive;

	// these variables added by ps
	uint16_t dawn;
	uint16_t dusk;
	uint8_t peak;
	uint8_t off_peak;
	uint8_t frost;
	uint16_t on_1;
	uint16_t off_1;
	uint16_t on_2;
	uint16_t off_2;
	uint8_t sensor;
	uint8_t out_0_status;
	uint8_t out_4_status;
	uint8_t out_5_status;
	uint8_t out_12_status;
	uint8_t out_13_status;
	uint8_t in_14_bounce;
	uint8_t five_enable;
	uint8_t override_0;
	uint32_t override_0_time;
	uint8_t base[32];
	uint8_t invert_0;
	uint8_t security; // was it interfering with dawn/dusk?
	uint32_t calibrate; // ADC Calibration
// PS END


	uint8_t mqtt_subscribe[4][32];
	uint8_t tcp_address[4];
	uint8_t tcp_gateway[4];
	uint8_t tcp_netmask[4];
	uint16_t checksum;
} SYSCFG;

typedef struct {
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

/*
extern char sta_ssid[64];
extern char sta_pass[64];
extern char mqtt_device_name[];
extern char mqtt_pass[];
extern char mqtt_user[];
extern char mqtt_client_id[];
extern char sta_user[];
extern char sta_pass[];
extern unsigned int mqtt_port;
extern char mqtt_host[];
*/
void ICACHE_FLASH_ATTR CFG_Save();
void ICACHE_FLASH_ATTR CFG_Load();

extern SYSCFG sysCfg;

#endif /* USER_CONFIG_H_ */
