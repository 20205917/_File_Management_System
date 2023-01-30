//
// Created by xgs on 23-1-26.
//
#include <malloc.h>
#include <string.h>
#include "ramfs.h"

#define MAX_FD_COUNT 65558

#define FILE 0
#define DIRECTORY 1

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

//init file system
void init_ramfs() {
    //init file descriptor table
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        fd_table.fds[i] = NULL;
    }
    //create root directory
    root = (File *) malloc(sizeof(File));
    root->type = DIRECTORY;
    root->name = "/";
    root->size = 0;
    root->parent = NULL;
    root->child = NULL;
    root->sibling = NULL;
}


//open file or directory
int ropen(const char *pathname, int flags) {
    //invalid path
    int res = justify_path(pathname);
    if (res == -1) {
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
                return -1;
            }
        } else {
            //file or directory not found
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
    return fd;
}

//create file or directory ,choose type FILE or DIRECTORY
File *create_file(const char *pathname, int type) {
    //find parent directory
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    strcpy(parent_path, pathname);

    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;
    File *parent = find_file(parent_path);
    if (parent == NULL) {
        free(parent_path);
        //parent directory not found
        return NULL;
    }
    //create file or directory
    File *file = (File *) malloc(sizeof(File));
    file->type = type;
    file->size = 0;
    file->parent = parent;
    file->child = NULL;
    file->sibling = NULL;
    file->content = NULL;
    file->name = (char *) malloc(strlen(name) + 1);
    strcpy(file->name, name);
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
    return file;
}

//find file
File *find_file(const char *pathname) {
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
        if (cur == NULL||cur->type==FILE) {
            //child not found
            break;
        }
        path = strtok(NULL, "/");
    }
    while (path != NULL) {
        path = strtok(NULL, "/");
    }
    free(tmp);
    return cur;
}

//clear file path
int justify_path(const char *pathname) {
    if (pathname == NULL || strlen(pathname) <= 1) {
        return -1;
    }
    if (pathname[0] != '/') {
        return -1;
    }
    //路径长度 <= 1024 字节。（变相地说，文件系统的路径深度存在上限）。
    if (strlen(pathname) > 1024) {
        return -1;
    }
    for (int i = 0; i < strlen(pathname); i++) {
        //only contain letter   number   english point
        if (!((pathname[i] >= 'a' && pathname[i] <= 'z') || (pathname[i] >= 'A' && pathname[i] <= 'Z') ||
              (pathname[i] >= '0' && pathname[i] <= '9') || pathname[i] == '.' || pathname[i] == '/')) {
            return -1;
        }
    }
//    单个文件和目录名长度 <= 32 字节
    char *tmp = (char *) malloc(strlen(pathname) + 1);
    strcpy(tmp, pathname);
    char *path = strtok(tmp, "/");
    while (path != NULL) {
        if (strlen(path) > 32) {
            return -1;
        }
        path = strtok(NULL, "/");
    }
    free(tmp);
    return 0;
}

//create directory
int rmkdir(const char *pathname) {
    //find . in pathname
    for (int i = 0; i < strlen(pathname); ++i) {
        if (pathname[i] == '.') {
            return -1;
        }
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
        return -1;
    }
    //create file or directory
    file = create_file(path, DIRECTORY);
    if (file == NULL) {
        //create file or directory failed
        return -1;
    }
    return 0;
}

//delete directory
int rrmdir(const char *pathname) {
    //find . in pathname
    for (int i = 0; i < strlen(pathname); ++i) {
        if (pathname[i] == '.') {
            return -1;
        }
    }
    if (justify_path(pathname) == -1) {
        return -1;
    }
    char *path = clean_path(pathname);
    //find file first
    File *file = find_file(path);
    if (file == NULL || file->link_count >= 1||file->type==FILE) {
        //file or directory not found
        return -1;
    }
    if (file->child != NULL) {
        //directory not empty
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
    free(file->name);
    free(file);
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
        return -1;
    }
    if (file->link_count >= 1||file->type==DIRECTORY) {
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
    free(file->content);
    free(file->name);
    free(file);
    return 0;
}


int rclose(int fd) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL) {
        return -1;
    }
    File *file = fd1->file;
    file->link_count--;//link count -1
    free(fd1);
    fd_table.fds[fd] = NULL;
    return 0;
}


off_t rseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL) {
        return -1;
    }
    File *file = fd1->file;
    if (whence == SEEK_SET) {
        if (offset < 0) {
            return -1;
        }
        fd1->offset = offset;
    } else if (whence == SEEK_CUR) {
        if (offset + fd1->offset < 0) { //offset+fd1->offset < 0
            return -1;
        }
        fd1->offset += offset;
    } else if (whence == SEEK_END) {
        if (offset + fd1->file->size < 0) { //offset+fd1->file->size < 0
            return -1;
        }
        fd1->offset = file->size + offset;
    } else {
        return -1;
    }
    return fd1->offset;
}

ssize_t rread(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL) {
        return -1;
    }
    if (fd1->flags&O_WRONLY) {
        return -1;
    }
    File *file = fd1->file;
    if (file->type == DIRECTORY) {
        return -1;
    }
    //empty file
    if (file->size == 0 || fd1->file->content == NULL) {
        return 0;
    }
    //check the buf
    if (buf == NULL) {
        return -1;
    }
    if (fd1->offset + count > file->size) {
        count = file->size - fd1->offset;
    }
    //check whether the buf size
    memcpy(buf, file->content + fd1->offset, count);
    fd1->offset += (long) count;
    return (long) count;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL || (!(fd1->flags & O_WRONLY || fd1->flags & O_RDWR))) {
        return -1;
    }
    File *file = fd1->file;
    if (file == NULL || file->type == DIRECTORY || buf == NULL) {
        return -1;
    }
    if (fd1->offset + count > file->size) {
        if (file->content == NULL) {
            file->content = malloc(fd1->offset + count);
        } else {
            char *tmp = malloc(fd1->offset + count);
            memset(tmp, 0, fd1->offset + count);
            memcpy(tmp, file->content, file->size);
            free(file->content);
            file->content = tmp;
        }
        file->size = (int) fd1->offset + (int) count;//new size
    }
    memcpy(file->content + fd1->offset, buf, count);
    fd1->offset += (long) count;
    return (long) count;
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