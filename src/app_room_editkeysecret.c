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

#include "bui.h"
#include "bui_bkb.h"
#include "bui_room.h"

#include "app.h"

#define APP_ROOM_EDITKEYSECRET_ACTIVE (*((app_room_editkeysecret_active_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_EDITKEYSECRET_ARGS (*((app_room_editkeysecret_args_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_editkeysecret_active_t {
	bui_bkb_bkb_t bkb;
} app_room_editkeysecret_active_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeysecret_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_editkeysecret_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static bool app_room_editkeysecret_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_editkeysecret_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_editkeysecret_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_bitmap_128x32_t *buffer);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_editkeysecret = {
	.enter = app_room_editkeysecret_enter,
	.exit = app_room_editkeysecret_exit,
	.tick = app_room_editkeysecret_tick,
	.button = app_room_editkeysecret_button,
	.draw = app_room_editkeysecret_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeysecret_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_alloc(ctx, sizeof(app_room_editkeysecret_active_t));
	bui_bkb_init(&APP_ROOM_EDITKEYSECRET_ACTIVE.bkb, app_base32_chars, sizeof(app_base32_chars),
			APP_ROOM_EDITKEYSECRET_ARGS.secret_buff, *APP_ROOM_EDITKEYSECRET_ARGS.secret_size,
			APP_KEY_SECRET_ENCODED_MAX, true);
	app_disp_invalidate();
}

static void app_room_editkeysecret_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	*APP_ROOM_EDITKEYSECRET_ARGS.secret_size = bui_bkb_get_type_buff_size(&APP_ROOM_EDITKEYSECRET_ACTIVE.bkb);
	bui_room_dealloc_frame(ctx);
}

static bool app_room_editkeysecret_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	return bui_bkb_animate(&APP_ROOM_EDITKEYSECRET_ACTIVE.bkb, elapsed);
}

static void app_room_editkeysecret_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		bui_room_exit(ctx);
	} else if (left) {
		bui_bkb_choose(&APP_ROOM_EDITKEYSECRET_ACTIVE.bkb, BUI_DIR_LEFT);
		app_disp_invalidate();
	} else {
		bui_bkb_choose(&APP_ROOM_EDITKEYSECRET_ACTIVE.bkb, BUI_DIR_RIGHT);
		app_disp_invalidate();
	}
}

static void app_room_editkeysecret_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_bitmap_128x32_t *buffer) {
	bui_bkb_draw(&APP_ROOM_EDITKEYSECRET_ACTIVE.bkb, buffer);
}
