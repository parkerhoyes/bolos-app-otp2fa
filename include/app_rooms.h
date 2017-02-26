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

#ifndef APP_ROOMS_H_
#define APP_ROOMS_H_

/*
 * Main Menu Layout:
 * - Title Screen, full-size
 * - Keys (selecting goes to Keys Menu), full-size
 * - Settings (selecting goes to Settings Menu), full-size
 * - Quit App (selecting quits), full-size
 *
 * Keys Menu Layout:
 * - Add New Key (selecting goes to New Key Menu), full-size
 * - *list of keys* (selecting goes to Manage Key Menu for the selected key), each 10px
 * - Back (selecting goes to Main Menu, focused on Keys Menu), full-size
 *
 * New Key Menu Layout:
 * - Key Name (selecting brings up the BKB to edit the entered name), full-size
 * - Key Secret (selecting brings up the specialized BKB to edit the entered secret), full-size
 * - Done (selecting goes to Keys Menu, focused on the new key), full-size
 *
 * Manage Key Menu Layout:
 * - Authenticate (selecting displays a new code that is retained until the user exits the menu), full-size
 * - Key Name (selecting brings up the BKB to edit the entered name), full-size
 * - Edit Counter (selecting brings up the BKB to edit the counter), full-size
 * - Delete Key (selecting brings up the Delete Key screen), full-size
 * - Back (selecting goes to Keys Menu, focused on the original key), full-size
 *
 * Settings Menu Layout:
 * - Reset (selecting brings up the Reset screen), full-size
 * - Back (selecting goes to Main Menu, focused on Settings), full-size
 */

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
extern const bui_room_t app_rooms_managekey;
extern const bui_room_t app_rooms_editkeyname;
extern const bui_room_t app_rooms_editkeysecret;
extern const bui_room_t app_rooms_editkeycounter;
extern const bui_room_t app_rooms_deletekey;
extern const bui_room_t app_rooms_settings;
extern const bui_room_t app_rooms_reset;

#endif
