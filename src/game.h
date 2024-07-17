#pragma once

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>

#include <stdio.h>

typedef struct {
    float x;
    float y;
} Vector2D;

typedef struct {
    Vector2D pos;

    float width;
    float height;
    float speed;
    int score;
} Paddle;

typedef struct {
    Vector2D pos;
    Vector2D dir;
    float speed;
    float radius;
} Ball;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    SDL_Texture *font_tex;
    float font_tex_w;
    float font_tex_h;

    Paddle p1;
    Paddle p2;

    Ball ball;

    bool isRunning;
    bool hasStarted;
    bool isPaused;
} GameState;

#define ERROR_EXIT(...) fprintf(stderr, __VA_ARGS__); return
#define ERROR_RETURN(R, ...) fprintf(stderr, __VA_ARGS__); return R

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 125
#define BALL_RADIUS 15
#define PADDLE_SPEED 300
#define BALL_SPEED 225

bool init();
void init_game_state(SDL_Window *window, SDL_Renderer *renderer);

void game_loop();

void reset_paddles(bool keep_score);
void reset_ball();
void set_scores_text();

void handle_events(float dt);
void update_game(float dt, float total_time);
void render();

void handle_player_tnput(float dt);

void animate_ball(float total_time);
void set_random_dir_ball(bool is_global_state, Ball *ball);

bool check_collision(Ball *ball, Paddle *p, bool is_left_side);
Vector2D get_paddle_normal( bool is_left_side);
Vector2D reflect_vec(Vector2D vec, Vector2D normal);
Vector2D find_vec_between_two_pos(Vector2D pos1, Vector2D po2);

void cleanup();
