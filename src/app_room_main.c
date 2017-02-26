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

#include "bui.h"
#include "bui_font.h"
#include "bui_menu.h"
#include "bui_room.h"

#include "app.h"

#define APP_ROOM_MAIN_ACTIVE (*((app_room_main_active_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_main_active_t {
	bui_menu_menu_t menu;
} app_room_main_active_t;

typedef struct app_room_main_inactive_t {
	uint8_t focus;
} app_room_main_inactive_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                Internal Variable Declarations & Definitions                //
//                                                                            //
//----------------------------------------------------------------------------//

static const uint8_t app_room_main_bitmap_icon_bitmap[] = {
	0x00, 0x00, 0x0F, 0xF0, 0x3F, 0xFC, 0x20, 0x04,
	0x60, 0x06, 0x60, 0x86, 0x61, 0x86, 0x60, 0x06,
	0x60, 0x06, 0x73, 0xCE, 0x73, 0xCE, 0x73, 0xCE,
	0x38, 0x1C, 0x3C, 0x3C, 0x0F, 0xF0, 0x00, 0x00,
};
#define APP_ROOM_MAIN_BITMAP_ICON ((bui_const_bitmap_t) { .w = 16, .h = 16, .bb = app_room_main_bitmap_icon_bitmap })

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_main_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_main_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static bool app_room_main_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_main_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_main_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_bitmap_128x32_t *buffer);

static uint8_t app_room_main_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_main_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_bitmap_128x32_t *buffer, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_main = {
	.enter = app_room_main_enter,
	.exit = app_room_main_exit,
	.tick = app_room_main_tick,
	.button = app_room_main_button,
	.draw = app_room_main_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_main_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	app_room_main_inactive_t inactive;
	if (up)
		inactive.focus = 0;
	else
		bui_room_pop(ctx, &inactive, sizeof(inactive));
	bui_room_alloc(ctx, sizeof(app_room_main_active_t));
	APP_ROOM_MAIN_ACTIVE.menu.elem_size_callback = app_room_main_elem_size;
	APP_ROOM_MAIN_ACTIVE.menu.elem_draw_callback = app_room_main_elem_draw;
	bui_menu_init(&APP_ROOM_MAIN_ACTIVE.menu, 4, inactive.focus, true);
	app_disp_invalidate();
}

static void app_room_main_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	if (!up)
		os_sched_exit(0); // Go back to the dashboard
	app_room_main_inactive_t inactive;
	inactive.focus = bui_menu_get_focused(&APP_ROOM_MAIN_ACTIVE.menu);
	bui_room_dealloc(ctx, sizeof(app_room_main_active_t));
	bui_room_push(ctx, &inactive, sizeof(inactive));
}

static bool app_room_main_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	return bui_menu_animate(&APP_ROOM_MAIN_ACTIVE.menu, elapsed);
}

static void app_room_main_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		switch (bui_menu_get_focused(&APP_ROOM_MAIN_ACTIVE.menu)) {
		case 1:
			bui_room_enter(ctx, &app_rooms_keys, NULL, 0);
			break;
		case 2:
			bui_room_enter(ctx, &app_rooms_settings, NULL, 0);
			break;
		case 3:
			bui_room_exit(ctx);
			break;
		}
	} else if (left) {
		bui_menu_scroll(&APP_ROOM_MAIN_ACTIVE.menu, true);
		app_disp_invalidate();
	} else {
		bui_menu_scroll(&APP_ROOM_MAIN_ACTIVE.menu, false);
		app_disp_invalidate();
	}
}

static void app_room_main_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_bitmap_128x32_t *buffer) {
	bui_menu_draw(&APP_ROOM_MAIN_ACTIVE.menu, buffer);
}

static uint8_t app_room_main_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	switch (i) {
		case 0: return 20;
		case 1: return 15;
		case 2: return 15;
		case 3: return 18;
	}
	// Impossible case
	return 0;
}

static void app_room_main_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_bitmap_128x32_t *buffer, int16_t y) {
	switch (i) {
	case 0:
		bui_draw_bitmap(buffer, APP_ROOM_MAIN_BITMAP_ICON, 0, 0, 8, y + 2, APP_ROOM_MAIN_BITMAP_ICON.w,
				APP_ROOM_MAIN_BITMAP_ICON.h);
		bui_font_draw_string(buffer, "OTP 2FA App", 32, y + 10, BUI_DIR_LEFT, bui_font_open_sans_extrabold_11);
		break;
	case 1:
		bui_font_draw_string(buffer, "Manage Keys", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 2:
		bui_font_draw_string(buffer, "Settings", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 3:
		bui_draw_bitmap(buffer, BUI_BITMAP_BADGE_DASHBOARD, 0, 0, 29, y + 2, BUI_BITMAP_BADGE_DASHBOARD.w,
				BUI_BITMAP_BADGE_DASHBOARD.h);
		bui_font_draw_string(buffer, "Quit app", 52, y + 9, BUI_DIR_LEFT, bui_font_open_sans_extrabold_11);
		break;
	}
}
