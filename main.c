#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "keyValStore.h"  // Header-Datei für die Datenhaltung
#include <sys/ipc.h>
#include <sys/shm.h>

#define PORT 5678          // TCP-Port, auf dem der Server hört
#define BUFFER_SIZE 256    // Maximale Größe des Buffers für Nachrichten
#define MAX_ENTRIES 10


//Kommunikation GET,PUT,DEL ausgelagert
void handle_client(int newsockfd) {
    char buffer[BUFFER_SIZE];
    int n;

    while (1) {
        //bzero(buffer, BUFFER_SIZE);  // buffer leeren

        n = read(newsockfd, buffer, BUFFER_SIZE - 1);  // daten vom client lesen

        // Extrahiere den Befehl, Schlüssel und den Wert
        char command[10], key[50], value[200];
        // Extrahiere den Befehl und den Schlüssel (bzw. Rest des Textes als Wert)
        sscanf(buffer, "%s %s %[^\n]", command, key, value);  // Format: Befehl Schlüssel Wert

        // Verarbeitung der Befehle
        if (strcmp(command, "QUIT") == 0) {
            printf("Der Kindprozess mit der ID %d hat den Server verlassen\n", getpid());
            write(newsockfd, buffer, strlen(buffer));  // Sende String und beende die Verbindung
            break;
        }

        // PUT Befehl (ohne Parsing des Werts)
        if (strcmp(command, "PUT") == 0) {
            char oldValue[100];// Versuche, den Wert zu setzen

            //alten wert speichern
            for (int i=0; i< currentIndex; i++) {
                if (strcmp(store[i].key,key)==0) {
                    strcpy(oldValue, store[i].value);
                    break;
                }
            }
            //versuchen wert zu setzen
            int result = put(key,value);

            if (result == 0) {
                // Kein alter Wert, einfach den neuen Wert zurückgeben
                sprintf(buffer, "PUT:%s:%s\n", key, value);
                write(newsockfd, buffer, strlen(buffer));
            } else {
                // Wenn überschrieben, gib den alten Wert aus
                sprintf(buffer, "PUT:%s:%s\n", key, oldValue);
                write(newsockfd, buffer, strlen(buffer));
            }
        }

        // GET Befehl
        else if (strcmp(command, "GET") == 0) {
            if (get(key, value) == 0) {  // Versuche, den Wert zu holen
                sprintf(buffer, "GET:%s:%s\n", key, value);  // Formatierte Antwort
                write(newsockfd, buffer, strlen(buffer));  // Sende Antwort an den Client
            } else {
                sprintf(buffer, "GET:%s:key_nonexistent\n", key);  // Fehlermeldung mit Key
                write(newsockfd, buffer, strlen(buffer));  // Fehlerantwort
            }
        }

        // DEL Befehl
        else if (strcmp(command, "DEL") == 0) {
            if (del(key) == 0) {  // Versuche, den Wert zu löschen
                sprintf(buffer, "DEL:%s:key_deleted\n", key);  // Formatierte Antwort
                write(newsockfd, buffer, strlen(buffer));  // Sende Antwort an den Client
            } else {
                sprintf(buffer, "DEL:%s:key_nonexistent\n", key);  // Fehler, wenn der Key nicht existiert
                write(newsockfd, buffer, strlen(buffer));  // Fehlerantwort
            }
        }

        // Ungültiger Befehl
        else {
            write(newsockfd, "Error: Ungültiger Befehl\n", strlen(buffer));
        }
    }

    close(newsockfd);  // Schließe die Client-Verbindung
}



// Die Funktion, die den Server startet
void start_server() {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[BUFFER_SIZE];  // Puffer zum Speichern der eingehenden Daten
    int n;

    // Erstelle einen TCP-Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Setze die Serveradresse
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;  // Der Server hört auf allen verfügbaren Interfaces
    serv_addr.sin_port = htons(PORT);  // Port 5678

    // Binde den Socket an die Adresse
    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));



    // Warten auf eingehende Verbindungen
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Ausgabe der Prozess-IDs
    printf("Server gestartet.\n");
    printf("Die Prozess ID des Vaters (Elternprozess) ist: %d\n", getppid());



    // Akzeptiere eine eingehende Verbindung
    // Kommunikationsschleife
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        //Kindprozess erzeugen
        pid_t pid = fork();

        if (pid == 0) {
            close(sockfd); //kindprozess braucht haupptsocket nicht mehr
            printf("Neuer Client verbunden!\n");
            printf("Die Prozess ID des Vaters (Elternprozess) ist: %d\n", getppid());
            printf("Die Prozess ID des aktuellen Client-Handlers (Kindprozess) ist: %d\n", getpid());
            handle_client(newsockfd); //kommunikation mit put,get,del befehlen
            exit(0);
        } else {
            close(newsockfd); //hauptprozess braucht client socket nicht mehr
        }
    }

    close(sockfd);
}




int main() {
    init_shared_memory();
    start_server();  // Starte den Server
    cleanup_shared_memory();
    return 0;
}
