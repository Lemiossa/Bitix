#ifndef ERRNO_H
#define ERRNO_H

#include "types.h"

extern int errno;

#define SETERR(e) (errno = (e))

#endif /* ERRNO_H */
