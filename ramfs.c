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
    memset(fd_table.fds,0,MAX_FD_COUNT*sizeof (Fd*));
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
    file->parent = parent;
    strcpy(file->name, name);
    //add file to parent directory
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
char *clean_path(const char *pathname) {
    //invalid path
    if (pathname == NULL || strlen(pathname) == 0 || strlen(pathname) > MAX_FILE_NAME_LENGTH || pathname[0] != '/') {
        return NULL;
    }
    if(strrchr(pathname,'.')!=NULL && strrchr(pathname,'/')!=NULL && strrchr(pathname,'.') - strrchr(pathname,'/')<0)
        return NULL;
    char *tmp;
    int p = 0;
    tmp = (char *) malloc(sizeof(char) * strlen(pathname) + 1);
    memset(tmp,0, strlen(pathname)+1);
    char *pathname1 = strdup(pathname);
    char *tok = strtok(pathname1, "/");
    while (tok!=NULL){
        tmp[p++] = '/';
        for (int i = 0;i < strlen(tok);i++)
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
    if (parent == NULL) {
        //parent directory not found
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

//SEEK_SET 0 ���ļ���������ƫ�������õ� offset ָ���λ��
//SEEK_CUR 1 ���ļ���������ƫ�������õ� ��ǰλ�� + offset �ֽڵ�λ��
//SEEK_END 2 ���ļ���������ƫ�������õ� �ļ�ĩβ + offset �ֽڵ�λ��
//rseek ����ƫ�������õ��ļ�ĩβ֮���λ�ã����ǲ�����ı��ļ��Ĵ�С��ֱ���������λ��д��
//�����ݡ��� �����ļ�ĩβ�ĵط�д�������ݺ�ԭ�����ļ�ĩβ��ʵ��д��λ��֮����ܳ���һ����
//϶�����ǹ涨Ӧ���� "\0" �����οռ䡣
off_t rseek(int fd, off_t offset, int whence){
    switch (whence) {
        case SEEK_SET:fd_table.fds[fd]->offset = offset;break;
        case SEEK_CUR:fd_table.fds[fd]->offset += offset;break;
        case SEEK_END:fd_table.fds[fd]->offset = fd_table.fds[fd]->file->size+offset;break;
    }
    if (fd_table.fds[fd]->offset<=0)
        fd_table.fds[fd]->offset = 0;
    return fd_table.fds[fd]->offset;
}

ssize_t rwrite(int fd, const void *buf, size_t count){
    Fd * pfd = fd_table.fds[fd];
    //�ռ䲻������ռ�
    if(pfd->file->size < pfd->offset+count){
        char * tmp = (char *) malloc(sizeof (char )* (pfd->offset+count));
        memset(tmp,0,(pfd->offset+count));
        strcpy(tmp,pfd->file->content);
        free(pfd->file->content);
        pfd->file->content = tmp;
    }
    int i=0;
    while ( i < count)
        pfd->file->content[pfd->offset++] = ((char *)buf)[i++];
    return i;
}

ssize_t rread(int fd, void *buf, size_t count){
    Fd * pfd = fd_table.fds[fd];
    int i = 0;
    for ( i = 0; i < count && pfd->offset < pfd->file->size; ++i)
        ((char *)buf)[i] = pfd->file->content[pfd->offset++];
    return i;
}