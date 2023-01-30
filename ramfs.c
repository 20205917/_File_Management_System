//
// Created by xgs on 23-1-26.
//
#include <malloc.h>
#include <string.h>
#include "ramfs.h"

//file system
FdTable fd_table;
Folder *root;

//init file system
void init_ramfs() {
    //init file descriptor table
    memset(fd_table.fds, 0, MAX_FD_COUNT * sizeof(Fd *));
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        fd_table.fds[i] = NULL;
    }
    //create root directory
    root = (Folder *) malloc(sizeof(Folder));
    root->parent = NULL;
    root->size = 0;
    root->fileSet = DefaultHashMap();
    root->folderSet = DefaultHashMap();
    strcpy(root->name, "/");
}


//open file
int ropen(const char *path, int flags) {
    //invalid path
    char *pathname = clean_path(path);
    if (pathname == NULL) {
        return -1;
    }
    //find file
    File *file = find_file(pathname);

    //file  not found
    if (file == NULL) {
        //create file
        if (flags & O_CREAT)
            file = create_file(pathname);
            //create failed
        else
            return -1;
    } else{
        free(pathname);
        return -1;
    }

    //create file descriptor
    Fd *fd = (Fd *) malloc(sizeof(Fd));
    fd->offset = 0;
    fd->file = file;
    fd->flags = flags;

    //deal with flags
    if (flags & O_APPEND) {
        fd->offset = file->size;
    }
    if (flags & O_TRUNC) {
        file->size = 0;
        if (file->content != NULL) {
            free(file->content);
            file->content = NULL;
        }
    }


    //find empty file descriptor
    for (int i = 1; i < MAX_FD_COUNT; i++) {
        if (fd_table.fds[i] == NULL) {
            fd_table.fds[i] = fd;
            return i;
        }
    }
    return -1;//no empty file descriptor
}

//create file
File *create_file(char *pathname) {
    //find parent directory
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    strcpy(parent_path, pathname);
    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;
    Folder *parent = find_folder(parent_path);
    if (parent == NULL) {
        //parent directory not found
        return NULL;
    }
    //create file
    File *file = (File *) malloc(sizeof(File));
    file->size = 0;
    file->content = NULL;
    strcpy(file->name, name);
    //add file to parent directory
    if (InsertHashMap(parent->fileSet, name, (intptr_t) file)==0)
        //add file or directory to parent directory failed
        return file;
    return NULL;
}


//find folder only
Folder *find_folder(char *pathname) {
    Folder *cur = root;//current file
    char *path = strtok(pathname, "/");
    //get file name
    while (path != NULL) {
        cur = (Folder *) GetHashMap(cur->folderSet, path);
        if (cur == NULL) {
            //path not found
            return NULL;
        }
        path = strtok(NULL, "/");
    }
    return cur;
}

//find file only
File *find_file(char *pathname) {
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    strcpy(parent_path, pathname);
    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;
    Folder *parent = find_folder(parent_path);
    if (parent == NULL) {
        //parent directory not found
        return NULL;
    }
    return (File *) GetHashMap(parent->fileSet, name);
}

int delete_file(char *pathname) {
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    strcpy(parent_path, pathname);
    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;

    Folder *parent = find_folder(parent_path);
    if (parent == NULL) {
        //parent directory not found
        return -1;
    }
    //get file
    File *file = (File *) GetHashMap(parent->fileSet, name);
    if (file == NULL) {
        //file not found
        return -1;
    }
    //clear all fd point to this file
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        if (fd_table.fds[i] != NULL && fd_table.fds[i]->file == file) {
            free(fd_table.fds[i]);
            fd_table.fds[i] = NULL;
        }
    }
    return RemoveHashMap(parent->fileSet, name);
}

int delete_dir(char *pathname) {
    if (strcmp(pathname, "/") == 0) {
        //can not delete root directory
        return -1;
    }
    Folder *folder = find_folder(pathname);
    if (folder == NULL) {
        //parent directory not found
        return -1;
    }
    DeleteHashMap(folder->fileSet);
    DeleteHashMap(folder->folderSet);

    return RemoveHashMap(folder->parent->folderSet, folder->name);
}

//clear file path
char *clean_path(const char *pathname) {
    //invalid path
    if (pathname == NULL || strlen(pathname) == 0 || strlen(pathname) > MAX_FILE_NAME_LENGTH || pathname[0] != '/') {
        return NULL;
    }
    if (strrchr(pathname, '.') != NULL && strrchr(pathname, '/') != NULL &&
        strrchr(pathname, '.') - strrchr(pathname, '/') < 0)
        return NULL;
    char *tmp;
    int p = 0;
    tmp = (char *) malloc(sizeof(char) * strlen(pathname) + 1);
    memset(tmp, 0, strlen(pathname) + 1);
    char *pathname1 = strdup(pathname);
    char *tok = strtok(pathname1, "/");
    while (tok != NULL) {
        tmp[p++] = '/';
        for (int i = 0; i < strlen(tok); i++)
            tmp[p++] = tok[i];
        tok = strtok(NULL, "/");
    }

    free(pathname1);
    return tmp;
}

//create directory
int rmkdir(const char *path) {
    char *pathname = clean_path(path);
    if (pathname == NULL)
        return -1;
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    strcpy(parent_path, pathname);
    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;
    Folder *parent = find_folder(parent_path);  //find parent directory
    if (parent == NULL || GetHashMap(parent->folderSet,name)!=(intptr_t)NULL) {
        //parent directory not found or new directory already exist
        return -1;
    }
    //detect whether the directory already exists
    if ((void *) GetHashMap(parent->folderSet, name) != NULL) {
        return -1;
    }
    //create directory
    Folder *folder = (Folder *) malloc(sizeof(Folder));
    if (folder == NULL)
        return -1;
    folder->parent = parent;
    folder->size = 0;
    folder->fileSet = DefaultHashMap();
    folder->folderSet = DefaultHashMap();
    strcpy(folder->name, pathname);
    InsertHashMap(parent->folderSet, name, (intptr_t) folder);

    free(pathname);
    free(parent_path);
    //create directory success
    return 0;
}

int rrmdir(const char *pathname) {
    char *path = clean_path(pathname);
    if (path == NULL) {
        return -1;
    }
    return delete_dir(path);
}

int runlink(const char *pathname) {
    char *path = clean_path(pathname);
    if (path == NULL) {
        return -1;
    }
    return delete_file(path);
}

int rclose(int fd) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    if (fd_table.fds[fd] == NULL) {
        return -1;
    }
    free(fd_table.fds[fd]);
    fd_table.fds[fd] = NULL;
    return 0;
}

//SEEK_SET 0 将文件描述符的偏移量设置到 offset 指向的位置
//SEEK_CUR 1 将文件描述符的偏移量设置到 当前位置 + offset 字节的位置
//SEEK_END 2 将文件描述符的偏移量设置到 文件末尾 + offset 字节的位置
//rseek 允许将偏移量设置到文件末尾之后的位置，但是并不会改变文件的大小，直到它在这个位置写入
//了数据。在 超过文件末尾的地方写入了数据后，原来的文件末尾到实际写入位置之间可能出现一个空
//隙，我们规定应当以 "\0" 填充这段空间。
off_t rseek(int fd, off_t offset, int whence) {
    //judge fd is valid
    if (fd < 0 || fd >= MAX_FD_COUNT || fd_table.fds[fd] == NULL) {
        return -1;
    }
    switch (whence) {
        case SEEK_SET:
            fd_table.fds[fd]->offset = offset;
            break;
        case SEEK_CUR:
            fd_table.fds[fd]->offset += offset;
            break;
        case SEEK_END:
            fd_table.fds[fd]->offset = fd_table.fds[fd]->file->size + offset;
            break;
        default:
            return -1;//invalid whence
    }
    if (fd_table.fds[fd]->offset <= 0)
        fd_table.fds[fd]->offset = 0;
    return fd_table.fds[fd]->offset;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    //judge fd is valid
    if (fd < 0 || fd >= MAX_FD_COUNT || fd_table.fds[fd] == NULL) {
        return -1;
    }
    //judge buf is valid
    if (buf == NULL || count <= 0) {
        return -1;
    }
    Fd *pfd = fd_table.fds[fd];
    //judge pfd is able to write
    if (!((pfd->flags & O_WRONLY) | (pfd->flags & O_RDWR))) {
        return -1;
    }

    //空间不够申请空间
    if (pfd->file->size < pfd->offset + count) {
        char *tmp = (char *) malloc(sizeof(char) * (pfd->offset + count));
        memset(tmp, 0, (pfd->offset + count));
        //复制原内容
        for (int i=0 ; i<pfd->file->size ; i++)
            tmp[i] = pfd->file->content[i];
        if(pfd->file->content!=NULL)
            free(pfd->file->content);
        pfd->file->content = tmp;
    }
    int i = 0;
    while (i < count)
        pfd->file->content[pfd->offset++] = ((char *) buf)[i++];

    if(pfd->file->size < pfd->offset)
        pfd->file->size = pfd->offset;

    return i;
}

ssize_t rread(int fd, void *buf, size_t count) {
    //judge fd is valid
    if (fd < 0 || fd >= MAX_FD_COUNT || fd_table.fds[fd] == NULL) {
        return -1;
    }
    //judge buf is valid
    if (buf == NULL || count <= 0) {
        return -1;
    }
    Fd *pfd = fd_table.fds[fd];
    //judge pfd is able to write
    if (!((pfd->flags & O_RDONLY) | (pfd->flags & O_RDWR))) {
        return -1;
    }
    int i;
    for (i = 0; i < count && pfd->offset < pfd->file->size; ++i)
        ((char *) buf)[i] = pfd->file->content[pfd->offset++];
    return i;
}
