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
#include "app_otp.h"

#define APP_ROOM_MANAGEKEY_ACTIVE (*((app_room_managekey_active_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_MANAGEKEY_PERSIST (*((app_room_managekey_persist_t*) app_room_ctx.frame_ptr))
#define APP_ROOM_MANAGEKEY_KEY (*app_get_key(APP_ROOM_MANAGEKEY_PERSIST.key_i))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

// This data is always on the stack at the bottom of the stack frame, whether this room is the active room or not
typedef struct __attribute__((aligned(4))) app_room_managekey_persist_t {
	uint64_t counter;
	uint8_t key_i;
	uint8_t name_size;
	char name_buff[APP_KEY_NAME_MAX];
} app_room_managekey_persist_t;

typedef struct app_room_managekey_active_t {
	bui_menu_menu_t menu;
	char auth_code[6]; // The 6-digit OTP code as a string, if generated
	bool has_auth_code; // true if the OTP code has been generated, false otherwise
} app_room_managekey_active_t;

typedef struct app_room_managekey_inactive_t {
	uint8_t focus;
} app_room_managekey_inactive_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_managekey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_managekey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_managekey_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_managekey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_managekey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

static uint8_t app_room_managekey_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_managekey_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_managekey = {
	.enter = app_room_managekey_enter,
	.exit = app_room_managekey_exit,
	.tick = app_room_managekey_tick,
	.button = app_room_managekey_button,
	.draw = app_room_managekey_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_managekey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	app_room_managekey_inactive_t inactive;
	if (up) {
		app_room_managekey_args_t args;
		bui_room_pop(ctx, &args, sizeof(args));
		bui_room_alloc(ctx, sizeof(app_room_managekey_persist_t) + sizeof(app_room_managekey_active_t));
		if (!app_get_key(args.key_i)->exists) {
			bui_room_exit(ctx);
			return;
		}
		APP_ROOM_MANAGEKEY_PERSIST.key_i = args.key_i;
		APP_ROOM_MANAGEKEY_PERSIST.name_size = APP_ROOM_MANAGEKEY_KEY.name.size;
		os_memcpy(APP_ROOM_MANAGEKEY_PERSIST.name_buff, APP_ROOM_MANAGEKEY_KEY.name.buff,
				APP_ROOM_MANAGEKEY_KEY.name.size);
		APP_ROOM_MANAGEKEY_PERSIST.counter = APP_ROOM_MANAGEKEY_KEY.counter;
		inactive.focus = 0;
	} else {
		bui_room_pop(ctx, &inactive, sizeof(inactive));
		bui_room_alloc(ctx, sizeof(app_room_managekey_active_t));
		if (!APP_ROOM_MANAGEKEY_KEY.exists) {
			bui_room_exit(ctx);
			return;
		}
		if (APP_ROOM_MANAGEKEY_KEY.counter != APP_ROOM_MANAGEKEY_PERSIST.counter) {
			app_key_set_counter(APP_ROOM_MANAGEKEY_PERSIST.key_i, APP_ROOM_MANAGEKEY_PERSIST.counter);
		} else if (!app_key_has_name(APP_ROOM_MANAGEKEY_PERSIST.key_i, APP_ROOM_MANAGEKEY_PERSIST.name_buff,
				APP_ROOM_MANAGEKEY_PERSIST.name_size)) {
			if (APP_ROOM_MANAGEKEY_PERSIST.name_size == 0) {
				app_key_set_name(APP_ROOM_MANAGEKEY_PERSIST.key_i, "Unnamed Key", 11);
				APP_ROOM_MANAGEKEY_PERSIST.name_size = 11;
				os_memcpy(APP_ROOM_MANAGEKEY_PERSIST.name_buff, "Unnamed Key", 11);
			} else {
				app_key_set_name(APP_ROOM_MANAGEKEY_PERSIST.key_i, APP_ROOM_MANAGEKEY_PERSIST.name_buff,
						APP_ROOM_MANAGEKEY_PERSIST.name_size);
			}
		}
	}
	APP_ROOM_MANAGEKEY_ACTIVE.menu.elem_size_callback = app_room_managekey_elem_size;
	APP_ROOM_MANAGEKEY_ACTIVE.menu.elem_draw_callback = app_room_managekey_elem_draw;
	APP_ROOM_MANAGEKEY_ACTIVE.has_auth_code = false;
	bui_menu_init(&APP_ROOM_MANAGEKEY_ACTIVE.menu, 6, inactive.focus, true);
	app_disp_invalidate();
}

static void app_room_managekey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	if (up) {
		app_room_managekey_inactive_t inactive;
		inactive.focus = bui_menu_get_focused(&APP_ROOM_MANAGEKEY_ACTIVE.menu);
		bui_room_dealloc(ctx, sizeof(app_room_managekey_active_t));
		bui_room_push(ctx, &inactive, sizeof(inactive));
	} else {
		bui_room_dealloc_frame(ctx);
	}
}

static void app_room_managekey_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	if (bui_menu_animate(&APP_ROOM_MANAGEKEY_ACTIVE.menu, elapsed))
		app_disp_invalidate();
}

static void app_room_managekey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		switch (bui_menu_get_focused(&APP_ROOM_MANAGEKEY_ACTIVE.menu)) {
		case 0: {
			app_otp_6digit(APP_ROOM_MANAGEKEY_KEY.secret.buff, APP_ROOM_MANAGEKEY_KEY.secret.size,
					APP_ROOM_MANAGEKEY_KEY.counter, APP_ROOM_MANAGEKEY_ACTIVE.auth_code);
			APP_ROOM_MANAGEKEY_ACTIVE.has_auth_code = true;
			app_key_set_counter(APP_ROOM_MANAGEKEY_PERSIST.key_i, APP_ROOM_MANAGEKEY_KEY.counter + 1);
			APP_ROOM_MANAGEKEY_PERSIST.counter += 1;
			app_disp_invalidate();
		} break;
		case 1: {
			app_room_editkeyname_args_t args;
			args.name_size = &APP_ROOM_MANAGEKEY_PERSIST.name_size;
			args.name_buff = APP_ROOM_MANAGEKEY_PERSIST.name_buff;
			bui_room_enter(ctx, &app_rooms_editkeyname, &args, sizeof(args));
		} break;
		case 2: {
			app_room_editkeycounter_args_t args;
			args.counter = &APP_ROOM_MANAGEKEY_PERSIST.counter;
			bui_room_enter(ctx, &app_rooms_editkeycounter, &args, sizeof(args));
		} break;
		case 3: {
			app_room_validatekey_args_t args;
			args.key_i = APP_ROOM_MANAGEKEY_PERSIST.key_i;
			bui_room_enter(ctx, &app_rooms_validatekey, &args, sizeof(args));
		} break;
		case 4: {
			app_room_deletekey_args_t args;
			args.key_i = APP_ROOM_MANAGEKEY_PERSIST.key_i;
			bui_room_enter(ctx, &app_rooms_deletekey, &args, sizeof(args));
		} break;
		case 5:
			bui_room_exit(ctx);
			break;
		}
	} else if (left) {
		bui_menu_scroll(&APP_ROOM_MANAGEKEY_ACTIVE.menu, true);
		app_disp_invalidate();
	} else {
		bui_menu_scroll(&APP_ROOM_MANAGEKEY_ACTIVE.menu, false);
		app_disp_invalidate();
	}
}

static void app_room_managekey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_menu_draw(&APP_ROOM_MANAGEKEY_ACTIVE.menu, bui_ctx);
}

static uint8_t app_room_managekey_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	switch (i) {
		case 0: return 28;
		case 1: return 25;
		case 2: return 25;
		case 3: return 15;
		case 4: return 15;
		case 5: return 15;
	}
	// Impossible case
	return 0;
}

static void app_room_managekey_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y) {
	switch (i) {
	case 0: {
		bui_font_draw_string(bui_ctx, "Authenticate", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		char text[12];
		if (APP_ROOM_MANAGEKEY_ACTIVE.has_auth_code) {
			for (uint8_t i = 0; i < 3; i++)
				text[i] = APP_ROOM_MANAGEKEY_ACTIVE.auth_code[i];
			text[3] = ' ';
			for (uint8_t i = 3; i < 6; i++)
				text[i + 1] = APP_ROOM_MANAGEKEY_ACTIVE.auth_code[i];
			text[7] = '\0';
		} else {
			os_memcpy(text, "- - - - - -", 12);
		}
		bui_font_draw_string(bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	} break;
	case 1: {
		bui_font_draw_string(bui_ctx, "Key Name:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		char text[APP_KEY_NAME_MAX + 1];
		os_memcpy(text, APP_ROOM_MANAGEKEY_PERSIST.name_buff, APP_ROOM_MANAGEKEY_PERSIST.name_size);
		text[APP_ROOM_MANAGEKEY_PERSIST.name_size] = '\0';
		bui_font_draw_string(bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
	} break;
	case 2: {
		bui_font_draw_string(bui_ctx, "Key Counter:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		char text[20 + 1];
		text[app_dec_encode(APP_ROOM_MANAGEKEY_PERSIST.counter, text)] = '\0';
		bui_font_draw_string(bui_ctx, text, 64, y + 15, BUI_DIR_TOP, bui_font_lucida_console_8);
	} break;
	case 3:
		bui_font_draw_string(bui_ctx, "Validate Key", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 4:
		bui_font_draw_string(bui_ctx, "Delete Key", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 5:
		bui_font_draw_string(bui_ctx, "Back", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	}
}
