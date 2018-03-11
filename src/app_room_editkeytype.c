/*
 * License for the BOLOS OTP 2FA Application project, originally found here:
 * https://github.com/parkerhoyes/bolos-app-otp2fa
 *
 * Copyright (C) 2018 Parker Hoyes <contact@parkerhoyes.com>
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

#define APP_ROOM_EDITKEYTYPE_ACTIVE (*((app_room_editkeytype_active_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_EDITKEYTYPE_ARGS (*((app_room_editkeytype_args_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_editkeytype_active_t {
	bui_menu_menu_t menu;
} app_room_editkeytype_active_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeytype_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event);

static void app_room_editkeytype_enter(bool up);
static void app_room_editkeytype_exit(bool up);
static void app_room_editkeytype_draw();
static void app_room_editkeytype_time_elapsed(uint32_t elapsed);
static void app_room_editkeytype_button_clicked(bui_button_id_t button);

static uint8_t app_room_editkeytype_elem_size(const bui_menu_menu_t *menu, uint8_t i);
static void app_room_editkeytype_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_editkeytype = {
	.event_handler = app_room_editkeytype_handle_event,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_editkeytype_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event) {
	switch (event->id) {
	case BUI_ROOM_EVENT_ENTER: {
		bool up = BUI_ROOM_EVENT_DATA_ENTER(event)->up;
		app_room_editkeytype_enter(up);
	} break;
	case BUI_ROOM_EVENT_EXIT: {
		bool up = BUI_ROOM_EVENT_DATA_EXIT(event)->up;
		app_room_editkeytype_exit(up);
	} break;
	case BUI_ROOM_EVENT_DRAW: {
		app_room_editkeytype_draw();
	} break;
	case BUI_ROOM_EVENT_FORWARD: {
		const bui_event_t *bui_event = BUI_ROOM_EVENT_DATA_FORWARD(event);
		switch (bui_event->id) {
		case BUI_EVENT_TIME_ELAPSED: {
			uint32_t elapsed = BUI_EVENT_DATA_TIME_ELAPSED(bui_event)->elapsed;
			app_room_editkeytype_time_elapsed(elapsed);
		} break;
		case BUI_EVENT_BUTTON_CLICKED: {
			bui_button_id_t button = BUI_EVENT_DATA_BUTTON_CLICKED(bui_event)->button;
			app_room_editkeytype_button_clicked(button);
		} break;
		// Other events are acknowledged
		default:
			break;
		}
	} break;
	}
}

static void app_room_editkeytype_enter(bool up) {
	bui_room_alloc(&app_room_ctx, sizeof(app_room_editkeytype_active_t));
	APP_ROOM_EDITKEYTYPE_ACTIVE.menu.elem_size_callback = app_room_editkeytype_elem_size;
	APP_ROOM_EDITKEYTYPE_ACTIVE.menu.elem_draw_callback = app_room_editkeytype_elem_draw;
	uint8_t focus = *APP_ROOM_EDITKEYTYPE_ARGS.type == APP_KEY_TYPE_TOTP ? 1 : 2;
	bui_menu_init(&APP_ROOM_EDITKEYTYPE_ACTIVE.menu, 3, focus, true);
	app_disp_invalidate();
}

static void app_room_editkeytype_exit(bool up) {
	bui_room_dealloc_frame(&app_room_ctx);
}

static void app_room_editkeytype_draw() {
	bui_menu_draw(&APP_ROOM_EDITKEYTYPE_ACTIVE.menu, &app_bui_ctx);
}

static void app_room_editkeytype_time_elapsed(uint32_t elapsed) {
	if (bui_menu_animate(&APP_ROOM_EDITKEYTYPE_ACTIVE.menu, elapsed))
		app_disp_invalidate();
}

static void app_room_editkeytype_button_clicked(bui_button_id_t button) {
	switch (button) {
	case BUI_BUTTON_NANOS_BOTH: {
		uint16_t focus = bui_menu_get_focused(&APP_ROOM_EDITKEYTYPE_ACTIVE.menu);
		if (focus == 0)
			break;
		*APP_ROOM_EDITKEYTYPE_ARGS.type = focus == 1 ? APP_KEY_TYPE_TOTP : APP_KEY_TYPE_HOTP;
		bui_room_exit(&app_room_ctx);
	} break;
	case BUI_BUTTON_NANOS_LEFT:
		bui_menu_scroll(&APP_ROOM_EDITKEYTYPE_ACTIVE.menu, true);
		app_disp_invalidate();
		break;
	case BUI_BUTTON_NANOS_RIGHT:
		bui_menu_scroll(&APP_ROOM_EDITKEYTYPE_ACTIVE.menu, false);
		app_disp_invalidate();
		break;
	}
}

static uint8_t app_room_editkeytype_elem_size(const bui_menu_menu_t *menu, uint8_t i) {
	switch (i) {
		case 0: return 15;
		case 1: return 10;
		case 2: return 10;
	}
	// Impossible case
	return 0;
}

static void app_room_editkeytype_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_ctx_t *bui_ctx, int16_t y) {
	switch (i) {
	case 0:
		bui_font_draw_string(&app_bui_ctx, "Key Type:", 64, y + 2, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
		break;
	case 1:
		bui_font_draw_string(&app_bui_ctx, "TOTP (time-based)", 64, y + 1, BUI_DIR_TOP, bui_font_lucida_console_8);
		break;
	case 2:
		bui_font_draw_string(&app_bui_ctx, "HOTP (counter-based)", 64, y + 1, BUI_DIR_TOP, bui_font_lucida_console_8);
		break;
	}
}
