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
File *find_file(const char *pathname, int type);

File *create_file(const char *pathname, int type);

//justify if the pathname is valid
int justify_path(const char *pathname, int type);

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
    root->size = 0;
    root->parent = NULL;
    root->child = NULL;
    root->sibling = NULL;
    strcpy(root->name, "/");
}


//open file or directory
int ropen(const char *path, int flags) {
    //invalid path
    int res = justify_path(path, FILE);
    if (res == -1) {
        return -1;
    }
    //find file or directory
    File *file = find_file(path, FILE);
    if (file == NULL) {
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

    if (flags & O_APPEND) {
        fd1->offset = file->size;
    } else {
        fd1->offset = 0;
    }
    fd1->file = file;
    //add file descriptor to file descriptor table
    fd_table.fds[fd] = fd1;
    //check flags
    if (flags & O_TRUNC && (flags & O_WRONLY || flags & O_RDWR)) {
        //truncate file
        file->size = 0;
        free(file->content);
        file->content = NULL;
    }
    file->link_count++;//link count +1
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
    File *parent = find_file(parent_path, DIRECTORY);
    if (parent == NULL) {
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
File *find_file(const char *pathname, int type) {
    File *cur = root;//current file
    char *path = strtok(pathname, "/");
    while (path != NULL) {
        if (strcmp(path, "") == 0) {
            path = strtok(NULL, "/");
            continue;
        }
        if (cur->child == NULL) {
            //child not found
            return NULL;
        }
        cur = cur->child;
        while (cur != NULL) {
            if (strcmp(cur->name, path) == 0 && cur->type == type) {
                //child found
                break;
            }
            cur = cur->sibling;
        }
        if (cur == NULL) {
            //child not found
            return NULL;
        }
        path = strtok(NULL, "/");
    }
    return cur;
}

//clear file path
int justify_path(const char *pathname, int type) {
    if (pathname == NULL || strlen(pathname) <= 1) {
        return -1;
    }
    if (pathname[0] != '/') {
        return -1;
    }
    if (type == FILE) {
        if (pathname[strlen(pathname) - 1] == '/') {
            return -1;
        }
    }
    return 0;
}

//create directory
int rmkdir(const char *pathname) {
    if (justify_path(pathname, DIRECTORY) == -1) {
        return -1;
    }
    //find file first
    File *file = find_file(pathname, DIRECTORY);
    if (file != NULL) {
        //file or directory already exists
        return -1;
    }
    //create file or directory
    file = create_file(pathname, DIRECTORY);
    if (file == NULL) {
        //create file or directory failed
        return -1;
    }
    return 0;
}

//delete directory
int rmdir(const char *pathname) {
    if (justify_path(pathname, DIRECTORY) == -1) {
        return -1;
    }
    //find file first
    File *file = find_file(pathname, DIRECTORY);
    if (file == NULL) {
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
    if (justify_path(pathname, FILE) == -1) {
        return -1;
    }
    //find file first
    File *file = find_file(pathname, FILE);
    if (file == NULL) {
        //file or directory not found
        return -1;
    }
    if (file->link_count >= 1) {
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
    if (file->type == DIRECTORY) {
        return -1;
    }
    if (whence == SEEK_SET) {
        if (offset < 0) {
            return -1;
        }
        fd1->offset = offset;
    } else if (whence == SEEK_CUR) {
        if (offset+fd1->offset < 0){ //offset+fd1->offset < 0
            return -1;
        }
        fd1->offset += offset;
    } else if (whence == SEEK_END) {
        if (offset+fd1->file->size < 0){ //offset+fd1->file->size < 0
            return -1;
        }
        fd1->offset = file->size + offset;
    } else {
        return -1;
    }
    return fd1->offset;
}
