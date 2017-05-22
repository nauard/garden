#include <stdio.h>
#include "csock.h"
#include <unistd.h>
#include "spec.h"

int main(int argc, char *argv[]) {
    char *service = (char *) "5656"; // notre n° de port
    char *host = (char *)"127.0.0.1";
    int comm; // Socket de communication (active)
    int n, nbRead;
    char msg[256];

    /* récupération des arguments */
    switch (argc) {
    case 3: // 2 params : le 2ème est le n° de port
        service = argv[2]; // n° de port
    case 2:	// 1 ou 2 params : le 1er est soit -s, soit une machine
        host = argv[1];
        if (!strcmp(host, "-h")) {
            fprintf(stderr, "usage: \n");
            fprintf(stderr, "%s [machine_serveur [port]]\n", argv[0]);
            fprintf(stderr, "%s -h pour l'aide\n", argv[0]);
            fprintf(stderr, "se connecte par defaut en localhost:5656\n");
            return -1;
        }
    }
	
    comm = client_socket(host, service, "tcp");
    // Si pas de serveur, on quitte
    if (comm < 0) {
        return 1;
    }

    char *answer = (char *) malloc(ANSWER_SIZE);

    /* Read the welcome message from the server */
    bzero(answer,ANSWER_SIZE);
    nbRead = read(comm, answer, ANSWER_SIZE);
    if (nbRead < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("\n(received %d bytes from server:)\n", nbRead);
    printf("%s", answer);

    while (strncmp(answer, "stopping", 8)) {
        bzero(msg,256);
        printf("\n\n\nentrez votre commande (exit pour quitter):\n");
        fgets(msg,255,stdin);
        //printf("\n\n");
        /* test special commands */
        if (!strncmp( msg, "exit", 4) ) {
            printf("bye\n");
            exit(0);
        }
        /* Send message to the server */
        n = write(comm, msg, strlen(msg));
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        /* Now read server response */
        bzero(answer,ANSWER_SIZE);
        nbRead = read(comm, answer, ANSWER_SIZE);
        if (nbRead < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        printf("\n(received %d bytes from server:)\n", nbRead);
        printf("%s", answer);
    }
    close(comm);

    free(answer);
    
    return 0;
}
