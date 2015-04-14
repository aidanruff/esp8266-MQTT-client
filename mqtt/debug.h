/*

 * debug.h
 *
 *  Created on: Dec 4, 2014
 *      Author: Minh
 */


extern int enable_debug_messages; // Needed by debug.h

#ifndef USER_DEBUG_H_
	#define USER_DEBUG_H_

	//#define INFO os_printf
	#define INFO if (enable_debug_messages) os_printf
#endif /* USER_DEBUG_H_ */

