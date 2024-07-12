#include <SDL2/SDL.h>

#include <stdio.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool isRunning;
} State;

bool init();
void cleanup();
State *create_state(SDL_Window *window, SDL_Renderer *render);

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

State *gState = NULL;

int main(int argc, char **argv) {
    if (!init()) {
        printf("Failed to initialize the game!\n");
        return -1;
    }

    SDL_Event e;
    SDL_Rect rect = {0, 0, 250, 250};
    while (gState->isRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gState->isRunning = false;
            }
        }

        SDL_RenderClear(gState->renderer);

        SDL_SetRenderDrawColor(gState->renderer, 0xFF, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(gState->renderer, &rect);
        SDL_SetRenderDrawColor(gState->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        SDL_RenderPresent(gState->renderer);
    }

    cleanup();

    return 0;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_Window *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Failed to initialize the Window! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_Renderer *renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Failed to initialize the Renderer! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    gState = create_state(window, renderer);

    return true;
}

void cleanup() {
    SDL_DestroyRenderer(gState->renderer);
    gState->renderer;

    SDL_DestroyWindow(gState->window);
    gState->window = NULL;

    SDL_Quit();
}

State *create_state(SDL_Window *window, SDL_Renderer *render) {
    State *state = malloc(sizeof(state));
    state->window = window;
    state->renderer = render;
    state->isRunning = true;

    return state;
}
