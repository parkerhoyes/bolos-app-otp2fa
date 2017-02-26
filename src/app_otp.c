/*
 * License for the BOLOS OTP 2FA Application project, originally found here:
 * https://github.com/parkerhoyes/bolos-app-otp2fa
 *
 * Copyright (C) 2017 Parker Hoyes <contact@parkerhoyes.com>
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

#include "app_otp.h"

#include <stdint.h>

#include "app_hmac_sha1.h"

void app_otp_6digit(const unsigned char *key, uint8_t key_len, uint64_t counter, char dest[6]) {
	unsigned char text[8] = {
		counter >> 56,
		counter >> 48,
		counter >> 40,
		counter >> 32,
		counter >> 24,
		counter >> 16,
		counter >> 8,
		counter
	};
	unsigned char digest[20];
	app_hmac_sha1_hash(key, key_len, text, 8, digest);
	app_otp_extract_6digit(digest, dest);
}

void app_otp_extract_6digit(const unsigned char digest[20], char dest[6]) {
	uint8_t offset = digest[19] & 0x0F;
	uint32_t code = digest[offset++] & 0x7F;
	code <<= 8;
	code |= digest[offset++];
	code <<= 8;
	code |= digest[offset++];
	code <<= 8;
	code |= digest[offset];
	code %= 1000000;
	for (uint8_t i = 0; i < 6; i++) {
		dest[5 - i] = '0' + code % 10;
		code /= 10;
	}
}
