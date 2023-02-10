# pwdman - Password Manager

pwdman is a simple command-line utility that manages and stores your passwords.
All stored passwords are stored in a an encrypted `.pwdman` file located under
the user's home directory.

## Dependencies
- libcryptopp
- libreadline
- libspl (included as submodule)
- dtest (downloaded automatically)

## Building

To build, run:

    make

## Testing

To run tests, you can use the `test` target:

    make test

## Install/uninstall

To install or uninstall, you can use the `install` or `uninstall` targets:

    make install/uninstall

## Copyright

Copyright (c) 2023 Noah Orensa.

Licensed under the MIT license. See **LICENSE** file in the project root for details.
