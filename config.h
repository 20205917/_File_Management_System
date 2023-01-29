//
// Created by 86136 on 2023/1/29.
//

#ifndef _FILE_MANAGEMENT_SYSTEM_CONFIG_H
#define _FILE_MANAGEMENT_SYSTEM_CONFIG_H

#include "hashmap.h"
#include "stdio.h"
#define MAX_FILE_NAME_LENGTH 256
#define MAX_FD_COUNT 65554


typedef struct Folder {
    char name[MAX_FILE_NAME_LENGTH]; //file name or directory name
    int size; //file size
    struct Folder *parent;
    HashMap *fileSet;
    HashMap *folderSet;
} Folder;

typedef struct File {
    char name[MAX_FILE_NAME_LENGTH]; //file name or directory name
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

//util functions
File *find_file(char *pathname);

File *create_file(char *pathname);

int delete_file(char *pathname);

Folder *find_folder(char *pathname);

int delete_dir(char *pathname);

//clear file path
char *clean_path(const char *pathname);


#endif //_FILE_MANAGEMENT_SYSTEM_CONFIG_H
