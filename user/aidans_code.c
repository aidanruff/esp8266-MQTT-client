/*
 * aidans_code.c
 *
 *  Created on: 16 Mar 2015
 *      Author: Aidan
 */



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
#include "aidans.h"


/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/


void ICACHE_FLASH_ATTR mqtt_init();


static void ICACHE_FLASH_ATTR MQTTLogonTimerCb(void *arg)
{
static int resetCnt=0;

if (allow_mqtt_init)
	{
	if (!logged_on_to_network)
		{
		int x=wifi_station_get_connect_status();
		if (x==STATION_GOT_IP)
			{
			logged_on_to_network = 1;
			if (!logged_on_to_mqtt)
				{
				INFO("\n\rNETWRK OK\n");
				mqtt_init();
				MQTT_Connect(&mqttClient);
				INFO("MQTT OK\r\n");
				logged_on_to_mqtt = 1;
				}
			}
		else
			{
			if (logged_on_to_mqtt) // Disconnect and reconnect later
				{
				MQTT_Disconnect(&mqttClient);
				INFO("Left MQTT\r\n");
				logged_on_to_mqtt = 0;
				}
			}

		}
	}
}


void setupwebpage_init(void)
{
int reset_count = 0;

while (!GPIO_INPUT_GET(LED_GPIO_4)) // Check to see if web page setup button is held low on powerup
	{
	INFO("*");
	++reset_count;
	os_delay_us(500000); // Half second tick
	if (reset_count > 6)
		{
		for (int i = 0; i < 5; i++) { os_delay_us(100000); digitalWrite(LED_GPIO_13, 1); os_delay_us(100000); digitalWrite(LED_GPIO_13, 0);}
		break;
		}
	}


if (reset_count < 6) // OK button not held down at power up, setup MQTT etc
	{

	wifiInit(STATION_MODE); // Only connect to the local network

	if (sysCfg.enable_webpage_control)
		{
		INFO("Setting up for normal MQTT Operation\r");
		builtInUrls[0].cgiArg = "/led.tpl";
		httpdInit(builtInUrls, 80);
		}

	INFO("ESP8266 in STA mode configured.\r\n");

	// Setup a timer to initialise the mqtt system 5 seconds after a wifi connection is established
	os_timer_disarm(&MQTTlogontimer);
	os_timer_setfn(&MQTTlogontimer, MQTTLogonTimerCb, NULL);
	os_timer_arm(&MQTTlogontimer, 5000, 1);
	allow_mqtt_init = 1; // Allow the MQTT system to connect
	}
else
	{
	//struct softap_config apConfig;
	INFO("Setting up for Web Page Configuration\r");

	httpdInit(builtInUrls, 80);
	wifiInit(STATIONAP_MODE); // Connect to the local network

	//wifi_softap_get_config(&apConfig);
	}

}


// Aidans END


