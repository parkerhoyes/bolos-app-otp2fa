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

static uint8_t app_room_ctx_stack[APP_ROOM_CTX_STACK_SIZE];
static int8_t app_disp_progress;
static bool app_disp_invalidated; // true if the display needs to be redrawn

/*
 * Internal Const (NVRAM) Variable Definitions
 */

static const app_key_t app_zeroed_key = {};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

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

bui_room_ctx_t app_room_ctx;
bui_bitmap_128x32_t app_disp_buffer;

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
	// Set a ticker interval of APP_TICKER_INTERVAL ms
	G_io_seproxyhal_spi_buffer[0] = SEPROXYHAL_TAG_SET_TICKER_INTERVAL;
	G_io_seproxyhal_spi_buffer[1] = 0; // Message length, high-byte
	G_io_seproxyhal_spi_buffer[2] = 2; // Message length, low-byte
	G_io_seproxyhal_spi_buffer[3] = 0; // Ticker interval, high-byte
	G_io_seproxyhal_spi_buffer[4] = APP_TICKER_INTERVAL; // Ticker interval, low-byte
	io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 5);

	// Initialize global vars
	app_disp_progress = -1;
	app_disp_invalidated = true;
	if (!N_app_persist.init)
		app_persist_init();

	// Launch the GUI
	bui_room_ctx_init(&app_room_ctx, app_room_ctx_stack, &app_rooms_main, NULL, 0);
}

void app_event_button_push(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		bui_room_current_button(&app_room_ctx, true, true);
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		bui_room_current_button(&app_room_ctx, true, false);
		break;
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		bui_room_current_button(&app_room_ctx, false, true);
		break;
	}
}

void app_event_ticker() {
	if (bui_room_current_tick(&app_room_ctx, APP_TICKER_INTERVAL))
		app_disp_invalidated = true;
	if (app_disp_invalidated && app_disp_progress == -1) {
		app_display();
		app_disp_invalidated = false;
	}
}

void app_event_display_processed() {
	if (app_disp_progress != -1)
		app_disp_progress = bui_display(&app_disp_buffer, app_disp_progress);
}

void app_disp_invalidate() {
	app_disp_invalidated = true;
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
		if (N_app_persist.keys[i].exists)
			continue;
		nvm_write(&N_app_persist.keys[i], (void*) src, sizeof(*src));
		return i;
	}
	return 0xFF;
}

void app_key_delete(uint8_t i) {
	nvm_write(&N_app_persist.keys[i], (void*) &app_zeroed_key, sizeof(app_zeroed_key));
}

bool app_key_has_name(uint8_t i, const char *src, uint8_t size) {
	app_key_name_t name = N_app_persist.keys[i].name;
	if (name.size != size)
		return false;
	for (uint8_t j = 0; j < size; j++) {
		if (name.buff[i] != src[i])
			return false;
	}
	return true;
}

void app_key_set_name(uint8_t i, char *src, uint8_t size) {
	app_key_name_t name;
	name.size = size;
	os_memcpy(name.buff, src, size);
	os_memset(&name.buff[size], 0, APP_KEY_NAME_MAX - size); // To prevent stack garbage from being written to NVRAM
	nvm_write(&N_app_persist.keys[i].name, &name, sizeof(name));
}

void app_key_set_secret(uint8_t i, uint8_t *src, uint8_t size) {
	app_key_secret_t secret;
	secret.size = size;
	os_memcpy(secret.buff, src, size);
	os_memset(&secret.buff[size], 0, APP_KEY_SECRET_MAX - size); // To prevent stack garbage from being written to NVRAM
	nvm_write(&N_app_persist.keys[i].secret, &secret, sizeof(secret));
}

void app_key_set_counter(uint8_t i, uint64_t src) {
	nvm_write(&N_app_persist.keys[i].counter, &src, sizeof(src));
}

uint8_t app_key_count() {
	uint8_t count = 0;
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++) {
		if (N_app_persist.keys[i].exists)
			count += 1;
	}
	return count;
}

uint8_t app_keys_sort(uint8_t dest[APP_N_KEYS_MAX]) {
	// TODO Make this an alphabetic sort
	uint8_t n = 0;
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++) {
		if (N_app_persist.keys[i].exists)
			dest[n++] = i;
	}
	return n;
}

uint8_t app_key_sort(uint8_t dest[APP_N_KEYS_MAX]) {
	uint8_t n = 0;
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++) {
		if (N_app_persist.keys[i].exists)
			dest[n++] = i;
	}
	return n;
}

void app_persist_wipe() {
	// TODO There must be a better way to do this...
	for (uint8_t i = 0; i < APP_N_KEYS_MAX; i++)
		nvm_write(&N_app_persist.keys[i], (void*) &app_zeroed_key, sizeof(app_zeroed_key));
}

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_display() {
	bui_fill(&app_disp_buffer, false);
	bui_room_current_draw(&app_room_ctx, &app_disp_buffer);
	if (app_disp_progress == -1)
		app_disp_progress = bui_display(&app_disp_buffer, 0);
	else
		app_disp_progress = 0;
}

static void app_persist_init() {
	bool init = true;
	nvm_write(&N_app_persist.init, &init, sizeof(init));
	// Since persistent flash storage is zero-initialized, all keys should have their exists field set to false
}