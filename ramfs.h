//
// Created by xgs on 23-1-26.
//

#ifndef _FILE_MANAGEMENT_SYSTEM_RAMFS_H
#define _FILE_MANAGEMENT_SYSTEM_RAMFS_H

#include <stdint.h>

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

#define FILE 0
#define DIRECTORY 1

typedef struct file {
    char name[MAX_FILE_NAME_LENGTH]; //file name or directory name
    int type; //type 0:file 1:directory
    int size; //file size
    struct file *parent; //parent directory
    struct file *child; //child directory or file
    struct file *sibling; //sibling directory or file
    char *content; //file content
} File;

//file descriptor
typedef struct fd {
    int offset; //file descriptor
    File *file; //file
} Fd;

//file descriptor table
typedef struct fd_table {
    Fd *fds[MAX_FD_COUNT]; //file descriptor table
    int fd_count; //file descriptor count
} FdTable;



typedef long off_t;
typedef long int ssize_t;

int ropen(const char *pathname, int flags);

int rclose(int fd);

ssize_t rwrite(int fd, const void *buf, size_t count);

ssize_t rread(int fd, void *buf, size_t count);

off_t rseek(int fd, off_t offset, int whence);

int rmkdir(const char *pathname);

int rrmdir(const char *pathname);

int runlink(const char *pathname);

void init_ramfs();


//util function
File *find_file(const char *pathname);

File *create_file(const char *pathname, int type);

//clear file path
char *clean_path(const char *pathname);

#endif //_FILE_MANAGEMENT_SYSTEM_RAMFS_H
