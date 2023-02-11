#include <malloc.h>
#include <string.h>
#include "ramfs.h"

#define MAX_FD_COUNT 65558

enum {
    FILE,
    DIRECTORY,
};

typedef struct file {
    char *name; //file name or directory name
    int type; //type 0:file 1:directory
    int size; //file size
    struct file *parent; //parent directory
    struct file *child; //child directory or file
    struct file *sibling; //sibling directory or file
    char *content; //file content
    int link_count; //link count
} File;

//file descriptor
typedef struct fd {
    off_t offset; //file descriptor
    int flags; //file descriptor flags
    File *file; //file
} Fd;

//file descriptor table
typedef struct fd_table {
    Fd *fds[MAX_FD_COUNT]; //file descriptor table
} FdTable;


//util function
File *find_file(const char *pathname);

File *create_file(const char *pathname, int type);

//justify if the pathname is valid
int justify_path(const char *pathname);

char *clean_path(const char *pathname);

//file system
FdTable fd_table;
File *root;

// free file
void free_file(File *file) {
    if (file == NULL) {
        return;
    }
    free(file->name);
    free(file->content);
    free(file);
}

//init file system
void init_ramfs() {
    //init file descriptor table
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        fd_table.fds[i] = NULL;
    }
    //create root directory
    root = (File *) malloc(sizeof(File));
    memset(root, 0, sizeof(File));
    root->type = DIRECTORY;
    root->name = "/";
}


//open file or directory
int ropen(const char *pathname, int flags) {
    //invalid path
    if (justify_path(pathname) == -1) {
        return -1;
    }
    char end = pathname[strlen(pathname) - 1];//get end char
    char *path = clean_path(pathname);
    //find file or directory
    File *file = find_file(path);
    if (file == NULL) {//没找到默认是文件,并且不合法
        if (end == '/') {
            free(path);
            return -1;
        }
        //file or directory not found
        if (flags & O_CREAT) {
            //create file or directory
            file = create_file(path, FILE);
            if (file == NULL) {
                //create file or directory failed
                free(path);
                return -1;
            }
        } else {
            //file or directory not found
            free(path);
            return -1;
        }
    }
    //find the first empty file descriptor
    int fd = 1;
    while (fd_table.fds[fd] != NULL) {
        fd++;
    }
    //create file descriptor
    Fd *fd1 = (Fd *) malloc(sizeof(Fd));

    fd1->flags = flags;
    fd1->file = file;
    fd1->offset = 0;
    //add file descriptor to file descriptor table
    fd_table.fds[fd] = fd1;
    file->link_count++;//link count +1

    if (file->type == FILE) {
        if (flags & O_APPEND) {
            fd1->offset = file->size;
        } else {
            fd1->offset = 0;
        }
        //check flags
        if ((flags & O_TRUNC) && ((flags & O_WRONLY) || (flags & O_RDWR))) {
            //truncate file
            file->size = 0;
            free(file->content);
            file->content = NULL;
        }
    }
    free(path);
    return fd;
}

//create file or directory ,choose type FILE or DIRECTORY
File *create_file(const char *pathname, int type) {
    //find parent directory
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    memset(parent_path, 0, strlen(pathname) + 1);
    strcpy(parent_path, pathname);

    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;
    char *tmp = clean_path(parent_path);
    File *parent = find_file(tmp);
    free(tmp);
    if (parent == NULL || parent->type != DIRECTORY) {
        free(parent_path);
        //parent directory not found
        return NULL;
    }
    //create file or directory
    File *file = (File *) malloc(sizeof(File));
    memset(file, 0, sizeof(File));
    file->type = type;
    file->parent = parent;
    file->name = (char *) malloc(strlen(name) + 1);
    memcpy(file->name, name, strlen(name) + 1);
    //add file or directory to parent directory
    if (parent->child == NULL) {
        parent->child = file;
    } else {
        File *child = parent->child;
        while (child->sibling != NULL) {
            child = child->sibling;
        }
        child->sibling = file;
    }
    free(parent_path);
    return file;
}

//find file
File *find_file(const char *pathname) {
    if (strcmp(pathname, "") == 0 || strcmp(pathname, "/") == 0) {
        return root;
    }
    char *tmp = (char *) malloc(strlen(pathname) + 1);
    strcpy(tmp, pathname);
    File *cur = root;//current file
    char *path = strtok(tmp, "/");
    while (path != NULL) {
        if (cur->child == NULL) {
            //child not found
            cur = NULL;
            break;
        }
        cur = cur->child;
        while (cur != NULL) {
            if (strcmp(cur->name, path) == 0) {
                //child found
                break;
            }
            cur = cur->sibling;
        }
        if (cur == NULL) {
            //child not found
            break;
        }
        path = strtok(NULL, "/");
    }
    while (path != NULL) {
        path = strtok(NULL, "/");
    }
    //get the end name of the path and compare with the current file name
    char *name = strrchr(pathname, '/');
    name++;
    if (cur != NULL && strcmp(cur->name, name) != 0) {
        cur = NULL;
    }
    free(tmp);
    return cur;
}

//justify if the pathname is valid
int justify_path(const char *pathname) {
    //路径长度 <= 1024 字节。（变相地说，文件系统的路径深度存在上限）。
    if (pathname[0] != '/' || strlen(pathname) > 1024)
        return -1;
    char *path_copy = (char *) malloc(strlen(pathname) + 1);
    memcpy(path_copy, pathname, strlen(pathname) + 1);
    char *tok = strtok(path_copy, "/");
    while (tok != NULL) {
        for (int i = 0; i < strlen(tok); ++i) {
            if (tok[i] < '0' || tok[i] > '9' && tok[i] < 'A' || tok[i] > 'Z' && tok[i] < 'a' || tok[i] > 'z') {
                free(path_copy);
                return -1;
            }
        }
        if (strlen(tok) > 32) {
            return -1;
        }
        tok = strtok(NULL, "/");
    }
    free(path_copy);
    return 0;
}

//create directory
int rmkdir(const char *pathname) {
    int length = strlen(pathname);
    if (length <= 1 || length > 1024) {
        return -1;
    }
    //invalid path
    if (justify_path(pathname) == -1) {
        return -1;
    }
    //clean path
    char *path = clean_path(pathname);
    //find file first
    File *file = find_file(path);
    if (file != NULL) {
        //file or directory already exists
        free(path);
        return -1;
    }
    //create file or directory
    file = create_file(path, DIRECTORY);
    if (file == NULL) {
        //create file or directory failed
        free(path);
        return -1;
    }
    free(path);
    return 0;
}

//delete directory
int rrmdir(const char *pathname) {
    //find . in pathname
    int length = strlen(pathname);
    if (length <= 1 || length > 1024) {
        return -1;
    }
    if (justify_path(pathname) == -1) {
        return -1;
    }
    char *path = clean_path(pathname);
    //find file first
    File *file = find_file(path);
    if (file == NULL || file->link_count >= 1 || file->type == FILE) {
        //file or directory not found
        free(path);
        return -1;
    }
    if (file->child != NULL) {
        //directory not empty
        free(path);
        return -1;
    }
    //delete file or directory
    File *parent = file->parent;
    if (parent->child == file) {
        parent->child = file->sibling;
    } else {
        File *child = parent->child;
        while (child->sibling != file) {
            child = child->sibling;
        }
        child->sibling = file->sibling;
    }
    free(path);
    free_file(file);
    return 0;
}

int runlink(const char *pathname) {
    if (justify_path(pathname) == -1) {
        return -1;
    }
    //find file first
    char *path = clean_path(pathname);

    File *file = find_file(path);
    if (file == NULL) {
        //file or directory not found
        free(path);
        return -1;
    }
    if (file->link_count >= 1 || file->type == DIRECTORY) {
        free(path);
        return -1;//link count >=1,can not delete
    }
    //delete file
    File *parent = file->parent;
    if (parent->child == file) {
        parent->child = file->sibling;
    } else {
        File *child = parent->child;
        while (child->sibling != file) {
            child = child->sibling;
        }
        child->sibling = file->sibling;
    }
    free(path);
    free_file(file);
    return 0;
}


int rclose(int fd) {
    if (fd < 0 || fd >= MAX_FD_COUNT || fd_table.fds[fd] == NULL) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    File *file = fd1->file;
    file->link_count--;//link count -1
    free(fd1);
    fd_table.fds[fd] = NULL;
    return 0;
}


off_t rseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_FD_COUNT || fd_table.fds[fd] == NULL) {
        return -1;
    }
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    File *file = fd1->file;
    if ((offset + fd1->offset < 0 && whence == SEEK_CUR) ||
        (offset + fd1->file->size < 0 && whence == SEEK_END) ||
        (offset < 0 && whence == SEEK_SET)) {
        return -1;
    }
    switch (whence) {
        case SEEK_SET:
            fd1->offset = offset;
            break;
        case SEEK_CUR:
            fd1->offset += offset;
            break;
        case SEEK_END:
            fd1->offset = file->size + offset;
            break;
    }
    return fd1->offset;
}

#define  READ  0
#define  WRITE  1

int justify_io_operation(int fd, void *buf, size_t count, int op) {
    if (fd < 0 || fd >= MAX_FD_COUNT || fd_table.fds[fd] == NULL) {
        return -1;
    }
    Fd* f_dec = fd_table.fds[fd];
    File *f = fd_table.fds[fd]->file;
    if (f->type == DIRECTORY || buf == NULL || count < 0) {
        return -1;
    }
    if (op == READ &&f_dec->flags == O_WRONLY) {
        return -1;
    }
    if (op == WRITE && f_dec->flags == O_RDONLY) {
        return -1;
    }
    return 0;
}


ssize_t rread(int fd, void *buf, size_t count) {
    if(justify_io_operation(fd, buf, count, READ) == -1){
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    File *file = fd1->file;
    fd1->offset + count-file->size>0?count = file->size - fd1->offset:count;
    memcpy(buf, file->content + fd1->offset, count);
    fd1->offset +=  count;
    return count;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if(justify_io_operation(fd, buf, count, WRITE) == -1){
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    File *file = fd1->file;
    if (fd1->offset + count > file->size) {
        file->content = realloc(file->content, fd1->offset + count);
        file->size = fd1->offset + count;
    }
    memcpy(file->content + fd1->offset, buf, count);
    fd1->offset += count;
    return count;
}


char *clean_path(const char *pathname) {
    int length = strlen(pathname);
    char *tmp = (char *) malloc(length + 1);
    memset(tmp, 0, strlen(pathname) + 1);
    int i;
    for (i = length - 1; i >= 0; i--) {
        if (pathname[i] == '/') {
            continue;
        } else {
            break;
        }
    }
    memcpy(tmp, pathname, i + 1);
    tmp[i + 1] = '\0';
    return tmp;
}