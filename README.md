# pwdman - Password Manager

[![MIT Licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

[![build](https://github.com/noahorensa/pwdman/workflows/test/badge.svg)](https://github.com/noahorensa/pwdman/actions?query=workflow%3Atest) [![build](https://github.com/noahorensa/pwdman/workflows/build/badge.svg)](https://github.com/noahorensa/pwdman/actions?query=workflow%3Abuild)

pwdman is a simple command-line utility that manages and stores your passwords.
All stored passwords are stored in a an encrypted `.pwdman` file located under
the user's home directory.

## Dependencies
- libspl (included as submodule)
- cryptopp (included as submodule)
- clip (included as submodule)
- dtest (downloaded automatically)
- libreadline-dev
- libx11-dev / libX11-devel
- libpng-dev / libpng-devel

## Build

To build, run:

    make

## Test

To run tests, you can use the `test` target:

    make test

## Install/uninstall

To install/uninstall, you can use the `install` and `uninstall` targets:

    make install
    make uninstall

## Copyright

Copyright (c) 2023 Noah Orensa.

Licensed under the MIT license. See **LICENSE** file in the project root for details.
