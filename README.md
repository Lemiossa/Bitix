# Bitix

## About

Hey! I'm Matheus, 14 years old, and recently I got into low-level programming and complex stuff. One day, I asked myself: "Can I make my own OS?"
Well... I decided to try. At first, I made a ton of crappy attempts — but each one taught me something. Now, I believe Bitix is the real deal. It's the result of everything I’ve learned so far.

## Dependencies

To build and run Bitix, you'll need:
- gcc - to compile the bootloader and kernel
- nasm - to compile low-level parts of the bootloader and kernel
- qemu - to run
- make - to automate everything

## Minimum requirements to run on a real machine

- 80386+ CPU (Protected Mode support required)
- 8MiB of RAM 

Right now, that’s more than enough — but I'm planning to make Bitix more feature-complete over time, so better to have some headroom

## Bugs

Honestly?
None found so far... But if you find any, feel free to open an issue or ping me.

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
