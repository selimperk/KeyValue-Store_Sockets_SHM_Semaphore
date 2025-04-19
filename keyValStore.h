#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H
#define MAX_ENTRIES 10

typedef struct {
    char key[50];
    char value[100];
} KeyValue;

KeyValue store[MAX_ENTRIES];
int currentIndex;

int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);

#endif
