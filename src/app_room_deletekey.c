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
#include "bui_font.h"
#include "bui_room.h"

#include "app.h"

#define APP_ROOM_DELETEKEY_ARGS (*((app_room_deletekey_args_t*) app_room_ctx.frame_ptr))
#define APP_ROOM_DELETEKEY_KEY (*app_get_key(APP_ROOM_DELETEKEY_ARGS.key_i))

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_deletekey_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event);

static void app_room_deletekey_enter(bool up);
static void app_room_deletekey_exit(bool up);
static void app_room_deletekey_draw();
static void app_room_deletekey_button_clicked(bui_button_id_t button);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_deletekey = {
	.event_handler = app_room_deletekey_handle_event,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_deletekey_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event) {
	switch (event->id) {
	case BUI_ROOM_EVENT_ENTER: {
		bool up = BUI_ROOM_EVENT_DATA_ENTER(event)->up;
		app_room_deletekey_enter(up);
	} break;
	case BUI_ROOM_EVENT_EXIT: {
		bool up = BUI_ROOM_EVENT_DATA_EXIT(event)->up;
		app_room_deletekey_exit(up);
	} break;
	case BUI_ROOM_EVENT_DRAW: {
		app_room_deletekey_draw();
	} break;
	case BUI_ROOM_EVENT_FORWARD: {
		const bui_event_t *bui_event = BUI_ROOM_EVENT_DATA_FORWARD(event);
		switch (bui_event->id) {
		case BUI_EVENT_BUTTON_CLICKED: {
			bui_button_id_t button = BUI_EVENT_DATA_BUTTON_CLICKED(bui_event)->button;
			app_room_deletekey_button_clicked(button);
		} break;
		// Other events are acknowledged
		default:
			break;
		}
	} break;
	}
}

static void app_room_deletekey_enter(bool up) {
	app_disp_invalidate();
}

static void app_room_deletekey_exit(bool up) {
	bui_room_dealloc_frame(&app_room_ctx);
}

static void app_room_deletekey_draw() {
	bui_font_draw_string(&app_bui_ctx, "Delete Key?", 64, 5, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	char name[APP_KEY_NAME_MAX + 1];
	os_memcpy(name, APP_ROOM_DELETEKEY_KEY.name.buff, APP_ROOM_DELETEKEY_KEY.name.size);
	name[APP_ROOM_DELETEKEY_KEY.name.size] = '\0';
	bui_font_draw_string(&app_bui_ctx, name, 64, 18, BUI_DIR_TOP, bui_font_lucida_console_8);
	bui_ctx_draw_bitmap_full(&app_bui_ctx, BUI_BMP_ICON_CROSS, 3, 12);
	bui_ctx_draw_bitmap_full(&app_bui_ctx, BUI_BMP_ICON_CHECK, 117, 13);
}

static void app_room_deletekey_button_clicked(bui_button_id_t button) {
	switch (button) {
	case BUI_BUTTON_NANOS_LEFT:
		bui_room_exit(&app_room_ctx);
		break;
	case BUI_BUTTON_NANOS_RIGHT:
		app_key_delete(APP_ROOM_DELETEKEY_ARGS.key_i);
		bui_room_exit(&app_room_ctx);
		break;
	}
}
