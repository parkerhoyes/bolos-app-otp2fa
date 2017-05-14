# License for the BOLOS OTP 2FA Application project, originally found here:
# https://github.com/parkerhoyes/bolos-app-otp2fa
#
# Copyright (C) 2017 Parker Hoyes <contact@parkerhoyes.com>
#
# This software is provided "as-is", without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the
# use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it freely,
# subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not claim
#    that you wrote the original software. If you use this software in a
#    product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

# START USER CONFIGURATION
#BOLOS_SDK :=
#BOLOS_ENV :=
# END USER CONFIGURATION

ifeq ($(BOLOS_SDK),)
$(error BOLOS_SDK is not set)
endif
$(info BOLOS_SDK=$(BOLOS_SDK))

ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
else
$(info BOLOS_ENV is undefined: clang and arm-none-eabi-* will be used from the PATH)
endif

include $(BOLOS_SDK)/Makefile.defines

# Main app configuration

APPNAME := "OTP 2FA"
APPVERSION := 1.0.0
APP_LOAD_PARAMS = --appFlags 0x00 $(COMMON_LOAD_PARAMS)

ifeq ($(TARGET_NAME),TARGET_BLUE)
ICONNAME := glyphs/app_icon_blue.gif
else
ICONNAME := glyphs/app_icon_nanos.gif
endif

# App build configuration

APP_SOURCE_PATH += src bui/src bui/include

# Main build configuration

all: default

DEFINES += OS_IO_SEPROXYHAL IO_SEPROXYHAL_BUFFER_SIZE_B=300
DEFINES += HAVE_BAGL HAVE_SPRINTF
DEFINES += PRINTF\(...\)=

DEFINES += UNUSED\(x\)=\(void\)x
DEFINES += APPVERSION=\"$(APPVERSION)\"

ifneq ($(BOLOS_ENV),)
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
else
CLANGPATH :=
GCCPATH :=
endif
CC := $(CLANGPATH)clang
AS := $(GCCPATH)arm-none-eabi-gcc
LD := $(AS)

CFLAGS += -O3 -Os
LDFLAGS += -O3 -Os
LDLIBS += -lm -lgcc -lc

# USB build configuration

#SDK_SOURCE_PATH += lib_stusb

#DEFINES += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=6 IO_HID_EP_LENGTH=64 HAVE_USB_APDU

# Rules

load: all
	python -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

include $(BOLOS_SDK)/Makefile.rules

dep/%.d: %.c Makefile
