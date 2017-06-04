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

#ifndef APP_H_
#define APP_H_

#include <stdbool.h>
#include <stdint.h>

#include "os.h"

#include "bui.h"
#include "bui_room.h"

#define APP_VER_MAJOR 1
#define APP_VER_MINOR 0
#define APP_VER_PATCH 0

#define APP_ROOM_CTX_STACK_SIZE 512
#define APP_KEY_NAME_MAX 20 // In characters
#define APP_KEY_SECRET_MAX 20 // In bytes
#define APP_KEY_SECRET_ENCODED_MAX ((APP_KEY_SECRET_MAX * 8 + 5 - 1) / 5) // In characters
#define APP_N_KEYS_MAX 64

#define N_app_persist (*(app_persist_t*) PIC(&N_app_persist_real))

//----------------------------------------------------------------------------//
//                                                                            //
//                  External Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_key_name_t {
	uint8_t size; // In characters
	char buff[APP_KEY_NAME_MAX];
} app_key_name_t;

typedef struct app_key_secret_t {
	uint8_t size; // In bytes
	uint8_t buff[APP_KEY_SECRET_MAX]; // Stores the secret decoded, big-endian
} app_key_secret_t;

typedef struct app_key_t {
	uint64_t counter;
	bool exists; // true if the key exists, false if it has been deleted
	app_key_name_t name;
	app_key_secret_t secret;
} app_key_t;

typedef struct app_key_slot_t {
	app_key_t key;
	uint8_t pad[64 - sizeof(app_key_t)]; // Padding to assure sizeof(app_key_slot_t) == 64
} app_key_slot_t;

_Static_assert(sizeof(app_key_slot_t) == 64, "sizeof(app_key_slot_t) must be 64");

// Persistent storage memory layout
typedef struct app_persist_t {
	bool init; // true if storage has been initialized, false otherwise
	uint8_t key_data[63 + sizeof(app_key_slot_t) * APP_N_KEYS_MAX];
} app_persist_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

/*
 * External Non-const (RAM) Variable Declarations
 */

extern bui_ctx_t app_bui_ctx;
extern bui_room_ctx_t app_room_ctx;

/*
 * External Const (NVRAM) Variable Declarations
 */

extern const char app_base32_chars[32];

/*
 * External Non-const Persistent (NVRAM) Variable Declarations
 */

extern app_persist_t N_app_persist_real;

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

void app_init();
void app_io_event();
void app_disp_invalidate();

/*
 * Encode the provided byte buffer as a base-32 string according to RFC 4648, with no padding.
 *
 * Args:
 *     src: the source data
 *     src_size: the number of bytes at src
 *     dest: the destination of the base-32 string (no null-terminator is written)
 * Returns:
 *     the number of characters written to dest
 */
uint8_t app_base32_encode(void *src, uint8_t src_size, char *dest);

/*
 * Decode the provided RFC 4648 base-32 string with no padding into a byte buffer. This algorithm is case-insensitive.
 *
 * Args:
 *     src: the source string; no data past src[src_size - 1] is ever read, so no null-terminator is necessary
 *     src_size: the number of characters in src
 *     dest: the destination byte buffer
 * Returns:
 *     the number of bytes written to dest
 */
uint8_t app_base32_decode(char *src, uint8_t src_size, void *dest);

/*
 * Encode a decimal integer as a string (with no null-terminator).
 *
 * Args:
 *     src: the integer
 *     dest: the destination for the string
 * Returns:
 *     the number of characters written to dest; always in [1, 20]
 */
uint8_t app_dec_encode(uint64_t src, char *dest);

/*
 * Decode a sequence of ASCII digits as an integer. If the integer being decoded does not fit within a 64-bit integer,
 * the result is unspecified.
 *
 * Args:
 *     src: the source string; no data past src[src_size - 1] is ever read, so no null-terminator is necessary
 *     src_size: the number of characters in src
 * Returns:
 *     the decoded integer
 */
uint64_t app_dec_decode(char *src, uint8_t src_size);

uint32_t app_find_byte(uint8_t *arr, uint32_t size, uint8_t b);

/*
 * Store a new key in N_app_persist.
 *
 * Args:
 *     src: the data for the new key; src->exists must be true
 * Returns:
 *     the index of the new key, or 0xFF if there's not enough space
 */
uint8_t app_key_new(const app_key_t *src);

app_key_t* app_get_key(uint8_t i);

/*
 * Delete a key stored in N_app_persist at the specified index.
 *
 * Args:
 *     i: the index of the key to be deleted
 */
void app_key_delete(uint8_t i);

bool app_key_has_name(uint8_t i, const char *src, uint8_t size);

void app_key_set_name(uint8_t i, char *src, uint8_t size);

void app_key_set_secret(uint8_t i, uint8_t *src, uint8_t size);

void app_key_set_counter(uint8_t i, uint64_t src);

uint8_t app_key_count();

/*
 * Sort all keys in N_app_persist.keys by their names, storing the indexes of the sorted keys in the specified array.
 *
 * Args:
 *     dest: the array in which to store the sorted indices
 * Returns:
 *     the number of indices stored in dest
 */
uint8_t app_keys_sort(uint8_t dest[APP_N_KEYS_MAX]);

void app_persist_wipe();

#endif
