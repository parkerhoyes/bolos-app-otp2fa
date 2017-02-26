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

#include "app_hmac_sha1.h"

#include <stdint.h>

#include "os.h"

#include "app_sha1.h"

void app_hmac_sha1_hash(const unsigned char *key, uint8_t key_len, const unsigned char *text, uint32_t text_len,
		unsigned char dest[20]) {
	unsigned char buffer[64];
	os_memcpy(buffer, key, key_len);
	if (key_len != 64)
		os_memset(&buffer[key_len], 0, 64 - key_len);
	for (uint8_t i = 0; i < 64; i++)
		buffer[i] ^= 0x36;
	app_sha1_ctx_t ctx;
	app_sha1_ctx_init(&ctx);
	app_sha1_ctx_update(&ctx, buffer, 64);
	app_sha1_ctx_update(&ctx, text, text_len);
	for (uint8_t i = 0; i < 64; i++)
		buffer[i] ^= (0x36 ^ 0x5C); // This will effectively "undo" the XOR with ipad, then XOR with opad
	unsigned char digest[20];
	app_sha1_ctx_hash(&ctx, digest);
	app_sha1_ctx_init(&ctx);
	app_sha1_ctx_update(&ctx, buffer, 64);
	app_sha1_ctx_update(&ctx, digest, 20);
	app_sha1_ctx_hash(&ctx, dest);
}
