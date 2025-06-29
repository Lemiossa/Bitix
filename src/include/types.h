#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef char s8;
typedef short s16;
typedef long s32;

typedef enum {false=0, true=1} bool;

typedef u32 uptr_t;
typedef u32 size_t;
typedef ulong time_t;

#define NULL ((void*)0)

#define ALIGN4(v)   (((v)+3)&~3)
#define ALIGN8(v)   (((v)+7)&~7)
#define ALIGN16(v)  (((v)+15)&~15)

#endif /* TYPES_H */
