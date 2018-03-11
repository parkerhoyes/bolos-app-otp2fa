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

#include <stdbool.h>
#include <stdint.h>

#include "os.h"
#include "os_io_seproxyhal.h"

#include "app.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

#define CLA 0xE0
#define INS_MAGIC 0x02
#define INS_SET_TIME 0x04

#define PROTO_HOST_MAGIC 0x72A5F76C
#define PROTO_DEVICE_MAGIC 0xF2D17183

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

			if (G_io_apdu_buffer[0] != CLA) {
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
			case INS_MAGIC:
				if (G_io_apdu_buffer[2] != 0x00 || G_io_apdu_buffer[3] != 0x00)
					THROW(0x6A86); // Incorrect parameters
				if (rx != 9)
					THROW(0x6700); // Incorrect length
				for (uint8_t i = 0; i < 4; i++) {
					if (G_io_apdu_buffer[5 + i] != (((uint32_t) PROTO_HOST_MAGIC >> (24 - 8 * i)) & 0xFF))
						THROW(0x6A80); // Invalid host magic
				}
				for (uint8_t i = 0; i < 4; i++)
					G_io_apdu_buffer[tx++] = ((uint64_t) PROTO_DEVICE_MAGIC >> (24 - 8 * i)) & 0xFF;
				THROW(0x9000);
			case INS_SET_TIME:
				if (G_io_apdu_buffer[2] != 0x00 || G_io_apdu_buffer[3] != 0x00)
					THROW(0x6A86); // Incorrect parameters
				if (rx != 17)
					THROW(0x6700); // Incorrect length
				uint64_t secs = 0;
				for (uint8_t i = 0; i < 8; i++) {
					secs <<= 8;
					secs |= G_io_apdu_buffer[5 + i];
				}
				int32_t offset = 0;
				for (uint8_t i = 0; i < 4; i++) {
					offset <<= 8;
					offset |= G_io_apdu_buffer[13 + i];
				}
				if (secs > 0x00000007FFFFFFFF)
					THROW(0x6A80); // Incorrect time data
				if (offset < 0 && -offset > (int64_t) secs)
					THROW(0x6A80); // Incorrect time data
				if ((offset < 0 ? -offset : offset) > 86400)
					THROW(0x6A80); // Incorrect time data
				app_set_time(secs, offset);
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

unsigned char io_event(unsigned char channel) {
	// Pass the event on to the app for handling
	app_io_event();

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

		USB_power(1);

		app_init();

		sample_main();
	} CATCH_OTHER(e) {} FINALLY {}
	} END_TRY;
}
