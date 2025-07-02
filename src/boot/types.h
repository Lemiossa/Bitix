#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long int ulong;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef signed char s8;
typedef signed short s16;
typedef signed long int s32;

typedef char bool;

typedef u32 ptr_t;
typedef u32 size_t;
typedef ulong time_t;

#define true 1
#define false 0
#define NULL ((void*)0)

#define ALIGN4(v)   (((v)+3)&~3)
#define ALIGN8(v)   (((v)+7)&~7)
#define ALIGN16(v)  (((v)+15)&~15)

#endif /* TYPES_H */
