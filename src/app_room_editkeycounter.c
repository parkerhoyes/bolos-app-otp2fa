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

static void app_room_editkeycounter_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event);

static void app_room_editkeycounter_enter(bool up);
static void app_room_editkeycounter_exit(bool up);
static void app_room_editkeycounter_draw();
static void app_room_editkeycounter_time_elapsed(uint32_t elapsed);
static void app_room_editkeycounter_button_clicked(bui_button_id_t button);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_editkeycounter = {
	.event_handler = app_room_editkeycounter_handle_event,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeycounter_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event) {
	switch (event->id) {
	case BUI_ROOM_EVENT_ENTER: {
		bool up = BUI_ROOM_EVENT_DATA_ENTER(event)->up;
		app_room_editkeycounter_enter(up);
	} break;
	case BUI_ROOM_EVENT_EXIT: {
		bool up = BUI_ROOM_EVENT_DATA_EXIT(event)->up;
		app_room_editkeycounter_exit(up);
	} break;
	case BUI_ROOM_EVENT_DRAW: {
		app_room_editkeycounter_draw();
	} break;
	case BUI_ROOM_EVENT_FORWARD: {
		const bui_event_t *bui_event = BUI_ROOM_EVENT_DATA_FORWARD(event);
		switch (bui_event->id) {
		case BUI_EVENT_TIME_ELAPSED: {
			uint32_t elapsed = BUI_EVENT_DATA_TIME_ELAPSED(bui_event)->elapsed;
			app_room_editkeycounter_time_elapsed(elapsed);
		} break;
		case BUI_EVENT_BUTTON_CLICKED: {
			bui_button_id_t button = BUI_EVENT_DATA_BUTTON_CLICKED(bui_event)->button;
			app_room_editkeycounter_button_clicked(button);
		} break;
		// Other events are acknowledged
		default:
			break;
		}
	} break;
	}
}

static void app_room_editkeycounter_enter(bool up) {
	bui_room_alloc(&app_room_ctx, sizeof(app_room_editkeycounter_active_t));
	uint8_t size = app_dec_encode(*APP_ROOM_EDITKEYCOUNTER_ARGS.counter, APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff);
	bui_bkb_init(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, bui_bkb_layout_numeric, sizeof(bui_bkb_layout_numeric),
			APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff, size, sizeof(APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff),
			true);
	app_disp_invalidate();
}

static void app_room_editkeycounter_exit(bool up) {
	uint8_t size = bui_bkb_get_type_buff_size(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb);
	*APP_ROOM_EDITKEYCOUNTER_ARGS.counter = app_dec_decode(APP_ROOM_EDITKEYCOUNTER_ACTIVE.counter_buff, size);
	bui_room_dealloc_frame(&app_room_ctx);
}

static void app_room_editkeycounter_draw() {
	bui_bkb_draw(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, &app_bui_ctx);
}

static void app_room_editkeycounter_time_elapsed(uint32_t elapsed) {
	if (bui_bkb_animate(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, elapsed))
		app_disp_invalidate();
}

static void app_room_editkeycounter_button_clicked(bui_button_id_t button) {
	switch (button) {
	case BUI_BUTTON_NANOS_BOTH:
		bui_room_exit(&app_room_ctx);
		break;
	case BUI_BUTTON_NANOS_LEFT:
		bui_bkb_choose(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, BUI_DIR_LEFT);
		app_disp_invalidate();
		break;
	case BUI_BUTTON_NANOS_RIGHT:
		bui_bkb_choose(&APP_ROOM_EDITKEYCOUNTER_ACTIVE.bkb, BUI_DIR_RIGHT);
		app_disp_invalidate();
		break;
	}
}
