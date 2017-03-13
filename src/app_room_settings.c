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

#define APP_ROOM_SETTINGS_ACTIVE (*((app_room_settings_active_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_settings_active_t {
	bui_menu_menu_t menu;
} app_room_settings_active_t;

typedef struct app_room_settings_inactive_t {
	uint8_t focus;
} app_room_settings_inactive_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_settings_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_settings_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_settings_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_settings_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_settings_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

static uint8_t app_room_settings_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_settings_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_settings = {
	.enter = app_room_settings_enter,
	.exit = app_room_settings_exit,
	.tick = app_room_settings_tick,
	.button = app_room_settings_button,
	.draw = app_room_settings_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_settings_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	app_room_settings_inactive_t inactive;
	if (up)
		inactive.focus = 0;
	else
		bui_room_pop(ctx, &inactive, sizeof(inactive));
	bui_room_alloc(ctx, sizeof(app_room_settings_active_t));
	APP_ROOM_SETTINGS_ACTIVE.menu.elem_size_callback = app_room_settings_elem_size;
	APP_ROOM_SETTINGS_ACTIVE.menu.elem_draw_callback = app_room_settings_elem_draw;
	bui_menu_init(&APP_ROOM_SETTINGS_ACTIVE.menu, 3, inactive.focus, true);
	app_disp_invalidate();
}

static void app_room_settings_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	if (up) {
		app_room_settings_inactive_t inactive;
		inactive.focus = bui_menu_get_focused(&APP_ROOM_SETTINGS_ACTIVE.menu);
		bui_room_dealloc(ctx, sizeof(app_room_settings_active_t));
		bui_room_push(ctx, &inactive, sizeof(inactive));
	} else {
		bui_room_dealloc_frame(ctx);
	}
}

static void app_room_settings_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	if (bui_menu_animate(&APP_ROOM_SETTINGS_ACTIVE.menu, elapsed))
		app_disp_invalidate();
}

static void app_room_settings_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		switch (bui_menu_get_focused(&APP_ROOM_SETTINGS_ACTIVE.menu)) {
		case 0:
			bui_room_enter(ctx, &app_rooms_reset, NULL, 0);
			break;
		case 1:
			bui_room_enter(ctx, &app_rooms_about, NULL, 0);
			break;
		case 2:
			bui_room_exit(ctx);
			break;
		}
	} else if (left) {
		bui_menu_scroll(&APP_ROOM_SETTINGS_ACTIVE.menu, true);
		app_disp_invalidate();
	} else {
		bui_menu_scroll(&APP_ROOM_SETTINGS_ACTIVE.menu, false);
		app_disp_invalidate();
	}
}

static void app_room_settings_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_menu_draw(&APP_ROOM_SETTINGS_ACTIVE.menu, bui_ctx);
}

static uint8_t app_room_settings_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	return 15;
}

static void app_room_settings_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y) {
	switch (i) {
	case 0:
		bui_font_draw_string(bui_ctx, "Reset", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 1:
		bui_font_draw_string(bui_ctx, "About", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 2:
		bui_font_draw_string(bui_ctx, "Back", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	}
}
