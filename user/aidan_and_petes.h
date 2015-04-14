/*
 * aidan_and_petes.h
 *
 *  Created on: 16 Mar 2015
 *      Author: Aidan
 */

#ifndef USER_AIDAN_AND_PETES_H_
#define USER_AIDAN_AND_PETES_H_

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



MQTT_Client mqttClient; // Main MQTT client



#endif /* USER_AIDAN_AND_PETES_H_ */
