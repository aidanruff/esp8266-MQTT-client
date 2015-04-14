#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
#define CFG_LOCATION	0x3C	/* Please don't change or if you know what you doing */

// Aidans webpage setup
#define USE_WIFI_MODE		STATIONAP_MODE
//#define WIFI_CLIENTSSID		"MyAP"
#define WIFI_CLIENTSSID		"kitchen"
#define WIFI_CLIENTPASSWORD	""
#define WIFI_AP_NAME		"ESP8266-Setup"
#define WIFI_AP_PASSWORD	""
#define STA_SSID "kitchen"
#define STA_PASS ""
#define STA_TYPE AUTH_WPA2_PSK
// Aidans webpage setup - END


#define MQTT_HOST			"192.168.2.12" //or "192.168.11.1"
#define MQTT_PORT			1883
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 // seconds

#define MQTT_CLIENT_ID		"ARPS_%08X"
#define MQTT_USER			"remote_units"
#define MQTT_PASS			"provence"

#define MQTT_BASE  "999"






// these variables added by ps
#define DEFAULT_DAWN  480 // 8am - minutes since midnight
#define DEFAULT_DUSK  1080 // 6pm - minutes since midnight
#define DEFAULT_PEAK   23  // temperature
#define DEFAULT_OFF_PEAK 19
#define DEFAULT_FROST  14
#define DEFAULT_ON_1  480  // 8am morning
#define DEFAULT_OFF_1  720 // 12 mid-day
#define DEFAULT_ON_2  900  // 3pm
#define DEFAULT_OFF_2 1380 // 11pm
#define DEFAULT_0_STATUS 0 // output 0=OFF, 1=ON, 2=ON TILL MIDNIGHT 3= ON TILL DAWN 4=HEAT
#define DEFAULT_4_STATUS 0
#define DEFAULT_5_STATUS 0
#define DEFAULT_12_STATUS 0
#define DEFAULT_13_STATUS 0
#define DEFAULT_14_IN 5
#define DEFAULT_SENSOR 0 // 0 for Dallas, 1 for DHT22
#define FIVE_ENABLE 0  // not an output - used to indicate clock working
#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/
#define DEFAULT_MANUAL 0   // manual override option off
#define DEFAULT_MANUAL_TIME 120 * 60 * 20   // 120 minutes before reverting to normal
#define DEFAULT_INVERT 1 // invert output for GPIO0  if 1 is inverted ie HIGH=OFF
// Default ADC calibration value
#define CALIBRATE 478

// #define CLIENT_SSL_ENABLE
#define DEFAULT_SECURITY	0
#define QUEUE_BUFFER_SIZE		 		2048

#define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311		/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#define OUT_ON 1
#define OUT_OFF 0


#endif
