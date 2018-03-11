/*
 * License for the BOLOS OTP 2FA Application project, originally found here:
 * https://github.com/parkerhoyes/bolos-app-otp2fa
 *
 * Copyright (C) 2018 Parker Hoyes <contact@parkerhoyes.com>
 *
 * This software is provided "as-is", without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 *    that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef APP_TIME_H_
#define APP_TIME_H_

#include <stdbool.h>
#include <stdint.h>

#define APP_TIME_SECS_MIN (60)
#define APP_TIME_SECS_HOUR (APP_TIME_SECS_MIN * 60)
#define APP_TIME_SECS_DAY (APP_TIME_SECS_HOUR * 24)
#define APP_TIME_SECS_YEAR (APP_TIME_SECS_DAY * 365)
#define APP_TIME_DAYS_4YEARS (365 * 4 + 1)
#define APP_TIME_DAYS_100YEARS (APP_TIME_DAYS_4YEARS * 25 - 1)
#define APP_TIME_DAYS_400YEARS (APP_TIME_DAYS_100YEARS * 4 + 1)

extern const uint16_t APP_TIME_MDAYS[12];
extern const uint16_t APP_TIME_MDAYS_LEAP[12];
extern const char * const APP_TIME_MONTH_NAMES[12];

// Examples in brackets are for 1970-01-01T00:00Z
typedef struct {
	// Current time in the local timezone
	struct {
		// Year component (1970)
		uint16_t years;
		// Months since January (0)
		uint8_t months;
		// Days since first day of the month (0)
		uint8_t days;
		// Hours component of the time (0)
		uint8_t hours;
		// Minutes component of the time (0)
		uint8_t mins;
		// Seconds component of the time (0)
		uint8_t secs;
	} time;
	// Offset that must be applied to UTC time to get the time in the local timezone
	struct {
		// true is negative, false is non-negative
		bool sign;
		// Magnitude of the hours component of the timezone offset from UTC (0)
		uint8_t hours;
		// Magnitude of the minutes component of the timezone offset from UTC (0)
		uint8_t mins;
		// Magnitude of the seconds component of the timezone offset from UTC (0)
		uint8_t secs;
	} tz;
} app_time_t;

/*
 * Store the time components for the specified time in dest. secs + offset must be non-negative.
 *
 * Args:
 *     secs: the UNIX timestamp
 *     offset: the number of seconds that must be added to the UTC time to get the time in the local timezone; must be
 *             in [-86400, 86400]
 */
void app_time_localtime(uint64_t secs, int32_t offset, app_time_t *dest);

/*
 * Create a human-readable date string using the specified time data (timezone is ignored). A null-terminator is written
 * to the end of the string.
 *
 * Args:
 *     time: the time
 *     buff: the buffer in which to store the resulting string with a capacity of at least cap bytes
 *     cap: the capacity of buff, in bytes; a value of at least 14 is recommended; must be >= 1
 * Returns:
 *     the number of characters written to buff, not counting the terminating null character
 */
uint8_t app_time_format_date(const app_time_t *time, char *buff, uint8_t cap);

/*
 * Create a human-readable timestamp using the specified time data (timezone is ignored). A null-terminator is written
 * to the end of the string.
 *
 * Args:
 *     time: the time
 *     buff: the buffer in which to store the resulting string with a capacity of at least cap bytes
 *     cap: the capacity of buff, in bytes; a value of at least 9 is recommended; must be >= 1
 * Returns:
 *     the number of characters written to buff, not counting the terminating null character
 */
uint8_t app_time_format_time(const app_time_t *time, char *buff, uint8_t cap);

/*
 * Create a human-readable representation of the timezone stored in the specified time data (seconds component is not
 * included). A null-terminator is written to the end of the string.
 *
 * Args:
 *     time: the time
 *     buff: a buffer in which to store the resulting string with a capacity of at least cap bytes
 *     cap: the capacity of buff, in bytes; a value of at least 10 is recommended; must be >= 1
 * Returns:
 *     the number of characters written to buff, not counting the terminating null character
 */
uint8_t app_time_format_timezone(const app_time_t *time, char *buff, uint8_t cap);

#endif
