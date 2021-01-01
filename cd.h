#ifndef CD_H
#define CD_H

#include <stdint.h>

void cd_init(void);

void cd_load(const char *filename, void *dst, uint32_t read_size);
int32_t cd_load_nosize(const char *filename, void *dst);
uint32_t cd_byte_size_get(const char *filename);

#endif /* !CD_H */
