
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <string.h>

typedef struct TROUBLE_CODE {
    char code[7];
    char *description;
    char *solution;
    int pending;
    struct TROUBLE_CODE *next;
} TROUBLE_CODE;

static TROUBLE_CODE *trouble_codes = NULL;

void add_trouble_code(const TROUBLE_CODE *init_code) {
    TROUBLE_CODE *next = trouble_codes;
    trouble_codes = (TROUBLE_CODE *)malloc(sizeof(TROUBLE_CODE));
    if (!trouble_codes) return;
    if (init_code) {
        strcpy(trouble_codes->code, init_code->code);
        trouble_codes->description = init_code->description;
        trouble_codes->solution = init_code->solution;
        trouble_codes->pending = init_code->pending;
    } else {
        trouble_codes->code[0] = 0;
        trouble_codes->description = NULL;
        trouble_codes->solution = NULL;
        trouble_codes->pending = 0;
    }
    trouble_codes->next = next;
}

void clear_trouble_codes() {
    TROUBLE_CODE *next;
    while (trouble_codes) {
        next = trouble_codes->next;
        if (trouble_codes->description) free(trouble_codes->description);
        if (trouble_codes->solution) free(trouble_codes->solution);
        free(trouble_codes);
        trouble_codes = next;
    }
}

int sdl2_display_trouble_codes(SDL_Window *window, SDL_Renderer *renderer) {
    int running = 1;
    while (running) {
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderClear(renderer);
        sdl2_draw_box_generic(50, 40, 600, 400, 220, 220, 220);
        sdl2_draw_text_generic("Códigos de Erro Lidos", 70, 60, 0, 0, 0);
        int y = 100;
        for (TROUBLE_CODE *tc = trouble_codes; tc; tc = tc->next) {
            sdl2_draw_text_generic(tc->code, 80, y, 40, 40, 40);
            sdl2_draw_text_generic(tc->description ? tc->description : "", 160, y, 60, 60, 60);
            y += 30;
        }
        sdl2_draw_box_generic(80, 420, 120, 40, 0, 180, 0);
        sdl2_draw_text_generic("Limpar Códigos", 90, 430, 255, 255, 255);
        sdl2_draw_box_generic(240, 420, 120, 40, 0, 120, 220);
        sdl2_draw_text_generic("Voltar", 270, 430, 255, 255, 255);
        SDL_RenderPresent(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return -1;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x, my = event.button.y;
                if (mx >= 80 && mx <= 200 && my >= 420 && my <= 460) {
                    int res = sdl2_confirm_clear_dialog(window, renderer);
                    if (res == 1) {
                        clear_trouble_codes();
                        running = 0;
                    }
                }
                if (mx >= 240 && mx <= 360 && my >= 420 && my <= 460) {
                    running = 0;
                }
            }
        }
        SDL_Delay(30);
    }
    return 0;
}

int sdl2_confirm_clear_dialog(SDL_Window *window, SDL_Renderer *renderer) {
    return sdl2_alert(
        "Atenção!", "Deseja realmente limpar todos os códigos de erro?", NULL,
        "Sim", "Não", 'y', 'n'
    );
}
