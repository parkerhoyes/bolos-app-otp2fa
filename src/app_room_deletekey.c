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

#define APP_ROOM_DELETEKEY_ARGS (*((app_room_deletekey_args_t*) app_room_ctx.frame_ptr))
#define APP_ROOM_DELETEKEY_KEY (N_app_persist.keys[APP_ROOM_DELETEKEY_ARGS.key_i])

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_deletekey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_deletekey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_deletekey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_deletekey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_bitmap_128x32_t *buffer);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_deletekey = {
	.enter = app_room_deletekey_enter,
	.exit = app_room_deletekey_exit,
	.tick = NULL,
	.button = app_room_deletekey_button,
	.draw = app_room_deletekey_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_deletekey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	app_disp_invalidate();
}

static void app_room_deletekey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_dealloc_frame(ctx);
}

static void app_room_deletekey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left) {
		bui_room_exit(ctx);
	} else if (right) {
		app_key_delete(APP_ROOM_DELETEKEY_ARGS.key_i);
		bui_room_exit(ctx);
	}
}

static void app_room_deletekey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_bitmap_128x32_t *buffer) {
	bui_font_draw_string(&app_disp_buffer, "Delete Key?", 64, 6, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	char name[APP_KEY_NAME_MAX + 1];
	os_memcpy(name, APP_ROOM_DELETEKEY_KEY.name.buff, APP_ROOM_DELETEKEY_KEY.name.size);
	name[APP_ROOM_DELETEKEY_KEY.name.size] = '\0';
	bui_font_draw_string(&app_disp_buffer, name, 64, 26, BUI_DIR_BOTTOM, bui_font_lucida_console_8);
	bui_draw_bitmap(&app_disp_buffer, BUI_BITMAP_ICON_CROSS, 0, 0, 3, 12, BUI_BITMAP_ICON_CROSS.w,
			BUI_BITMAP_ICON_CROSS.h);
	bui_draw_bitmap(&app_disp_buffer, BUI_BITMAP_ICON_CHECK, 0, 0, 117, 13, BUI_BITMAP_ICON_CHECK.w,
			BUI_BITMAP_ICON_CHECK.h);
}
