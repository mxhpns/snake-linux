#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define SNAKE_INCREASE 3
#define COLOR_BLACK	0
#define COLOR_RED	1
#define COLOR_GREEN	2
#define COLOR_YELLOW	3
#define COLOR_BLUE	4
#define COLOR_MAGENTA	5
#define COLOR_CYAN	6
#define COLOR_WHITE	7

volatile sig_atomic_t signal_status = 0; /* Indicates a caught signal */

void sighandler(int s) {
    signal_status = s;
}

void init() {
    initscr();
    savetty();
    nonl();
    cbreak();
    noecho();
    timeout(0);
    leaveok(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) {
        start_color();
        if (use_default_colors() != ERR) {
            init_pair(COLOR_BLACK, -1, -1);
            init_pair(COLOR_GREEN, COLOR_GREEN, -1);
            init_pair(COLOR_WHITE, COLOR_WHITE, -1);
            init_pair(COLOR_RED, COLOR_RED, -1);
            init_pair(COLOR_CYAN, COLOR_CYAN, -1);
            init_pair(COLOR_MAGENTA, COLOR_MAGENTA, -1);
            init_pair(COLOR_BLUE, COLOR_BLUE, -1);
            init_pair(COLOR_YELLOW, COLOR_YELLOW, -1);
        } else {
            init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
            init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
            init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
            init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
        }
    }
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGWINCH, sighandler);
    signal(SIGTSTP, sighandler);
}

void finish() {
    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();
    exit(0);
}

char **board;

int lines, cols;

void clear_board() {
    int i, j;
    for (i = 0; i < LINES; i++) {
        for (j = 0; j < COLS; j++) {
            board[i][j] = ' ';
        }
    }
}

void init_board() {
    int i;
    lines = LINES;
    cols = COLS;
    board = (char**) malloc(LINES * sizeof(char*));
    if (!board) {
        finish();
    }
    for (i = 0; i < LINES; i++) {
        board[i] = (char*) malloc(COLS * sizeof(char));
        if (!board[i]) {
            finish();
        }
    }
    clear_board();
}

#define MAX_SNAKE_LENGTH 500

typedef struct Coord {
    int row;
    int col;
} Coord;

Coord snake[MAX_SNAKE_LENGTH];

#define POINT '*'
#define INIT_SNAKE_LENGTH 5

int snakeSize = 1;
int snakeLength = INIT_SNAKE_LENGTH;

void put(int row, int col, char ch) {
    board[row][col] = ch;
    move(row, col);
    addch(ch);
}

Coord get_random_pos() {
    Coord res;
    while (1) {
        res.row = rand() % lines;
        res.col = rand() % cols;
        if (board[res.row][res.col] == ' ') {
            break;
        }
    }
    return res;
}

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define STOP 4

#define APPLE '@'

int direction = STOP;
int game_over = 0;

void put_apple();

void change_direction() {
    int keypress;
    keypress = wgetch(stdscr);
    if (keypress == ERR) {
        return;
    }
    if (keypress == 'q') {
        finish();
    }
    if (game_over) {
        if (keypress == 'r') {
            clear_board();
            snakeLength = INIT_SNAKE_LENGTH;
            snakeSize = 1;
            snake[0] = get_random_pos();
            direction = STOP;
            game_over = 0;
            clear();
            refresh();
            attron(COLOR_PAIR(rand() % 8));
            put(snake[0].row, snake[0].col, POINT);
            put_apple();
        }
        return;
    }
    if (keypress == 'a') {
        direction = LEFT;
    } else if (keypress == 'd') {
        direction = RIGHT;
    } else if (keypress == 's') {
        direction = DOWN;
    } else if (keypress == 'w') {
        direction = UP;
    }
}

void change_head() {
    if (direction == RIGHT) {
        snake[0].col++;
    } else if (direction == LEFT) {
        snake[0].col--;
    } else if (direction == UP) {
        snake[0].row--;
    } else if (direction == DOWN) {
        snake[0].row++;
    }
}

int verify_head() {
    if (snake[0].row < 0 || snake[0].col < 0
        || snake[0].row >= lines || snake[0].col >= cols) {
        return -1;
    }
    if (board[snake[0].row][snake[0].col] != ' '
        && board[snake[0].row][snake[0].col] != APPLE) {
        return -1;
    }
    return 0;
}

void logic() {
    memmove(&snake[1], &snake[0], sizeof(Coord) * snakeSize);
    change_head();
    if (verify_head()) {
        direction = STOP;
        game_over = 1;
        return;
    }
    if (board[snake[0].row][snake[0].col] == APPLE) {
        snakeLength += SNAKE_INCREASE;
        put_apple();
    }
    attron(COLOR_PAIR(rand() % 8));
    put(snake[0].row, snake[0].col, POINT);
    if (snakeSize < snakeLength) {
        snakeSize++;
    } else {
        put(snake[snakeSize].row, snake[snakeSize].col, ' ');
    }
}

void put_apple() {
    Coord pos = get_random_pos();
    attron(COLOR_PAIR(COLOR_YELLOW));
    put(pos.row, pos.col, APPLE);
}

int main() {
    srand((unsigned) time(NULL));
    init();
    init_board();
    snake[0] = get_random_pos();
    attron(COLOR_PAIR(COLOR_GREEN));
    put(snake[0].row, snake[0].col, POINT);
    put_apple();
    while (1) {
        if (signal_status) {
            finish();
        }
        if (direction != STOP && !game_over) {
            logic();
        }
        change_direction();
        napms(100);
        change_direction();
    }
    finish();
    endwin();
    return 0;
}

