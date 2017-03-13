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

static void app_room_newkey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_newkey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_newkey_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_newkey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_newkey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

static uint8_t app_room_newkey_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_newkey_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_newkey = {
	.enter = app_room_newkey_enter,
	.exit = app_room_newkey_exit,
	.tick = app_room_newkey_tick,
	.button = app_room_newkey_button,
	.draw = app_room_newkey_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_newkey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	app_room_newkey_inactive_t inactive;
	if (up) {
		inactive.focus = 0;
		app_room_newkey_persist_t *persist = bui_room_alloc(ctx, sizeof(app_room_newkey_persist_t));
		persist->name_size = 0;
		persist->secret_size = 0;
	} else {
		bui_room_pop(ctx, &inactive, sizeof(inactive));
	}
	bui_room_alloc(ctx, sizeof(app_room_newkey_active_t));
	APP_ROOM_NEWKEY_ACTIVE.menu.elem_size_callback = app_room_newkey_elem_size;
	APP_ROOM_NEWKEY_ACTIVE.menu.elem_draw_callback = app_room_newkey_elem_draw;
	bui_menu_init(&APP_ROOM_NEWKEY_ACTIVE.menu, 3, inactive.focus, true);
	app_disp_invalidate();
}

static void app_room_newkey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	if (!up) {
		if (APP_ROOM_NEWKEY_PERSIST.name_size == 0) {
			APP_ROOM_NEWKEY_PERSIST.name_size = 11;
			os_memcpy(APP_ROOM_NEWKEY_PERSIST.name_buff, "Unnamed Key", 11);
		}
		app_key_t new_key;
		new_key.exists = true;
		new_key.name.size = APP_ROOM_NEWKEY_PERSIST.name_size;
		os_memcpy(new_key.name.buff, APP_ROOM_NEWKEY_PERSIST.name_buff, APP_ROOM_NEWKEY_PERSIST.name_size);
		new_key.secret.size = app_base32_decode(APP_ROOM_NEWKEY_PERSIST.secret_buff,
				APP_ROOM_NEWKEY_PERSIST.secret_size, new_key.secret.buff);
		new_key.counter = 1;
		app_key_new(&new_key); // There will always be enough space due to the check by app_rooms_keys
		bui_room_dealloc_frame(ctx);
		return;
	}
	app_room_newkey_inactive_t inactive;
	inactive.focus = bui_menu_get_focused(&APP_ROOM_NEWKEY_ACTIVE.menu);
	bui_room_dealloc(ctx, sizeof(app_room_newkey_active_t));
	bui_room_push(ctx, &inactive, sizeof(inactive));
}

static void app_room_newkey_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	if (bui_menu_animate(&APP_ROOM_NEWKEY_ACTIVE.menu, elapsed))
		app_disp_invalidate();
}

static void app_room_newkey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		switch (bui_menu_get_focused(&APP_ROOM_NEWKEY_ACTIVE.menu)) {
		case 0: {
			app_room_editkeyname_args_t args;
			args.name_size = &APP_ROOM_NEWKEY_PERSIST.name_size;
			args.name_buff = APP_ROOM_NEWKEY_PERSIST.name_buff;
			bui_room_enter(ctx, &app_rooms_editkeyname, &args, sizeof(args));
		} break;
		case 1: {
			app_room_editkeysecret_args_t args;
			args.secret_size = &APP_ROOM_NEWKEY_PERSIST.secret_size;
			args.secret_buff = APP_ROOM_NEWKEY_PERSIST.secret_buff;
			bui_room_enter(ctx, &app_rooms_editkeysecret, &args, sizeof(args));
		} break;
		case 2:
			bui_room_exit(ctx);
			break;
		}
	} else if (left) {
		bui_menu_scroll(&APP_ROOM_NEWKEY_ACTIVE.menu, true);
		app_disp_invalidate();
	} else {
		bui_menu_scroll(&APP_ROOM_NEWKEY_ACTIVE.menu, false);
		app_disp_invalidate();
	}
}

static void app_room_newkey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_menu_draw(&APP_ROOM_NEWKEY_ACTIVE.menu, bui_ctx);
}

static uint8_t app_room_newkey_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	switch (i) {
		case 0: return 25;
		case 1: return 25;
		case 2: return 15;
	}
	// Impossible case
	return 0;
}

static void app_room_newkey_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y) {
	switch (i) {
	case 0: {
		bui_font_draw_string(bui_ctx, "Key Name:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		char text[APP_KEY_NAME_MAX + 1];
		if (APP_ROOM_NEWKEY_PERSIST.name_size == 0) {
			os_memcpy(text, "<type name>", 12);
		} else {
			os_memcpy(text, APP_ROOM_NEWKEY_PERSIST.name_buff, APP_ROOM_NEWKEY_PERSIST.name_size);
			text[APP_ROOM_NEWKEY_PERSIST.name_size] = '\0';
		}
		bui_font_draw_string(bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
	} break;
	case 1: {
		bui_font_draw_string(bui_ctx, "Key Secret:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		char text[APP_KEY_SECRET_ENCODED_MAX + 1];
		if (APP_ROOM_NEWKEY_PERSIST.secret_size == 0) {
			os_memcpy(text, "<type secret>", 14);
		} else {
			os_memcpy(text, APP_ROOM_NEWKEY_PERSIST.secret_buff, APP_ROOM_NEWKEY_PERSIST.secret_size);
			text[APP_ROOM_NEWKEY_PERSIST.secret_size] = '\0';
		}
		bui_font_draw_string(bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
	} break;
	case 2:
		bui_font_draw_string(bui_ctx, "Done", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	}
}
