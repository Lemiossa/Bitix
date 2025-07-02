# Bitix

Hi, how are you? I hope so.
My name is Matheus Leme, and... if you're here... Congratulations, you've just found the project of an unemployed teenager.

This is BitixOS, I nicknamed it that way because I really like minix, my goal is to try to get inspiration from it.

I'm still new to the OSdevs community, so don't judge me for poorly documented or inefficient code, it's worth remembering that this is a very difficult task.
My goal is to make an OS that can run simple games... and maybe even a 3D game full of hacks.
You can see that in the first commit I already have a bootloader with basic multiboot, it's not the best, but I tried to make something functional.
Well... that's it, below I'll leave the dependencies to compile the project to run on virtual and real machines.

dependencies
- dev86
- make
- gcc
- coreutils(It usually comes already installed except in ultra compact distros like tiny core or alpine)
- qemu(i386)

Compile using:
```bash
make
```

Run on qemu using:
```bash
make run
```

Install:
```bash
make install DEVICE=/dev/sdX [DEBUG=true]
```
