# BOLOS OTP 2FA App v1.0.0

[This repository](https://github.com/parkerhoyes/bolos-app-otp2fa) contains an
application for the [Ledger Nano S](https://github.com/LedgerHQ/ledger-nano-s)
that implements standards for OTP-based [two-factor
authentication](https://en.wikipedia.org/wiki/Multi-factor_authentication).
Specifically, this application supports the Time-based One-time Password
Algorithm (TOTP) and the HMAC-based One-time Password Algorithm (HOTP), as
specified by [RFC 6238](https://tools.ietf.org/html/rfc6238) and [RFC
4226](https://tools.ietf.org/html/rfc4226), respectively. These 2FA standards
are implemented using parameters compatible with [Google
Authenticator](https://en.wikipedia.org/wiki/Google_Authenticator), and as such
this app can be used to authenticate for services which support Google
Authenticator.

This is a standalone app which can largely function on the Ledger Nano S alone
(without needing to connect to a host computer via USB), except when generating
TOTP authentication codes which require knowledge of the current time. As the
Ledger Nano S does not contain a real-time clock, the device must be connected
to a host computer via USB to determine the time. However, the time is confirmed
by the user on the device so the host computer need not be trusted.

All 2FA keys are stored in the flash memory of the device's Secure Element and
entered manually by the user directly into the device (these are usually
represented as a 32 character code). The app does not allow users to export 2FA
keys after they have been entered into the device. Note, however, that you can
enter the same 2FA keys into multiple devices while setting up 2FA with your
accounts.

Ledger devices also support FIDO U2F via the [official U2F
app](https://github.com/LedgerHQ/blue-app-u2f) made by Ledger. FIDO U2F is
widely believed to be more secure than the OTP standards implemented by this
app, however there are many services which support TOTP or HOTP but not FIDO
U2F.

Precompiled versions of this application are available for download [on my
website](https://parkerhoyes.com/bolos-apps).

## Development Cycle

This repository will follow a Git branching model similar to that described in
[Vincent Driessen's *A successful Git branching
model*](http://nvie.com/posts/a-successful-git-branching-model/) and a
versioning scheme similar to that defined by [Semantic Versioning
2.0.0](http://semver.org/).

## License

This application is distributed under the terms of the very permissive [Zlib
License](https://opensource.org/licenses/Zlib). The exact text of this license
is reproduced in the `LICENSE.txt` file as well as at the top of every source
file in this repository.
