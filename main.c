#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "keyValStore.h" // Header-Datei für die Datenhaltung

#define PORT 5678          // TCP-Port, auf dem der Server hört
#define BUFFER_SIZE 256    // Maximale Größe des Buffers für Nachrichten

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
    bzero((char *) &serv_addr, sizeof(serv_addr));  // Setze die Struktur auf Null
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;  // Der Server hört auf allen verfügbaren Interfaces
    serv_addr.sin_port = htons(PORT);  // Port 5678

    // Binde den Socket an die Adresse
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR beim Binden des Sockets");
        exit(1);
    }

    // Warten auf eingehende Verbindungen
    listen(sockfd, 5); //warum n=5?..........................................
    clilen = sizeof(cli_addr); //warum clilen? was ist das?....................................

    // Akzeptiere eine eingehende Verbindung
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR beim Akzeptieren");
        exit(1);
    }

    // Kommunikationsschleife
    while (1) {
        bzero(buffer, BUFFER_SIZE);  // Leere den Buffer
        n = read(newsockfd, buffer, BUFFER_SIZE - 1);  // Lese Daten vom Client --> warum buffersize -1?............
        if (n < 0) {
            perror("ERROR beim Lesen vom Socket");
            exit(1);
        }

        // Parsing der Eingabe (Befehl, Schlüssel, Wert) ---> warum muss geparsed werden?....................
        char command[10], key[50], value[100];
        int parsed = sscanf(buffer, "%s %s %s", command, key, value);  // Zerlege die Eingabe in Befehl, Schlüssel, Wert

        // Verarbeitung der Befehle
        if (strcmp(command, "QUIT") == 0) {
            write(newsockfd, "Bis zum nächsten Mal!\n", 8);  // Sende String und beende die Verbindung
            break;
        }

        // PUT Befehl
        if (strcmp(command, "PUT") == 0 && parsed == 3) { //warum parsed?..................................
            if (put(key, value) == 0) {  // Versuche, den Wert zu setzen
                sprintf(buffer, "PUT:%s:%s", key, value);  // Formatierte Antwort --> wozu?...................................
                write(newsockfd, buffer, strlen(buffer));  // Sende Antwort an den Client --> warum strlen von buffer?..........................
            } else {
                write(newsockfd, "Error: Store ist voll!\n", 21);  // warum n=21?...........................................................
            }
        }
        // GET Befehl
        else if (strcmp(command, "GET") == 0 && parsed == 2) { //warum parsed?..................................
            if (get(key, value) == 0) {  // Versuche, den Wert zu holen
                sprintf(buffer, "GET:%s:%s", key, value);  // Formatierte Antwort
                write(newsockfd, buffer, strlen(buffer));  // Sende Antwort an den Client
            } else {
                write(newsockfd, "GET:key_nonexistent\n", 20);  // Fehler, wenn der Schlüssel nicht existiert
            }
        }
        // DEL Befehl
        else if (strcmp(command, "DEL") == 0 && parsed == 2) {
            if (del(key) == 0) {  // Versuche, den Wert zu löschen
                sprintf(buffer, "DEL:%s:key_deleted", key);  // Formatierte Antwort
                write(newsockfd, buffer, strlen(buffer));  // Sende Antwort an den Client
            } else {
                write(newsockfd, "DEL:key_nonexistent\n", 20);  // Fehler, wenn der Schlüssel nicht existiert
            }
        }
        // Ungültiger Befehl
        else {
            write(newsockfd, "Error: Ungültiger Befehl\n", 23);
        }
    }

    close(newsockfd);  // Schließe die Client-Verbindung
    close(sockfd);     // Schließe den Server-Socket
}

int main() {
    start_server();  // Starte den Server
    return 0;
}