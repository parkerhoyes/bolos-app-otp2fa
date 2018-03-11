# License for the BOLOS OTP 2FA Application project, originally found here:
# https://github.com/parkerhoyes/bolos-app-otp2fa
#
# Copyright (C) 2017, 2018 Parker Hoyes <contact@parkerhoyes.com>
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
#CLANGPATH :=
#GCCPATH :=
# END USER CONFIGURATION

ifeq ($(BOLOS_SDK),)
$(error BOLOS_SDK is not set)
endif
$(info BOLOS_SDK=$(BOLOS_SDK))
include $(BOLOS_SDK)/Makefile.defines

# Main app configuration

APPNAME := "OTP 2FA"
APPVERSION_MAJOR := 1
APPVERSION_MINOR := 0
APPVERSION_PATCH := 0
APPVERSION := $(MAJOR).$(MINOR).$(PATCH)
APP_LOAD_PARAMS = --appFlags 0x00 $(COMMON_LOAD_PARAMS)

ifeq ($(TARGET_NAME),TARGET_BLUE)
ICONNAME := glyphs/app_icon_blue.gif
else
ICONNAME := glyphs/app_icon_nanos.gif
endif

# App build configuration

APP_SOURCE_PATH += src bui/src bui/include

# Main build configuration

SDK_SOURCE_PATH += lib_stusb lib_stusb_impl lib_u2f

DEFINES += OS_IO_SEPROXYHAL IO_SEPROXYHAL_BUFFER_SIZE_B=300
DEFINES += HAVE_BAGL HAVE_SPRINTF
DEFINES += PRINTF\(...\)=

DEFINES += UNUSED\(x\)=\(void\)x
DEFINES += APPVERSION_MAJOR=$(APPVERSION_MAJOR)
DEFINES += APPVERSION_MINOR=$(APPVERSION_MINOR)
DEFINES += APPVERSION_PATCH=$(APPVERSION_PATCH)
DEFINES += APPVERSION=\"$(APPVERSION)\"

DEFINES += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=6 IO_HID_EP_LENGTH=64 HAVE_USB_APDU

DEFINES += USB_SEGMENT_SIZE=64

DEFINES += HAVE_IO_U2F U2F_PROXY_MAGIC=\"OTP2FA\"

# Toolchain

ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
$(info BOLOS_ENV is set: using preset CLANGPATH and GCCPATH)
$(info CLANGPATH=$(CLANGPATH))
$(info GCCPATH=$(GCCPATH))
else
$(info BOLOS_ENV is not set: falling back to CLANGPATH and GCCPATH)
endif
ifeq ($(CLANGPATH),)
$(info CLANGPATH is not set: clang will be used from PATH)
endif
ifeq ($(GCCPATH),)
$(info GCCPATH is not set: arm-none-eabi-* will be used from PATH)
endif

CC := $(CLANGPATH)clang
CFLAGS += -O3 -Os

AS := $(GCCPATH)arm-none-eabi-gcc
AFLAGS :=

LD := $(GCCPATH)arm-none-eabi-gcc
LDFLAGS += -O3 -Os
LDLIBS += -lm -lgcc -lc

# Rules

all: default

load: all
	python -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

dep/%.d: %.c Makefile

# Import generic rules from the SDK

include $(BOLOS_SDK)/Makefile.rules
