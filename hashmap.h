//
// Created by 86136 on 2023/1/27.
//
#ifndef _FILE_MANAGEMENT_SYSTEM_HASHMAP_H
#define _FILE_MANAGEMENT_SYSTEM_HASHMAP_H

#include <stdint.h>

typedef struct HashNode {
    char *key;

    intptr_t value;

    struct HashNode *next; // 当key相同时，指向集合中的下一个节点

} HashNode;


typedef struct HashMap {
    int size; // hash map不重复node的数量

    HashNode **hashArr; // 存key值不重复的node，key重复的node链接在HashNode->next

} HashMap;

int myHash(char *x);

HashMap *CreateHashMap(int n);

int InsertHashMap(HashMap *hashMap, char *key, intptr_t value);

intptr_t GetHashMap(HashMap *hashMap, char *key);

void DeleteHashMap(HashMap *hashMap);

int RemoveHashMap(HashMap *hashMap, char *key);

void PrintHashMap(HashMap *hashMap);

void hashMapTest(void);


#endif //_FILE_MANAGEMENT_SYSTEM_HASHMAP_H
