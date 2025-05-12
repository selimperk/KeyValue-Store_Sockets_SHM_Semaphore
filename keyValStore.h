#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H
#define MAX_ENTRIES 10

typedef struct {
    char key[50];
    char value[100];
} KeyValue;

extern KeyValue *store;
extern int currentIndex;
extern int shm_id;

void init_shared_memory();  // Deklaration von init_shared_memory
void cleanup_shared_memory();  // Deklaration von cleanup_shared_memory

int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);

#endif
