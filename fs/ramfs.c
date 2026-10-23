#include <airlys/ramfs.h>
#include <airlys/heap.h>

static uint32_t ramfs_next_inode = 1;

static size_t kstrlen(const char* str) {
    size_t len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
}

static void kstrncpy(char* dest, const char* src, size_t n) {
    size_t i = 0;
    while (i < n - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static uint64_t ramfs_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    if (!node || !buffer || !node->internal_data) return 0;
    if (offset >= node->size) return 0;

    if (offset + size > node->size) {
        size = node->size - offset;
    }

    const uint8_t* src = (const uint8_t*)node->internal_data + offset;
    for (uint64_t i = 0; i < size; i++) {
        buffer[i] = src[i];
    }
    return size;
}

static struct vfs_node* ramfs_finddir(vfs_node_t* node, const char* name) {
    if (!node || !name) return NULL;

    for (uint32_t i = 0; i < node->num_children; i++) {
        if (kstrcmp(node->children[i]->name, name) == 0) {
            return node->children[i];
        }
    }
    return NULL;
}

static struct vfs_node* ramfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!node || index >= node->num_children) return NULL;
    return node->children[index];
}

vfs_node_t* ramfs_create_dir(vfs_node_t* parent, const char* name) {
    vfs_node_t* dir = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!dir) return NULL;

    kstrncpy(dir->name, name, sizeof(dir->name));
    dir->flags = VFS_DIRECTORY;
    dir->size = 0;
    dir->inode = ramfs_next_inode++;
    dir->read = NULL;
    dir->write = NULL;
    dir->finddir = ramfs_finddir;
    dir->readdir = ramfs_readdir;
    dir->parent = parent;
    dir->num_children = 0;
    dir->internal_data = NULL;

    for (size_t i = 0; i < VFS_MAX_CHILDREN; i++) {
        dir->children[i] = NULL;
    }

    if (parent && parent->num_children < VFS_MAX_CHILDREN) {
        parent->children[parent->num_children++] = dir;
    }

    return dir;
}

vfs_node_t* ramfs_create_file(vfs_node_t* parent, const char* name, const char* content) {
    vfs_node_t* file = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!file) return NULL;

    kstrncpy(file->name, name, sizeof(file->name));
    file->flags = VFS_FILE;
    file->inode = ramfs_next_inode++;
    file->read = ramfs_read;
    file->write = NULL;
    file->finddir = NULL;
    file->readdir = NULL;
    file->parent = parent;
    file->num_children = 0;

    size_t len = content ? kstrlen(content) : 0;
    file->size = len;

    if (len > 0) {
        char* data = (char*)kmalloc(len + 1);
        if (data) {
            for (size_t i = 0; i < len; i++) {
                data[i] = content[i];
            }
            data[len] = '\0';
            file->internal_data = data;
        } else {
            file->internal_data = NULL;
        }
    } else {
        file->internal_data = NULL;
    }

    for (size_t i = 0; i < VFS_MAX_CHILDREN; i++) {
        file->children[i] = NULL;
    }

    if (parent && parent->num_children < VFS_MAX_CHILDREN) {
        parent->children[parent->num_children++] = file;
    }

    return file;
}

bool ramfs_delete_node(vfs_node_t* parent, const char* name) {
    if (!parent || !name) return false;

    for (uint32_t i = 0; i < parent->num_children; i++) {
        if (kstrcmp(parent->children[i]->name, name) == 0) {
            vfs_node_t* target = parent->children[i];

            /* If directory, ensure it has no children */
            if ((target->flags & VFS_DIRECTORY) && target->num_children > 0) {
                return false; /* Directory not empty */
            }

            if (target->internal_data) {
                kfree(target->internal_data);
                target->internal_data = NULL;
            }
            kfree(target);

            /* Shift remaining children left */
            for (uint32_t j = i; j < parent->num_children - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children[parent->num_children - 1] = NULL;
            parent->num_children--;
            return true;
        }
    }
    return false;
}

vfs_node_t* ramfs_init(void) {
    /* Create Root "/" directory */
    vfs_node_t* root = ramfs_create_dir(NULL, "/");
    vfs_set_root(root);

    /* Standard filesystem directory tree */
    vfs_node_t* etc = ramfs_create_dir(root, "etc");
    vfs_node_t* bin = ramfs_create_dir(root, "bin");
    vfs_node_t* home = ramfs_create_dir(root, "home");
    vfs_node_t* user = ramfs_create_dir(home, "user");
    vfs_node_t* dev = ramfs_create_dir(root, "dev");

    (void)bin;
    (void)dev;

    /* System configuration & metadata files */
    ramfs_create_file(etc, "os-release",
        "NAME=\"Airlys Sovereign OS\"\n"
        "VERSION=\"0.2.0-genesis\"\n"
        "ID=\"airlys\"\n"
        "ARCH=\"x86_64\"\n"
        "LEAD_ARCHITECT=\"Er. Adam Eehan\"\n"
        "ORGANIZATION=\"Aira Group of Technology\"\n"
        "COVENANT=\"100% Free Lifetime Compute for Students Worldwide\"\n");

    ramfs_create_file(etc, "covenant.txt",
        "=================================================================\n"
        "          THE SOVEREIGN OPEN-SOURCE COVENANT                     \n"
        "       Aira Group of Technology & Er. Adam Eehan                 \n"
        "=================================================================\n"
        "Pledge: 100% Free Lifetime Sovereign OS, Software & Cloud Compute\n"
        "for students, underprivileged youth, and open-source creators    \n"
        "worldwide. Funded by ethical enterprise adoption in the West.   \n"
        "Knowledge and computational power are birthrights, not luxury!   \n");

    ramfs_create_file(user, "welcome.txt",
        "Welcome to Airlys Sovereign Terminal!\n"
        "Engineered from first principles by Er. Adam Eehan.\n\n"
        "Useful Commands:\n"
        "  help      - Show all commands\n"
        "  ls        - List directory contents\n"
        "  cat <f>   - Display file content\n"
        "  whoami    - Display current user & authority\n"
        "  ps        - Display active running tasks\n"
        "  free      - Memory statistics\n"
        "  covenant  - Read our public open-source pledge\n"
        "  aira <cmd>- Sovereign root privilege execution\n");

    return root;
}
