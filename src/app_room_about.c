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

#define APP_ROOM_ABOUT_ACTIVE (*((app_room_about_active_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_about_active_t {
	bui_menu_menu_t menu;
} app_room_about_active_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_about_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_about_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_about_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_about_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_about_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

static uint8_t app_room_about_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_about_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_about = {
	.enter = app_room_about_enter,
	.exit = app_room_about_exit,
	.tick = app_room_about_tick,
	.button = app_room_about_button,
	.draw = app_room_about_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_about_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_alloc(ctx, sizeof(app_room_about_active_t));
	APP_ROOM_ABOUT_ACTIVE.menu.elem_size_callback = app_room_about_elem_size;
	APP_ROOM_ABOUT_ACTIVE.menu.elem_draw_callback = app_room_about_elem_draw;
	bui_menu_init(&APP_ROOM_ABOUT_ACTIVE.menu, 3, 0, true);
	app_disp_invalidate();
}

static void app_room_about_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_dealloc_frame(ctx);
}

static void app_room_about_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	if (bui_menu_animate(&APP_ROOM_ABOUT_ACTIVE.menu, elapsed))
		app_disp_invalidate();
}

static void app_room_about_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		switch (bui_menu_get_focused(&APP_ROOM_ABOUT_ACTIVE.menu)) {
		case 2:
			bui_room_exit(ctx);
			break;
		}
	} else if (left) {
		bui_menu_scroll(&APP_ROOM_ABOUT_ACTIVE.menu, true);
		app_disp_invalidate();
	} else {
		bui_menu_scroll(&APP_ROOM_ABOUT_ACTIVE.menu, false);
		app_disp_invalidate();
	}
}

static void app_room_about_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_menu_draw(&APP_ROOM_ABOUT_ACTIVE.menu, bui_ctx);
}

static uint8_t app_room_about_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	switch (i) {
		case 0: return 28;
		case 1: return 32;
		case 2: return 15;
	}
	// Impossible case
	return 0;
}

static void app_room_about_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y) {
	switch (i) {
	case 0:
		bui_font_draw_string(bui_ctx, "App Version:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		{
			// TODO This wouldn't work with multi-digit version number components
			char ver_text[7];
			ver_text[0] = 'v';
			ver_text[1] = '0' + APP_VER_MAJOR;
			ver_text[2] = '.';
			ver_text[3] = '0' + APP_VER_MINOR;
			ver_text[4] = '.';
			ver_text[5] = '0' + APP_VER_PATCH;
			ver_text[6] = '\0';
			bui_font_draw_string(bui_ctx, ver_text, 64, y + 15, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		}
		break;
	case 1:
		bui_font_draw_string(bui_ctx, "Developed By:", 64, y + 1, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		bui_font_draw_string(bui_ctx, "Parker Hoyes", 64, y + 13, BUI_DIR_TOP, bui_font_lucida_console_8);
		bui_font_draw_string(bui_ctx, "parkerhoyes.com", 64, y + 22, BUI_DIR_TOP, bui_font_lucida_console_8);
		break;
	case 2:
		bui_font_draw_string(bui_ctx, "Back", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	}
}
