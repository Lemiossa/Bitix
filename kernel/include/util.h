/************************************
 * util.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef UTIL_H
#define UTIL_H

#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define ALIGN_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))

int get_path_parts(char *path, char **parts, int max);

#endif /* UTIL_H */
