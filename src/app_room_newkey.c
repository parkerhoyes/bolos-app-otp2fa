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

#include "app_rooms.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "os.h"

#include "bui.h"
#include "bui_font.h"
#include "bui_menu.h"
#include "bui_room.h"

#include "app.h"

#define APP_ROOM_NEWKEY_ACTIVE (*((app_room_newkey_active_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_NEWKEY_PERSIST (*((app_room_newkey_persist_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

// This data is always on the stack at the bottom of the stack frame, whether this room is the active room or not
typedef struct __attribute__((aligned(4))) app_room_newkey_persist_t {
	app_key_type_t type;
	uint8_t name_size;
	char name_buff[APP_KEY_NAME_MAX];
	uint8_t secret_size;
	char secret_buff[APP_KEY_SECRET_ENCODED_MAX]; // Stores the secret encoded in base-32
} app_room_newkey_persist_t;

typedef struct app_room_newkey_active_t {
	bui_menu_menu_t menu;
} app_room_newkey_active_t;

typedef struct app_room_newkey_inactive_t {
	uint8_t focus;
} app_room_newkey_inactive_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_newkey_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event);

static void app_room_newkey_enter(bool up);
static void app_room_newkey_exit(bool up);
static void app_room_newkey_draw();
static void app_room_newkey_time_elapsed(uint32_t elapsed);
static void app_room_newkey_button_clicked(bui_button_id_t button);

static uint8_t app_room_newkey_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_newkey_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_newkey = {
	.event_handler = app_room_newkey_handle_event,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_newkey_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event) {
	switch (event->id) {
	case BUI_ROOM_EVENT_ENTER: {
		bool up = BUI_ROOM_EVENT_DATA_ENTER(event)->up;
		app_room_newkey_enter(up);
	} break;
	case BUI_ROOM_EVENT_EXIT: {
		bool up = BUI_ROOM_EVENT_DATA_EXIT(event)->up;
		app_room_newkey_exit(up);
	} break;
	case BUI_ROOM_EVENT_DRAW: {
		app_room_newkey_draw();
	} break;
	case BUI_ROOM_EVENT_FORWARD: {
		const bui_event_t *bui_event = BUI_ROOM_EVENT_DATA_FORWARD(event);
		switch (bui_event->id) {
		case BUI_EVENT_TIME_ELAPSED: {
			uint32_t elapsed = BUI_EVENT_DATA_TIME_ELAPSED(bui_event)->elapsed;
			app_room_newkey_time_elapsed(elapsed);
		} break;
		case BUI_EVENT_BUTTON_CLICKED: {
			bui_button_id_t button = BUI_EVENT_DATA_BUTTON_CLICKED(bui_event)->button;
			app_room_newkey_button_clicked(button);
		} break;
		// Other events are acknowledged
		default:
			break;
		}
	} break;
	}
}

static void app_room_newkey_enter(bool up) {
	app_room_newkey_inactive_t inactive;
	if (up) {
		inactive.focus = 0;
		app_room_newkey_persist_t *persist = bui_room_alloc(&app_room_ctx, sizeof(app_room_newkey_persist_t));
		persist->name_size = 0;
		persist->secret_size = 0;
		persist->type = APP_KEY_TYPE_TOTP;
	} else {
		bui_room_pop(&app_room_ctx, &inactive, sizeof(inactive));
	}
	bui_room_alloc(&app_room_ctx, sizeof(app_room_newkey_active_t));
	APP_ROOM_NEWKEY_ACTIVE.menu.elem_size_callback = app_room_newkey_elem_size;
	APP_ROOM_NEWKEY_ACTIVE.menu.elem_draw_callback = app_room_newkey_elem_draw;
	bui_menu_init(&APP_ROOM_NEWKEY_ACTIVE.menu, 4, inactive.focus, true);
	app_disp_invalidate();
}

static void app_room_newkey_exit(bool up) {
	if (!up) {
		if (APP_ROOM_NEWKEY_PERSIST.name_size == 0) {
			APP_ROOM_NEWKEY_PERSIST.name_size = 11;
			os_memcpy(APP_ROOM_NEWKEY_PERSIST.name_buff, "Unnamed Key", 11);
		}
		app_key_t new_key;
		new_key.exists = true;
		new_key.type = APP_ROOM_NEWKEY_PERSIST.type;
		new_key.name.size = APP_ROOM_NEWKEY_PERSIST.name_size;
		os_memcpy(new_key.name.buff, APP_ROOM_NEWKEY_PERSIST.name_buff, APP_ROOM_NEWKEY_PERSIST.name_size);
		new_key.secret.size = app_base32_decode(APP_ROOM_NEWKEY_PERSIST.secret_buff,
				APP_ROOM_NEWKEY_PERSIST.secret_size, new_key.secret.buff);
		new_key.counter = 1;
		app_key_new(&new_key); // There will always be enough space due to the check by app_rooms_keys
		bui_room_dealloc_frame(&app_room_ctx);
		return;
	}
	app_room_newkey_inactive_t inactive;
	inactive.focus = bui_menu_get_focused(&APP_ROOM_NEWKEY_ACTIVE.menu);
	bui_room_dealloc(&app_room_ctx, sizeof(app_room_newkey_active_t));
	bui_room_push(&app_room_ctx, &inactive, sizeof(inactive));
}

static void app_room_newkey_draw() {
	bui_menu_draw(&APP_ROOM_NEWKEY_ACTIVE.menu, &app_bui_ctx);
}

static void app_room_newkey_time_elapsed(uint32_t elapsed) {
	if (bui_menu_animate(&APP_ROOM_NEWKEY_ACTIVE.menu, elapsed))
		app_disp_invalidate();
}

static void app_room_newkey_button_clicked(bui_button_id_t button) {
	switch (button) {
	case BUI_BUTTON_NANOS_BOTH:
		switch (bui_menu_get_focused(&APP_ROOM_NEWKEY_ACTIVE.menu)) {
		case 0: {
			app_room_editkeytype_args_t args;
			args.type = &APP_ROOM_NEWKEY_PERSIST.type;
			bui_room_enter(&app_room_ctx, &app_rooms_editkeytype, &args, sizeof(args));
		} break;
		case 1: {
			app_room_editkeyname_args_t args;
			args.name_size = &APP_ROOM_NEWKEY_PERSIST.name_size;
			args.name_buff = APP_ROOM_NEWKEY_PERSIST.name_buff;
			bui_room_enter(&app_room_ctx, &app_rooms_editkeyname, &args, sizeof(args));
		} break;
		case 2: {
			app_room_editkeysecret_args_t args;
			args.secret_size = &APP_ROOM_NEWKEY_PERSIST.secret_size;
			args.secret_buff = APP_ROOM_NEWKEY_PERSIST.secret_buff;
			bui_room_enter(&app_room_ctx, &app_rooms_editkeysecret, &args, sizeof(args));
		} break;
		case 3:
			bui_room_exit(&app_room_ctx);
			break;
		}
		break;
	case BUI_BUTTON_NANOS_LEFT:
		bui_menu_scroll(&APP_ROOM_NEWKEY_ACTIVE.menu, true);
		app_disp_invalidate();
		break;
	case BUI_BUTTON_NANOS_RIGHT:
		bui_menu_scroll(&APP_ROOM_NEWKEY_ACTIVE.menu, false);
		app_disp_invalidate();
		break;
	}
}

static uint8_t app_room_newkey_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	switch (i) {
		case 0: return 25;
		case 1: return 25;
		case 2: return APP_ROOM_NEWKEY_PERSIST.secret_size > 19 ? 31 : 25;
		case 3: return 15;
	}
	// Impossible case
	return 0;
}

static void app_room_newkey_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y) {
	switch (i) {
	case 0: {
		bui_font_draw_string(&app_bui_ctx, "Key Type:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		const char *text = APP_ROOM_NEWKEY_PERSIST.type == APP_KEY_TYPE_TOTP ? "TOTP (time-based)" :
				"HOTP (counter-based)";
		bui_font_draw_string(&app_bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
	} break;
	case 1: {
		bui_font_draw_string(&app_bui_ctx, "Key Name:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		char text[APP_KEY_NAME_MAX + 1];
		if (APP_ROOM_NEWKEY_PERSIST.name_size == 0) {
			os_memcpy(text, "<type name>", 12);
		} else {
			os_memcpy(text, APP_ROOM_NEWKEY_PERSIST.name_buff, APP_ROOM_NEWKEY_PERSIST.name_size);
			text[APP_ROOM_NEWKEY_PERSIST.name_size] = '\0';
		}
		bui_font_draw_string(&app_bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
	} break;
	case 2: {
		bool large = APP_ROOM_NEWKEY_PERSIST.secret_size > 19;
		bui_font_draw_string(&app_bui_ctx, "Key Secret:", 64, y + (large ? 1 : 2), BUI_DIR_TOP,
				bui_font_open_sans_extrabold_11);
		if (APP_ROOM_NEWKEY_PERSIST.secret_size == 0) {
			bui_font_draw_string(&app_bui_ctx, "<type secret>", 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
		} else if (APP_ROOM_NEWKEY_PERSIST.secret_size <= 19) {
			char text[20];
			os_memcpy(text, APP_ROOM_NEWKEY_PERSIST.secret_buff, APP_ROOM_NEWKEY_PERSIST.secret_size);
			text[APP_ROOM_NEWKEY_PERSIST.secret_size] = '\0';
			bui_font_draw_string(&app_bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
		} else if (APP_ROOM_NEWKEY_PERSIST.secret_size <= 38) {
			char text[20];
			os_memcpy(text, APP_ROOM_NEWKEY_PERSIST.secret_buff, 19);
			text[19] = '\0';
			bui_font_draw_string(&app_bui_ctx, text, 64, y + 13, BUI_DIR_TOP, bui_font_lucida_console_8);
			os_memcpy(text, &APP_ROOM_NEWKEY_PERSIST.secret_buff[19], APP_ROOM_NEWKEY_PERSIST.secret_size - 19);
			text[APP_ROOM_NEWKEY_PERSIST.secret_size - 19] = '\0';
			bui_font_draw_string(&app_bui_ctx, text, 64, y + 22, BUI_DIR_TOP, bui_font_lucida_console_8);
		} else {
			char text[20];
			os_memcpy(text, APP_ROOM_NEWKEY_PERSIST.secret_buff, 19);
			text[19] = '\0';
			bui_font_draw_string(&app_bui_ctx, text, 64, y + 13, BUI_DIR_TOP, bui_font_lucida_console_8);
			os_memcpy(text, &APP_ROOM_NEWKEY_PERSIST.secret_buff[19], 16);
			os_memcpy(&text[16], "...", 3);
			text[19] = '\0';
			bui_font_draw_string(&app_bui_ctx, text, 64, y + 22, BUI_DIR_TOP, bui_font_lucida_console_8);
		}
	} break;
	case 3:
		bui_font_draw_string(&app_bui_ctx, "Done", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	}
}
