#include <yaul.h>

#include <sys/cdefs.h>

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t _sector[FILECLIENT_SECTOR_SIZE];

void
cd_init(void)
{
}

void
cd_load(const char *filename, void *dst, uint32_t read_size)
{
        uint8_t *dst_p;
        dst_p = dst;

        const uint32_t sector_count = fileclient_sector_count_request(filename);
        const uint32_t calc_sector_count = min(sector_count, (read_size >> 11) + 1);

        for (uint32_t sector = 0; sector < calc_sector_count; sector++) {
                fileclient_sector_request(filename, sector, _sector);

                const uint32_t copy_len = min(read_size, ISO9660_SECTOR_SIZE);
                (void)memcpy(dst_p, _sector, copy_len);

                dst_p += copy_len;
        }
}

int32_t
cd_load_nosize(char *filename, void *dst)
{
        const uint32_t byte_size = fileclient_byte_size_request(filename);

        cd_load(filename, dst, byte_size);

        return byte_size;
}

uint32_t
cd_byte_size_get(const char *filename)
{
        const uint32_t byte_size = fileclient_byte_size_request(filename);

        return byte_size;
}
