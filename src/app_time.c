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

#include "app_time.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "os.h"

const uint16_t APP_TIME_MDAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const uint16_t APP_TIME_MDAYS_LEAP[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char * const APP_TIME_MONTH_NAMES[12] = {
	"Jan.",
	"Feb.",
	"Mar.",
	"Apr.",
	"May",
	"Jun.",
	"Jul.",
	"Aug.",
	"Sept.",
	"Oct.",
	"Nov.",
	"Dec.",
};

void app_time_localtime(uint64_t secs, int32_t offset, app_time_t *dest) {
	secs += offset;
	os_memset(dest, 0, sizeof(*dest));

	uint64_t days = secs / APP_TIME_SECS_DAY;
	secs %= APP_TIME_SECS_DAY;
	dest->time.hours = secs / APP_TIME_SECS_HOUR;
	secs %= APP_TIME_SECS_HOUR;
	dest->time.mins = secs / APP_TIME_SECS_MIN;
	secs %= APP_TIME_SECS_MIN;
	dest->time.secs = secs;

	uint64_t years = 1970 + (days / APP_TIME_DAYS_400YEARS * 400);
	days %= APP_TIME_DAYS_400YEARS;
	while (true) {
		bool is_leap = (years + 30) % 400 == 0;
		uint16_t inc = is_leap ? APP_TIME_DAYS_100YEARS + 1 : APP_TIME_DAYS_100YEARS;
		if (days >= inc) {
			days -= inc;
			years += 100;
		} else {
			break;
		}
	}
	while (true) {
		years += 2;
		bool is_leap = (years % 100 != 0) || (years % 400 == 0);
		years -= 2;
		uint16_t inc = is_leap ? APP_TIME_DAYS_4YEARS : APP_TIME_DAYS_4YEARS - 1;
		if (days >= inc) {
			days -= inc;
			years += 4;
		} else {
			break;
		}
	}
	bool is_leap;
	while (true) {
		is_leap = (years % 4 == 0) && ((years % 100 != 0) || (years % 400 == 0));
		uint16_t inc = is_leap ? 366 : 365;
		if (days >= inc) {
			days -= inc;
			years++;
		} else {
			break;
		}
	}
	dest->time.years = years;

	const uint16_t *mdays = is_leap ? APP_TIME_MDAYS_LEAP : APP_TIME_MDAYS;
	for (dest->time.months = 0;; dest->time.months++) {
		if (days < mdays[dest->time.months]) {
			dest->time.days = days;
			break;
		}
		days -= mdays[dest->time.months];
	}

	if (offset < 0) {
		dest->tz.sign = true;
		offset = -offset;
	}
	dest->tz.hours = offset / APP_TIME_SECS_HOUR;
	offset -= dest->tz.hours * APP_TIME_SECS_HOUR;
	dest->tz.mins = offset / APP_TIME_SECS_MIN;
	offset -= dest->tz.mins * APP_TIME_SECS_MIN;
	dest->tz.secs = offset;
}

uint8_t app_time_format_date(const app_time_t *time, char *buff, uint8_t cap) {
	// Note: the return value of snprintf appears to be incorrect
	snprintf(buff, cap, "%u %s %u", time->time.days + 1, (const char*) PIC(APP_TIME_MONTH_NAMES[time->time.months]),
			time->time.years);
	return strlen(buff);
}

uint8_t app_time_format_time(const app_time_t *time, char *buff, uint8_t cap) {
	// Note: the return value of snprintf appears to be incorrect
	snprintf(buff, cap, "%02u:%02u:%02u", time->time.hours, time->time.mins, time->time.secs);
	return strlen(buff);
}

uint8_t app_time_format_timezone(const app_time_t *time, char *buff, uint8_t cap) {
	// Note: the return value of snprintf appears to be incorrect
	snprintf(buff, cap, "UTC%s%02u:%02u", time->tz.sign ? "-" : "+", time->tz.hours, time->tz.mins);
	return strlen(buff);
}
