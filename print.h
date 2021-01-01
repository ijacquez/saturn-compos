#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>

void print_init(void);
void print_buffer(uint16_t start_x, uint16_t start_y, const char *__restrict fmt, ...);
void print_draw(void);

#endif /* !PRINT_H */
