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

#include "bui.h"
#include "bui_font.h"
#include "bui_room.h"

#include "app.h"
#include "app_time.h"

#define APP_ROOM_VERIFYTIME_PERSIST (*((app_room_verifytime_persist_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_VERIFYTIME_ARGS (*((app_room_verifytime_args_t*) app_room_ctx.frame_ptr))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_verifytime_persist_t {
	char msg[60];
} app_room_verifytime_persist_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_verifytime_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event);

static void app_room_verifytime_enter(bool up);
static void app_room_verifytime_exit(bool up);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_verifytime = {
	.event_handler = app_room_verifytime_handle_event,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_verifytime_handle_event(bui_room_ctx_t *ctx, const bui_room_event_t *event) {
	switch (event->id) {
	case BUI_ROOM_EVENT_ENTER: {
		bool up = BUI_ROOM_EVENT_DATA_ENTER(event)->up;
		app_room_verifytime_enter(up);
	} break;
	case BUI_ROOM_EVENT_EXIT: {
		bool up = BUI_ROOM_EVENT_DATA_EXIT(event)->up;
		app_room_verifytime_exit(up);
	} break;
	// Other events are acknowledged
	default:
		break;
	}
}

static void app_room_verifytime_enter(bool up) {
	if (up) {
		bui_room_alloc(&app_room_ctx, sizeof(app_room_verifytime_persist_t));
		app_time_t timedata;
		app_time_localtime(*APP_ROOM_VERIFYTIME_ARGS.secs, APP_ROOM_VERIFYTIME_ARGS.offset, &timedata);
		uint8_t i = 0;
		os_memcpy(&APP_ROOM_VERIFYTIME_PERSIST.msg[i], "Is this the correct time?\n", 26);
		i += 26;
		i += app_time_format_date(&timedata, &APP_ROOM_VERIFYTIME_PERSIST.msg[i], 15);
		APP_ROOM_VERIFYTIME_PERSIST.msg[i++] = '\n';
		i += app_time_format_time(&timedata, &APP_ROOM_VERIFYTIME_PERSIST.msg[i], 9);
		APP_ROOM_VERIFYTIME_PERSIST.msg[i++] = ' ';
		i += app_time_format_timezone(&timedata, &APP_ROOM_VERIFYTIME_PERSIST.msg[i], 10);
		// No need to add a null terminator because the previous call already did
		bui_room_confirm_args_t args = {
			.msg = APP_ROOM_VERIFYTIME_PERSIST.msg,
			.font = bui_font_lucida_console_8,
		};
		app_disp_invalidate();
		bui_room_enter(&app_room_ctx, &bui_room_confirm, &args, sizeof(args));
	} else {
		bui_room_confirm_ret_t ret;
		bui_room_pop(&app_room_ctx, &ret, sizeof(ret));
		*APP_ROOM_VERIFYTIME_ARGS.time_verified = ret.confirmed;
		bui_room_exit(&app_room_ctx);
	}
}

static void app_room_verifytime_exit(bool up) {
	if (!up)
		bui_room_dealloc_frame(&app_room_ctx);
}
