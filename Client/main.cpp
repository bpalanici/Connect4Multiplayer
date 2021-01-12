#include "window.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QApplication>
#include <bits/stdc++.h>

using namespace std;


static int sd;

int Connect_to_server(char *argv[]) {
    int port;
    struct sockaddr_in server;	// structura folosita pentru conectare

    /* stabilim portul */
    port = atoi (argv[2]);

    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons (port);

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Window w;

   // if (argc != 3) {
        argc = 3;
        char c1[] = "127.0.0.1";//, c2[] = "8840";
        argv[1] = c1;
       // argv[2] = c2;
  //  }
    if (Connect_to_server(argv)) exit(1);
    char msg[105] = "";
    memset(msg, 0, sizeof (msg));
    if (read(sd, msg, 100) < 0) {
        printf("ERROR AT RECEIVING COLOR AND PLAYER\n");
        exit(1);
    }
    w.sd = sd;
    w.player_no = msg[0];
    w.move_player = w.player_no;
    w.color_player = msg[1];
    w.color_opponent = msg[2];
    memset(msg, 0, sizeof (msg));

    sprintf(msg, "Player %d", w.player_no);
    w.setWindowTitle(msg);
    printf("PLAYER AND COLOR RECEIVED : %d %d %d\n", w.player_no, w.color_player, w.color_opponent);
    fflush(stdout);
    w.preshow();
    w.show();
    return a.exec();
}
