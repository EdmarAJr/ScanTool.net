#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <string.h>
#include "globals.h"
#include "serial.h"
#include "get_port_names.h"
#include "error_handlers.h"

// Variáveis globais para lista de portas
char *comport_list_strings = NULL;
int *comport_list_numbers = NULL;
int comport_list_size = 0;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

// Includes padrão e dependências
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Removed duplicate includes

void clear_comport_list() {
   if (comport_list_strings != NULL) {
      free(comport_list_strings);
      comport_list_strings = NULL;
   }
   if (comport_list_numbers != NULL) {
      free(comport_list_numbers);
      comport_list_numbers = NULL;
   }
   comport_list_size = 0;
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <string.h>
#include "globals.h"
#include "serial.h"
#include "get_port_names.h"
#include "error_handlers.h"
// Variáveis globais para lista de portas
char *comport_list_strings = NULL;
int *comport_list_numbers = NULL;
int comport_list_size = 0;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

// Função para desenhar texto
void sdl2_draw_text(const char *text, int x, int y, int r, int g, int b) {
   SDL_Color color = {r, g, b};
   SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
   if (!surface) return;
   SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
   SDL_Rect dst = {x, y, surface->w, surface->h};
   SDL_RenderCopy(renderer, texture, NULL, &dst);
   SDL_DestroyTexture(texture);
   SDL_FreeSurface(surface);
}

// Função para desenhar botão
void sdl2_draw_button(const char *label, int x, int y, int w, int h, int r, int g, int b, bool selected) {
   SDL_Rect rect = {x, y, w, h};
   SDL_SetRenderDrawColor(renderer, r, g, b, 255);
   SDL_RenderFillRect(renderer, &rect);
   if (selected) {
      SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
      SDL_RenderDrawRect(renderer, &rect);
   }
   sdl2_draw_text(label, x + 10, y + 10, 255, 255, 255);
}

// Função principal de opções
int display_options() {
    int running = 1;
    int baud_rates[3] = {9600, 38400, 115200};
    int selected_baud = comport.baud_rate;
    int selected_idx = 0;
    int i, mx, my, bx, by;
    for (i = 0; i < 3; i++) {
        if (baud_rates[i] == selected_baud) selected_idx = i;
    }
    SDL_Event event;
    while (running) {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_RenderClear(renderer);
        SDL_Rect box = {100, 80, 400, 300};
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &box);

        sdl2_draw_text("Opções do Programa", 130, 100, 0, 0, 120);
        sdl2_draw_text("Sistema de Medidas: Métrico", 130, 140, 0, 0, 0);
        sdl2_draw_text("Baud Rate:", 130, 180, 0, 0, 0);

        for (i = 0; i < 3; i++) {
            sdl2_draw_button((i == 0 ? "9600" : (i == 1 ? "38400" : "115200")), 130 + i*90, 210, 80, 40, 0, 120, 200, selected_idx == i);
        }
        sdl2_draw_button("Salvar", 130, 280, 100, 40, 0, 180, 0, false);
        sdl2_draw_button("Cancelar", 250, 280, 100, 40, 180, 0, 0, false);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                return 0;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                mx = event.button.x;
                my = event.button.y;
                for (i = 0; i < 3; i++) {
                    bx = 130 + i*90;
                    by = 210;
                    if (mx >= bx && mx <= bx+80 && my >= by && my <= by+40) {
                        selected_idx = i;
                    }
                }
                if (mx >= 130 && mx <= 230 && my >= 280 && my <= 320) {
                    comport.baud_rate = baud_rates[selected_idx];
                    running = 0;
                    return 1;
                }
                if (mx >= 250 && mx <= 350 && my >= 280 && my <= 320) {
                    running = 0;
                    return 0;
                }
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                    return 0;
                }
            }
        }
        SDL_Delay(30);
    }
    return 0;
}
void init_comport_list() {
    comport_list_size = 2;
    comport_list_strings = malloc(2 * (sizeof(char) * PORT_NAME_BUF_SIZE));
    if (comport_list_strings == NULL)
        fatal_error("Could not allocate memory for comport_list_strings.");
    comport_list_numbers = malloc(2 * sizeof(int));
    if (comport_list_numbers == NULL)
        fatal_error("Could not allocate memory for comport_list_numbers.");
    // Nomes típicos
    sprintf(comport_list_strings + 0 * PORT_NAME_BUF_SIZE, "/dev/ttyUSB0");
    sprintf(comport_list_strings + 1 * PORT_NAME_BUF_SIZE, "/dev/ttyS0");
    comport_list_numbers[0] = 0;
    comport_list_numbers[1] = 1;
}


void clear_comport_list()
{
   if (comport_list_strings != NULL)
   {
      free(comport_list_strings);
      comport_list_strings = NULL;
   }
   if (comport_list_numbers != NULL)
   {
      free(comport_list_numbers);
      comport_list_numbers = NULL;
   }
   comport_list_size = 0;
}
