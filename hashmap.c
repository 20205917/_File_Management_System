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
    // ����һ��node�ڵ�
    HashNode* node = (HashNode*)calloc(1, sizeof(HashNode));
    if (node == NULL) {
        return -1;// �ڴ����ʧ�ܣ�����-1
    }

    // ���Ƽ���ֵ
    node->key = strdup(key);
    node->value = value;
    node->type = t;
    node->next = NULL;
    // ��hash������࣬��ȡkeyλ��
    int index = myHash(key) % DEFAULT_HASHMAP_SIZE;
    // �����ǰλ��û��node���ͽ�������node����
    if (hashMap->hashArr[index] == NULL)
        hashMap->hashArr[index] = node;
        // �����Ҫ�����е�node�������
    else {
        // ���ڱ���node����ʱ�α�
        HashNode *temp = hashMap->hashArr[index];
        // ��¼��һ��node
        HashNode *prev = temp;
        // ѭ�����������һ���ڵ�node_end��next
        while (temp != NULL) {
            // �������key��ͬ����ֱ������node��value���Ǿɵ�
            if (strcmp(temp->key, node->key) == 0) {
                // ������value
                temp->value = value;
                return 0; // ����1��ʾ����ɹ�
            }
            prev = temp;
            temp = temp->next;
        }
        // ���һ���ڵ�node_end��nextָ���½���node
        prev->next = node;
    }
    hashMap->size++;
    return 0;
}

intptr_t GetHashMap(HashMap* hashMap,int t,char* key)
{
    // ��hash������࣬��ȡkeyλ��
    int index = myHash(key) % DEFAULT_HASHMAP_SIZE;
    // ���ڱ���node����ʱ�α�
    HashNode *temp = hashMap->hashArr[index];
    // ѭ�����������һ���ڵ�node_end��next
    while (temp != NULL) {
        // �������key��ͬ��������node��value���Ǿɵ�
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
        // ���ڱ���node����ʱ�α�
        HashNode *temp = hashMap->hashArr[i];
        HashNode *prev;
        // ѭ�����������һ���ڵ�node_end��next
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
    // ��hash������࣬��ȡkeyλ��
    int index = myHash(key) % hashMap->size;
    // ���ڱ���node����ʱ�α�
    HashNode *temp = hashMap->hashArr[index];
    if (temp == NULL)
        return -1;

    // �����һ����ƥ����
    if (strcmp(temp->key, key) == 0) {
        hashMap->hashArr[index] = temp->next;
        free(temp->key);
        free(temp);
        return 0;
    }
    else {
        HashNode *prev = temp;
        temp = temp->next;
        // ѭ�����������һ���ڵ�node_end��next
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
