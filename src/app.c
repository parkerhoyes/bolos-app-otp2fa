/*
 * License for the BOLOS OTP 2FA Application project, originally found here:
 * https://github.com/parkerhoyes/bolos-app-otp2fa
 *
 * Copyright (C) 2017, 2018 Parker Hoyes <contact@parkerhoyes.com>
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

#include "app.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "os.h"
#include "os_io_seproxyhal.h"

#include "bui.h"
#include "bui_room.h"

#include "app_rooms.h"

#define APP_TICKER_INTERVAL 40

//----------------------------------------------------------------------------//
//                                                                            //
//                Internal Variable Declarations & Definitions                //
//                                                                            //
//----------------------------------------------------------------------------//

/*
 * Internal Non-const (RAM) Variable Definitions
 */

static uint8_t app_room_ctx_stack[APP_ROOM_CTX_STACK_SIZE] __attribute__((aligned(4)));
static bool app_disp_invalidated; // true if the display needs to be redrawn
static app_key_slot_t *app_persist_keys;
static uint64_t app_time; // current time as a UNIX timestamp, in MILLIseconds
static int32_t app_time_offset; // offset of current timezone from UTC, in seconds

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

/*
 * Compare two strings lexicographically.
 *
 * Args:
 *     str1: the first string; null-terminator is not required
 *     str1_len: the number of characters in str1
 *     str2: the second string; null-terminator is not required
 *     str2_len: the number of characters in str2
 * Returns:
 *     1 if str1 > str2, 0 if str1 == str2, -1 if str1 < str2
 */
static int8_t app_strcmp(const char *str1, uint8_t str1_len, const char *str2, uint8_t str2_len);

static void app_handle_bui_event(bui_ctx_t *ctx, const bui_event_t *event);

static void app_display();

static void app_persist_init();

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

/*
 * External Non-const (RAM) Variable Definitions
 */

bui_ctx_t app_bui_ctx;
bui_room_ctx_t app_room_ctx;

/*
 * External Const (NVRAM) Variable Definitions
 */

const char app_base32_chars[32] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', '2', '3', '4', '5', '6', '7',
};

/*
 * External Non-const Persistent (NVRAM) Variable Definitions
 */

app_persist_t N_app_persist_real;

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

void app_init() {
	// Initialize global vars
	app_disp_invalidated = true;
	app_persist_keys = (app_key_slot_t*) &N_app_persist.key_data[(64 - ((uintptr_t) N_app_persist.key_data & 63)) & 63];
	app_time = 0;
	app_time_offset = 0;
	bui_ctx_init(&app_bui_ctx);
	bui_ctx_set_event_handler(&app_bui_ctx, app_handle_bui_event);
	bui_ctx_set_ticker(&app_bui_ctx, APP_TICKER_INTERVAL);
	if (!N_app_persist.init)
		app_persist_init();

	// Launch the GUI
	bui_room_ctx_init(&app_room_ctx, app_room_ctx_stack, &app_rooms_main, NULL, 0);

	// Draw the first frame
	app_display();
}

void app_io_event() {
	// Pass the event on to BUI for handling
	bui_ctx_seproxyhal_event(&app_bui_ctx, true);
}

void app_disp_invalidate() {
	app_disp_invalidated = true;
}

void app_set_time(uint64_t secs, int32_t offset) {
	app_time = secs * 1000;
	app_time_offset = offset;
}

uint64_t app_get_time() {
	uint64_t secs = app_time / 1000;
	secs &= 0x00000007FFFFFFFF;
	return secs;
}

int32_t app_get_timezone() {
	return app_time_offset;
}

uint8_t app_base32_encode(void *src, uint8_t src_size, char *dest) {
	char *start = dest;
	for (uint8_t i = 0; i + 4 < src_size * 8; i += 5) {
		// Get the 5 bit digit
		uint8_t digit = ((uint8_t*) src)[i / 8] << (i % 8);
		if (i % 8 > 3)
			digit |= ((uint8_t*) src)[i / 8 + 1] >> (8 - i % 8);
		digit >>= 3;
		*dest++ = app_base32_chars[digit];
	}
	return (uint8_t) (dest - start);
}

uint8_t app_base32_decode(char *src, uint8_t src_size, void *dest) {
	uint8_t i = 0;
	for (; src_size != 0; src_size--) {
		uint8_t digit = (uint8_t) *src;
		if (digit >= 'a' && digit <= 'z') {
			digit -= 'a';
		} else if (digit >= 'A' && digit <= 'Z') {
			digit -= 'A';
		} else if (digit >= '2' && digit <= '6') {
			digit -= '2';
			digit += 26;
		} else {
			digit = 31;
		}
		switch (i % 8) {
		case 0:
			((uint8_t*) dest)[i / 8] = digit << 3;
			break;
		case 1:
			((uint8_t*) dest)[i / 8] |= digit << 2;
			break;
		case 2:
			((uint8_t*) dest)[i / 8] |= digit << 1;
			break;
		case 3:
			((uint8_t*) dest)[i / 8] |= digit;
			break;
		case 4:
			((uint8_t*) dest)[i / 8] |= digit >> 1;
			((uint8_t*) dest)[i / 8 + 1] = (digit << 7) & 0x80;
			break;
		case 5:
			((uint8_t*) dest)[i / 8] |= digit >> 2;
			((uint8_t*) dest)[i / 8 + 1] = (digit << 6) & 0xC0;
			break;
		case 6:
			((uint8_t*) dest)[i / 8] |= digit >> 3;
			((uint8_t*) dest)[i / 8 + 1] = (digit << 5) & 0xE0;
			break;
		case 7:
			((uint8_t*) dest)[i / 8] |= digit >> 4;
			((uint8_t*) dest)[i / 8 + 1] = (digit << 4) & 0xF0;
			break;
		}
		src += 1;
		i += 5;
	}
	return (i + 8 - 1) / 8;
}

uint8_t app_dec_encode(uint64_t src, char *dest) {
	if (src == 0) {
		*dest = '0';
		return 1;
	}
	uint8_t n = 0;
	for (uint64_t x = src; x != 0; x /= 10)
		n += 1;
	dest += n;
	for (; src != 0; src /= 10)
		*(--dest) = '0' + src % 10;
	return n;
}

uint64_t app_dec_decode(char *src, uint8_t src_size) {
	uint64_t n = 0;
	for (; src_size != 0; src_size--) {
		n *= 10;
		n += *src++ - '0';
	}
	return n;
}

uint32_t app_find_byte(uint8_t *arr, uint32_t size, uint8_t b) {
	for (uint32_t i = 0; i < size; i++) {
		if (arr[i] == b)
			return i;
	}
	return 0xFFFFFFFF;
}

uint8_t app_key_new(const app_key_t *src) {
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++) {
		if (app_get_key(i)->exists)
			continue;
		nvm_write(app_get_key(i), (void*) src, sizeof(*src));
		return i;
	}
	return 0xFF;
}

app_key_t* app_get_key(uint8_t i) {
	return &app_persist_keys[i].key;
}

void app_key_delete(uint8_t i) {
	nvm_write(app_get_key(i), NULL, sizeof(app_key_t));
}

bool app_key_has_name(uint8_t i, const char *src, uint8_t size) {
	app_key_name_t name = app_get_key(i)->name;
	if (name.size != size)
		return false;
	for (uint8_t j = 0; j < size; j++) {
		if (name.buff[i] != src[i])
			return false;
	}
	return true;
}

void app_key_set_type(uint8_t i, app_key_type_t type) {
	nvm_write(&app_get_key(i)->type, &type, sizeof(type));
}

void app_key_set_name(uint8_t i, char *src, uint8_t size) {
	app_key_name_t name;
	name.size = size;
	os_memcpy(name.buff, src, size);
	os_memset(&name.buff[size], 0, APP_KEY_NAME_MAX - size); // To prevent stack garbage from being written to NVRAM
	nvm_write(&app_get_key(i)->name, &name, sizeof(name));
}

void app_key_set_secret(uint8_t i, uint8_t *src, uint8_t size) {
	app_key_secret_t secret;
	secret.size = size;
	os_memcpy(secret.buff, src, size);
	os_memset(&secret.buff[size], 0, APP_KEY_SECRET_MAX - size); // To prevent stack garbage from being written to NVRAM
	nvm_write(&app_get_key(i)->secret, &secret, sizeof(secret));
}

void app_key_set_counter(uint8_t i, uint64_t src) {
	nvm_write(&app_get_key(i)->counter, &src, sizeof(src));
}

uint8_t app_key_count() {
	uint8_t count = 0;
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++) {
		if (app_get_key(i)->exists)
			count += 1;
	}
	return count;
}

uint8_t app_keys_sort(uint8_t dest[APP_N_KEYS_MAX]) {
	uint8_t n = 0;
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++) {
		const app_key_t *key = app_get_key(i);
		if (!key->exists)
			continue;
		if (n == 0) {
			dest[n++] = i;
			continue;
		}
		for (uint8_t j = 0; j < n; j++) {
			const app_key_name_t *name1 = &key->name;
			const app_key_name_t *name2 = &app_get_key(dest[j])->name;
			int8_t cmp = app_strcmp(name1->buff, name1->size, name2->buff, name2->size);
			if (cmp < 0) {
				os_memmove(&dest[j + 1], &dest[j], n - j);
				dest[j] = i;
				n += 1;
				goto sort_next_key;
			}
		}
		dest[n++] = i;
	sort_next_key:
		continue;
	}
	return n;
}

void app_persist_wipe() {
	nvm_write(&N_app_persist, NULL, sizeof(N_app_persist));
	app_persist_init();
}

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static int8_t app_strcmp(const char *str1, uint8_t str1_len, const char *str2, uint8_t str2_len) {
	uint8_t min_len = str1_len < str2_len ? str1_len : str2_len;
	for (uint8_t i = 0; i < min_len; i++) {
		if (str1[i] < str2[i])
			return -1;
		if (str1[i] > str2[i])
			return 1;
	}
	if (str1_len > str2_len)
		return 1;
	if (str1_len < str2_len)
		return -1;
	return 0;
}

static void app_handle_bui_event(bui_ctx_t *ctx, const bui_event_t *event) {
	bui_room_forward_event(&app_room_ctx, event);
	switch (event->id) {
	case BUI_EVENT_TIME_ELAPSED: {
		if (app_disp_invalidated && bui_ctx_is_displayed(&app_bui_ctx)) {
			app_display();
			app_disp_invalidated = false;
		}
		if (app_time != 0)
			app_time += APP_TICKER_INTERVAL;
	} break;
	// Other events are acknowledged
	default:
		break;
	}
}

static void app_display() {
	bui_ctx_fill(&app_bui_ctx, BUI_CLR_BLACK);
	// Draw the current room by dispatching event BUI_ROOM_EVENT_DRAW
	{
		bui_room_event_data_draw_t data = { .bui_ctx = &app_bui_ctx };
		bui_room_event_t event = { .id = BUI_ROOM_EVENT_DRAW, .data = &data };
		bui_room_dispatch_event(&app_room_ctx, &event);
	}
	bui_ctx_display(&app_bui_ctx);
}

static void app_persist_init() {
	bool init = true;
	nvm_write(&N_app_persist.init, &init, sizeof(init));
	// Since persistent flash storage is zero-initialized, all keys should have their exists field set to false
}
