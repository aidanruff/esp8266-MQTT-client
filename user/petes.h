/*
 * petes.h
 *
 *  Created on: 16 Mar 2015
 *      Author: Aidan
 */

#ifndef USER_PETES_H_
#define USER_PETES_H_


static uint32_t rainb12;

uint8_t gotser = 0;
uint8_t ser_temp_inb = 0;
char t_serin_buf[128];
char serin_buf[128];

typedef struct{
	uint8_t reda;
	uint8_t greena;
	uint8_t bluea;

	uint8_t red;
	uint8_t green;
	uint8_t blue;

	uint32_t rainbow;
	uint16_t rgbnum;
	uint16_t rgbdelay;
	uint8_t  buffer[900];
} LEDS;

LEDS rgb;

typedef struct {
	uint8_t channel;
	uint16_t frequency;
	uint8_t actual;
	uint8_t bright;
	uint32_t timeout;
	uint8_t minimum;
} PWM;
PWM pwm;

uint32_t time_timeout;

uint32_t pinchange_downcounter;


#define DELAY 10000 /* milliseconds for temperature 10 second intervals */

LOCAL os_timer_t temperature_timer;
LOCAL os_timer_t rtc_timer;
LOCAL os_timer_t bounce_timer;

int temperature, humidity, analog;

uint8_t in_bounce_count;
uint8_t in_value;
uint32_t in_count = 0;

uint8_t state13 = 0;

unsigned long myrtc = 0;

uint8_t got_ds_reading = 0;

/************************* Bits of the Arduino time library with bits added ******/
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

tm_t tm;



#endif /* USER_PETES_H_ */
