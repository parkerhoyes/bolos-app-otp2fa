# BOLOS OTP 2FA App v1.0.0

[This repository](https://github.com/parkerhoyes/bolos-app-otp2fa) contains an
application for the [Ledger Nano S](https://github.com/LedgerHQ/ledger-nano-s)
that implements an HMAC-Based One-Time Password Algorithm (specifically [RFC
4226](https://tools.ietf.org/html/rfc4226)) commonly used for [Second Factor
Authentication](https://en.wikipedia.org/wiki/Multi-factor_authentication). This
application currently only supports counter-based one time passwords (HOTP), not
time-based one time passwords (TOTP).

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
