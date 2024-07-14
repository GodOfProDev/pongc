#include <math.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
} Point2D;

typedef struct {
    Point2D pos;
    int width;
    int height;
    float speed;
} Paddle;

typedef struct {
    Point2D pos;
    Point2D dir;
    float radius;
    float speed;
} Ball;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;

    Paddle p1;
    Paddle p2;
    Ball ball;

    bool isRunning;
} GameState;

bool init();

void initPaddles(GameState *state);

void initBall(GameState *state);

void cleanup();

GameState *create_state(SDL_Window *window, SDL_Renderer *render);

void renderLoop();

void renderPaddles();

void renderBall();

void handleEvents();

void handlePlayerInput(SDL_KeyCode key);

void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, float radius);

bool checkCollision(Paddle *p, Ball *b);

void updateBall();

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

GameState *gState = NULL;

int main(int argc, char **argv) {
    if (!init()) {
        printf("Failed to initialize the game!\n");
        return -1;
    }

    while (gState->isRunning) {
        int startTicks = SDL_GetTicks();

        handleEvents();
        updateBall();
        renderLoop();

        int delta = SDL_GetTicks() - startTicks;
        if (delta < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - delta);
        }
    }

    cleanup();

    return 0;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_Window *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                          SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Failed to initialize the Window! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Failed to initialize the Renderer! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    gState = create_state(window, renderer);

    return true;
}

void cleanup() {
    SDL_DestroyRenderer(gState->renderer);
    SDL_DestroyWindow(gState->window);

    gState->renderer = NULL;
    gState->window = NULL;

    SDL_Quit();
}

GameState *create_state(SDL_Window *window, SDL_Renderer *render) {
    GameState *state = malloc(sizeof(GameState));

    state->window = window;
    state->renderer = render;
    state->isRunning = true;

    initPaddles(state);
    initBall(state);

    return state;
}

void renderLoop() {
    SDL_SetRenderDrawColor(gState->renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(gState->renderer);

    renderBall();
    renderPaddles();

    SDL_RenderPresent(gState->renderer);
}

void renderPaddles() {
    SDL_SetRenderDrawColor(gState->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    Paddle p1 = gState->p1;
    Paddle p2 = gState->p2;

    SDL_Rect p1_rect = {p1.pos.x, p1.pos.y, p1.width, p1.height};
    SDL_RenderFillRect(gState->renderer, &p1_rect);

    SDL_Rect p2_rect = {p2.pos.x, p2.pos.y, p2.width, p2.height};
    SDL_RenderFillRect(gState->renderer, &p2_rect);
}

void renderBall() {
    SDL_SetRenderDrawColor(gState->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    Ball ball = gState->ball;
    drawCircle(gState->renderer, ball.pos.x, ball.pos.y, ball.radius);
}

void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            gState->isRunning = false;
        } else if (e.type == SDL_KEYDOWN) {
            handlePlayerInput(e.key.keysym.sym);
        }
    }
}

void handlePlayerInput(SDL_KeyCode key) {
    Paddle p1 = gState->p1;
    Paddle p2 = gState->p2;
    switch (key) {
        case SDLK_w:
            p1.pos.y -= p1.speed;
            break;
        case SDLK_s:
            p1.pos.y += p1.speed;
            break;

        case SDLK_UP:
            p2.pos.y -= p2.speed;
            break;
        case SDLK_DOWN:
            p2.pos.y += p2.speed;
            break;
        default:
            return;
    }

    gState->p1 = p1;
    gState->p2 = p2;
}

void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, float radius) {
    for (int x = centerX - radius; x <= centerX + radius; x++) {
        for (int y = centerY - radius; y <= centerY + radius; y++) {
            int dx = x - centerX;
            int dy = y - centerY;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void updateBall() {
    Ball ball = gState->ball;
    Paddle p1 = gState->p1;
    Paddle p2 = gState->p2;

    ball.pos.x += ball.speed * ball.dir.x;
    ball.pos.y += ball.speed * ball.dir.y;

    if (checkCollision(&p1, &ball)) {
        ball.dir.x = 1;
        ball.dir.y = 0.25;
    } else if (checkCollision(&p2, &ball)) {
        ball.dir.x = -1;
        ball.dir.y = -0.25;
    }

    gState->ball = ball;
}

bool checkCollision(Paddle *p, Ball *b) {
    // Find the closest point on the paddle to the center of the ball
    float closestX = fmaxf(p->pos.x, fminf(b->pos.x, p->pos.x + p->width));
    float closestY = fmaxf(p->pos.y, fminf(b->pos.y, p->pos.y + p->height));

    // Calculate the distance between the closest point and the ball's center
    float distanceX = b->pos.x - closestX;
    float distanceY = b->pos.y - closestY;

    // Check if the distance is less than the ball's radius
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
    return distanceSquared < (b->radius * b->radius);
}

void initPaddles(GameState *state) {
    Paddle p1 = {
        .width = 10,
        .height = 165,
        .speed = 10
    };

    p1.pos.y = (SCREEN_HEIGHT / 2.0) - (p1.height / 2.0);
    p1.pos.x = p1.width + 16;

    Paddle p2 = {
        .width = 10,
        .height = 165,
        .speed = 10
    };

    p2.pos.y = (SCREEN_HEIGHT / 2.0) - (p2.height / 2.0);
    p2.pos.x = SCREEN_WIDTH - (p2.width + 32);

    state->p1 = p1;
    state->p2 = p2;
}

void initBall(GameState *state) {
    Ball ball = {
        .radius = 13,
        .speed = 2,
    };

    ball.pos.x = (SCREEN_WIDTH / 2.0);
    ball.pos.y = (SCREEN_HEIGHT / 2.0);
    ball.dir.x = -1;
    ball.dir.y = 0;

    state->ball = ball;
}
