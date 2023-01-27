//
// Created by xgs on 23-1-26.
//
#include <malloc.h>
#include <string.h>
#include "ramfs.h"

#define FILE      0
//file system
FdTable fd_table;
Folder *root;

//init file system
void init_ramfs() {
    //init file descriptor table
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
            file = create_file(pathname);
        } else {
            //file or directory not found
            return -1;
        }
    }
    //create file descriptor
    Fd *fd = (Fd *) malloc(sizeof(Fd));
    fd->offset = 0;
    fd->file = file;
    fd->flags = flags;

    //find empty file descriptor
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        if (fd_table.fds[i] == NULL) {
            fd_table.fds[i] = fd;
            return i;
        }
    }
    return  -1;//no empty file descriptor
}

//create file or directory ,choose type FILE or DIRECTORY
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
    //create file or directory
    File *file = (File *) malloc(sizeof(File));
    file->size = 0;
    file->parent = parent;
    strcpy(file->name, name);
    //add file or directory to parent directory
    if (InsertHashMap(parent->fileSet, name, (intptr_t) file)) {
        //add file or directory to parent directory failed
        return file;
    }
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
    return RemoveHashMap(folder->parent->folderSet, folder->name);
}


//clear file path
char *clean_path(char *pathname) {
    //invalid path
    if (pathname == NULL || strlen(pathname) == 0 || strlen(pathname) > MAX_FILE_NAME_LENGTH || pathname[0] != '/') {
        return NULL;
    }
    char *tmp = (char *) malloc(sizeof(char) * strlen(pathname) + 1);
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
int rmkdir(const char *path) {
    char *pathname = clean_path(path);
    if (pathname == NULL) {
        return -1;
    }
    //create directory
    Folder *folder = create_dir(pathname);
    free(pathname);
    if (folder == NULL) {
        //create directory failed
        return -1;
    }
    return 1;
}


int rrmdir(const char *pathname) {
    char *path = clean_path(pathname);
    if (path == NULL) {
        return -1;
    }
    return delete_dir(path);
}

int runlink(const char *pathname){
    char *path = clean_path(pathname);
    if (path == NULL) {
        return -1;
    }
    return delete_file(path);
}

int rclose(int fd){
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    if (fd_table.fds[fd] == NULL) {
        return -1;
    }
    free(fd_table.fds[fd]);
    fd_table.fds[fd] = NULL;
    return 1;
}