#include "game.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

GameState *gState = NULL;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        ERROR_RETURN(false, "Failed to init SDL Video\n");
    }

    if (TTF_Init() == -1) {
        ERROR_RETURN(false, "Failed to init SDL_ttf\n");
    }

    SDL_Window *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        ERROR_RETURN(false, "Failed to create a window!\n");
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        ERROR_RETURN(false, "Failed to create the renderer!\n");
    }

    TTF_Font* font = TTF_OpenFont("../res/font.ttf", 24); // 24 is the font size
    if (font == NULL) {
        ERROR_RETURN(false, "Failed to load the font!\n");
    }

    srand(time(NULL));
    init_game_state(window, renderer);

    gState->font = font;
    set_scores_text();

    return true;
}

void init_game_state(SDL_Window *window, SDL_Renderer *renderer) {
    GameState *state = malloc(sizeof(GameState));

    state->window = window;
    state->renderer = renderer;
    state->isRunning = true;
    state->hasStarted = false;

    gState = state;

    reset_paddles(false);
    reset_ball();
}

void game_loop() {
    uint32_t current_time, last_time = 0;
    float delta_time;
    float total_time = 0;
    while (gState->isRunning) {
        current_time = SDL_GetTicks();
        delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        total_time += delta_time;

        handle_events(delta_time);
        handle_player_input(delta_time);
        update_game(delta_time, total_time);
        render();
    }
}

void reset_paddles(bool keep_score) {
    Paddle p1 = {
        .pos = {
            .x = PADDLE_WIDTH + 16,
            .y = (SCREEN_HEIGHT / 2.0) - (PADDLE_HEIGHT / 2.0)
        },
        .width = PADDLE_WIDTH,
        .height = PADDLE_HEIGHT,
        .speed = PADDLE_SPEED,
    };

    Paddle p2 = {
        .pos = {
            .x = SCREEN_WIDTH - PADDLE_WIDTH - 16,
            .y = (SCREEN_HEIGHT / 2.0) - (PADDLE_HEIGHT / 2.0)
        },
        .width = PADDLE_WIDTH,
        .height = PADDLE_HEIGHT,
        .speed = PADDLE_SPEED
    };

    if (keep_score) {
        p1.score = gState->p1.score;
        p2.score = gState->p2.score;
    }

    gState->p1 = p1;
    gState->p2 = p2;
}

void reset_ball() {
    Ball ball = {
        .pos = {
            .x = SCREEN_WIDTH / 2.0,
            .y = (SCREEN_HEIGHT / 2.0 + BALL_RADIUS / 2.0)
        },
        .speed = BALL_SPEED,
        .radius = BALL_RADIUS
    };
    set_random_dir_ball(false, &ball);

    gState->ball = ball;
}

void handle_events(float dt) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            gState->isRunning = false;
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_SPACE && !gState->hasStarted) {
                gState->hasStarted = true;
                set_random_dir_ball(true, NULL);
            }
        }
    }
}

void update_game(float dt, float total_time) {
    // Animate Ball
    if (!gState->hasStarted) {
        animate_ball(total_time);
        return;
    }

    // Move ball
    Ball ball = gState->ball;
    Paddle p1 = gState->p1;
    Paddle p2 = gState->p2;

    ball.pos.x += ball.dir.x * ball.speed * dt;
    ball.pos.y += ball.dir.y * ball.speed * dt;

    if (ball.pos.x <= 0) {
        gState->p2.score++;
        set_scores_text();
        reset_paddles(true);
        reset_ball();
        gState->hasStarted = false;
        return;
    }
    if (ball.pos.x >= SCREEN_WIDTH - BALL_RADIUS) {
        gState->p1.score++;
        set_scores_text();
        reset_paddles(true);
        reset_ball();
        gState->hasStarted = false;
        return;
    }

    if (ball.pos.y <= 0 || ball.pos.y >= SCREEN_HEIGHT - BALL_RADIUS) {
        ball.dir.y *= -1;
    }

    // Check paddle collision
    if (check_collision(&ball, &p1, true)) {
        Vector2D normal = get_paddle_normal(true);
        ball.dir = reflect_vec(ball.dir, normal);
    }
    if (check_collision(&ball, &p2, false)) {
        Vector2D normal = get_paddle_normal(false);
        ball.dir = reflect_vec(ball.dir, normal);
    }

    gState->ball = ball;
}

void render() {
    SDL_Renderer *renderer = gState->renderer;
    Paddle p1 = gState->p1;
    Paddle p2 = gState->p2;
    Ball ball = gState->ball;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    SDL_Rect p1_rect = {p1.pos.x, p1.pos.y, p1.width, p1.height};
    SDL_RenderFillRect(renderer, &p1_rect);

    SDL_Rect p2_rect = {p2.pos.x, p2.pos.y, p2.width, p2.height};
    SDL_RenderFillRect(renderer, &p2_rect);

    SDL_Rect ball_rect = {ball.pos.x, ball.pos.y, ball.radius, ball.radius};
    SDL_RenderFillRect(renderer, &ball_rect);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_Rect messageRect = {(SCREEN_WIDTH) / 2.0 - 24, 16, gState->font_tex_w, gState->font_tex_h};
    SDL_RenderCopy(renderer, gState->font_tex, NULL, &messageRect);

    SDL_RenderPresent(renderer);
}

void handle_player_input(float dt) {
    Paddle p1 = gState->p1;
    Paddle p2 = gState->p2;

    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

    if (currentKeyStates[SDL_SCANCODE_W]) {
            p1.pos.y -= p1.speed * dt;
    }
    if (currentKeyStates[SDL_SCANCODE_S]) {
            p1.pos.y += p1.speed * dt;
    }
    if (currentKeyStates[SDL_SCANCODE_UP]) {
            p2.pos.y -= p2.speed * dt;
    }
    if (currentKeyStates[SDL_SCANCODE_DOWN]) {
            p2.pos.y += p2.speed * dt;
    }

    if (p1.pos.y <= 0) {
        p1.pos.y += p1.speed * dt;
    }
    if (p1.pos.y >= SCREEN_HEIGHT - p1.height) {
        p1.pos.y -= p1.speed * dt;
    }
    if (p2.pos.y <= 0) {
        p2.pos.y += p2.speed * dt;
    }
    if (p2.pos.y >= SCREEN_HEIGHT - p2.height) {
        p2.pos.y -= p2.speed * dt;
    }

    gState->p1 = p1;
    gState->p2 = p2;
}

void cleanup() {
    SDL_DestroyWindow(gState->window);
    gState->window = NULL;

    SDL_DestroyRenderer(gState->renderer);
    gState->renderer = NULL;

    SDL_DestroyTexture(gState->font_tex);
    gState->font_tex = NULL;

    free(gState);
    gState = NULL;

    TTF_Quit();
    SDL_Quit();
}

void animate_ball(float total_time) {
    gState->ball.pos.y = (sin(total_time * 2) + 1) / 2 * (SCREEN_HEIGHT - 5 * BALL_RADIUS) + BALL_RADIUS;
}

void set_random_dir_ball(bool is_global_state, Ball *ball) {
    float x, y, length;

    x = ((float)rand() / RAND_MAX) - 0.5;
    y = ((float)rand() / RAND_MAX) - 0.5;

    //Normalize the vector
    length = sqrt(x*x + y*y);
    x /= length;
    y /= length;

   if (!is_global_state) {
       ball->dir.x = x;
       ball->dir.y = y;
   } else {
       gState->ball.dir.x = x;
       gState->ball.dir.y = y;
   }
}

bool check_collision(Ball *ball, Paddle *p, bool is_left_side) {
    if (is_left_side) {
        if (ball->pos.x >= p->pos.x && ball->pos.x <= p->pos.x + p->width &&
            ball->pos.y >= p->pos.y && ball->pos.y <= p->pos.y + p->height) {
            return true;
        }
    } else {
        if (ball->pos.x + ball->radius <= p->pos.x + p->width && ball->pos.x + ball->radius >= p->pos.x &&
            ball->pos.y >= p->pos.y && ball->pos.y <= p->pos.y + p->height) {
            return true;
        }
    }

    return false;
}

Vector2D get_paddle_normal(bool is_left_side) {
    if (is_left_side) {
        return (Vector2D){.x = 1.0f, .y = 0};
    }
    return (Vector2D){.x = -1.0f, .y = 0};
}

Vector2D reflect_vec(Vector2D vec, Vector2D normal) {
    float dot = vec.x * normal.x + vec.y * normal.y;
    Vector2D r = {
        vec.x - 2 * dot * normal.x,
        vec.y - 2 * dot * normal.y
    };
    return r;
}

void set_scores_text() {
    SDL_Color textColor = {0xFF, 0xFF, 0xFF, 0xFF};
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%i  :  %i", gState->p1.score, gState->p2.score);
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(gState->font, buffer, textColor);
    if (surfaceMessage == NULL) {
        ERROR_EXIT("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    }

    // Convert surface to texture
    SDL_Texture* message = SDL_CreateTextureFromSurface(gState->renderer, surfaceMessage);
    if (message == NULL) {
        ERROR_EXIT("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
    }

    gState->font_tex_w = surfaceMessage->w;
    gState->font_tex_h = surfaceMessage->h;

    SDL_FreeSurface(surfaceMessage);

    gState->font_tex = message;
}
