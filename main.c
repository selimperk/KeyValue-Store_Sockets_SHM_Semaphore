#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "keyValStore.h"  // Header-Datei für die Datenhaltung

#define PORT 5678          // TCP-Port, auf dem der Server hört
#define BUFFER_SIZE 256    // Maximale Größe des Buffers für Nachrichten
#define MAX_ENTRIES 10

// Die Funktion, die den Server startet
void start_server() {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[BUFFER_SIZE];  // Puffer zum Speichern der eingehenden Daten
    int n;

    // Erstelle einen TCP-Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR beim Öffnen des Sockets");
        exit(1);
    }

    // Setze die Serveradresse
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;  // Der Server hört auf allen verfügbaren Interfaces
    serv_addr.sin_port = htons(PORT);  // Port 5678

    // Binde den Socket an die Adresse
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR beim Binden des Sockets");
        exit(1);
    }

    // Warten auf eingehende Verbindungen
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Akzeptiere eine eingehende Verbindung
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR beim Akzeptieren");
        exit(1);
    }

    // Kommunikationsschleife
    while (1) {
        bzero(buffer, BUFFER_SIZE);  // Leere den Buffer
        n = read(newsockfd, buffer, BUFFER_SIZE - 1);  // Lese Daten vom Client
        if (n < 0) {
            perror("ERROR beim Lesen vom Socket");
            exit(1);
        }

        // Extrahiere den Befehl, Schlüssel und den Wert
        char command[10], key[50], value[200];

        // Extrahiere den Befehl und den Schlüssel (bzw. Rest des Textes als Wert)
        sscanf(buffer, "%s %s %[^\n]", command, key, value);  // Format: Befehl Schlüssel Wert

        // Verarbeitung der Befehle
        if (strcmp(command, "QUIT") == 0) {
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
    close(sockfd);     // Schließe den Server-Socket
}

int main() {
    start_server();  // Starte den Server
    return 0;
}
