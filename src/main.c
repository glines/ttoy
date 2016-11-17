#include <SDL.h>
#include <assert.h>
#include <libtsm.h>
#include <stdlib.h>

#include "fonts.h"
#include "pty.h"
#include "terminal.h"

#include "glyphAtlas.h"

struct st_Shelltoy {
  st_Terminal terminal;
} shelltoy;

void st_initSDL() {
  SDL_Init(SDL_INIT_VIDEO);
}

void st_quitSDL() {
  SDL_Quit();
}

void st_dispatchEvents() {
  /* Dispatch all events in the SDL event queue */
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            /* TODO: Support event dispatch to multiple terminals here */
            /* Inform the terminal of the new window size */
            st_Terminal_windowSizeChanged(&shelltoy.terminal,
                event.window.data1,  /* width */
                event.window.data2  /* height */
                );
            break;
        }
        break;
      default:
        if (event.type == st_PTY_eventType()) {
          fprintf(stderr, "Recieved PTY event\n");
          /* Instruct the pty to read from the pseudo terminal */
          st_PTY *pty = (st_PTY *)event.user.data1;
          st_PTY_read(pty);
        }
    }
  }
}

/* TODO: Load 'toy' information from JSON files with '.toy' extensions */
/* TODO: Load toys from shared libraries */

/* TODO: Actually get this font path as user input */
const char *FONT_FACE_PATH = "/nix/store/fvwp39z54ka2s7h3gawhfmayrqjnd05a-dejavu-fonts-2.37/share/fonts/truetype/DejaVuSansMono.ttf";
/* TODO: We might even want to read a small font into memory */

int main(int argc, char** argv) {
  st_initSDL();
  st_Fonts_init();

  st_Terminal_init(&shelltoy.terminal);

  fprintf(stderr, "Made it past terminal init\n");  /* XXX */

  while (1) {
    st_dispatchEvents();
    st_Terminal_draw(&shelltoy.terminal);
    SDL_Delay(50);
    /* TODO: Write a more intelligent loop that waits for vsync, etc. */
  }
  st_Terminal_destroy(&shelltoy.terminal);

  st_quitSDL();

  return EXIT_SUCCESS;
}
