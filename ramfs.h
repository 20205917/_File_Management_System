//
// Created by xgs on 23-1-26.
//

#ifndef _FILE_MANAGEMENT_SYSTEM_RAMFS_H
#define _FILE_MANAGEMENT_SYSTEM_RAMFS_H

#include <stdint.h>
#include "hashmap.h"
#include "stdio.h"

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_TRUNC  01000
#define O_APPEND 02000


#define MAX_FILE_NAME_LENGTH 256
#define MAX_FD_COUNT 65554

#define DIRECTORY 1
typedef intptr_t ssize_t;
typedef uintptr_t size_t;
typedef long off_t;
typedef struct Folder {
    char name[MAX_FILE_NAME_LENGTH]; //file name or directory name
    int size; //file size
    struct Folder *parent;
    HashMap *fileSet;
    HashMap *folderSet;
} Folder;

typedef struct File {
    char name[MAX_FILE_NAME_LENGTH]; //file name or directory name
    struct Folder *parent;
    int size; //file size
    char *content; //file content
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





int ropen(const char *pathname, int flags);

int rclose(int fd);

ssize_t rwrite(int fd, const void *buf, size_t count);

ssize_t rread(int fd, void *buf, size_t count);

off_t rseek(int fd, off_t offset, int whence);

int rmkdir(const char *pathname);

int rrmdir(const char *pathname);

int runlink(const char *pathname);

void init_ramfs();

//util functions
File *find_file(char *pathname);

File *create_file(char *pathname);

int delete_file(char *pathname);

Folder *find_folder(char *pathname);

int delete_dir(char *pathname);

//clear file path
char *clean_path(const char *pathname);


#endif //_FILE_MANAGEMENT_SYSTEM_RAMFS_H
