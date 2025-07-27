#pragma once

#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define SECTION(x) __attribute__((section(x)))
#define NOOPT __attribute__((optimize("O0")))

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned int size_t;
typedef uint32_t uintptr_t;

#define NULL ((void*)0)
typedef char bool;
#define true 1
#define false 0

#define ALIGN_UP(x, align)   (((x)+((align)-1))&~((align)-1))
#define ALIGN_DOWN(x, align) ((x)&~((align)-1))

#define UNUSED(x) (void)(x)

#define KiB(x) ((x)*1024)
#define MiB(x) (KiB(x)*1024)
#define SECTOR_SIZE 512

#define MIN(a, b) ({typeof(a)_a=(a);typeof(b)_b=(b);_a<_b?_a:_b;})
#define MAX(a, b) ({typeof(a)_a=(a);typeof(b)_b=(b);_a>_b?_a:_b;})

typedef char* va_list;

#define va_start(ap, last) (ap=(char*)&last+sizeof(last))
#define va_arg(ap, type) (*(type*)((ap+=sizeof(type))-sizeof(type)))
#define va_end(ap) (ap=NULL)
