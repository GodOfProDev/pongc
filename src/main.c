#include <SDL2/SDL.h>

#include <stdio.h>

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
} State;

bool init();

void cleanup();

State *create_state(SDL_Window *window, SDL_Renderer *render);

void renderPaddles();

void renderBall();

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

State *gState = NULL;

int main(int argc, char **argv) {
    if (!init()) {
        printf("Failed to initialize the game!\n");
        return -1;
    }

    puts("1");

    SDL_Event e;
    while (gState->isRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gState->isRunning = false;
            } /*else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        break;
                    case SDLK_s:
                        break;
                    case SDLK_UP:
                        break;
                    case SDLK_DOWN:
                        break;
                    default:
                        break;
                }
            }*/
        }

        SDL_SetRenderDrawColor(gState->renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(gState->renderer);

        renderBall();
        renderPaddles();

        SDL_RenderPresent(gState->renderer);
    }

    puts("2");

    cleanup();

    puts("3");

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

State *create_state(SDL_Window *window, SDL_Renderer *render) {
    State *state = malloc(sizeof(state));
    state->window = window;
    state->renderer = render;
    state->isRunning = true;

    Paddle p1;
    p1.width = 10;
    p1.height = 165;
    p1.pos.y = (SCREEN_HEIGHT / 2.0) - (p1.height / 2.0);
    p1.pos.x = p1.width + 16;
    p1.speed = 10;

    Paddle p2;
    p2.width = 10;
    p2.height = 165;
    p2.pos.y = (SCREEN_HEIGHT / 2.0) - (p2.height / 2.0);
    p2.pos.x = SCREEN_WIDTH - (p2.width + 16);
    p2.speed = 10;

    state->p1 = p1;
    state->p2 = p2;

    Ball ball;
    ball.pos.x = 0;
    ball.pos.y = 0;
    ball.radius = 5;
    ball.speed = 15;
    ball.dir.x = -1;
    ball.dir.y = 0;

    state->ball = ball;

    return state;
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
}
