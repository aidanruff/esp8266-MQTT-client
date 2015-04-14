/*
 * aidans.h
 *
 *  Created on: 16 Mar 2015
 *      Author: Aidan
 */

#ifndef USER_AIDANS_H_
#define USER_AIDANS_H_

extern int enable_debug_messages; // Needed by debug.h
uint8_t allow_mqtt_init; // Required to control MQTT logon
uint8_t logged_on_to_network = 0; // is wifi connected?
uint8_t logged_on_to_mqtt = 0; // is mqtt connected?

static ETSTimer MQTTlogontimer;
uint8_t allow_mqtt_init = 0; // Required to control MQTT logon

HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/index.tpl"},
	{"/flash.bin", cgiReadFlash, NULL},
	{"/led.tpl", cgiEspFsTemplate, tplLed},
	{"/index.tpl", cgiEspFsTemplate, tplCounter},
	{"/led.cgi", cgiLed, NULL},

	//Routines to make the /wifi URL and everything beneath it work.

//Enable the line below to protect the WiFi configuration with an username/password combo.
//	{"/wifi/*", authBasic, myPassFn},

	{"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/wifi/connect.cgi", cgiWiFiConnect, NULL},
	{"/wifi/setmode.cgi", cgiWifiSetMode, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};



#endif /* USER_AIDANS_H_ */
