#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include "keyValStore.h"

SharedData *shared_data = NULL;

int init_store() {
    key_t key = 1234;
    int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        return -1;
    }

    shared_data = (SharedData*) shmat(shmid, NULL, 0);
    if (shared_data == (void*) -1) {
        perror("shmat");
        return -1;
    }

    if (shared_data->store_count == 0) {
        memset(shared_data, 0, sizeof(SharedData));
        shared_data->owner_pid = -1;
    }

    return 0;
}

static int find_index(const char* key) {
    for (int i = 0; i < shared_data->store_count; i++) {
        if (strcmp(shared_data->store[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

int put(const char* key, const char* value) {
    int index = find_index(key);
    if (index >= 0) {
        strncpy(shared_data->store[index].value, value, sizeof(shared_data->store[index].value));
        notify_subscribers(key, "PUT", value);
        return 1;
    }
    if (shared_data->store_count >= MAX_STORE) return -1;

    strncpy(shared_data->store[shared_data->store_count].key, key, sizeof(shared_data->store[shared_data->store_count].key));
    strncpy(shared_data->store[shared_data->store_count].value, value, sizeof(shared_data->store[shared_data->store_count].value));
    shared_data->store_count++;

    notify_subscribers(key, "PUT", value);
    return 0;
}

int get(const char* key, char* res) {
    int index = find_index(key);
    if (index >= 0) {
        strncpy(res, shared_data->store[index].value, 256);
        return 0;
    }
    return -1;
}

int del(const char* key) {
    int index = find_index(key);
    if (index >= 0) {
        notify_subscribers(key, "DEL", "key_deleted");
        for (int i = index; i < shared_data->store_count - 1; i++) {
            shared_data->store[i] = shared_data->store[i + 1];
        }
        shared_data->store_count--;
        return 0;
    }
    return -1;
}

void subscribe(const char* key, int pid) {
    for (int i = 0; i < MAX_STORE; i++) {
        if (strcmp(shared_data->subscriptions[i].key, key) == 0 ||
            shared_data->subscriptions[i].key[0] == '\0') {

            if (shared_data->subscriptions[i].key[0] == '\0')
                strncpy(shared_data->subscriptions[i].key, key, sizeof(shared_data->subscriptions[i].key));

            for (int j = 0; j < shared_data->subscriptions[i].sub_count; j++) {
                if (shared_data->subscriptions[i].subscribers[j] == pid)
                    return;
            }

            shared_data->subscriptions[i].subscribers[shared_data->subscriptions[i].sub_count++] = pid;
            return;
        }
    }
}

void notify_subscribers(const char* key, const char* action, const char* value) {
    for (int i = 0; i < MAX_STORE; i++) {
        if (strcmp(shared_data->subscriptions[i].key, key) == 0) {
            for (int j = 0; j < shared_data->subscriptions[i].sub_count; j++) {
                int pid = shared_data->subscriptions[i].subscribers[j];
                char fifo_name[64];
                snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", pid);

                FILE* fifo = fopen(fifo_name, "w");
                if (fifo) {
                    fprintf(fifo, "%s:%s:%s\n", action, key, value);
                    fclose(fifo);
                }
            }
        }
    }
}
