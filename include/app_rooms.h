/*
 * License for the BOLOS OTP 2FA Application project, originally found here:
 * https://github.com/parkerhoyes/bolos-app-otp2fa
 *
 * Copyright (C) 2017, 2018 Parker Hoyes <contact@parkerhoyes.com>
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

#ifndef APP_ROOMS_H_
#define APP_ROOMS_H_

#include <stdint.h>

#include "bui_room.h"

#include "app.h"

//----------------------------------------------------------------------------//
//                                                                            //
//                  External Type Declarations & Definitions                  //
//                                                                            //
//----------------------------------------------------------------------------//

typedef struct app_room_managekey_args_t {
	uint8_t key_i; // The index of the key to be managed in N_app_persist.keys
} app_room_managekey_args_t;

typedef struct __attribute__((aligned(4))) app_room_verifytime_args_t {
	const uint64_t *secs;
	bool *time_verified;
	int32_t offset;
} app_room_verifytime_args_t;

typedef struct __attribute__((aligned(4))) app_room_editkeytype_args_t {
	app_key_type_t *type;
} app_room_editkeytype_args_t;

typedef struct __attribute__((aligned(4))) app_room_editkeyname_args_t {
	uint8_t *name_size;
	char *name_buff;
} app_room_editkeyname_args_t;

typedef struct __attribute__((aligned(4))) app_room_editkeysecret_args_t {
	uint8_t *secret_size;
	char *secret_buff; // Stores the secret encoded in base-32
} app_room_editkeysecret_args_t;

typedef struct __attribute__((aligned(4))) app_room_editkeycounter_args_t {
	uint64_t *counter;
} app_room_editkeycounter_args_t;

typedef struct __attribute__((aligned(4))) app_room_validatekey_args_t {
	uint8_t key_i; // The index of the key to be validated
} app_room_validatekey_args_t;

typedef struct app_room_deletekey_args_t {
	uint8_t key_i; // The index of the key to be deleted, if the user confirms the deletion
} app_room_deletekey_args_t;

//----------------------------------------------------------------------------//
//                                                                            //
//                       External Variable Declarations                       //
//                                                                            //
//----------------------------------------------------------------------------//

extern const bui_room_t app_rooms_main;
extern const bui_room_t app_rooms_keys;
extern const bui_room_t app_rooms_newkey;
extern const bui_room_t app_rooms_keysfull;
extern const bui_room_t app_rooms_managekey;
extern const bui_room_t app_rooms_verifytime;
extern const bui_room_t app_rooms_editkeytype;
extern const bui_room_t app_rooms_editkeyname;
extern const bui_room_t app_rooms_editkeysecret;
extern const bui_room_t app_rooms_editkeycounter;
extern const bui_room_t app_rooms_validatekey;
extern const bui_room_t app_rooms_deletekey;
extern const bui_room_t app_rooms_settings;
extern const bui_room_t app_rooms_reset;
extern const bui_room_t app_rooms_about;

#endif
