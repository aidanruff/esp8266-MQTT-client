/*
Some random cgi routines. Used in the LED example and the page that returns the entire
flash as a binary. Also handles the hit counter on the main page.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd.h"
#include "cgi.h"
#include <ip_addr.h>
#include "espmissingincludes.h"
#include <gpio.h>
#include <config.h>

#include "iodefs.h"


extern int enable_debug_messages; // Needed by debug.h
#include "debug.h"

typedef struct {
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
	uint8_t Wday;   // day of week, sunday is day 1
	uint8_t Day;
	uint8_t Month;
	uint16_t Year;
	unsigned long Valid;
} tm_t;

extern tm_t tm;

extern int temperature, humidity, analog;

static char currLedState=0;
static char currRelayState=0;
static char web_pass[32] = "";


//Cgi that turns the LED on or off according to the 'led' param in the POST data
int ICACHE_FLASH_ATTR cgiLed(HttpdConnData *connData)
{
int len;
char buff[1024];

if (connData->conn==NULL)
	{
	//Connection aborted. Clean up.
	return HTTPD_CGI_DONE;
	}

// Check the authorisation password to ensure that user is authorised to change the settings
httpdFindArg(connData->postBuff, "web_pass", buff, sizeof(buff));
os_strcpy(web_pass, buff); // Store the password that has been entered for the next time the page is sent

if (!strcmp(buff, sysCfg.web_pass)) // Password OK
	{
	// LED command
	len = httpdFindArg(connData->postBuff, "led", buff, sizeof(buff));
	if (len > 0)
		{
		currLedState=atoi(buff);
		digitalWrite(LED_GPIO_13, currLedState);
		}

	// Relay command
	len = httpdFindArg(connData->postBuff, "relay", buff, sizeof(buff));
	if (len > 0)
		{
		currRelayState=atoi(buff);
		digitalWrite(LED_GPIO_0, currRelayState);
		}

	}

httpdRedirect(connData, "led.tpl");
return HTTPD_CGI_DONE;
}



//Template code for the led page.
void ICACHE_FLASH_ATTR tplLed(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return;


	os_sprintf(buff, "Unknown");
	if (os_strcmp(token, "ledstate") == 0)
		{
		if (currLedState)
			{
			os_sprintf(buff, "on");
			}
		else
			{
			os_sprintf(buff, "off");
			}
		}

	if (os_strcmp(token, "relaystate") == 0)
		{
		if (currRelayState)
			{
			os_sprintf(buff, "on");
			}
		else
			{
			os_sprintf(buff, "off");
			}
		}

	// Various %% substitutions

	if (os_strcmp(token, "time") == 0)
		os_sprintf(buff, "%d:%d:%d", tm.Hour, tm.Minute, tm.Second);

	if (os_strcmp(token, "date") == 0)
		os_sprintf(buff, "%d/%d/%d", tm.Day, tm.Month, tm.Year);


	if (os_strcmp(token, "temperature") == 0)
		os_sprintf(buff, "%d", temperature);

	if (os_strcmp(token, "analog") == 0)
		os_sprintf(buff, "%d", analog);

	if (os_strcmp(token, "humidity") == 0)
		os_sprintf(buff, "%d", humidity);

	if (os_strcmp(token, "web_pass") == 0)
		os_sprintf(buff, web_pass);

	if (os_strcmp(token, "mqtt_id") == 0)
		os_sprintf(buff, sysCfg.base);


	httpdSend(connData, buff, -1);
}

static long hitCounter=0;

//Template code for the counter on the index page.
void ICACHE_FLASH_ATTR tplCounter(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return;

	if (os_strcmp(token, "counter")==0) {
		hitCounter++;
		os_sprintf(buff, "%ld", hitCounter);
	}
	httpdSend(connData, buff, -1);
}


//Cgi that reads the SPI flash. Assumes 512KByte flash.
int ICACHE_FLASH_ATTR cgiReadFlash(HttpdConnData *connData) {
	int *pos=(int *)&connData->cgiData;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	if (*pos==0) {
		INFO("Start flash download.\n");
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "application/bin");
		httpdEndHeaders(connData);
		*pos=0x40200000;
		return HTTPD_CGI_MORE;
	}
	//Send 1K of flash per call. We will get called again if we haven't sent 512K yet.
	espconn_sent(connData->conn, (uint8 *)(*pos), 1024);
	*pos+=1024;
	if (*pos>=0x40200000+(512*1024)) return HTTPD_CGI_DONE; else return HTTPD_CGI_MORE;
}

