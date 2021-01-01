#include <stdint.h>
#include <math.h>

#include <yaul.h>

#include "enemies.h"
#include "spritecode.h"
#include "graphics/objects.h"

void
enemies_lvl_process(const lvl_t *lvl)
{
        const object_t * const objects = (object_t *)((uintptr_t)lvl +
            sizeof(lvl_t) +
            lvl->playfield_byte_size +
            lvl->near_byte_size +
            lvl->far_byte_size +
            lvl->overlap_byte_size);

        const uint32_t objects_count = lvl->objects_byte_size / sizeof(object_t);

        float_init();
        worm_init();

        for (uint32_t i = 0; i < objects_count; i++) {
                switch (objects[i].id) {
                case OBJECT_ID_FLOAT:
                        float_load();
                        float_make(objects[i].x, objects[i].y);
                        break;
                case OBJECT_ID_WORM:
                        worm_load();
                        worm_make(objects[i].x, objects[i].y);
                        break;
                }
        }
}
