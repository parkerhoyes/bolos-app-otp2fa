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

#ifndef APP_SHA1_H_
#define APP_SHA1_H_

#include <stdint.h>

typedef struct {
	uint32_t digest[5];
	unsigned char buffer[64];
	uint8_t buffer_size;
	uint64_t transforms;
} app_sha1_ctx_t;

/*
 * Initialize an SHA-1 hash context.
 *
 * Args:
 *     ctx: the context to be initialized
 */
void app_sha1_ctx_init(app_sha1_ctx_t *ctx);

/*
 * Update an already initialized SHA-1 hash context with more data to be appended to the message.
 *
 * Args:
 *     ctx: the SHA-1 hash context
 *     data: the string of bytes to be appended to the message; if len is 0, this need not be a valid pointer
 *     len: the number of bytes in data
 */
void app_sha1_ctx_update(app_sha1_ctx_t *ctx, const unsigned char *data, uint32_t len);

/*
 * Perform one iteration of the SHA-1 hash function on the block of data in the specified hash context. If the context's
 * buffer is not full, the remaining space will be filled with zero bytes. ctx->buffer_size is set to 0, but the
 * original contents of the buffer are left intact.
 *
 * Args:
 *     ctx: the SHA-1 hash context
 */
void app_sha1_ctx_iterate(app_sha1_ctx_t *ctx);

/*
 * Calculate the hash of the message for the specified initialized SHA-1 hash context. This method pollutes the hash
 * context, which must be initialized again before being reused.
 *
 * Args:
 *     ctx: the SHA-1 hash context
 *     digest_dest: the destination in which to store the resulting hash
 */
void app_sha1_ctx_hash(app_sha1_ctx_t *ctx, unsigned char digest_dest[20]);

#endif
