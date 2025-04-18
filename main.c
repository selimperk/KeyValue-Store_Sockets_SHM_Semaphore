#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h> //struct für sockaddr_in und in_addr
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int main(void) {

    //socket erstellen
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("creating stream socket");
        exit(2);
    }

    //struktur von sock_addr füllen
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5678);
    //socket binden
    bind(sock, (struct sockaddr *)&server, sizeof(server));

    //auf verbindung hören n=5 ist warteschlange
    int ret = listen(sock, 5);
    //verbindung akzeptieren
    int connection_fd = accept(sock, (struct sockaddr *)NULL, NULL); //warum NULL?

    //datei empfangen
    int number_bytes = recv(connection_fd, (void *)&number_bytes, sizeof(number_bytes), 0);
    //datei senden
    int number_bytes_send=send(connection_fd, (void *)&number_bytes, sizeof(number_bytes), 0);


    //SHM einrichten --> Multiclientfähig werden --> gegenseitig daten austauschen
    int id,result,*shar_mem; //so richtig mit pointer?
    //shmget neues segment anlegen
    id=shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0600);
    //shmat Addressraum anhängen stat NULL 0? evtl casten?
    shar_mem =(int *)shmat(id, 0, 0);
    //shmdt Addressraum aushängen
    result=shmdt(shar_mem);
    //shmctl steuerung
    result=shmctl(id, IPC_RMID, NULL); //hier nur vorübergehend..RMID ist cmd
    return 0;
}
