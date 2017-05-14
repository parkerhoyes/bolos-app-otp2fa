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

#include <stdbool.h>

#include "os.h"
#include "os_io_seproxyhal.h"

#include "app.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

void sample_main() {
	volatile unsigned int rx = 0;
	volatile unsigned int tx = 0;
	volatile unsigned int flags = 0;

	while (true) {
		volatile unsigned short sw = 0;

		BEGIN_TRY {
		TRY {
			rx = tx;
			tx = 0; // Ensure no race in catch_other if io_exchange throws an error
			rx = io_exchange(CHANNEL_APDU | flags, rx);
			flags = 0;

			// No APDU received, well, reset the session, and reset the bootloader configuration
			if (rx == 0) {
				THROW(0x6982);
			}

			if (G_io_apdu_buffer[0] != 0x80) {
				THROW(0x6E00);
			}

			// Unauthenticated instruction
			switch (G_io_apdu_buffer[1]) {
			case 0x00: // Reset
				flags |= IO_RESET_AFTER_REPLIED;
				THROW(0x9000);
				break;
			case 0x01: // Case 1
				THROW(0x9000);
				break;
			case 0x02: // Echo
				tx = rx;
				THROW(0x9000);
				break;
			case 0xFF: // Return to dashboard
				goto return_to_dashboard;
			default:
				THROW(0x6D00);
				break;
			}
		} CATCH_OTHER(e) {
			switch (e & 0xF000) {
			case 0x6000:
			case 0x9000:
				sw = e;
				break;
			default:
				sw = 0x6800 | (e & 0x7FF);
				break;
			}
			// Unexpected exception => report
			G_io_apdu_buffer[tx] = sw >> 8;
			G_io_apdu_buffer[tx + 1] = sw;
			tx += 2;
		} FINALLY {}
		} END_TRY;
	}

return_to_dashboard:
	return;
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
	switch (channel & ~(IO_FLAGS)) {
	case CHANNEL_KEYBOARD:
		break;
	// Multiplexed io exchange over a SPI channel and TLV encapsulated protocol
	case CHANNEL_SPI:
		if (tx_len) {
			io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

			if (channel & IO_RESET_AFTER_REPLIED) {
				reset();
			}
			return 0; // Nothing received from the master so far (it's a tx transaction)
		} else {
			return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
		}
	default:
		THROW(INVALID_PARAMETER);
	}
	return 0;
}

unsigned int handle_button_push(unsigned int button_mask, unsigned int button_mask_counter) {
	app_event_button_push(button_mask, button_mask_counter);
	return 0;
}

unsigned char io_event(unsigned char channel) {
	switch (G_io_seproxyhal_spi_buffer[0]) {
	case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
		io_seproxyhal_button_push(&handle_button_push, G_io_seproxyhal_spi_buffer[3] >> 1);
		break;
	case SEPROXYHAL_TAG_TICKER_EVENT:
		app_event_ticker();
		break;
	case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
		app_event_display_processed();
		break;
	// Unknown events are acknowledged
	default:
		break;
	}

	// Close the event if not done previously (by a display or whatever)
	if (!io_seproxyhal_spi_is_status_sent()) {
		io_seproxyhal_general_status();
	}

	// Command has been processed, DO NOT reset the current APDU transport
	return 1;
}

__attribute__((section(".boot"))) int main() {
	// Exit critical section
	__asm volatile("cpsie i");

	// Ensure exception will work as planned
	os_boot();

	BEGIN_TRY {
	TRY {
		io_seproxyhal_init();

		app_init();

		sample_main();
	} CATCH_OTHER(e) {} FINALLY {}
	} END_TRY;
}
