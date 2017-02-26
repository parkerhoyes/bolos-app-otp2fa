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

/*
 * NOTICE: This implementation of the SHA-1 hash function is derived from an
 * implementation by Steve Reid <steve@edmweb.com> which is in the public
 * domain. That implementation may be found here: https://github.com/clibs/sha1
 */

#include "app_sha1.h"

#include <stdbool.h>
#include <stdint.h>

#include "os.h"

#define BLOCK_INTS 16 // Number of 32-bit integers in a single SHA-1 block
#define BLOCK_BYTES (BLOCK_INTS * 4)

#define DIGEST_INTS 5 // Number of 32-bit integers in a single SHA-1 digest
#define DIGEST_BYTES (DIGEST_INTS * 4)

static inline uint32_t app_sha1_rol(uint32_t value, uint8_t bits) {
	return (value << bits) | (value >> (32 - bits));
}

static inline uint32_t app_sha1_blk(uint32_t block[BLOCK_INTS], uint8_t i) {
	return app_sha1_rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i], 1);
}

static inline void app_sha1_r0(uint32_t block[BLOCK_INTS], uint32_t v, uint32_t *w, uint32_t x, uint32_t y, uint32_t *z,
		uint8_t i) {
	*z += ((*w & (x ^ y)) ^ y) + block[i] + 0x5A827999 + app_sha1_rol(v, 5);
	*w = app_sha1_rol(*w, 30);
}


static inline void app_sha1_r1(uint32_t block[BLOCK_INTS], uint32_t v, uint32_t *w, uint32_t x, uint32_t y, uint32_t *z,
		uint8_t i) {
	block[i] = app_sha1_blk(block, i);
	*z += ((*w & (x ^ y)) ^ y) + block[i] + 0x5A827999 + app_sha1_rol(v, 5);
	*w = app_sha1_rol(*w, 30);
}


static inline void app_sha1_r2(uint32_t block[BLOCK_INTS], uint32_t v, uint32_t *w, uint32_t x, uint32_t y, uint32_t *z,
		uint8_t i) {
	block[i] = app_sha1_blk(block, i);
	*z += (*w ^ x ^ y) + block[i] + 0x6ED9EBA1 + app_sha1_rol(v, 5);
	*w = app_sha1_rol(*w, 30);
}


static inline void app_sha1_r3(uint32_t block[BLOCK_INTS], uint32_t v, uint32_t *w, uint32_t x, uint32_t y, uint32_t *z,
		uint8_t i) {
	block[i] = app_sha1_blk(block, i);
	*z += (((*w | x) & y) | (*w & x)) + block[i] + 0x8F1BBCDC + app_sha1_rol(v, 5);
	*w = app_sha1_rol(*w, 30);
}


static inline void app_sha1_r4(uint32_t block[BLOCK_INTS], uint32_t v, uint32_t *w, uint32_t x, uint32_t y, uint32_t *z,
		uint8_t i) {
	block[i] = app_sha1_blk(block, i);
	*z += (*w ^ x ^ y) + block[i] + 0xCA62C1D6 + app_sha1_rol(v, 5);
	*w = app_sha1_rol(*w, 30);
}

/*
 * Hash a single 512-bit block. This is the core of the algorithm.
 */
static inline void app_sha1_transform(uint32_t digest[DIGEST_BYTES], uint32_t block[BLOCK_INTS], uint64_t *transforms) {
	// Copy digest to working vars
	uint32_t a = digest[0];
	uint32_t b = digest[1];
	uint32_t c = digest[2];
	uint32_t d = digest[3];
	uint32_t e = digest[4];

	// 4 rounds of 20 operations each. Loop unrolled.
	app_sha1_r0(block, a, &b, c, d, &e,  0);
	app_sha1_r0(block, e, &a, b, c, &d,  1);
	app_sha1_r0(block, d, &e, a, b, &c,  2);
	app_sha1_r0(block, c, &d, e, a, &b,  3);
	app_sha1_r0(block, b, &c, d, e, &a,  4);
	app_sha1_r0(block, a, &b, c, d, &e,  5);
	app_sha1_r0(block, e, &a, b, c, &d,  6);
	app_sha1_r0(block, d, &e, a, b, &c,  7);
	app_sha1_r0(block, c, &d, e, a, &b,  8);
	app_sha1_r0(block, b, &c, d, e, &a,  9);
	app_sha1_r0(block, a, &b, c, d, &e, 10);
	app_sha1_r0(block, e, &a, b, c, &d, 11);
	app_sha1_r0(block, d, &e, a, b, &c, 12);
	app_sha1_r0(block, c, &d, e, a, &b, 13);
	app_sha1_r0(block, b, &c, d, e, &a, 14);
	app_sha1_r0(block, a, &b, c, d, &e, 15);
	app_sha1_r1(block, e, &a, b, c, &d,  0);
	app_sha1_r1(block, d, &e, a, b, &c,  1);
	app_sha1_r1(block, c, &d, e, a, &b,  2);
	app_sha1_r1(block, b, &c, d, e, &a,  3);
	app_sha1_r2(block, a, &b, c, d, &e,  4);
	app_sha1_r2(block, e, &a, b, c, &d,  5);
	app_sha1_r2(block, d, &e, a, b, &c,  6);
	app_sha1_r2(block, c, &d, e, a, &b,  7);
	app_sha1_r2(block, b, &c, d, e, &a,  8);
	app_sha1_r2(block, a, &b, c, d, &e,  9);
	app_sha1_r2(block, e, &a, b, c, &d, 10);
	app_sha1_r2(block, d, &e, a, b, &c, 11);
	app_sha1_r2(block, c, &d, e, a, &b, 12);
	app_sha1_r2(block, b, &c, d, e, &a, 13);
	app_sha1_r2(block, a, &b, c, d, &e, 14);
	app_sha1_r2(block, e, &a, b, c, &d, 15);
	app_sha1_r2(block, d, &e, a, b, &c,  0);
	app_sha1_r2(block, c, &d, e, a, &b,  1);
	app_sha1_r2(block, b, &c, d, e, &a,  2);
	app_sha1_r2(block, a, &b, c, d, &e,  3);
	app_sha1_r2(block, e, &a, b, c, &d,  4);
	app_sha1_r2(block, d, &e, a, b, &c,  5);
	app_sha1_r2(block, c, &d, e, a, &b,  6);
	app_sha1_r2(block, b, &c, d, e, &a,  7);
	app_sha1_r3(block, a, &b, c, d, &e,  8);
	app_sha1_r3(block, e, &a, b, c, &d,  9);
	app_sha1_r3(block, d, &e, a, b, &c, 10);
	app_sha1_r3(block, c, &d, e, a, &b, 11);
	app_sha1_r3(block, b, &c, d, e, &a, 12);
	app_sha1_r3(block, a, &b, c, d, &e, 13);
	app_sha1_r3(block, e, &a, b, c, &d, 14);
	app_sha1_r3(block, d, &e, a, b, &c, 15);
	app_sha1_r3(block, c, &d, e, a, &b,  0);
	app_sha1_r3(block, b, &c, d, e, &a,  1);
	app_sha1_r3(block, a, &b, c, d, &e,  2);
	app_sha1_r3(block, e, &a, b, c, &d,  3);
	app_sha1_r3(block, d, &e, a, b, &c,  4);
	app_sha1_r3(block, c, &d, e, a, &b,  5);
	app_sha1_r3(block, b, &c, d, e, &a,  6);
	app_sha1_r3(block, a, &b, c, d, &e,  7);
	app_sha1_r3(block, e, &a, b, c, &d,  8);
	app_sha1_r3(block, d, &e, a, b, &c,  9);
	app_sha1_r3(block, c, &d, e, a, &b, 10);
	app_sha1_r3(block, b, &c, d, e, &a, 11);
	app_sha1_r4(block, a, &b, c, d, &e, 12);
	app_sha1_r4(block, e, &a, b, c, &d, 13);
	app_sha1_r4(block, d, &e, a, b, &c, 14);
	app_sha1_r4(block, c, &d, e, a, &b, 15);
	app_sha1_r4(block, b, &c, d, e, &a,  0);
	app_sha1_r4(block, a, &b, c, d, &e,  1);
	app_sha1_r4(block, e, &a, b, c, &d,  2);
	app_sha1_r4(block, d, &e, a, b, &c,  3);
	app_sha1_r4(block, c, &d, e, a, &b,  4);
	app_sha1_r4(block, b, &c, d, e, &a,  5);
	app_sha1_r4(block, a, &b, c, d, &e,  6);
	app_sha1_r4(block, e, &a, b, c, &d,  7);
	app_sha1_r4(block, d, &e, a, b, &c,  8);
	app_sha1_r4(block, c, &d, e, a, &b,  9);
	app_sha1_r4(block, b, &c, d, e, &a, 10);
	app_sha1_r4(block, a, &b, c, d, &e, 11);
	app_sha1_r4(block, e, &a, b, c, &d, 12);
	app_sha1_r4(block, d, &e, a, b, &c, 13);
	app_sha1_r4(block, c, &d, e, a, &b, 14);
	app_sha1_r4(block, b, &c, d, e, &a, 15);

	// Add the working vars back into digest
	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;

	// Count the number of transformations
	*transforms += 1;
}

static inline void app_sha1_buffer_to_block(const unsigned char buffer[BLOCK_BYTES], uint32_t block[BLOCK_INTS]) {
	// Convert the byte buffer to a uint32_t array (MSB)
	for (uint8_t i = 0; i < BLOCK_INTS; i++) {
		block[i] = (buffer[4 * i + 3] & 0xFF)
				| (buffer[4 * i + 2] & 0xFF) << 8
				| (buffer[4 * i + 1] & 0xFF) << 16
				| (buffer[4 * i + 0] & 0xFF) << 24;
	}
}

static inline void app_sha1_digest_to_buffer(const uint32_t digest[DIGEST_INTS], unsigned char buffer[DIGEST_BYTES]) {
	// Convert the uint32_t array (MSB) to byte buffer
	for (uint8_t i = 0; i < DIGEST_BYTES; i++) {
		buffer[i] = digest[i / 4] >> ((3 - i % 4) * 8);
	}
}

void app_sha1_ctx_init(app_sha1_ctx_t *ctx) {
	// SHA1 initialization constants
	ctx->digest[0] = 0x67452301;
	ctx->digest[1] = 0xEFCDAB89;
	ctx->digest[2] = 0x98BADCFE;
	ctx->digest[3] = 0x10325476;
	ctx->digest[4] = 0xC3D2E1F0;

	// Reset contents
	ctx->buffer_size = 0;
	ctx->transforms = 0;
}

void app_sha1_ctx_update(app_sha1_ctx_t *ctx, const unsigned char *data, uint32_t len) {
	if (ctx->buffer_size == 64) {
		app_sha1_ctx_iterate(ctx);
	}
	while (len != 0) {
		uint8_t n = BLOCK_BYTES - ctx->buffer_size;
		if (len < n)
			n = len;
		os_memcpy(ctx->buffer, data, n);
		ctx->buffer_size += n;
		if (ctx->buffer_size != BLOCK_BYTES)
			return;
		data += n;
		len -= n;
		uint32_t block[BLOCK_INTS];
		app_sha1_buffer_to_block(ctx->buffer, block);
		app_sha1_transform(ctx->digest, block, &ctx->transforms);
		ctx->buffer_size = 0;
	}
}

void app_sha1_ctx_iterate(app_sha1_ctx_t *ctx) {
	if (ctx->buffer_size != BLOCK_BYTES)
		os_memset(&ctx->buffer[ctx->buffer_size], 0, BLOCK_BYTES - ctx->buffer_size);
	uint32_t block[BLOCK_INTS];
	app_sha1_buffer_to_block(ctx->buffer, block);
	app_sha1_transform(ctx->digest, block, &ctx->transforms);
	ctx->buffer_size = 0;
}

void app_sha1_ctx_hash(app_sha1_ctx_t *ctx, unsigned char digest_dest[DIGEST_BYTES]) {
	if (ctx->buffer_size == 64) {
		app_sha1_ctx_iterate(ctx);
	}

	// Total number of hashed bits
	uint64_t total_bits = (ctx->transforms * BLOCK_BYTES + ctx->buffer_size) * 8;

	// Add padding
	ctx->buffer[ctx->buffer_size++] = 0x80;
	if (ctx->buffer_size != BLOCK_BYTES)
		os_memset(&ctx->buffer[ctx->buffer_size], 0, BLOCK_BYTES - ctx->buffer_size);

	uint32_t block[BLOCK_INTS];
	app_sha1_buffer_to_block(ctx->buffer, block);

	// Add more padding such that the buffer length is 56 bytes
	if (ctx->buffer_size > BLOCK_BYTES - 8) {
		app_sha1_transform(ctx->digest, block, &ctx->transforms);
		os_memset(block, 0, sizeof(block[0]) * (BLOCK_INTS - 2));
	}

	// Append total_bits, split this uint64_t into two uint32_t
	block[BLOCK_INTS - 1] = total_bits;
	block[BLOCK_INTS - 2] = (total_bits >> 32);
	app_sha1_transform(ctx->digest, block, &ctx->transforms);

	app_sha1_digest_to_buffer(ctx->digest, digest_dest);
}
