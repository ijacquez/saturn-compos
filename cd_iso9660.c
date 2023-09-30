#include <sys/cdefs.h>

#include <yaul.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static cdfs_filelist_t _filelist;
static cdfs_filelist_entry_t _filelist_entries[CDFS_FILELIST_ENTRIES_COUNT];

static uint8_t _sector[CDFS_SECTOR_SIZE];

static const cdfs_filelist_entry_t *_file_entry_find(const char *filename);

static sector_buffer_t _sectors[16];

static cdfs_config_t _cdfs_config = {
    .sector_read   = cdfs_sector_read,
    .sector_count  = sizeof(_sectors) / sizeof(sector_buffer_t),
    .sectors       = _sectors
};

void
cd_init(void)
{
        cd_block_init();
        cdfs_init();
        cdfs_config_set(&_cdfs_config);

        /* Load the maximum number. We have to free the allocated filelist
         * entries, but since we never exit, we don't have to */
        cdfs_filelist_entry_t * const filelist_entries =
            cdfs_entries_alloc(-1);
        assert(filelist_entries != NULL);

        cdfs_filelist_init(&_filelist, filelist_entries, -1);
        cdfs_filelist_root_read(&_filelist);
}

void
cd_load(const char *filename, void *data_buf, uint32_t read_size)
{
        const cdfs_filelist_entry_t * const file_entry = _file_entry_find(filename);
        assert(file_entry != NULL);

        uint8_t *data_p;
        data_p = data_buf;

        const uint32_t sector_count = min(file_entry->sector_count, (read_size >> 11) + 1);

        dbgio_printf("cd_load(%s, 0x%08X, %i)\n", filename, (uintptr_t)data_buf, read_size);

        /* Loop through and copy each sector, one at a time */
        for (uint32_t sector = 0; sector < sector_count; sector++) {
                int ret __unused;
                ret = cd_block_sector_read(file_entry->starting_fad + sector, _sector);
                assert(ret == 0);

                const uint32_t copy_len = min(read_size, CDFS_SECTOR_SIZE);
                (void)memcpy(data_p, _sector, copy_len);

                dbgio_printf("-- sector: %lu, sector_count: %lu, memcpy 0x%08X, copy_len: %lu\n",
                    file_entry->starting_fad + sector,
                    sector_count,
                    (uintptr_t)data_p,
                    copy_len);

                data_p += copy_len;
        }
}

int32_t
cd_load_nosize(const char *filename, void *data_buf)
{
        dbgio_printf("cd_load_nosize(%s, 0x%08X)\n", filename, (uintptr_t)data_buf);

        const cdfs_filelist_entry_t * const file_entry = _file_entry_find(filename);
        assert(file_entry != NULL);

        uint8_t *data_p;
        data_p = data_buf;

        for (uint32_t sector = 0; sector < file_entry->sector_count; sector++) {
                int ret __unused;
                ret = cd_block_sector_read(file_entry->starting_fad + sector, _sector);
                assert(ret == 0);

                (void)memcpy(data_p, _sector, CDFS_SECTOR_SIZE);

                data_p += CDFS_SECTOR_SIZE;
        }

        return file_entry->size;
}

uint32_t
cd_byte_size_get(const char *filename)
{
        const cdfs_filelist_entry_t * const file_entry = _file_entry_find(filename);
        assert(file_entry != NULL);

        return file_entry->size;
}

static const cdfs_filelist_entry_t *
_file_entry_find(const char *filename)
{
        for (uint32_t i = 0; i < CDFS_FILELIST_ENTRIES_COUNT; i++) {
                const cdfs_filelist_entry_t * const file_entry = &_filelist.entries[i];

                if ((strcmp(file_entry->name, filename)) == 0) {
                        return file_entry;
                }
        }

        return NULL;
}
