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
#include "app_otp.h"

#define APP_ROOM_VALIDATEKEY_ARGS (*((app_room_validatekey_args_t*) app_room_ctx.frame_ptr))
#define APP_ROOM_VALIDATEKEY_ACTIVE (*((app_room_validatekey_active_t*) app_room_ctx.stack_ptr - 1))
#define APP_ROOM_VALIDATEKEY_KEY (*app_get_key(APP_ROOM_VALIDATEKEY_ARGS.key_i))

//----------------------------------------------------------------------------//
//                                                                            //
//                  Internal Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_validatekey_active_t {
	char auth_code[6]; // The 6-digit OTP code as a string
} app_room_validatekey_active_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_validatekey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_validatekey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up);
static void app_room_validatekey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right);
static void app_room_validatekey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx);

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

const bui_room_t app_rooms_validatekey = {
	.enter = app_room_validatekey_enter,
	.exit = app_room_validatekey_exit,
	.tick = NULL,
	.button = app_room_validatekey_button,
	.draw = app_room_validatekey_draw,
};

//----------------------------------------------------------------------------//
//                                                                            //
//                       Internal Function Definitions                        //
//                                                                            //
//----------------------------------------------------------------------------//

static void app_room_validatekey_enter(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_alloc(ctx, sizeof(app_room_validatekey_active_t));
	const app_key_t *key = &APP_ROOM_VALIDATEKEY_KEY;
	app_otp_6digit(key->secret.buff, key->secret.size, 0, APP_ROOM_VALIDATEKEY_ACTIVE.auth_code);
	app_disp_invalidate();
}

static void app_room_validatekey_exit(bui_room_ctx_t *ctx, bui_room_t *room, bool up) {
	bui_room_dealloc_frame(ctx);
}

static void app_room_validatekey_button(bui_room_ctx_t *ctx, bui_room_t *room, bool left, bool right) {
	if (left && right)
		bui_room_exit(ctx);
}

static void app_room_validatekey_draw(bui_room_ctx_t *ctx, const bui_room_t *room, bui_ctx_t *bui_ctx) {
	{
		char otp_text[8];
		for (uint8_t i = 0; i < 3; i++)
			otp_text[i] = APP_ROOM_VALIDATEKEY_ACTIVE.auth_code[i];
		otp_text[3] = ' ';
		for (uint8_t i = 3; i < 6; i++)
			otp_text[i + 1] = APP_ROOM_VALIDATEKEY_ACTIVE.auth_code[i];
		otp_text[7] = '\0';
		bui_font_draw_string(bui_ctx, otp_text, 64, 6, BUI_DIR_TOP, bui_font_open_sans_extrabold_11);
	}
	{
		char name_text[APP_KEY_NAME_MAX + 1];
		const app_key_t *key = &APP_ROOM_VALIDATEKEY_KEY;
		os_memcpy(name_text, key->name.buff, key->name.size);
		name_text[key->name.size] = '\0';
		bui_font_draw_string(bui_ctx, name_text, 64, 26, BUI_DIR_BOTTOM, bui_font_lucida_console_8);
	}
}
