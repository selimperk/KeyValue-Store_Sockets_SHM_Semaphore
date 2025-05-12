#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>

// Shared Memory ID
int shm_id, currentIndex;
// Zeiger auf den Shared Memory Bereich
KeyValue *store;

// Initialisierung von Shared Memory
void init_shared_memory() {
    // Erstelle Shared Memory für MAX_ENTRIES * KeyValue
    shm_id = shmget(IPC_PRIVATE, MAX_ENTRIES * sizeof(KeyValue), IPC_CREAT | 0600);
    // Binde den Shared Memory an den Adressraum des Prozesses
    store = (KeyValue *)shmat(shm_id, 0, 0);
}

// Funktion zum Entfernen des Shared Memorys
void cleanup_shared_memory() {
    // Trenne den Shared Memory Bereich vom Adressraum
    shmdt(store);
    // Lösche den Shared Memory Bereich
    shmctl(shm_id, IPC_RMID, NULL);
}





//wahrscheinlich hier ein fehler



// PUT: Fügt ein Key-Value-Paar hinzu oder überschreibt den Wert eines bestehenden Keys
int put(char* key, char* value) {
    // Überprüfen, ob der Schlüssel bereits existiert
    for (int i = 0; i < currentIndex; i++) {
        if (strcmp(store[i].key, key) == 0) {
            // Wert überschreiben
            strcpy(store[i].value, value);
            printf("Der Wert wurde überschrieben\n");
            return 1;
        }
    }

    // Wenn der Schlüssel nicht existiert, fügen wir ihn hinzu
    if (currentIndex < MAX_ENTRIES) {
        strcpy(store[currentIndex].key, key);
        strcpy(store[currentIndex].value, value);
        currentIndex++;
        printf("Der Schlüssel wurde erfolgreich hinzugefügt\n");
        return 0;
    }
    printf("Fehler: Der Store ist leider voll\n");
    return -1;
}

// GET: Gibt den Wert für einen gegebenen Schlüssel zurück
int get(char* key, char* res) {
    for (int i = 0; i < currentIndex; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strcpy(res, store[i].value);
            printf("Der Schlüssel wurde gefunden!\n");
            return 0;  // Erfolgreich gefunden
        }
    }
    printf("Fehler: Der Schlüssel wurde nicht gefunden\n"); //hier muss die namenskonvention rein..........
    return -1;  // Fehler: Schlüssel nicht gefunden
}

// DELETE: Löscht ein Key-Value-Paar
int del(char* key) {
    for (int i = 0; i < currentIndex; i++) {
        if (strcmp(store[i].key, key) == 0) {
            // Verschiebe alle folgenden Einträge um eins nach vorne
            for (int j = i; j < currentIndex - 1; j++) {
                store[j] = store[j + 1];
            }
            currentIndex--;  // Verringere den Index
            printf("Key-Value-Paar wurde erfolgreich gelöscht!\n");
            return 0;
        }
    }
    printf("Fehler: Kein Schlüssel gefunden!\n"); //hier namenskonvention rein...........
    return -1;
}
