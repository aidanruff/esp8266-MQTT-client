/*
/* config.c
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
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "mqtt.h"
#include "config.h"
#include "user_config.h"

extern int enable_debug_messages; // Needed by debug.h
#include "debug.h"

SYSCFG sysCfg;
SAVE_FLAG saveFlag;

void ICACHE_FLASH_ATTR
CFG_Save()
{
	 spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	                   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));

	if (saveFlag.flag == 0) {
		spi_flash_erase_sector(CFG_LOCATION + 1);
		spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 1;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	} else {
		spi_flash_erase_sector(CFG_LOCATION + 0);
		spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 0;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	}
}

void ICACHE_FLASH_ATTR
CFG_Load()
{

	INFO("\r\nload ...\r\n");
	spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
				   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	if (saveFlag.flag == 0) {
		spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	} else {
		spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	}
	if(sysCfg.cfg_holder != CFG_HOLDER){
		os_memset(&sysCfg, 0x00, sizeof sysCfg);


		sysCfg.cfg_holder = CFG_HOLDER;


		os_sprintf(sysCfg.sta_ssid, "%s", WIFI_CLIENTSSID);
		os_sprintf(sysCfg.sta_pwd, "%s", WIFI_CLIENTPASSWORD);
		sysCfg.sta_type = STA_TYPE;

		os_sprintf(sysCfg.device_id, MQTT_CLIENT_ID, system_get_chip_id());
		os_sprintf(sysCfg.mqtt_host, "%s", MQTT_HOST);
		sysCfg.mqtt_port = MQTT_PORT;
		os_sprintf(sysCfg.mqtt_user, "%s", MQTT_USER);
		os_sprintf(sysCfg.mqtt_pass, "%s", MQTT_PASS);
	    os_sprintf(sysCfg.base, "%s", MQTT_BASE);

	    // Aidans
		os_sprintf(sysCfg.web_user, "admin"); // Default web password
		os_sprintf(sysCfg.web_pass, "password"); // Default web password
		sysCfg.enable_webpage_control = 1; // Enable webpage control
		// END Aidans

		sysCfg.security = DEFAULT_SECURITY;	/* default non ssl */

		sysCfg.mqtt_keepalive = MQTT_KEEPALIVE;

		sysCfg.dawn=DEFAULT_DAWN;
		sysCfg.dusk=DEFAULT_DUSK;
		sysCfg.peak=DEFAULT_PEAK;
		sysCfg.off_peak=DEFAULT_OFF_PEAK;
		sysCfg.frost=DEFAULT_FROST;
		sysCfg.on_1=DEFAULT_ON_1;
		sysCfg.off_1=DEFAULT_OFF_1;
		sysCfg.on_2=DEFAULT_ON_2;
		sysCfg.off_2=DEFAULT_OFF_2;
		sysCfg.out_0_status=DEFAULT_0_STATUS;
		sysCfg.out_4_status=DEFAULT_4_STATUS;
		sysCfg.out_5_status=DEFAULT_5_STATUS;
		sysCfg.out_12_status=DEFAULT_12_STATUS;
		sysCfg.out_13_status=DEFAULT_13_STATUS;
		sysCfg.in_14_bounce=DEFAULT_14_IN;
		sysCfg.sensor=DEFAULT_SENSOR;
		sysCfg.five_enable=FIVE_ENABLE;
        sysCfg.override_0=DEFAULT_MANUAL;
        sysCfg.override_0_time=DEFAULT_MANUAL_TIME;
        sysCfg.invert_0=DEFAULT_INVERT;
        sysCfg.calibrate = CALIBRATE;

		// AIDANS
		os_sprintf(sysCfg.web_user, "admin"); // Default web password
		os_sprintf(sysCfg.web_pass, "password"); // Default web password
		sysCfg.enable_webpage_control = 1; // Enable webpage control
		// aidan end
		
		INFO(" default configuration\r\n");

		CFG_Save();
	}

}
