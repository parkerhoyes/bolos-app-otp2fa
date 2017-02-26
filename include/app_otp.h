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

#include <stdint.h>

/*
 * Generate a 6-digit OTP number using the HMAC-SHA-1 algorithm with the specified key and counter value. To generate a
 * TOTP value, the message should be the number of 30-second periods elapsed since the Unix epoch. Alternatively, to
 * generate an HOTP value, the message should be the counter that is incremented with each new code.
 *
 * Args:
 *     key: the byte buffer containing the key, big-endian
 *     key_len: the number of bytes in key; must be <= 64
 *     counter: the counter
 *     dest: the buffer in which to store the resulting 6-digit number, encoded as an ASCII string with no null
 *           terminator
 */
void app_otp_6digit(const unsigned char *key, uint8_t key_len, uint64_t counter, char dest[6]);

/*
 * Generate a 6-digit OTP number using the specified 160-bit hash (which may be calculated using
 * app_hmac_sha1_hash(...)).
 *
 * Args:
 *     digest: the 160-bit HMAC hash, big-endian
 *     dest: the buffer in which to store the resulting 6-digit number, encoded as an ASCII string with no null
 *           terminator
 */
void app_otp_extract_6digit(const unsigned char digest[20], char dest[6]);
