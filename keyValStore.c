#include "keyValStore.h"
#include <stdio.h>
#include <string.h>

#define MAX_ENTRIES 10  // Begrenzte Anzahl von Key-Value-Paaren --> muss das sein?............

// Struktur für Key-Value-Paare
typedef struct {
    char key[50];
    char value[100];
} KeyValue;

// Array für den Key-Value-Store
KeyValue store[MAX_ENTRIES];
int currentIndex = 0;  // Zeiger auf die nächste freie Stelle im Store

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
