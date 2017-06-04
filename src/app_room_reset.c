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

static void app_room_reset_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event);

static void app_room_reset_enter(bool up);
static void app_room_reset_draw();
static void app_room_reset_button_clicked(bui_button_id_t button);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_reset = {
	.event_handler = app_room_reset_handle_event,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_reset_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event) {
	switch (event->id) {
	case BUI_ROOM_EVENT_ENTER: {
		bool up = BUI_ROOM_EVENT_DATA_ENTER(event)->up;
		app_room_reset_enter(up);
	} break;
	case BUI_ROOM_EVENT_DRAW: {
		app_room_reset_draw();
	} break;
	case BUI_ROOM_EVENT_FORWARD: {
		const bui_event_t *bui_event = BUI_ROOM_EVENT_DATA_FORWARD(event);
		switch (bui_event->id) {
		case BUI_EVENT_BUTTON_CLICKED: {
			bui_button_id_t button = BUI_EVENT_DATA_BUTTON_CLICKED(bui_event)->button;
			app_room_reset_button_clicked(button);
		} break;
		// Other events are acknowledged
		default:
			break;
		}
	} break;
	// Other events are acknowledged
	default:
		break;
	}
}

static void app_room_reset_enter(bool up) {
	app_disp_invalidate();
}

static void app_room_reset_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	bui_font_draw_string(&app_bui_ctx, "Reset all", 64, 4, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	bui_font_draw_string(&app_bui_ctx, "app data?", 64, 17, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	bui_ctx_draw_bitmap_full(&app_bui_ctx, BUI_BMP_ICON_CROSS, 3, 12);
	bui_ctx_draw_bitmap_full(&app_bui_ctx, BUI_BMP_ICON_CHECK, 117, 13);
}

static void app_room_reset_button_clicked(bui_button_id_t button) {
	switch (button) {
	case BUI_BUTTON_NANOS_LEFT:
		bui_room_exit(&app_room_ctx);
		break;
	case BUI_BUTTON_NANOS_RIGHT:
		app_persist_wipe();
		bui_room_exit(&app_room_ctx);
		break;
	}
}
