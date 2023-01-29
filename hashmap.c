#include <string.h>
#include <stdlib.h>
#include "hashmap.h"



HashMap* CreateHashMap(int n)
{
    HashMap* hashMap = (HashMap*)calloc(1, sizeof(HashMap));
    hashMap->hashArr = (HashNode**)calloc(n, sizeof(HashNode*));
    if (hashMap==NULL || hashMap->hashArr==NULL) {
        return NULL;
    }
    hashMap->size = 0;
    return hashMap;
}


int InsertHashMap(HashMap* hashMap, char* key,int t, intptr_t value)
{
    // 创建一个node节点
    HashNode* node = (HashNode*)calloc(1, sizeof(HashNode));
    if (node == NULL) {
        return -1;// 内存分配失败，返回-1
    }

    // 复制键和值
    node->key = strdup(key);
    node->value = value;
    node->type = t;
    node->next = NULL;
    // 对hash结果求余，获取key位置
    int index = myHash(key) % DEFAULT_HASHMAP_SIZE;
    // 如果当前位置没有node，就将创建的node加入
    if (hashMap->hashArr[index] == NULL)
        hashMap->hashArr[index] = node;
        // 否则就要在已有的node往后添加
    else {
        // 用于遍历node的临时游标
        HashNode *temp = hashMap->hashArr[index];
        // 记录上一个node
        HashNode *prev = temp;
        // 循环遍历至最后一个节点node_end的next
        while (temp != NULL) {
            // 如果两个key相同，则直接用新node的value覆盖旧的
            if (strcmp(temp->key, node->key) == 0) {
                // 复制新value
                temp->value = value;
                return 0; // 返回1表示插入成功
            }
            prev = temp;
            temp = temp->next;
        }
        // 最后一个节点node_end的next指向新建的node
        prev->next = node;
    }
    hashMap->size++;
    return 0;
}

intptr_t GetHashMap(HashMap* hashMap,int t,char* key)
{
    // 对hash结果求余，获取key位置
    int index = myHash(key) % DEFAULT_HASHMAP_SIZE;
    // 用于遍历node的临时游标
    HashNode *temp = hashMap->hashArr[index];
    // 循环遍历至最后一个节点node_end的next
    while (temp != NULL) {
        // 如果两个key相同，则用新node的value覆盖旧的
        if (temp->type = t && strcmp(temp->key, key) == 0)
            return temp->value;
        temp = temp->next;
    }
    return (intptr_t)NULL;
}


void DeleteHashMap(HashMap* hashMap)
{
    for (int i = 0; i < hashMap->size; i++)
    {
        // 用于遍历node的临时游标
        HashNode *temp = hashMap->hashArr[i];
        HashNode *prev;
        // 循环遍历至最后一个节点node_end的next
        while (temp != NULL) {
            prev = temp;
            temp = temp->next;
            free(prev->key);
            free(prev);
        }
    }
    free(hashMap->hashArr);
    hashMap->hashArr = NULL;
    free(hashMap);
    hashMap = NULL;
}


int RemoveHashMap(HashMap* hashMap, char* key)
{
    // 对hash结果求余，获取key位置
    int index = myHash(key) % hashMap->size;
    // 用于遍历node的临时游标
    HashNode *temp = hashMap->hashArr[index];
    if (temp == NULL)
        return -1;

    // 如果第一个就匹配中
    if (strcmp(temp->key, key) == 0) {
        hashMap->hashArr[index] = temp->next;
        free(temp->key);
        free(temp);
        return 0;
    }
    else {
        HashNode *prev = temp;
        temp = temp->next;
        // 循环遍历至最后一个节点node_end的next
        while (temp != NULL) {
            if (strcmp(temp->key, key) == 0) {
                prev->next = temp->next;
                free(temp->key);
                free(temp);
                return 0;
            }
            prev = temp;
            temp = temp->next;
        }
    }
    hashMap->size--;
    return -1;
}
// Path: hashmap.h
int myHash(char *x) {
    int hash = 0;
    char *s;
    for (s = x; *s; s++)
        hash = hash * 31 + *s;
    return hash;
}

HashMap *DefaultHashMap(){
    return CreateHashMap(DEFAULT_HASHMAP_SIZE);
}
