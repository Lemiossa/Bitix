#ifndef KERNEL_H
#define KERNEL_H

#include <types.h>
#include <utils.h>
#include <x86.h>

/* Syscalls */

#define SYSCALL_PUTC 1
#define SYSCALL_PUTS 2
#define SYSCALL_SET_CURSOR 3
#define SYSCALL_CLEAR_SCREEN 4
#define SYSCALL_SET_VID 5
#define SYSCALL_READBLOCK 6
#define SYSCALL_WRITEBLOCK 7
#define SYSCALL_SLEEP 8
#define SYSCALL_GET_CURSOR_POSITION 9
#define SYSCALL_DRAW_PIXEL 10
#define SYSCALL_GETCWD 11
#define SYSCALL_PUTXY_CT 12
#define SYSCALL_GET_KEY 13

#define KSEG 0x0600
#define USERSEG KSEG + 0x1000
#define USEROFF 0x0000

#endif /* KERNEL_H */
