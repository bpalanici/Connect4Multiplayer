#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdc++.h>

/* portul folosit */

using namespace std;

static struct sockaddr_in server;	// structura folosita de server
static struct sockaddr_in from1, from2; //descriptorul de socket
static int client[3];
static int sd;
static int PORT;

void Closing_Clients_connections() {
    close(client[1]);
    close(client[2]);
}

char receive_move(int player) {
    char msg[105];
    size_t msglength;
    memset(msg, 0, sizeof (msg));
    if (read(client[player], msg, 1) <= 0) {
        msg[0] = 'Q';
        msglength = 1;
        write(client[3 - player], msg, msglength);
        Closing_Clients_connections();
        exit(0);
    }
    char response = msg[0];
    printf("Received message from some player %d\n", (int)response);
    fflush(stdout);
    return response;
}

inline bool inside(int n, int m, int x, int y) {
    return (1 <= x && x <= m && 1 <= y && y <= n);
}

int analize_board(int board[15][15], int n, int m) {
    int dx[] = {0, 0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {0, -1, -1, 0, 1, 1, 1, 0, -1};
    for (int i = 1; i <= n; i++)
        for (int i1 = 1; i1 <= m; i1++)
            for (int i2 = 1; i2 <= 8; i2++) {
                int x = i1, y = i;
                int x1 = x + dx[i2], x2 = x1 + dx[i2], x3 = x2 + dx[i2];
                int y1 = y + dy[i2], y2 = y1 + dy[i2], y3 = y2 + dy[i2];
                if (inside(n, m, x3, y3) && board[y][x] != -1 &&
                    board[y][x] == board[y1][x1] && board[y][x] == board[y2][x2] && board[y][x] == board[y3][x3])
                    return board[y][x];
             }
    return -1;
}

void execute_game() {

    int child_pid;
    if ((child_pid = fork()) == -1) {
        Closing_Clients_connections();
        exit(0);
    } else if (child_pid > 0) {
        // parinte
        Closing_Clients_connections();
        return;
    }

    //-----------------------------------------------------------------
    // copil
    srand(time(nullptr));
    close(sd);
    int start_player = 1, nr_lines, nr_columns, nr_games = 0, move_no, player, result;
    char move;
    size_t msglength = 0;
    char msg[105];
    int board[15][15], colors[3] = {0, 0, 0}, column_open_position[15];
    vector< vector<int> > games_record;
    vector <int> game_moves;
    pair<int, int> score;
    score = make_pair(0, 0);

    //DECLARATIONS
    //------------------------------------------------------------------
    /* s-a realizat conexiunea, se astepta mesajul */

    bzero (msg, 100);
    printf ("[server]Trimitem mesaj jocatorului 1 ca e jucatorul 1...\n");
    fflush (stdout);

    colors[1] = rand() % 5;
    colors[2] = rand() % 5;
    while (colors[1] == colors[2]) colors[2] = rand() % 5;
    msg[0] = 1;
    msg[1] = static_cast<char>(colors[1]);
    msg[2] = static_cast<char>(colors[2]);
    msg[3] = 0;
    msglength = 3;
    if (write(client[1], msg, msglength) <= 0) {
        printf("Error sending players initial identity message");
        Closing_Clients_connections();
        exit(0);		/* continuam sa ascultam */
    }

    msg[0] = 2;
    msg[1] = static_cast<char>(colors[2]);
    msg[2] = static_cast<char>(colors[1]);
    msg[3] = 0;
    msglength = 3;
    if (write(client[2], msg, msglength) <= 0) {
        printf("Error sending players initial identity message");



        Closing_Clients_connections();
        exit(0);		/* continuam sa ascultam */
    }

    printf ("[server]La fel jucatorul 2...Asteptam dimensiunea tablei\n");
    fflush (stdout);

    /* citirea dimensiunii tablei */
    if (read (client[1], msg, 100) <= 0) {
        perror ("[server]Eroare la read() de la client.\n");
        Closing_Clients_connections();
        exit(0);		/* continuam sa ascultam */
    }
    if (write(client[2], msg, strlen(msg)) <= 0) {
        printf("Error sending players initial identity message");
        Closing_Clients_connections();
        exit(0);		/* continuam sa ascultam */
    }
    printf ("[server]Dim tablei a fost receptionata si trimisa la j2...%s\n", msg);
    fflush(stdout);
    {
        char *p = strtok(msg, " \n x");
        nr_lines = atoi(p);
        p = strtok(nullptr, " \n x");
        nr_columns = atoi(p);
    }

    bzero (msg, 100);
    msglength = 0;
    printf ("[server] LINK STARTO::\n");
    fflush (stdout);
    // PRE GAME PREPARATION
    //------------------------------------------------------
    //GAME STARTED
    for (;;) {
        game_moves.clear();
        for (int i = 0; i <= 10; i++)
            for (int i1 = 1; i1 <= 10; i1++)
                board[i][i1] = -1;
        for (int i = 1; i <= nr_columns; i++)
            column_open_position[i] = 1;

        nr_games++;
        printf("164 "); fflush(stdout);
        move_no = 1;
        player = start_player;
        game_moves.push_back(start_player);
        while (move_no <= nr_lines * nr_columns) {
            move_no++;
            move = receive_move(player);
            printf("164 "); fflush(stdout);
            if (move == 'r') {
                if (player == 1) {
                    score.second++;
                    start_player = 2;
                    msg[0] = 'W';
                    msg[1] = 0;
                    msglength = 1;
                    if (write(client[2], msg, msglength) <= 0 ) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[1], msg, msglength);
                        Closing_Clients_connections();
                    }
                    break;
                }
                else {
                    score.first++;
                    start_player = 1;
                    msg[0] = 'W';
                    msg[1] = 0;
                    msglength = 1;
                    if (write(client[1], msg, msglength) <= 0 ) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[2], msg, msglength);
                        Closing_Clients_connections();
                    }
                    break;
                }
            }
            if (move == 'q') {
                if (player == 1) {
                    msg[0] = 'q';
                    msg[1] = 0;
                    msglength = 1;
                    write(client[2], msg, msglength);
                }
                else {
                    msg[0] = 'q';
                    msg[1] = 0;
                    msglength = 1;
                    write(client[1], msg, msglength);
                }
                Closing_Clients_connections();
                exit(0);
            }
            if (move == 'R') {
                int game_request;
                if (read(client[player], &game_request, sizeof(int)) <= 0) {
                    msg[0] = 'Q';
                    msglength = 1;
                    if (write(client[3 - player], msg, msglength) < 0) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[player], msg, msglength);
                        Closing_Clients_connections();
                        exit(0);
                    }
                    Closing_Clients_connections();
                    exit(0);
                }
                game_request--;
                int Size = static_cast<int>(games_record[game_request].size());
                if (write(client[player], &Size, sizeof(int)) < 0) {
                    msg[0] = 'Q';
                    msglength = 1;
                    write(client[3 - player], msg, msglength);
                    Closing_Clients_connections();
                    exit(0);
                }
                for (int i = 0; i < Size; i++)
                    if (write(client[player], &(games_record[game_request][i]), sizeof(int)) < 0) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[3 - player], msg, msglength);
                        Closing_Clients_connections();
                        exit(0);
                    }
                move_no--;
                continue;
            }

            board[column_open_position[move]][move] = colors[player];
            printf("Current board at move %d is :\n", move_no);
            for (int i = 1; i <= nr_lines; i++) {
                for (int i1 = 1; i1 <= nr_columns; i1++)
                    printf("%d ", board[i][i1]);
                printf("\n");
            }


            column_open_position[move]++;

            game_moves.push_back(move);
            result = analize_board(board, nr_lines, nr_columns);            
            if (result != -1) {
                printf("Winner is player %d(1 is %d, 2 is %d\n", result, colors[1], colors[2]);
                fflush(stdout);
                if (result == colors[1]) {
                    score.first++;
                    msg[0] = 'W';
                    msg[1] = move;
                    msg[2] = 0;
                    msglength = 2;
                    start_player = 1;
                    if (write(client[1], msg, msglength) < 0) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[2], msg, msglength);
                        Closing_Clients_connections();
                        exit(0);
                    }
                    msg[0] = 'L';
                    msg[1] = move;
                    msg[2] = 0;
                    msglength = 2;
                    if (write(client[2], msg, msglength) < 0) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[1], msg, msglength);
                        Closing_Clients_connections();
                        exit(0);
                    }
                }
                else if (result == colors[2]) {
                    score.second++;
                    start_player = 2;
                    msg[0] = 'W';
                    msg[1] = move;
                    msg[2] = 0;
                    msglength = 2;
                    if (write(client[2], msg, msglength) < 0) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[1], msg, msglength);
                        Closing_Clients_connections();
                        exit(0);
                    }
                    msg[0] = 'L';
                    msg[1] = move;
                    msg[2] = 0;
                    msglength = 2;
                    if (write(client[1], msg, msglength) < 0) {
                        msg[0] = 'Q';
                        msglength = 1;
                        write(client[2], msg, msglength);
                        Closing_Clients_connections();
                        exit(0);
                    }
                }
                break;
            }
            player = 3 - player;
            printf("sending %d move to player %d\n", move, player);
            fflush(stdout);
            if (move_no > nr_lines * nr_columns) {
                msg[0] = move;
                msg[1] = 'd';
                msg[2] = 0;
                msglength = 2;
                write(client[player], msg, msglength);
                if (write(client[player], msg, msglength) < 0) {
                    msg[0] = 'Q';
                    msglength = 1;
                    write(client[3 - player], msg, msglength);
                    Closing_Clients_connections();
                    exit(0);
                }
                break;
            }
            else if (write(client[player], &move, 1) < 0) {
                msg[0] = 'Q';
                msglength = 1;
                write(client[3 - player], msg, msglength);
                Closing_Clients_connections();
                exit(0);
            }

        }
        if (move_no == 1 + nr_lines * nr_columns && result == -1) {
            score.first++, score.second++;
            msg[0] = 'd';
            msg[1] = 0;
            msglength = 1;
            if (write(client[3 - player], msg, msglength) < 0) {
                msg[0] = 'Q';
                msglength = 1;
                write(client[player], msg, msglength);
                Closing_Clients_connections();
                exit(0);
            }
        }
        games_record.push_back(game_moves);

    }
}

int main () {
    srand(time(nullptr));
    int client_no = 0;

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
       perror ("[server]Eroare la socket().\n");
       return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from1, sizeof (from1));
    bzero (&from2, sizeof (from2));
    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */


    int error_numbers = 0;
    PORT = 5000;
    server.sin_port = htons (PORT);
    while (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        perror ("[server]Eroare la bind(),  incercam alt port\n");
        error_numbers++;
        PORT = rand() % 5000 + 5000;
        server.sin_port = htons (PORT);
        if(error_numbers == 15) {
            printf("Prea multe erori, asta e");
            exit(1);
        }

    }

    if (listen (sd, 1) == -1) {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }

    while (1) {
        socklen_t length1 = sizeof (from1), length2 = sizeof (from2);

        printf ("[server]Asteptam la portul %d...\n", PORT);
        fflush (stdout);

        client[1] = accept (sd, (struct sockaddr *) &from1, &length1);

        if (client[1] < 0) {
            perror ("[server]Eroare la accept() Client 1.\n");
            continue;
        }

        printf ("[server]Acceptam clientul %d...\n", client_no++);
        fflush (stdout);

        client[2] = accept (sd, (struct sockaddr *) &from2, &length2);

        printf ("[server]Acceptam clientul %d...\n", client_no++);
        fflush (stdout);

        if (client[2] < 0) {
            perror ("[server]Eroare la accept() Client 2.\n");
            continue;
        }
        execute_game();
    }				/* while */
}				/* main */
