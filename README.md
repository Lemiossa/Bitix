# Bitix

## About

Hey! I'm Matheus, 14 years old, and recently I got into low-level programming and complex stuff. One day, I asked myself: "Can I make my own OS?"
Well... I decided to try. At first, I made a ton of crappy attempts — but each one taught me something. Now, I believe Bitix is the real deal. It's the result of everything I’ve learned so far.

## Dependencies

- gcc - to compile the bootloader and kernel
- nasm - to compile low-level parts of the bootloader and kernel
- qemu - to run
- make - to automate everything

## Minimum requirements to run on a real machine

- 80386+ CPU - Protected mode support
- 8MiB of RAM - at the moment, this is quite excessive, but I intend to make the OS quite complete, so I recommend this.

## Bugs

Well... I haven't found any bugs so far.

## How to compile

To compile, Use:
```sh
make
```

To run:
```sh
make qemu
```

To install:
```sh
# NO SUDO
make install
```
