/*
 * petes_code.c
 *
 *  Created on: 16 Mar 2015
 *      Author: Aidan
 */

#include "aidan_and_petes.h"
#include "petes.h"







void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttConnectedCb(uint32_t *args) {
	char *baseBuf = (char*) os_zalloc(84);  // added ps
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Connected Device %s\r\n", sysCfg.base);
	MQTT_Subscribe(client, "toesp", 0);
	os_sprintf(baseBuf, "%s/toesp", sysCfg.base);
	MQTT_Subscribe(client, baseBuf, 0);
	os_sprintf(baseBuf, "%s/", sysCfg.base);
	MQTT_Publish(&mqttClient, "esplogon", baseBuf, strlen(baseBuf), 0, 0);
	os_free(baseBuf);
}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Published\r\n");
}



#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // API starts months from 1, this array starts from 0

void ICACHE_FLASH_ATTR convertTime() {
	// given myrtc as time in Linux format break into time components
	// this is a more compact version of the C library localtime function
	// note that internally the year is offset from 1970 - compensated at the end

	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;

	time = (uint32_t) myrtc;
	tm.Second = time % 60;
	time /= 60; // now it is minutes
	tm.Minute = time % 60;
	time /= 60; // now it is hours
	tm.Hour = time % 24;
	time /= 24; // now it is days
	tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

	year = 0;
	days = 0;
	while ((unsigned) (days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	tm.Year = year; // year is offset from 1970

	days -= LEAP_YEAR(year) ? 366 : 365;
	time -= days; // now it is days in this year, starting at 0

	days = 0;
	month = 0;
	monthLength = 0;
	for (month = 0; month < 12; month++) {
		if (month == 1) { // february
			if (LEAP_YEAR(year)) {
				monthLength = 29;
			}
			else {
				monthLength = 28;
			}
		}
		else {
			monthLength = monthDays[month];
		}

		if (time >= monthLength) {
			time -= monthLength;
		}
		else {
			break;
		}
	}
	tm.Month = month + 1;  // jan is month 1
	tm.Day = time + 1;     // day of month
	tm.Year = tm.Year + 1970;
}

/* end time */

#define MAXTIMINGS 10000
#define BREAKTIME 20
static void ICACHE_FLASH_ATTR readDHT() {
	int counter = 0;
	int laststate = 1;
	int i = 0;
	int j = 0;
	int checksum = 0;
	//int bitidx = 0;
	//int bits[250];

	int data[100];

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	GPIO_OUTPUT_SET(2, 1);
	os_delay_us(250000);
	GPIO_OUTPUT_SET(2, 0);
	os_delay_us(20000);
	GPIO_OUTPUT_SET(2, 1);
	os_delay_us(40);
	GPIO_DIS_OUTPUT(2);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);

	// wait for pin to drop?
	while (GPIO_INPUT_GET(2) == 1 && i < 100000) {
		os_delay_us(1);
		i++;
	}
	if (i == 100000) return;

	// read data!

	for (i = 0; i < MAXTIMINGS; i++) {
		counter = 0;
		while ( GPIO_INPUT_GET(2) == laststate) {
			counter++;
			os_delay_us(1);
			if (counter == 1000) break;
		}
		laststate = GPIO_INPUT_GET(2);
		if (counter == 1000) break;

		//bits[bitidx++] = counter;

		if ((i > 3) && (i % 2 == 0)) {
			// shove each bit into the storage bytes
			data[j / 8] <<= 1;
			if (counter > BREAKTIME) data[j / 8] |= 1;
			j++;
		}
	}

	float temp_p, hum_p;
	if (j >= 39) {
		checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
		if (data[4] == checksum) {
			/* yay! checksum is valid */

			hum_p = data[0] * 256 + data[1];
			hum_p /= 10;

			temp_p = (data[2] & 0x7F) * 256 + data[3];
			temp_p /= 10.0;
			if (data[2] & 0x80) temp_p *= -1;
			temperature = temp_p;
			humidity = hum_p;
		}
	}
}


/*
 * Adaptation of Paul Stoffregen's One wire library to the ESP8266 and
 * Necromant's Frankenstein firmware by Erland Lewin <erland@lewin.nu>
 *
 * Paul's original library site:
 *   http://www.pjrc.com/teensy/td_libs_OneWire.html
 *
 * See also http://playground.arduino.cc/Learning/OneWire
 *
 * Stripped down to bare minimum by Peter Scargill for single DS18B20 or DS18B20P integer read
 */

static int gpioPin;

void ICACHE_FLASH_ATTR ds_init(int gpio) {
	//set gpio2 as gpio pin
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); //disable pulldown
	PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO2_U); //enable pull up R
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U); // Configure the GPIO with internal pull-up -PIN_PULLUP_EN( gpio );
	GPIO_DIS_OUTPUT(gpio);
	gpioPin = gpio;
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return;

void ICACHE_FLASH_ATTR ds_reset(void) {
	uint8_t retries = 125;
	GPIO_DIS_OUTPUT(gpioPin);
	// wait until the wire is high... just in case
	do {
		if (--retries == 0) return;
		os_delay_us(2);
	} while (!GPIO_INPUT_GET(gpioPin));
	GPIO_OUTPUT_SET(gpioPin, 0);
	os_delay_us(480);
	GPIO_DIS_OUTPUT(gpioPin);
	os_delay_us(480);
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static inline void write_bit(int v) {
	// IO_REG_TYPE mask=bitmask;
	//	volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
	GPIO_OUTPUT_SET(gpioPin, 0);
	if (v) {
		// noInterrupts();
		//	DIRECT_WRITE_LOW(reg, mask);
		//	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
		os_delay_us(10);
		GPIO_OUTPUT_SET(gpioPin, 1);
		// DIRECT_WRITE_HIGH(reg, mask);	// drive output high
		// interrupts();
		os_delay_us(55);
	}
	else {
		// noInterrupts();
		//	DIRECT_WRITE_LOW(reg, mask);
		//	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
		os_delay_us(65);
		GPIO_OUTPUT_SET(gpioPin, 1);
		//	DIRECT_WRITE_HIGH(reg, mask);	// drive output high
		//		interrupts();
		os_delay_us(5);
	}
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static inline int read_bit(void) {
	//IO_REG_TYPE mask=bitmask;
	//volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
	int r;
	// noInterrupts();
	GPIO_OUTPUT_SET(gpioPin, 0);
	// DIRECT_MODE_OUTPUT(reg, mask);
	// DIRECT_WRITE_LOW(reg, mask);
	os_delay_us(3);
	GPIO_DIS_OUTPUT(gpioPin);
	// DIRECT_MODE_INPUT(reg, mask);	// let pin float, pull up will raise
	os_delay_us(10);
	// r = DIRECT_READ(reg, mask);
	r = GPIO_INPUT_GET(gpioPin);
	// interrupts();
	os_delay_us(53);
	return r;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
void ds_write(uint8_t v, int power) {
	uint8_t bitMask;
	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		write_bit((bitMask & v) ? 1 : 0);
	}
	if (!power) {
		// noInterrupts();
		GPIO_DIS_OUTPUT(gpioPin);
		GPIO_OUTPUT_SET(gpioPin, 0);
		// DIRECT_MODE_INPUT(baseReg, bitmask);
		// DIRECT_WRITE_LOW(baseReg, bitmask);
		// interrupts();
	}
}

//
// Read a byte
//
uint8_t ds_read() {
	uint8_t bitMask;
	uint8_t r = 0;
	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		if (read_bit()) r |= bitMask;
	}
	return r;
}

void ICACHE_FLASH_ATTR SEND_WS_12_0() {
	uint8_t time;
	time = 4;
	while (time--)
		WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 4, rainb12);
	time = 9;
	while (time--)
		WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 8, rainb12);
}

void ICACHE_FLASH_ATTR SEND_WS_12_1() {
	uint8_t time;
	time = 8;
	while (time--)
		WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 4, rainb12);
	time = 6;
	while (time--)
		WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 8, rainb12);
}

void ICACHE_FLASH_ATTR WS2812OutBuffer(uint8_t * buffer, uint16_t length, uint16_t repetition) {
	uint16_t i;
	os_intr_lock();

	for (i = 0; i < 4; i++)
		WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 8, rainb12);
	while (repetition--) {
		for (i = 0; i < length; i++) {
			uint8_t mask = 0x80;
			uint8_t byte = buffer[i];
			while (mask) {
				(byte & mask) ? SEND_WS_12_1() : SEND_WS_12_0();
				mask >>= 1;
			}
		}
	}
	os_intr_unlock();
}

void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len) {
	char topicBuf[topic_len + 1];
	char dataBuf[data_len + 1];
	char tBuf[84];
	char token[84];
	char stringvalue[84];
	uint8_t msg_type;

	MQTT_Client* client = (MQTT_Client*) args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	msg_type = 0;
	os_sprintf(tBuf, "%s/toesp", sysCfg.base);
	if (strcmp(topicBuf, tBuf) == 0) {
		os_sprintf(tBuf, "%s/fromesp", sysCfg.base);
		msg_type = 2;
	} // may need response
	if (strcmp(topicBuf, "toesp") == 0) msg_type = 1; // no response
	if (dataBuf[0] != '{') msg_type = 0;

	if (msg_type) {
		char *bufptr;
		char *tokenptr;
		char strvalue[128]; // could be passing a sizable message
		char token[128]; // don't need it this big but reused in EXT
		int32_t intvalue;
		int32_t arg1;
		int32_t arg2;
		int32_t arg3;
		int32_t arg4;
		int32_t arg5;
		int8_t arg_count;
		int8_t argtype;
		int8_t negative;
		uint8_t do_update;
		uint8_t is_query;
		uint8_t no_args;

		bufptr = dataBuf + 1;
		do_update = 0;
		while (*bufptr && (*bufptr != '}')) {
			strvalue[0] = 0;
			token[0] = 0;
			tokenptr = token;
			intvalue = 0;
			arg1 = 0;
			arg2 = 0;
			arg3 = 0;
			arg4 = 0;
			arg5 = 0;
			arg_count = 0;
			is_query = 0;
			no_args = 0;

			while (*bufptr && (*bufptr != ':') && (*bufptr != ';') && (*bufptr != '=') && (*bufptr != '}')) {
				*tokenptr++ = *bufptr++;
				*tokenptr = 0; // build up command
			}
			if ((*bufptr == ':') || (*bufptr == '=') || (*bufptr == ';') || (*bufptr == '}')) // otherwise we're screwed, just give up - no cmd
			{
				if (*(tokenptr - 1) == '?') {
					*(tokenptr - 1) = 0;
					is_query = 1;
				}
				if (*bufptr == ';') no_args = 1;
				else {
					if (*bufptr != '}') bufptr++;
					while (*bufptr == ' ')
						bufptr++; // skip whitespace
					if (*bufptr == '"') {
						argtype = 1;
						bufptr++;
					}
					else argtype = 0; // string or number
				}
				if (no_args == 0) {
					if (argtype) // build up a string?
					{
						tokenptr = strvalue; // re-use pointer
						while (*bufptr && (*bufptr != '"')) {
							*tokenptr++ = *bufptr++;
							*tokenptr = 0;
						}
						while ((*bufptr == '"') || (*bufptr == ' '))
							bufptr++;
					}
					else // or build up a number
					{
						while (1) {
							if (*bufptr == '-') {
								negative = 1;
								bufptr++;
							}
							else negative = 0; // if int check for neg int
							while ((*bufptr >= '0') && (*bufptr <= '9')) {
								intvalue *= 10;
								intvalue += (*bufptr++ - '0');
							}
							if (negative) intvalue = -intvalue; // at this point we have a number in intvalue
							while (*bufptr == ' ')
								bufptr++; // skip whitespace
							if (*bufptr == ',') {
								bufptr++;
								arg_count++;
								while (*bufptr == ' ')
									bufptr++; // skip whitespace
								switch (arg_count) {
								case 1:
									arg1 = intvalue;
									break;
								case 2:
									arg2 = intvalue;
									break;
								case 3:
									arg3 = intvalue;
									break;
								case 4:
									arg4 = intvalue;
									break;
								case 5:
									arg5 = intvalue;
									break;
								}
								intvalue = 0;
								negative = 0;
							}
							else break;

						} // end of while 1
						if (arg_count) switch (arg_count) {
						case 1:
							arg2 = intvalue;
							break;
						case 2:
							arg3 = intvalue;
							break;
						case 3:
							arg4 = intvalue;
							break;
						case 4:
							arg5 = intvalue;
							break;
						}

						else arg1 = intvalue;
						while (*bufptr == ' ')
							bufptr++; // skip whitespace
						if ((*bufptr != ';') && (*bufptr != '}')) intvalue = 0;
					}
				} // end of if check for no args
				  // got token, status (ie query or not) and (optionally) some kind of value

				if ((*bufptr == ';') || (*bufptr == '}')) {
					bufptr++;
					// here is where we look at TOKEN and take actions accordingly..

					if (strcmp(token, "ext") == 0) {
						char *gotat = strchr(strvalue, '~');
						if (gotat != NULL) {
							*gotat = 0; // separate the strings
							strcpy(token, strvalue);
							os_sprintf(&token[gotat - strvalue], "%lu", myrtc);
							gotat++;
							strcat(token, gotat);
							strcpy(strvalue, token);
						}
						os_printf("{%s}\r\n", strvalue);
					}
					else if (strcmp(token, "in_temp") == 0) temperature = intvalue;
					else if (strcmp(token, "temp_type") == 0) {
						if (sysCfg.sensor != intvalue) sysCfg.sensor = intvalue;
						do_update = 1;
					}
					else if (strcmp(token, "time") == 0) {
						tm.Valid = 86400;
						myrtc = intvalue;
					}
					else if (strcmp(token, "dawn") == 0) sysCfg.dawn = intvalue;
					else if (strcmp(token, "dusk") == 0) sysCfg.dusk = intvalue;
					else if (strcmp(token, "peak") == 0) {
						if (sysCfg.peak != intvalue) {
							sysCfg.peak = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "off_peak") == 0) {
						if (sysCfg.off_peak != intvalue) {
							sysCfg.off_peak = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "rainbow12") == 0) { // number of leds - and the number of repetitions
						rgb.rgbnum = arg1;
						rgb.rainbow = arg2;
					}
					else if (strcmp(token, "frost") == 0) {
						if (sysCfg.frost != intvalue) {
							sysCfg.frost = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "on_1") == 0) {
						if (sysCfg.on_1 != intvalue) {
							sysCfg.on_1 = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "off_1") == 0) {
						if (sysCfg.off_1 != intvalue) {
							sysCfg.off_1 = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "on_2") == 0) {
						if (sysCfg.on_2 != intvalue) {
							sysCfg.on_2 = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "off_2") == 0) {
						if (sysCfg.off_2 != intvalue) {
							sysCfg.off_2 = intvalue;
							do_update = 1;
						}
					}
					else if (strcmp(token, "enable5") == 0) {
						if (sysCfg.five_enable != intvalue) {
							sysCfg.five_enable = intvalue;
							do_update = 1;
						}
					}

					else if (strcmp(token, "invert") == 0) {
						intvalue&=1;
						if (sysCfg.invert_0 != intvalue) {
							sysCfg.invert_0 = intvalue;
							do_update = 1;
						}
					}

					else if (strcmp(token, "debug") == 0) {
						if (intvalue == 1)
						os_printf(
								"Debug1: Time: %02d:%02d:%02d %02d/%02d/%02d Time Code: %lu dusk: %02d:%02d dawn: %02d:%02d\r\n",
								tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year, myrtc, sysCfg.dusk / 60,
								sysCfg.dusk % 60, sysCfg.dawn / 60, sysCfg.dawn % 60);
						else if (intvalue == 2)
						os_printf(
								"Debug2: On1: %02d:%02d Off1: %02d:%02d On2: %02d:%02d Off2: %02d:%02d Peak: %dc Off-peak: %dc Frost %dc \r\n",
								sysCfg.on_1 / 60, sysCfg.on_1 % 60, sysCfg.off_1 / 60, sysCfg.off_1 % 60, sysCfg.on_2 / 60,
								sysCfg.on_2 % 60, sysCfg.off_2 / 60, sysCfg.off_2 % 60, sysCfg.peak, sysCfg.off_peak,
								sysCfg.frost);
					}
					else if (strcmp(token, "temperature") == 0) {
						if (is_query) {
							strcpy(token, tBuf);
							os_sprintf(strvalue, "%d", temperature);
							strcat(token, "/temperature");
							if (args == NULL)
							os_printf("Temperature=%d\r\n", temperature);
							if (msg_type == 2) MQTT_Publish(client, token, strvalue, strlen(strvalue), 0, 0);
						}
						else temperature = intvalue;
					}
					else if (strcmp(token, "humidity") == 0) {
						strcpy(token, tBuf);
						os_sprintf(strvalue, "%d", humidity);
						strcat(token, "/humidity");
						if (args == NULL)
						os_printf("Humidity=%d\r\n", humidity);
						if (msg_type == 2) MQTT_Publish(client, token, strvalue, strlen(strvalue), 0, 0);
					}
					else if (strcmp(token, "adc") == 0) {
						strcpy(token, tBuf);
						os_sprintf(strvalue, "%d", analog);
						strcat(token, "/adc");
						if (args == NULL)
						os_printf("ADC=%d\r\n", analog);
						if (msg_type == 2) MQTT_Publish(client, token, strvalue, strlen(strvalue), 0, 0);
					}

					else if (strcmp(token, "ssid") == 0) {
						strcpy(sysCfg.sta_ssid, strvalue);
						do_update = 1;
					}
					else if (strcmp(token, "pass") == 0) {
						strcpy(sysCfg.sta_pwd, strvalue);
						do_update = 1;
					}
					else if (strcmp(token, "mqtt_host") == 0) {
						strcpy(sysCfg.mqtt_host, strvalue);
						do_update = 1;
					}
					else if (strcmp(token, "mqtt_port") == 0) {
						sysCfg.mqtt_port = intvalue;
						do_update = 1;
					}
					else if (strcmp(token, "mqtt_user") == 0) {
						strcpy(sysCfg.mqtt_user, strvalue);
						do_update = 1;
					}
					else if (strcmp(token, "mqtt_pass") == 0) {
						strcpy(sysCfg.mqtt_pass, strvalue);
						do_update = 1;
					}

					else if (strcmp(token, "in") == 0) {
						strcpy(token, tBuf);
						os_sprintf(strvalue, "%d", in_value);
						strcat(token, "/in");
						if (args == NULL)
						os_printf("INPUT=%d\r\n", in_value);
						if (msg_type == 2) MQTT_Publish(client, token, strvalue, strlen(strvalue), 0, 0);
					}
					else if (strcmp(token, "in_count") == 0) {
						strcpy(token, tBuf);
						os_sprintf(strvalue, "%lu", in_count);
						strcat(token, "/in_count");
						if (args == NULL)
						os_printf("INPUT COUNT=%d\r\n", in_count);
						if (msg_type == 2) {
							MQTT_Publish(client, token, strvalue, strlen(strvalue), 0, 0);
							in_count = 0;
						}
					}
					else if (strcmp(token, "rgb12") == 0) {
						rgb.red = arg2;
						rgb.green = arg1;
						rgb.blue = arg3;
						rgb.rgbnum = arg4;
						rgb.rgbdelay = arg5;
						if (rgb.rgbnum == 0) rgb.rgbnum = 10;
						if (rgb.rgbdelay == 0) rgb.rgbdelay = 1;
					}

					else if (strcmp(token, "pwm") == 0) {
						pwm.bright = arg1;
						pwm.timeout = arg2;
						pwm.minimum = arg3;
					}

					else if (strcmp(token, "out1") == 0) {
						if (is_query) {
							strcpy(token, tBuf);
							strcat(token, "/out1");
							if (args == NULL)
							os_printf("OUT1=%d\r\n", sysCfg.out_0_status);
							if (msg_type == 2) MQTT_Publish(client, token, (sysCfg.out_0_status == 0) ? "OFF" : "ON",
									(sysCfg.out_0_status == 0) ? 3 : 2, 0, 0);
						}

						else {
							if (sysCfg.out_0_status != intvalue) {
								sysCfg.out_0_status = intvalue;
								do_update = 1;
							}
							if (sysCfg.out_0_status == 0) // turn off
							{
								GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));
							}
							else if (sysCfg.out_0_status == 1) {
								GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
							}
							// otherwise let timed event every minute handle things
						}
					}
					else if (strcmp(token, "out2") == 0) {
						if (is_query) {
							strcpy(token, tBuf);
							strcat(token, "/out2");
							if (args == NULL)
							os_printf("OUT2=%d\r\n", sysCfg.out_4_status);
							if (msg_type == 2) MQTT_Publish(client, token, (sysCfg.out_4_status == 0) ? "OFF" : "ON",
									(sysCfg.out_4_status == 0) ? 3 : 2, 0, 0);
						}
						else

						{
							if (sysCfg.out_4_status != intvalue) {
								sysCfg.out_4_status = intvalue;
								do_update = 1;
							} // only update if actual change
							GPIO_OUTPUT_SET(LED_GPIO_4, ((sysCfg.out_4_status==1) ? OUT_ON : OUT_OFF)); //
						}
					}

					else if (strcmp(token, "out3") == 0) {
						if (is_query) {
							strcpy(token, tBuf);
							strcat(token, "/out3");
							if (args == NULL)
							os_printf("OUT3=%d\r\n", sysCfg.out_5_status);
							if (msg_type == 2) MQTT_Publish(client, token, (sysCfg.out_5_status == 0) ? "OFF" : "ON",
									(sysCfg.out_5_status == 0) ? 3 : 2, 0, 0);
						}
						else {
							if (sysCfg.out_5_status != intvalue) {
								sysCfg.out_5_status = intvalue;
								do_update = 1;
							} // only update if actual change
							GPIO_OUTPUT_SET(LED_GPIO_5, ((sysCfg.out_5_status==1) ? OUT_ON : OUT_OFF)); //
						}
					}

					else if (strcmp(token, "out4") == 0) {
						if (is_query) {
							strcpy(token, tBuf);
							strcat(token, "/out4");
							if (args == NULL)
							os_printf("OUT4=%d\r\n", sysCfg.out_12_status);
							if (msg_type == 2) MQTT_Publish(client, token, (sysCfg.out_12_status == 0) ? "OFF" : "ON",
									(sysCfg.out_12_status == 0) ? 3 : 2, 0, 0);
						}
						else {
							if (sysCfg.out_12_status != intvalue) {
								sysCfg.out_12_status = intvalue;
								do_update = 1;
							} // only update if actual change
							GPIO_OUTPUT_SET(LED_GPIO_12, ((sysCfg.out_12_status==1) ? OUT_ON : OUT_OFF)); //
						}
					}

					else if ((strcmp(token, "out5") == 0) && (sysCfg.five_enable)) {
						if (is_query) {
							strcpy(token, tBuf);
							strcat(token, "/out5");
							if (args == NULL)
							os_printf("OUT5=%d\r\n", sysCfg.out_13_status);
							if (msg_type == 2) MQTT_Publish(client, token, (sysCfg.out_13_status == 0) ? "OFF" : "ON",
									(sysCfg.out_13_status == 0) ? 3 : 2, 0, 0);
						}
						else {
							if (sysCfg.out_13_status != intvalue) {
								sysCfg.out_13_status = intvalue;
								do_update = 1;
							} // only update if actual change
							GPIO_OUTPUT_SET(LED_GPIO_13, ((sysCfg.out_13_status==1) ? OUT_ON : OUT_OFF)); //
						}
					}

					else if (strcmp(token, "id") == 0) {
						if (is_query) os_printf("ID=%s\r\n", sysCfg.base);
						else {
							strcpy(sysCfg.base, strvalue);
							CFG_Save();
							system_restart();
						}
					}
					else if (strcmp(token, "reset") == 0) {
						CFG_Save();
						system_restart();
					}

					else if (strcmp(token, "in_bounce") == 0) {
						sysCfg.in_14_bounce = intvalue;
						do_update = 1;
					}
					else if (strcmp(token, "override") == 0) {
						sysCfg.override_0 = intvalue;
						if (intvalue == 0) GPIO_OUTPUT_SET(LED_GPIO_0, ((sysCfg.out_0_status==1) ? OUT_ON : OUT_OFF)); //restore
						do_update = 1;
					}
					else if (strcmp(token, "meminfo") == 0) {
						system_print_meminfo();
					}
					else if (strcmp(token, "override_time") == 0) {
						sysCfg.override_0_time = (uint32_t) intvalue * 60 * 20;
						do_update = 1;
					}
				    else if (strcmp(token, "voltage") == 0)
						{
						strcpy(token, tBuf);
						os_sprintf(strvalue, "%d.%02d", (analog*1000/sysCfg.calibrate)/100,(analog*1000/sysCfg.calibrate)%100);
						strcat(token, "/voltage");
						if (args == NULL)
						os_printf("Voltage=%d.%02d\r\n", (analog*1000/sysCfg.calibrate)/100,(analog*1000/sysCfg.calibrate)%100);

						if (msg_type == 2)
							MQTT_Publish(client, token, strvalue, strlen(strvalue), 0, 0);
						}
				    else if (strcmp(token,"calibrate") == 0)
						{
					    sysCfg.calibrate = intvalue;
					    do_update = 1;
					    }
					else os_printf("Eh?\r\n");

				}
			}
		} // loop through multiple messages before initiating any possible save
		if (do_update == 1) CFG_Save();
	}
}

static const uint8_t ledTable[256] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
		10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24,
		25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 40, 41, 42, 43, 44, 45, 46, 46,
		47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 67, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78,
		80, 81, 82, 83, 85, 86, 87, 89, 90, 91, 93, 94, 95, 97, 98, 99, 101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116,
		117, 119, 121, 122, 124, 125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151, 153, 155, 157,
		159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206,
		208, 210, 212, 215, 217, 219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255 };

static const uint8_t PWMTable[100] = { 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 9,
		10, 11, 11, 12, 13, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 30, 31, 33, 34, 36, 38, 40, 42, 44, 46,
		48, 51, 53, 56, 59, 62, 64, 68, 71, 74, 78, 81, 85, 89, 93, 97, 102, 107, 111, 116, 122, 127, 133, 139, 145, 151, 157,
		164, 171, 178, 186, 194, 202, 210, 218, 226, 234, 244, 250, 255 };

// Input a value 0 to 255 to get a colour value.
LOCAL void colourWheel(uint8_t WheelPos) {
	if (WheelPos < 85) {
		rgb.reda = (uint8_t) (WheelPos * 3);
		rgb.greena = (uint8_t) (255 - WheelPos * 3);
		rgb.bluea = 0;
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		rgb.reda = (uint8_t) (255 - WheelPos * 3);
		rgb.greena = 0;
		rgb.bluea = (uint8_t) (WheelPos * 3);

	}
	else {
		WheelPos -= 170;
		rgb.reda = 0;
		rgb.greena = (uint8_t) (WheelPos * 3);
		rgb.bluea = (uint8_t) (255 - WheelPos * 3);
	}
}

//*** added PS - not perfect but good for starters. Bounce programmable. This is 50ms period.
LOCAL void ICACHE_FLASH_ATTR bounce_cb(void *arg) {
	int8_t pinchanged;
	char tBuf[84];
	pinchanged = 0;

	if (gotser) {
		gotser = 0;
		mqttDataCb(NULL, "toesp", 5, serin_buf, strlen(serin_buf));
	}

	if (pwm.timeout) {
		if (--pwm.timeout == 0) pwm.bright = pwm.minimum;
	}
	if (pwm.actual != pwm.bright) {
		if (pwm.bright > 99) pwm.bright = 99;
		if (pwm.actual > pwm.bright) pwm.actual--;
		else if (pwm.actual < pwm.bright) pwm.actual++;
		pwm.channel = PWMTable[pwm.actual];
		pwm_set_duty(pwm.channel, 0);
		pwm_start();
	}

	if (in_bounce_count > sysCfg.in_14_bounce) {
		in_bounce_count = 0;
		in_value = GPIO_INPUT_GET(LED_GPIO_14);
		// send new value...

		os_sprintf(tBuf, "%s/fromesp/in1", sysCfg.base);
		if (in_value) MQTT_Publish(&mqttClient, tBuf, "1", 1, 0, 0);
		else MQTT_Publish(&mqttClient, tBuf, "0", 1, 0, 0);

		if (in_value == 0) {
			in_count++;
			pinchanged = -1;
		}
		else {
			pinchanged = 1;
		}
	}
	if (in_value != GPIO_INPUT_GET(LED_GPIO_14)) in_bounce_count++;
	else in_bounce_count = 0;


//Handle Serial LEDS on GPIO12

	if (rgb.rainbow) {

		rgb.rainbow--;
		if (rgb.rainbow == 0) {
			rgb.red = 0;
			rgb.green = 0;
			rgb.blue = 0;
			rgb.rgbdelay = 5;
		}
		else {
			colourWheel((uint8_t) (rgb.rainbow & 255));
			rgb.buffer[0] = rgb.reda;
			rgb.buffer[1] = rgb.greena;
			rgb.buffer[2] = rgb.bluea;
			WS2812OutBuffer(rgb.buffer, 3, rgb.rgbnum); // 3 leds in array, number of repetitions
		}
	}

	else
	/// rgb to fade from any colour to any other colour for any number of LEDS for any given period in secs
	if ((rgb.red != rgb.reda) || (rgb.green != rgb.greena) || (rgb.blue != rgb.bluea)) {

		if (rgb.reda < rgb.red) rgb.reda += ((rgb.red - rgb.reda) / (rgb.rgbdelay * 20)) + 1;
		if (rgb.greena < rgb.green) rgb.greena += ((rgb.green - rgb.greena) / (rgb.rgbdelay * 20)) + 1;
		if (rgb.bluea < rgb.blue) rgb.bluea += ((rgb.blue - rgb.bluea) / (rgb.rgbdelay * 20)) + 1;
		if (rgb.reda > rgb.red) rgb.reda -= ((rgb.reda - rgb.red) / (rgb.rgbdelay * 20)) + 1;
		if (rgb.greena > rgb.green) rgb.greena -= ((rgb.greena - rgb.green) / (rgb.rgbdelay * 20)) + 1;
		if (rgb.bluea > rgb.blue) rgb.bluea -= ((rgb.bluea - rgb.blue) / (rgb.rgbdelay * 20)) + 1;

		if (rgb.rgbnum == 0) {
			rgb.rgbnum = 1;
			rgb.reda = rgb.red;
			rgb.greena = rgb.green;
			rgb.bluea = rgb.blue;
		} // instant
		rgb.buffer[0] = ledTable[rgb.reda];
		rgb.buffer[1] = ledTable[rgb.greena];
		rgb.buffer[2] = ledTable[rgb.bluea];

		WS2812OutBuffer(rgb.buffer, 3, rgb.rgbnum); // 3 leds in array, number of repetitions
	}

	// now check for manual override on output 0
	if (sysCfg.override_0) {
		if (pinchanged == 1) {
			pinchange_downcounter = sysCfg.override_0_time;
			digitalWrite(0, 1);
		} // overwrite output 0 with button
		if (pinchanged == -1) {
			pinchange_downcounter = sysCfg.override_0_time;
			digitalWrite(0, 0);
		}
	}
	if (pinchange_downcounter) {
		if (--pinchange_downcounter == 0) GPIO_OUTPUT_SET(LED_GPIO_0, ((sysCfg.out_0_status==1) ? OUT_ON : OUT_OFF));
	}
}

//*** added PS
LOCAL void ICACHE_FLASH_ATTR temperature_cb(void *arg) {
	if (sysCfg.sensor == 1) readDHT();
	else if (sysCfg.sensor == 0) {
		ds_reset();
		ds_write(0xcc, 1);
		ds_write(0xbe, 1);
		temperature = (int) ds_read();
		temperature = temperature + (int) ds_read() * 256;
		temperature /= 16;
		if (temperature > 100) temperature -= 4096;
		ds_reset();
		ds_write(0xcc, 1);
		ds_write(0x44, 1);
	}
	if (got_ds_reading == 0) temperature = 20; // ignore first reading - second reading force average
	got_ds_reading = 1;

}

LOCAL void ICACHE_FLASH_ATTR rtc_cb(void *arg) {

	++myrtc;
	if (tm.Valid) {
		time_timeout = 0;
		tm.Valid--;
		if (tm.Valid == 0) {
			if (sysCfg.five_enable == 0) state13 = 0;
			GPIO_OUTPUT_SET(LED_GPIO_13, OUT_OFF);
		}
		else {
			if (sysCfg.five_enable == 0) {
				if (state13 == 1) {
					state13 = 0;
					GPIO_OUTPUT_SET(LED_GPIO_13, OUT_OFF);
				}
				else {
					state13 = 1;
					GPIO_OUTPUT_SET(LED_GPIO_13, OUT_ON);
				}
			}
		}
	}

	if (tm.Valid < 1000) // no valid time? Time is sent on powerup (mqttConnect) and also every 24 hours so unit has dawn dusk info
	{
		if (time_timeout++ == 120) // 2 after minutes of no time ask for it every 2 mins. Should never be needed
		{
			char tbuf[40];
			time_timeout = 0;
			os_sprintf(tbuf, "%s/", sysCfg.base);
			MQTT_Publish(&mqttClient, "esplogon", tbuf, strlen(tbuf), 0, 0);
		}
	}

	convertTime();
	analog = system_adc_read();

	if (tm.Second == 0) // **** timed once every minute handles sysCfg.out_status
	{

		// need temporary minutes in the day
		int t_mins, doit;
		t_mins = tm.Hour;
		t_mins *= 60;
		t_mins += tm.Minute;
		switch (sysCfg.out_0_status) {
		case 0:
			break; // covered elsewhere
		case 1:
			if (tm.Valid == 0)
			GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));
			break; // no time, turn OUT1 off after a day
		case 2:
			if (t_mins > sysCfg.dusk)
			GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
			else
			GPIO_OUTPUT_SET(LED_GPIO_0,(sysCfg.invert_0));
			break;
		case 3:
			if ((t_mins > sysCfg.dusk) || (t_mins < sysCfg.dawn))
			GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
			else
			GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));
			break;
		case 4:
			if ((t_mins > sysCfg.dawn) && (t_mins < sysCfg.dusk))
			GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
			else
			GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));
			break;
		case 5:
			doit = 0;
			if (sysCfg.on_1 > sysCfg.off_1) {
				if ((t_mins > sysCfg.on_1) || (t_mins < sysCfg.off_1)) doit = 1;
			}
			else {
				if ((t_mins > sysCfg.on_1) && (t_mins < sysCfg.off_1)) doit = 1;
			}
			if (sysCfg.on_1 > sysCfg.off_2) {
				if ((t_mins > sysCfg.on_2) || (t_mins < sysCfg.off_2)) doit = 1;
			}
			else {
				if ((t_mins > sysCfg.on_2) && (t_mins < sysCfg.off_2)) doit = 1;
			}

			if (doit) {
				if (temperature < sysCfg.peak)
				GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
				else
				GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));
			}
			else // if not peak - OR if the time is not set due to bad connection, say  -default to off-peak
			{
				if (temperature < sysCfg.off_peak)
				GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
				else
				GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));

			}
			break;
		default:
			if ((tm.Minute == 0) && (tm.Hour == 0)) {
				sysCfg.out_0_status--;
				CFG_Save();
			}
			// frost setting if over 5
			if (sysCfg.out_0_status > 5) {
				if (temperature < sysCfg.frost)
				GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0^1));
				else
				GPIO_OUTPUT_SET(LED_GPIO_0, (sysCfg.invert_0));
			}
			break;
		}
	}
}



void ICACHE_FLASH_ATTR mqtt_init()
{
// Set up a timer to read the temperature
// os_timer_disarm(ETSTimer *ptimer)
os_timer_disarm(&temperature_timer);
// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
os_timer_setfn(&temperature_timer, (os_timer_func_t *) temperature_cb,
		(void *) 0);
// void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
os_timer_arm(&temperature_timer, DELAY, 1);

os_timer_disarm(&rtc_timer);
os_timer_setfn(&rtc_timer, (os_timer_func_t *) rtc_cb, (void *) 0);
os_timer_arm(&rtc_timer, 1000, 1);

os_timer_disarm(&bounce_timer);
os_timer_setfn(&bounce_timer, (os_timer_func_t *) bounce_cb, (void *) 0);
os_timer_arm(&bounce_timer, 50, 1);
// end ps added

MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port,
		sysCfg.security);
//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user,
		sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
MQTT_OnConnected(&mqttClient, mqttConnectedCb);
MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
MQTT_OnPublished(&mqttClient, mqttPublishedCb);
MQTT_OnData(&mqttClient, mqttDataCb);


INFO("\r\nMQTT OK\r\n");

ds_init(2); // one off for the DS18B20
os_sprintf(serin_buf, "%s", sysCfg.base);
MQTT_Publish(&mqttClient, "esplogon", serin_buf, strlen(serin_buf), 0, 0);
}


// *** end added PS



void ICACHE_FLASH_ATTR petes_initialisation(void)
{

//uart_init(BIT_RATE_115200, BIT_RATE_115200);
//Enable TxD pin
PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

//Set baud rate and other serial parameters to 115200,n,8,1
uart_div_modify(0, UART_CLK_FREQ/BIT_RATE_115200);
WRITE_PERI_REG(UART_CONF0(0), (STICK_PARITY_DIS)|(ONE_STOP_BIT << UART_STOP_BIT_NUM_S)| \
			(EIGHT_BITS << UART_BIT_NUM_S));

//Reset tx & rx fifo
SET_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST|UART_TXFIFO_RST);
CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST|UART_TXFIFO_RST);
//Clear pending interrupts
WRITE_PERI_REG(UART_INT_CLR(0), 0xffff);


os_delay_us(1000000);
tm.Valid = 0; //ps time is invalid

INFO("\r\nLoading Config\r\n");
CFG_Load();
INFO("\r\nConfig loaded\r\n");
//system_set_os_print(enable_debug_messages);

rainb12=1<<12;

rgb.reda = 0;
rgb.greena = 0;
rgb.bluea = 0;
rgb.red = 0;
rgb.green = 0;
rgb.blue = 0;
rgb.rainbow=0;

pinchange_downcounter=0;

// ps added
PIN_FUNC_SELECT(LED_GPIO_MUX, LED_GPIO_FUNC_0);
PIN_FUNC_SELECT(LED_GPIO_MUX_4, LED_GPIO_FUNC_4);
PIN_FUNC_SELECT(LED_GPIO_MUX_5, LED_GPIO_FUNC_5);
PIN_FUNC_SELECT(LED_GPIO_MUX_12, LED_GPIO_FUNC_12);
PIN_FUNC_SELECT(LED_GPIO_MUX_13, LED_GPIO_FUNC_13);
PIN_FUNC_SELECT(LED_GPIO_MUX_14, LED_GPIO_FUNC_14);

// PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
// PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);

in_bounce_count = 0;

//if (sysCfg.out_0_status == 1)
//	GPIO_OUTPUT_SET(LED_GPIO_0, OUT_ON); // otherwise let timed event handle it
//else
	GPIO_OUTPUT_SET(LED_GPIO_0, OUT_OFF); // but start with off to ensure port set properly!!

//if (sysCfg.out_4_status == 1)
//	GPIO_OUTPUT_SET(LED_GPIO_4, OUT_ON); // on or off by default
//else
//	GPIO_OUTPUT_SET(LED_GPIO_4, OUT_OFF);

//if (sysCfg.out_5_status == 1)
//	GPIO_OUTPUT_SET(LED_GPIO_5, OUT_ON); // on or off by default
//else
//	GPIO_OUTPUT_SET(LED_GPIO_5, OUT_OFF);

if (sysCfg.out_12_status == 1)
	GPIO_OUTPUT_SET(LED_GPIO_12, OUT_ON); // on or off by default RGB?
else
	GPIO_OUTPUT_SET(LED_GPIO_12, OUT_OFF);

if (sysCfg.out_13_status == 1)
	GPIO_OUTPUT_SET(LED_GPIO_13, OUT_ON); // on or off by default
else
	GPIO_OUTPUT_SET(LED_GPIO_13, OUT_OFF);

// try to set 14 as an input
PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);//disable pulldown
PIN_PULLDWN_DIS(PERIPHS_IO_MUX_MTMS_U); //enable pull up R
//PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);// Configure the GPIO with internal pull-up -PIN_PULLUP_EN( gpio );
GPIO_DIS_OUTPUT(LED_GPIO_14);

//Aidan Added
// Enable pull down on Relay output GPIO0
PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); // Relay
PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO0_U); // Enable pull-down
PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U); // Disable pull-up

// Set 5 as an input - pushbutton
PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO5);//disable pulldown
//PIN_PULLDWN_DIS(PERIPHS_IO_MUX_MTMS_U); //enable pull up R
//PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);// Configure the GPIO with internal pull-up -PIN_PULLUP_EN( gpio );
GPIO_DIS_OUTPUT(LED_GPIO_5);


// Set up a timer to read the temperature
os_timer_disarm(&temperature_timer);
os_timer_setfn(&temperature_timer, (os_timer_func_t *) temperature_cb, (void *) 0);
os_timer_arm(&temperature_timer, DELAY, 1);

// Set up a timer for real time clock once per second
os_timer_disarm(&rtc_timer);
os_timer_setfn(&rtc_timer, (os_timer_func_t *) rtc_cb, (void *) 0);
os_timer_arm(&rtc_timer, 1000, 1);

// Set up a debounce timer 20 times a second
os_timer_disarm(&bounce_timer);
os_timer_setfn(&bounce_timer, (os_timer_func_t *) bounce_cb, (void *) 0);
os_timer_arm(&bounce_timer, 50, 1);

pwm.channel = 0;
pwm.frequency = 500;
pwm_init(pwm.frequency, &pwm.channel);
pwm_set_duty(pwm.channel, 0);
pwm_start();


}







