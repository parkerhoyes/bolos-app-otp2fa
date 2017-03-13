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

#define APP_ROOM_EDITKEYCOUNTER_ACTIVE (*((app_room_editkeycounter_active_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_EDITKEYCOUNTER_ARGS (*((app_room_editkeycounter_args_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_editkeycounter_active_t {
	bui_bkb_bkb_t bkb;
	char counter_buff[20];
} app_room_editkeycounter_active_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeycounter_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_editkeycounter_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_editkeycounter_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed);
static void app_room_editkeycounter_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_editkeycounter_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_editkeycounter = {
	.enter = app_room_editkeycounter_enter,
	.exit = app_room_editkeycounter_exit,
	.tick = app_room_editkeycounter_tick,
	.button = app_room_editkeycounter_button,
	.draw = app_room_editkeycounter_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeycounter_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_alloc(ctx, sizeof(app_room_editkeycounter_active_t));
	uint8_t size = app_dec_encode(*APP_ROOM_EDITKEYCOUNTER_ARGS.counter, APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff);
	bui_bkb_init(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, bui_bkb_layout_numeric, sizeof(bui_bkb_layout_numeric),
			APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff, size, sizeof(APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff),
			true);
	app_disp_invalidate();
}

static void app_room_editkeycounter_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	uint8_t size = bui_bkb_get_type_buff_size(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb);
	*APP_ROOM_EDITKEYCOUNTER_ARGS.counter = app_dec_decode(APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff, size);
	bui_room_dealloc_frame(ctx);
}

static void app_room_editkeycounter_tick(bui_room_ctx_t *ctx, bui_room_t *room, uint32_t elapsed) {
	if (bui_bkb_animate(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, elapsed))
		app_disp_invalidate();
}

static void app_room_editkeycounter_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right) {
		bui_room_exit(ctx);
	} else if (left) {
		bui_bkb_choose(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, BUI_DIR_LEFT);
		app_disp_invalidate();
	} else {
		bui_bkb_choose(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, BUI_DIR_RIGHT);
		app_disp_invalidate();
	}
}

static void app_room_editkeycounter_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_bkb_draw(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, bui_ctx);
}
