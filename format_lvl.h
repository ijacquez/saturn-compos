#ifndef FORMAT_LVL_H
#define FORMAT_LVL_H

#include <sys/cdefs.h>

#include <assert.h>
#include <stdint.h>

#include <fix16.h>

typedef enum layer_flags {
        LAYER_FLAGS_NONE = 0
} layer_flags_t;

typedef enum object_flags {
        OBJECT_FLAGS_NONE = 0
} object_flags_t;

typedef struct object {
        uint16_t id;
        object_flags_t flags;
        fix16_t x;
        fix16_t y;
} __packed object_t;

typedef struct layer {
        uint16_t width;
        uint16_t height;
        uint32_t pnd_byte_size;
        uint32_t cg_byte_size;
        uint16_t palette_count;
        layer_flags_t flags;
} __packed layer_t;

typedef struct lvl {
        const char signature[4];
        fix16_t player_x;
        fix16_t player_y;
        uint32_t playfield_byte_size;
        uint32_t near_byte_size;
        uint32_t far_byte_size;
        uint32_t overlap_byte_size;
        uint32_t objects_byte_size;
} __packed lvl_t;

typedef enum map_value_flags {
        MAP_VALUE_FLAG_NONE         = 0,
        MAP_VALUE_FLAG_COLLISION    = 1 << 0,
        MAP_VALUE_FLAG_INSTANT_KILL = 1 << 1,
        MAP_VALUE_FLAG_TRIGGER      = 1 << 2,

        /* Used for alignment */
        MAP_VALUE_FLAG_ALIGNED      = 0xFF
} __packed map_value_flags_t;

static_assert(sizeof(map_value_flags_t) == 1);

typedef struct map_value {
        uint16_t pnd;
        map_value_flags_t flags;
        uint8_t index;
} __packed map_value_t;

static_assert(sizeof(map_value_t) == 4);

#endif /* !FORMAT_LVL_H */
