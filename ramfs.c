//
// Created by xgs on 23-1-26.
//
#include "ramfs.h"

typedef struct file{
    char name[MAX_FILE_NAME_LENGTH]; //file name or directory name
    int type; //type 0:file 1:directory
    int size; //file size
    struct file* parent; //parent directory
    struct file* child; //child directory or file
    struct file* sibling; //sibling directory or file
    char* content; //file content
} File;

//file descriptor
typedef struct fd{
    int fd; //file descriptor
    File* file; //file
}Fd;

//file descriptor table
typedef struct fd_table{
    Fd* fds[MAX_FD_COUNT]; //file descriptor table
    int fd_count; //file descriptor count
}FdTable;

//file system
FdTable fd_table;


