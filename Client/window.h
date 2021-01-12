#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QGraphicsRectItem>
#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <bits/stdc++.h>

using namespace std;

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:

    explicit Window(QWidget *parent = nullptr);
    void preshow();
    void receive_move();
    void restart_and_update_board();
    ~Window();

    vector<QColor> Colors_vector;
    QString QStringScore, label_text;
    size_t msglength = 0;
    char msg[105];
    QGraphicsScene *scene, *scene_for_color1, *scene_for_color2;
    int sd, player_no, start_player, color_player, color_opponent, nr_lines, nr_columns, square_size, game_no;
    int req_move_player, move_player, move_no, req_move_no;
    bool new_game_state, board_drawed, req_game, start_game;
    QGraphicsEllipseItem *P1_pieces[15][15], *P2_pieces[15][15], *white_pieces[15][15];
    pair <int, int> score;
    vector<int> current_game;
    vector<int> requested_game;
    int open_position[15], req_open_position[15];

private slots:

    void on_send_pushButton_clicked();

    void on_send_size_pushButton_clicked();

    void on_req_pushButton_clicked();

    void on_quit_pushButton_clicked();

    void on_resign_pushButton_clicked();

    void on_show_pushButton_clicked();

    void on_prev_pushButton_clicked();

    void on_next_move_pushButton_clicked();

    void on_last_move_pushButton_clicked();

    void on_Start_Game_p2_pushButton_clicked();

private:
    Ui::Window *ui;





};

#endif // WINDOW_H
