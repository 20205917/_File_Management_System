//
// Created by xgs on 23-1-26.
//
#ifndef _FILE_MANAGEMENT_SYSTEM_RAMFS_H
#define _FILE_MANAGEMENT_SYSTEM_RAMFS_H

#include <stdint.h>
#include "config.h"

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_TRUNC  01000
#define O_APPEND 02000


typedef intptr_t ssize_t;
typedef uintptr_t size_t;
typedef long off_t;


int ropen(const char *pathname, int flags);

int rclose(int fd);

ssize_t rwrite(int fd, const void *buf, size_t count);

ssize_t rread(int fd, void *buf, size_t count);

off_t rseek(int fd, off_t offset, int whence);

int rmkdir(const char *pathname);

int rrmdir(const char *pathname);

int runlink(const char *pathname);

void init_ramfs();




#endif //_FILE_MANAGEMENT_SYSTEM_RAMFS_H
