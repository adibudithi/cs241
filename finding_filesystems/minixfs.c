/**
 * Finding Filesystems
 * CS 241 - Fall 2019
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode* node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    node->mode = ((node->mode >> RWX_BITS_NUMBER) << RWX_BITS_NUMBER) | new_permissions;
    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    if (owner != (uid_t) -1) node->uid = owner;
    if (group != (gid_t) -1) node->gid = group;
    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* node = get_inode(fs, path);
    if (node) {
        clock_gettime(CLOCK_REALTIME, &node->atim);
        clock_gettime(CLOCK_REALTIME, &node->mtim);
        clock_gettime(CLOCK_REALTIME, &node->ctim);
        return NULL;
    }

    inode_number inode_num = first_unused_inode(fs);
    if (inode_num == -1) return NULL;

    const char* filename;
    inode* parent = parent_directory(fs, path, &filename);
    node = fs->inode_root + inode_num;
    init_inode(parent, node);
    int size = (int) parent->size;
    int bnum = 0;

    while (size >= 0) {
        data_block_number dbnum = parent->direct[bnum];
        if (dbnum == UNASSIGNED_NODE) {
            add_data_block_to_inode(fs, parent);
            dbnum = parent->direct[bnum];
        }

        char* orig_block = (char*) (fs->data_root + dbnum);
        char* block = orig_block;

        while (size >= 0 && block != (orig_block + sizeof(data_block))) {
            if (*block == 0) {
                minixfs_dirent dirent;
                dirent.name = strdup(filename);
                dirent.inode_num = inode_num;
                make_string_from_dirent(block, dirent);
                free(dirent.name);
                parent->size += FILE_NAME_ENTRY;
                return node;
            }

            block += FILE_NAME_ENTRY;
            size -= FILE_NAME_ENTRY;
        }

        bnum++;
    }

    return NULL;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        char* blocks = GET_DATA_MAP(fs->meta);
        int free = 0;
        int used = 0;
        for (size_t i = 0; i < fs->meta->dblock_count; i++) {
            if ((int) blocks[i] == 0) free++;
            else used++;
        }
        char buffer[100000];
        sprintf(buffer, "Free blocks: %zd\nUsed blocks: %zd\n", free, used);
        sprintf(buf, "%s", buffer + (size_t) *off);
        return strlen(buffer + (size_t) *off);
    }
    // TODO implement your own virtual file here
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    int block_count = (int) ((*off + count) / sizeof(data_block) + 1);
    if (minixfs_min_blockcount(fs, path, block_count) == -1) {
        errno = ENOSPC;
        return -1;
    }

    inode* node = get_inode(fs, path);
    if (!node) {
        node = minixfs_create_inode_for_path(fs, path);
        if (!node) {
            errno = ENOSPC;
            return -1;
        }
    }
    int unwritten = (int) count;
    int bnum = *off / sizeof(data_block);
    int boff = *off % sizeof(data_block);

    while (unwritten > 0) {
        char* block;
        if (bnum < NUM_DIRECT_BLOCKS) {
            // Direct blook look up
            data_block_number dbnum = node->direct[bnum];
            block = (char*) (fs->data_root + dbnum);

        } else {
            // INDIRECT
            int ibnum = bnum - NUM_DIRECT_BLOCKS;
            data_block_number* iblocks = (data_block_number*) (fs->data_root + node->indirect);
            block = (char*) (fs->data_root + iblocks[ibnum]);
        }

        memcpy(block + boff, buf, MIN((sizeof(data_block) - boff), (size_t) unwritten));

        buf += MIN((sizeof(data_block) - boff), (size_t) unwritten);
        unwritten -= MIN((sizeof(data_block) - boff), (size_t) unwritten);
        bnum++;
        boff = 0;
    }

    clock_gettime(CLOCK_REALTIME, &node->atim);
    
    *off += count;
    node->size = *off;
    return count;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    inode* node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }

    int unread = (int) MIN(node->size - *off, count);
    int bnum = *off / sizeof(data_block);
    int boff = *off % sizeof(data_block);
    count = unread;

    while (unread > 0) {
        char* block;
        if (bnum < NUM_DIRECT_BLOCKS) {
            data_block_number dbnum = node->direct[bnum];
            block = (char*) (fs->data_root + dbnum);
        } else {
            int ibnum = bnum - NUM_DIRECT_BLOCKS;
            data_block_number* iblocks = (data_block_number*) (fs->data_root + node->indirect);
            block = (char*) (fs->data_root + iblocks[ibnum]);
        }

        memcpy(buf, block + boff, MIN((sizeof(data_block) - boff), (size_t) unread));

        buf += MIN((sizeof(data_block) - boff), (size_t) unread);
        unread -= MIN((sizeof(data_block) - boff), (size_t) unread);
        bnum++;
        boff = 0;
    }

    clock_gettime(CLOCK_REALTIME, &node->atim);
    *off += count;
    return count;
}
