#!/usr/bin/env python

# License for the BOLOS OTP 2FA Application project, originally found here:
# https://github.com/parkerhoyes/bolos-app-otp2fa
#
# Copyright (C) 2018 Parker Hoyes <contact@parkerhoyes.com>
#
# This software is provided "as-is", without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the
# use of this software.
#
# Permission is granted to anyone to use this software for any purpose, including
# commercial applications, and to alter it and redistribute it freely, subject to
# the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not claim
#    that you wrote the original software. If you use this software in a product,
#    an acknowledgment in the product documentation would be appreciated but is
#    not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

"""
This module can be used by the host computer to send the current time (and timezone) to a device running the OTP 2FA
application. It can be imported or used as a script.

This script is designed for Python 2.7. The following dependencies are required:

- ledgerblue (version 0.1.17)
- pytz
- tzlocal
"""

import datetime
import pytz
import struct
import time
import tzlocal

from ledgerblue.comm import getDongle

CLA = 0xE0
INS_MAGIC = 0x02
INS_SET_TIME = 0x04

PROTO_HOST_MAGIC = 0x72A5F76C
PROTO_DEVICE_MAGIC = 0xF2D17183

def get_time_data():
    """Get the current time and the offset of the current timezone from UTC.

    Returns:
        (secs, offset) where secs is the current UNIX timestamp as an int, and offset is the number of seconds by which
        UTC time must be offset to get the local time, as an int.
    """
    secs = round(time.time())
    # offset = int(-(time.altzone if time.daylight != 0 else time.timezone))
    dt = datetime.datetime.utcfromtimestamp(secs)
    dt = pytz.utc.localize(dt)
    dt = dt.astimezone(tzlocal.get_localzone())
    offset = round(dt.utcoffset().total_seconds())
    return secs, offset

def exchange_magic(dongle):
    header = bytearray([CLA, INS_MAGIC, 0x00, 0x00, 4])
    data = bytearray(struct.pack('>I', PROTO_HOST_MAGIC))
    rx = dongle.exchange(header + data)
    if bytearray(rx) != bytearray(struct.pack('>I', PROTO_DEVICE_MAGIC)):
        raise ValueError('Invalid device protocol magic')

def exchange_set_time(dongle):
    secs, offset = get_time_data()
    header = bytearray([CLA, INS_SET_TIME, 0x00, 0x00, 8 + 4])
    data = bytearray(struct.pack('>Qi', secs, offset))
    rx = dongle.exchange(header + data)
    if rx != bytearray():
        raise ValueError('Invalid response to INS_SET_TIME')

def serve_time(dongle=None, interval=10):
    if dongle is None:
        dongle = getDongle(True)
    print('Verifying protocol compatibility...')
    exchange_magic(dongle)
    while True:
        print('Transmitting current time...')
        exchange_set_time(dongle)
        time.sleep(interval)

if __name__ == '__main__':
    serve_time()
