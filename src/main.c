#include "game.h"

int main(int argc, char **argv) {
    if (!init()) {
        ERROR_RETURN(-1, "Failed to init SDL2\n");
    }

    game_loop();

    cleanup();
    return 0;
}
