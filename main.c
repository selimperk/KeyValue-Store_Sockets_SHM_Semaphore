#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "keyValStore.h"

void strip(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = 0;
        len--;
    }
}

void handle_client(int conn) {
    char buffer[256], msg[512];
    pid_t pid = getpid();

    char fifo_name[64];
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", pid);
    mkfifo(fifo_name, 0666);

    FILE *f = fdopen(conn, "r+");

    if (fork() == 0) {
        FILE *fifo = fopen(fifo_name, "r");
        char notif[256];
        while (fgets(notif, sizeof(notif), fifo)) {
            fprintf(f, "%s", notif);
            fflush(f);
        }
        fclose(fifo);
        exit(0);
    }

    while (fgets(buffer, sizeof(buffer), f)) {
        strip(buffer);

        char *cmd = strtok(buffer, " ");
        if (!cmd) continue;

        if (strcmp(cmd, "BEG") == 0) {
            if (shared_data->lock == 0) {
                shared_data->lock = 1;
                shared_data->owner_pid = pid;
                fprintf(f, "OK: Transaction started\n");
            } else {
                fprintf(f, "ERROR: Transaction already active\n");
            }
            fflush(f);
            continue;
        }

        if (strcmp(cmd, "END") == 0) {
            if (shared_data->lock == 1 && shared_data->owner_pid == pid) {
                shared_data->lock = 0;
                shared_data->owner_pid = -1;
                fprintf(f, "OK: Transaction ended\n");
            } else {
                fprintf(f, "ERROR: You don't own the transaction\n");
            }
            fflush(f);
            continue;
        }

        if ((strcmp(cmd, "PUT") == 0 || strcmp(cmd, "GET") == 0 || strcmp(cmd, "DEL") == 0)
            && shared_data->lock == 1 && shared_data->owner_pid != pid) {
            fprintf(f, "ERROR: Store is locked by another client\n");
            fflush(f);
            continue;
        }

        if (strcmp(cmd, "SUB") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                subscribe(key, pid);
                char value[256];
                if (get(key, value) == 0)
                    fprintf(f, "SUB:%s:%s\n", key, value);
                else
                    fprintf(f, "SUB:%s:key_nonexistent\n", key);
                fflush(f);
            }
        }

        else if (strcmp(cmd, "PUT") == 0) {
            char *key = strtok(NULL, " ");
            char *val = strtok(NULL, "\n");
            if (key && val) {
                put(key, val);
                fprintf(f, "PUT:%s:%s\n", key, val);
                fflush(f);
            }
        }

        else if (strcmp(cmd, "GET") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                char res[256];
                if (get(key, res) == 0)
                    fprintf(f, "GET:%s:%s\n", key, res);
                else
                    fprintf(f, "GET:%s:key_nonexistent\n", key);
                fflush(f);
            }
        }

        else if (strcmp(cmd, "DEL") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                if (del(key) == 0)
                    fprintf(f, "DEL:%s:key_deleted\n", key);
                else
                    fprintf(f, "DEL:%s:key_nonexistent\n", key);
                fflush(f);
            }
        }

        else if (strcmp(cmd, "QUIT") == 0) {
            fprintf(f, "Goodbye\n");
            fflush(f);
            break;
        }
    }

    fclose(f);
    close(conn);
    unlink(fifo_name);
}

int main() {
    if (init_store() != 0) {
        fprintf(stderr, "Shared memory konnte nicht initialisiert werden.\n");
        return 1;
    }

    int sock_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    signal(SIGCHLD, SIG_IGN);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5678);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(sock_fd, 5);
    printf("Server läuft auf Port 5678...\n");

    while (1) {
        int conn = accept(sock_fd, (struct sockaddr*)&client_addr, &client_len);
        if (conn < 0) {
            perror("accept");
            continue;
        }

        if (fork() == 0) {
            close(sock_fd);
            handle_client(conn);
            exit(0);
        }

        close(conn);
    }

    close(sock_fd);
    return 0;
}