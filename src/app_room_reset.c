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

#include "bui.h"
#include "bui_bkb.h"
#include "bui_font.h"
#include "bui_room.h"

#include "app.h"

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_reset_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_reset_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_reset_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_reset = {
	.enter = app_room_reset_enter,
	.exit = NULL,
	.tick = NULL,
	.button = app_room_reset_button,
	.draw = app_room_reset_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_reset_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	app_disp_invalidate();
}

static void app_room_reset_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left) {
		bui_room_exit(ctx);
	} else if (right) {
		app_persist_wipe();
		bui_room_exit(ctx);
	}
}

static void app_room_reset_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_font_draw_string(bui_ctx, "Reset all", 64, 4, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	bui_font_draw_string(bui_ctx, "app data?", 64, 17, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	bui_ctx_draw_mbitmap_full(bui_ctx, BUI_BITMAP_ICON_CROSS, 3, 12);
	bui_ctx_draw_mbitmap_full(bui_ctx, BUI_BITMAP_ICON_CHECK, 117, 13);
}
