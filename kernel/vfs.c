#include <airlys/vfs.h>
#include <airlys/heap.h>

static vfs_node_t* vfs_root = NULL;

static int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void vfs_init(void) {
    vfs_root = NULL;
}

vfs_node_t* vfs_get_root(void) {
    return vfs_root;
}

void vfs_set_root(vfs_node_t* root) {
    vfs_root = root;
}

uint64_t vfs_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    if (node && node->read) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

uint64_t vfs_write(vfs_node_t* node, uint64_t offset, uint64_t size, const uint8_t* buffer) {
    if (node && node->write) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if (node && node->finddir) {
        return node->finddir(node, name);
    }
    return NULL;
}

vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (node && node->readdir) {
        return node->readdir(node, index);
    }
    return NULL;
}

vfs_node_t* vfs_resolve_path(const char* path) {
    if (!path || !vfs_root) return NULL;
    if (kstrcmp(path, "/") == 0) return vfs_root;

    const char* ptr = path;
    if (*ptr == '/') ptr++; /* Skip leading slash */

    vfs_node_t* curr = vfs_root;
    char token[VFS_MAX_NAME];

    while (*ptr != '\0') {
        size_t idx = 0;
        while (*ptr != '/' && *ptr != '\0' && idx < VFS_MAX_NAME - 1) {
            token[idx++] = *ptr++;
        }
        token[idx] = '\0';
        if (*ptr == '/') ptr++;

        if (idx > 0) {
            vfs_node_t* next = vfs_finddir(curr, token);
            if (!next) {
                return NULL; /* Path component not found */
            }
            curr = next;
        }
    }

    return curr;
}
