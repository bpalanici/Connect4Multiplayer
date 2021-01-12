#include "window.h"
#include "ui_window.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <QChar>
#include <QMessageBox>

using namespace Qt;
using namespace std;

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window) {
    ui->setupUi(this);
    scene = new QGraphicsScene(this);    
}

void  Window::preshow() {    
    if(this->player_no == 1) {
        ui->Start_Game_p2_pushButton->hide();
        ui->move_comboBox->hide();
        ui->req_comboBox->hide();
        ui->send_pushButton->hide();
        //ui->status_label->hide();
        ui->quit_pushButton->hide();
        ui->resign_pushButton->hide();
        ui->req_pushButton->hide();
        ui->GraphWindow->hide();
        ui->prev_pushButton->hide();
        ui->next_move_pushButton->hide();
        ui->show_pushButton->hide();
        ui->last_move_pushButton->hide();
        ui->color1_graphicsView->hide();
        ui->color2_graphicsView->hide();
        ui->label_2->hide();
        ui->label->hide();
        ui->score_label->hide();

        QStringList string_list;
        QString string_item;
        string_item.insert(0, "4x5");
        string_list.insert(0, string_item);
        string_item.clear();
        string_item.insert(0, "5x6");
        string_list.insert(1, string_item);
        string_item.clear();
        string_item.insert(0, "6x7");
        string_list.insert(2, string_item);
        string_item.clear();
        string_item.insert(0, "7x8");
        string_list.insert(3, string_item);
        string_item.clear();
        string_item.insert(0, "7x9");
        string_list.insert(4, string_item);
        string_item.clear();
        string_item.insert(0, "7x10");
        string_list.insert(5, string_item);
        string_item.clear();
        string_item.insert(0, "8x8");
        string_list.insert(6, string_item);


        //4x5, 5x6, 7x8, 7x9, 7x10, 8x8, 6x7
        ui->size_comboBox->addItems(string_list);
    }
    else {
        ui->send_size_pushButton->hide();
        ui->size_comboBox->hide();
        if (read (sd, msg, 100) <= 0) {
            QMessageBox::information(
                this,
                tr("Quit"),
                tr("Something happened and server / opponent has crashed, so game over, bye!") );
            exit(0);
        }
        {
            char *p = strtok(msg, " \n x");
            nr_lines = atoi(p);
            p = strtok(nullptr, " \n x");
            nr_columns = atoi(p);
        }
        //ui->GraphWindow->setFixedSize(nr_columns * 55, nr_lines * 55);

        square_size = (ui->GraphWindow->width() - 10) / max(nr_lines, nr_columns);


        //printf("nr_columns: %d, nr_lines, %d; square size : %d", nr_columns, nr_lines, square_size);
      //  label_text.clear();
      //  label_text.insert(0, "Waiting for Opp Move");
        //ui->status_label->setText(label_text);

    }
    score = make_pair(0, 0);

    Colors_vector.push_back(Qt::blue);
    Colors_vector.push_back(Qt::red);
    Colors_vector.push_back(Qt::green);
    Colors_vector.push_back(Qt::cyan);
    Colors_vector.push_back(Qt::magenta);

    QBrush brush;
    brush.setColor(Colors_vector[color_player]);
    brush.setStyle(Qt::SolidPattern);
    scene_for_color1 = new QGraphicsScene(this);
    scene_for_color1->setBackgroundBrush(brush);

    ui->color1_graphicsView->setScene(scene_for_color1);

    brush.setColor(Colors_vector[color_opponent]);
    scene_for_color2 = new QGraphicsScene(this);
    scene_for_color2->setBackgroundBrush(brush);
    ui->color2_graphicsView->setScene(scene_for_color2);    

    brush.setColor(Qt::yellow);
    brush.setStyle(Qt::Dense3Pattern);
    scene->setBackgroundBrush(brush);

    start_player = 1;
    if(player_no == 2) restart_and_update_board();
}

void Window::receive_move() {

    memset(msg, 0, sizeof (msg));
    msglength = 0;
    //printf("line 139, waiting to read");
    fflush(stdout);
    start_game = false;
    if (read(sd, msg, 1000) <= 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("Something happened and server / opponent has crashed, so game over, bye!") );
        exit(0);
    }
    start_game = true;
   // label_text.clear();
 //   label_text.insert(0, "Your move.");
    //ui->status_label->setText(label_text);
    //printf("\nline 147 just read : %d si mutarea, eventual %d\n", msg[0], msg[1]);
    fflush(stdout);
    msglength = strlen(msg);
    if (msg[0] == 'q') {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("The game has ended, bye!") );
        exit(0);
    }
    if (msg[0] == 'Q') {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("Something happened and server / opponent has crashed, so game over, bye!"));
        exit(0);
    }
    if (msg[0] == 'W') {
        QMessageBox::information(
            this,
            tr("Win"),
            tr("You Win!! Click new game button or quit."));
        score.first++;
        start_player = player_no;
        on_last_move_pushButton_clicked();
        ui->Start_Game_p2_pushButton->show();
        start_game = false;
        return;
    }
    if (msg[0] == 'L') {
        QMessageBox::information(
            this,
            tr("Defeat"),
            tr("You Lost... Click new game button or quit."));
        score.second++;
        current_game.push_back(msg[1]);
        start_player = 3 - player_no;
        on_last_move_pushButton_clicked();
        ui->Start_Game_p2_pushButton->show();
        start_game = false;
        return;
    }
    if (msg[0] == 'd') {
        //printf("line 185 reached");
        QMessageBox::information(
            this,
            tr("Draw"),
            tr("Draw, no moves possible. Click new game button or quit."));
        score.first++;
        score.second++;
        start_player = 1;
        on_last_move_pushButton_clicked();
        ui->Start_Game_p2_pushButton->show();
        start_game = false;
        return;
    }
    int move = msg[0];
    current_game.push_back(move);
    if (msg[1] == 'd') {
        //printf("line 199 reached");
        fflush(stdout);
        QMessageBox::information(
            this,
            tr("Draw"),
            tr("Draw, no moves possible. Click new game button or quit."));
        score.first++;
        score.second++;
        start_player = 1;
        on_last_move_pushButton_clicked();
        ui->Start_Game_p2_pushButton->show();
        start_game = false;
        return;
    }
    //printf("\n\nCurrent game at line 180 : ");
    for (auto it : current_game)
        //printf("%d ", it);
    //printf("\n");
    on_last_move_pushButton_clicked();
}

void Window::restart_and_update_board() {

    //printf("line 210, game restart started\n");
    fflush(stdout);

    QString score1, score2;
    QStringScore.clear();
    game_no++;
    for (int i = score.first; i > 0; i /= 10)
        score1.push_back((i % 10) + '0');
    for (int i = score.second; i > 0; i /= 10)
        score2.push_back((i % 10) + '0');
    reverse(score1.begin(), score1.end());
    reverse(score2.begin(), score2.end());
    if (!score.first) score1.push_back('0');
    if (!score.second) score2.push_back('0');
    for (auto it : score1)
        QStringScore.push_back(it);
    QStringScore.push_back(" - ");
    for (auto it : score2)
        QStringScore.push_back(it);
    ui->score_label->setText(QStringScore);
    for (int i = 1; i <= nr_lines; i++)
        for (int i1 = 1; i1 <= nr_columns; i1++)
            open_position[i1] = 1;
    current_game.clear();

    //printf("\n\n square size %d \n\n", square_size);
    fflush(stdout);
    if (!board_drawed) {
        QBrush yellowBrush(yellow);
        QBrush whiteBrush(white);
        QBrush p1Brush(Colors_vector[color_player]);
        QBrush p2Brush(Colors_vector[color_opponent]);
        QBrush winBrush(black);
        QPen bluePen(blue);
        QPen whitePen(white);
        QPen winPen(black);
        QPen p1Pen(Colors_vector[color_player]);
        QPen p2Pen(Colors_vector[color_opponent]);
        whitePen.setWidth(1);
        bluePen.setWidth(3);
        for (int i = 1; i <= nr_lines; i++)
            for (int i1 = 1; i1 <= nr_columns; i1++) {
                scene->addRect(1 + i1 * square_size, 1 + i * square_size, square_size, square_size, bluePen, yellowBrush);
                P1_pieces[nr_lines + 1 - i][i1] = scene->addEllipse(3 + i1 * square_size, 3 + i * square_size, square_size - 5, square_size - 5, p1Pen, p1Brush);
                P2_pieces[nr_lines + 1 - i][i1] = scene->addEllipse(3 + i1 * square_size, 3 + i * square_size, square_size - 5, square_size - 5, p2Pen, p2Brush);
                white_pieces[nr_lines + 1 - i][i1] = scene->addEllipse(3 + i1 * square_size, 3 + i * square_size, square_size - 5, square_size - 5, whitePen, whiteBrush);
                //win_pieces[nr_lines + 1 - i][i1] = scene->addEllipse(3 + i1 * square_size, 3 + i * square_size, square_size / 2, square_size / 2, winPen, winBrush);
                P1_pieces[nr_lines + 1 - i][i1]->setZValue(-1);
                P2_pieces[nr_lines + 1 - i][i1]->setZValue(-1);
                //win_pieces[nr_lines + 1 - i][i1]->setZValue(-1);
                white_pieces[nr_lines + 1 - i][i1]->setZValue(1);
            }
        board_drawed = true;
        ui->GraphWindow->setScene(scene);
    }
    else {
        for (int i = 1; i <= nr_lines; i++)
            for (int i1 = 1; i1 <= nr_columns; i1++) {
                white_pieces[i][i1]->setZValue(1);
                P1_pieces[i][i1]->setZValue(-1);
                P2_pieces[i][i1]->setZValue(-1);
            }
    }

    QStringList moves;
    QString possible_move;
    moves.clear();
    move_no = 0;
    for (int i = 1; i <= nr_columns; i++) {
        possible_move.clear();
        if (i != 10)
            possible_move.push_back(i + '0');
        else
            possible_move.push_back('1'),
            possible_move.push_back('0');

        moves.push_back(possible_move);
    }
    ui->move_comboBox->clear();
    ui->move_comboBox->addItems(moves);

    QString addstring;
    for (int i = game_no - 2; i > 0; i /= 10)
        addstring.push_back((i % 10) + '0');
    reverse(addstring.begin(), addstring.end());
    //ui->req_comboBox->clear();
    if (game_no >= 2)
        ui->req_comboBox->addItem(addstring);
    if (ui->req_comboBox->itemText(0).length() == 0)
        ui->req_comboBox->removeItem(0);
    current_game.push_back(start_player);
    //printf("line 180, game restarted\n");
    fflush(stdout);
}

Window::~Window()
{
    delete ui;
}

void Window::on_send_size_pushButton_clicked() { //Only player 1 will execute this
    QString text;
    text = ui->size_comboBox->currentText();
    memset (msg, 0, sizeof(msg));
    for (auto it : text)
        msg[msglength++] = it.toLatin1();
    if (write (sd, msg, msglength) < 0) {
      perror ("Write error sending board size.\n");
      exit( errno);
    }
    {
        char *p = strtok(msg, " \n x");
        nr_lines = atoi(p);
        p = strtok(nullptr, " \n x");
        nr_columns = atoi(p);
    }
    ui->Start_Game_p2_pushButton->show();
    ui->move_comboBox->show();
    ui->req_comboBox->show();
    ui->send_pushButton->show();
    //ui->status_label->show();
    ui->quit_pushButton->show();
    ui->resign_pushButton->show();
    ui->req_pushButton->show();
    ui->GraphWindow->show();
    ui->prev_pushButton->show();
    ui->next_move_pushButton->show();
    ui->show_pushButton->show();
    ui->last_move_pushButton->show();
    ui->color1_graphicsView->show();
    ui->color2_graphicsView->show();
    ui->label_2->show();
    ui->label->show();
    ui->score_label->show();
    ui->send_size_pushButton->hide();
    ui->size_comboBox->hide();
    //ui->GraphWindow->setFixedSize(nr_columns * 55, nr_lines * 55);

    square_size = (ui->GraphWindow->width() - 10) / max(nr_lines, nr_columns);
    //printf("nr_columns: %d, nr_lines, %d; square size : %d", nr_columns, nr_lines, square_size);
    fflush(stdout);
    restart_and_update_board();
   // label_text.clear();
   // label_text.insert(0, "Your Move, player1");
   // ui->status_label->setText(label_text);
/*
    if(read(sd, msg, 105) < 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("The game has ended, some error reading from server happened") );
        exit(0);
    }*/
}

void Window::on_send_pushButton_clicked() {/* ---------------------------------
    */
    if (!start_game) return;
   // label_text.clear();
   // label_text.insert(0, "Waiting for Opp Move");
   // ui->status_label->setText(label_text);
    //ui->status_label->update();
    QString text = ui->move_comboBox->currentText();
    int move = 0;
    for (auto it : text)
        move = move * 10 + (it.toLatin1() - '0');

    //printf("message sent %d move\n", move);
    fflush(stdout);
    current_game.push_back(move);
    msg[0] = move;
    msg[1] = 0;
    msglength = 1;
    //printf("message sent %d move and %d msg\n", move, msg[0]);
    fflush(stdout);
    if(write(sd, msg, msglength) < 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("The game has ended, some error writing to server happened") );
        exit(0);
    }
    //printf("message sent %d", msg[0]);
    fflush(stdout);
    on_last_move_pushButton_clicked();

    receive_move();
}

void Window::on_req_pushButton_clicked() {
    if (!start_game) return;
    if (ui->req_comboBox->count() == 0) return;
    if (ui->req_comboBox->currentText().length() == 0) return;
    int game = 0;
    requested_game.clear();
    QString st = ui->req_comboBox->currentText();
    for (auto it : st)
        game = game * 10 + (it.toLatin1() - '0');
    msg[0] = 'R';
    msg[1] = 0;
    msglength = 1;
    if(write(sd, msg, msglength) < 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("The game has ended, some error writing to server happened") );
        exit(0);
    }
    if(write(sd, &game, sizeof(int)) < 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("The game has ended, some error writing to server happened") );
        exit(0);
    }
    int size;
    if(read(sd, &size, sizeof(int)) <= 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("Something happened and server / opponent has crashed, so game over, bye!") );
        exit(0);
    }
    for (int i = 1, x; i <= size; i++)
        if(read(sd, &x, sizeof(int)) <= 0) {
            QMessageBox::information(
                this,
                tr("Quit"),
                tr("Something happened and server / opponent has crashed, so game over, bye!") );
            exit(0);
        }
        else requested_game.push_back(x);
    on_show_pushButton_clicked();
}

void Window::on_quit_pushButton_clicked() {
    msg[0] = 'q';
    msg[1] = 0;
    write(sd, msg, 1);

    QMessageBox::information(
        this,
        tr("Quit"),
        tr("The game has ended") );

    exit(0);
}

void Window::on_resign_pushButton_clicked() {
    if (!start_game) return;
    score.second++;
    start_player = 3 - player_no;
    start_game = false;
    ui->Start_Game_p2_pushButton->show();

    msg[0] = 'r';
    msg[1] = 0;
    msglength = 1;
    if(write(sd, msg, msglength) < 0) {
        QMessageBox::information(
            this,
            tr("Quit"),
            tr("The game has ended, some error writing to server happened") );
        exit(0);
    }
    //receive_move();----------------------------------------------------------------
}

void Window::on_show_pushButton_clicked() {
    if (!start_game) return;
    if (!requested_game.size()) {
        QMessageBox::information(
            this,
            tr("Error"),
            tr("No former game requested, or the game requested has no moves"));
        return;
    }
    for (int i = 1; i <= nr_lines; i++)
        for (int i1 = 1; i1 <= nr_columns; i1++) {
            P1_pieces[i][i1]->setZValue(-1);
            P2_pieces[i][i1]->setZValue(-1);
            //win_pieces[i][i1]->setZValue(-1);
            white_pieces[i][i1]->setZValue(1);
        }

    int player_to_move = requested_game.front();

    for (int i = 1; i <= nr_columns; i++)
        req_open_position[i] = 1;

    for (size_t i = 1; i < requested_game.size(); i++) {
        int move = requested_game[i];

        white_pieces[req_open_position[move]][move]->setZValue(-1);
        if (player_to_move == player_no)
            P1_pieces[req_open_position[move]][move]->setZValue(1),
            P2_pieces[req_open_position[move]][move]->setZValue(-1);

        else
            P2_pieces[req_open_position[move]][move]->setZValue(1),
            P1_pieces[req_open_position[move]][move]->setZValue(-1);
        req_open_position[move]++;
        player_to_move = 3 - player_to_move;
    }
    req_move_no = requested_game.size();
    req_move_player = player_to_move;
    req_game = true;
}

void Window::on_last_move_pushButton_clicked() {
    if (!start_game) return;
    for (int i = 1; i <= nr_lines; i++)
        for (int i1 = 1; i1 <= nr_columns; i1++) {
            P1_pieces[i][i1]->setZValue(-1);
            P2_pieces[i][i1]->setZValue(-1);
            //win_pieces[i][i1]->setZValue(-1);
            white_pieces[i][i1]->setZValue(1);
        }
    if (!current_game.size())
        return;
    int player_to_move = current_game.front();

    for (int i = 1; i <= nr_columns; i++)
        open_position[i] = 1;

    for (size_t i = 1; i < current_game.size(); i++) {
        int move = current_game[i];

        white_pieces[open_position[move]][move]->setZValue(-1);
        if (player_to_move == player_no)
            P1_pieces[open_position[move]][move]->setZValue(1),
            P2_pieces[open_position[move]][move]->setZValue(-1);

        else
            P2_pieces[open_position[move]][move]->setZValue(1),
            P1_pieces[open_position[move]][move]->setZValue(-1);
        open_position[move]++;
        player_to_move = 3 - player_to_move;
    }
    move_no = current_game.size();
    move_player = player_to_move;
    req_game = false;

    QStringList moves;
    QString possible_move;
    moves.clear();

    for (int i = 1; i <= nr_columns; i++)
        if (open_position[i] <= nr_lines) {
            possible_move.clear();
            if (i != 10)
                possible_move.push_back(i + '0');
            else
                possible_move.push_back('1'),
                possible_move.push_back('0');
            moves.insert(i - 1, possible_move);
        }
    ui->move_comboBox->clear();
    ui->move_comboBox->addItems(moves);
    scene->update();
    //printf("player to move is now, %d, for next / prev  ", move_player);
    fflush(stdout);
}

void Window::on_prev_pushButton_clicked() {
    if (!start_game) return;
    if (req_game) {
        if (req_move_no <= 1)
            return;
        req_move_no--;
        req_move_player = 3 - req_move_player;
        int move = requested_game[req_move_no];
        req_open_position[move]--;
        P1_pieces[req_open_position[move]][move]->setZValue(-1);
        P2_pieces[req_open_position[move]][move]->setZValue(-1);
        white_pieces[req_open_position[move]][move]->setZValue(1);
        return;
    }
    else {
        if (move_no <= 1)
            return;
        move_no--;
        move_player = 3 - move_player;
        int move = current_game[move_no];
        open_position[move]--;
        P1_pieces[open_position[move]][move]->setZValue(-1);
        P2_pieces[open_position[move]][move]->setZValue(-1);
        white_pieces[open_position[move]][move]->setZValue(1);
        return;
    }
}

void Window::on_next_move_pushButton_clicked() {
    if (!start_game) return;
    if (req_game) {
        if (req_move_no == requested_game.size())
            return;

        int move = requested_game[req_move_no];
        if (req_move_player == player_no)
            P1_pieces[req_open_position[move]][move]->setZValue(1),
            P2_pieces[req_open_position[move]][move]->setZValue(-1);
        else
            P1_pieces[req_open_position[move]][move]->setZValue(-1),
            P2_pieces[req_open_position[move]][move]->setZValue(1);
        white_pieces[req_open_position[move]][move]->setZValue(-1);
        req_open_position[move]++;
        req_move_player = 3 - req_move_player;
        req_move_no++;
        return;
    }
    else {
        if (move_no == current_game.size())
            return;

        int move = current_game[move_no];
        if (move_player == player_no)
            P1_pieces[open_position[move]][move]->setZValue(1),
            P2_pieces[open_position[move]][move]->setZValue(-1);
        else
            P1_pieces[open_position[move]][move]->setZValue(-1),
            P2_pieces[open_position[move]][move]->setZValue(1);
        white_pieces[open_position[move]][move]->setZValue(-1);
        open_position[move]++;
        move_no++;
        move_player = 3 - move_player;
        return;
    }
}

void Window::on_Start_Game_p2_pushButton_clicked() {
    ui->Start_Game_p2_pushButton->hide();
    start_game = true;
    restart_and_update_board();
    if (player_no != start_player) {
        //label_text.clear();
    //    label_text.insert(0, "Waiting for Opp Move");
        //ui->status_label->setText(label_text);
        //printf("Receiving move, player start 2\n");
        fflush(stdout);
        receive_move();
    }
    else {
      //  label_text.clear();
      //  label_text.insert(0, "Your Move");
    //    ui->status_label->setText(label_text);
    }
}
