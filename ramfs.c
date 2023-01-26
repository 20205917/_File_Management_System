//
// Created by xgs on 23-1-26.
//
#include <malloc.h>
#include <string.h>
#include "ramfs.h"

//file system
FdTable fd_table;
File *root;

//init file system
void init_ramfs() {
    //init file descriptor table
    fd_table.fd_count = 0;
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        fd_table.fds[i] = NULL;
    }
    //create root directory
    root = (File *) malloc(sizeof(File));
    root->type = 1;
    root->size = 0;
    root->parent = NULL;
    root->child = NULL;
    root->sibling = NULL;
    strcpy(root->name, "/");
}


//open file or directory
int ropen(const char *path, int flags) {
    //invalid path
    char *pathname = clean_path(path);
    if (pathname == NULL) {
        return -1;
    }
    //find file or directory
    File *file = find_file(pathname);
    if (file == NULL) {
        //file or directory not found
        if (flags & O_CREAT) {
            //create file or directory
            file = create_file(pathname, FILE);
        } else {
            //file or directory not found
            return -1;
        }
    }
    //create file descriptor
    Fd *fd = (Fd *) malloc(sizeof(Fd));
    fd->offset = 0;
    fd->file = file;
    //add file descriptor to file descriptor table
    fd_table.fds[fd_table.fd_count] = fd;
    return fd_table.fd_count++;
}

//create file or directory ,choose type FILE or DIRECTORY
File *create_file(char *pathname, int type) {
    //TODO
    return NULL;
}

//find file
File *find_file(char *pathname) {
    File *cur = root;//current file
    char *path = strtok(pathname, "/");
    while (path != NULL) {
        if (cur->child == NULL) {
            //child not found
            return NULL;
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
            return NULL;
        }
        path = strtok(NULL, "/");
    }
    return cur;
}

//clear file path
char *clean_path(char *pathname) {
    //invalid path
    if (pathname == NULL || strlen(pathname) == 0 || strlen(pathname) > MAX_FILE_NAME_LENGTH || pathname[0] != '/') {
        return NULL;
    }
    char *tmp = (char *) malloc(sizeof(char) * MAX_FILE_NAME_LENGTH);
    //we use two pointer to copy pathname to tmp
    //we will jump all the '/' in pathname ,if we meet '.'
    int p = 0;
    int isFile = 0;
    for (int i = 0; i < strlen(pathname); ++i) {
        if (pathname[i] == '/') {
            if (isFile) {
                free(tmp);
                return NULL;
            }
            continue;
        } else {
            tmp[p++] = '/';
            for (int j = i; j < strlen(pathname); ++j) {
                if (pathname[j] == '/') {
                    if (isFile) {
                        free(tmp);
                        return NULL;
                    }
                    i = j;
                    break;
                } else {
                    if (pathname[j] == '.') {
                        isFile = 1;
                    }
                    tmp[p++] = pathname[j];
                }
            }
        }
    }
    return tmp;
}

//create directory
int rmkdir(const char *pathname) {
    //find file first
    File *file = find_file(pathname);
    if (file != NULL) {
        //file or directory already exists
        return -1;
    }
    //create file or directory
    create_file(pathname, DIRECTORY);
    return 0;
}


