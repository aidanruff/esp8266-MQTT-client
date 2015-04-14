/* main.c
 *
 * Copyright (c) 2014-2015, Aidan Ruff (aidan@ruffs.org) & Peter Scargill (pete@scargill.net)
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

int enable_debug_messages = 1; // Needed by debug.h


#include "espmissingincludes.h"
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"


// Aidans Additions

#include "iodefs.h"

#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "auth.h"

#include "aidan_and_petes.h"
/*
uint8_t ICACHE_RODATA_ATTR ledTable2[] = \
		"=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n\
		=========================Hello there folks==================================\r\n";
*/

void ICACHE_FLASH_ATTR user_init(void)
{
// ================================================================================================

petes_initialisation(); // Sets up the ports and initialises various values needed by the MQTT call backs, time etc

// Quick flash to say the board is alive
digitalWrite(LED_GPIO_0, 0); // Relay off
for (int i = 0; i < 3; i++) { os_delay_us(200000); digitalWrite(LED_GPIO_13, 1); os_delay_us(200000); digitalWrite(LED_GPIO_13, 0);}
//INFO(ledTable2);
// ================================================================================================
setupwebpage_init(); // Set up the configuration/control web pages

}








