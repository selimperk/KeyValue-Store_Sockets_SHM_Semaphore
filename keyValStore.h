#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#define MAX_STORE 50
#define MAX_SUBS_PER_KEY 16

typedef struct {
    char key[64];
    char value[256];
} KeyValue;

typedef struct {
    char key[64];
    int subscribers[MAX_SUBS_PER_KEY];
    int sub_count;
} Subscription;

typedef struct {
    KeyValue store[MAX_STORE];
    Subscription subscriptions[MAX_STORE];
    int store_count;

    int lock;
    int owner_pid;
} SharedData;

extern SharedData *shared_data;

int init_store();
int put(const char* key, const char* value);
int get(const char* key, char* res);
int del(const char* key);
void subscribe(const char* key, int pid);
void notify_subscribers(const char* key, const char* action, const char* value);

#endif