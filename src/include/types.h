#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long int ulong;

typedef signed char schar;
typedef signed int sint;
typedef signed short sshort;
typedef signed long int slong;

typedef enum { false=0, true=1 } bool;

typedef ulong uptr_t;
typedef ulong size_t;
typedef ulong time_t;

#define NULL ((void*)0)

#define ALIGN4(v)   (((v)+3)&~3)
#define ALIGN8(v)   (((v)+7)&~7)
#define ALIGN16(v)  (((v)+15)&~15)

#endif /* TYPES_H */
