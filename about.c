extern SDL_Renderer *renderer;
#ifndef SCANTOOL_VERSION_EX_STR
#define SCANTOOL_VERSION_EX_STR "1.0"
#endif
#ifndef SCANTOOL_PLATFORM_STR
#define SCANTOOL_PLATFORM_STR "Unknown Platform"
#endif
#ifndef SCANTOOL_COPYRIGHT_STR
#define SCANTOOL_COPYRIGHT_STR "Copyright (c) 2024"
#endif
#define VER_STR   "Version " SCANTOOL_VERSION_EX_STR " for " SCANTOOL_PLATFORM_STR ", " SCANTOOL_COPYRIGHT_STR
// Exibe a janela "Sobre" usando SDL2
int display_about()
{
   int running = 1, result = 0;
   SDL_Event event;
   // Caixa principal
   int win_x = 100, win_y = 100, win_w = 500, win_h = 300;
   while (running) {
      // Fundo da caixa
      SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
      SDL_Rect rect = {win_x, win_y, win_w, win_h};
      SDL_RenderFillRect(renderer, &rect);

      // Título
      sdl2_draw_text("ScanTool.net", win_x + 20, win_y + 20, 0, 0, 120);
      sdl2_draw_text(VER_STR, win_x + 20, win_y + 60, 0, 0, 0);

      // Informações OBD
      sdl2_draw_text("Interface:", win_x + 20, win_y + 110, 0, 0, 0);
      sdl2_draw_text(obd_interface, win_x + 120, win_y + 110, 0, 80, 0);
      sdl2_draw_text("Fabricante:", win_x + 20, win_y + 140, 0, 0, 0);
      sdl2_draw_text(obd_mfr, win_x + 120, win_y + 140, 0, 80, 0);
      sdl2_draw_text("Protocolo:", win_x + 20, win_y + 170, 0, 0, 0);
      sdl2_draw_text(obd_protocol, win_x + 120, win_y + 170, 0, 80, 0);
      sdl2_draw_text("Sistema:", win_x + 20, win_y + 200, 0, 0, 0);
      sdl2_draw_text(obd_system, win_x + 120, win_y + 200, 0, 80, 0);

      // Botão OK
      SDL_SetRenderDrawColor(renderer, 0, 120, 0, 255);
      SDL_Rect btn = {win_x + win_w - 120, win_y + win_h - 60, 100, 40};
      SDL_RenderFillRect(renderer, &btn);
      sdl2_draw_text("OK", win_x + win_w - 90, win_y + win_h - 50, 255, 255, 255);

      SDL_RenderPresent(renderer);

      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            running = 0;
            result = -1;
         }
         if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;
            if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
               running = 0;
               result = 1;
            }
         }
         if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
               running = 0;
               result = 0;
            }
         }
      }
      SDL_Delay(30);
   }
   return result;
}
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "custom_gui.h"
#include "serial.h"
#include "sensors.h"
#include "options.h"
#include "version.h"
#include "about.h"

#define MSG_REFRESH  MSG_USER



static char whatisit[256];
static char whatcanitdo[256];
static char wheretoget[256];

static char obd_interface[64];
static char obd_mfr[64];
static char obd_protocol[64];
static char obd_system[64];

#define VER_STR   "Version " SCANTOOL_VERSION_EX_STR " for " SCANTOOL_PLATFORM_STR ", " SCANTOOL_COPYRIGHT_STR

void clear_obd_info()
{
   strcpy(obd_interface, "N/A");
   strcpy(obd_mfr, "N/A");
   strcpy(obd_protocol, "N/A");
   strcpy(obd_system, "N/A");
}


void format_id_string(char *str)
{
   if (strncmp(str, "ELM32", 4) == 0)
   {
      memmove(str + 7, str + 6, strlen(str) - 5);
      str[6] = ' ';
   }
   else if (strncmp(str, "OBDLinkCI", 9) == 0)
   {
      memmove(str + 11, str + 9, strlen(str) - 8);
      memmove(str + 8, str + 7, 2);
      str[7] = ' ';
      str[10] = ' ';
   }
   else if (strncmp(str, "OBDLink", 7) == 0)
   {
      memmove(str + 8, str + 7, strlen(str) - 6);
      str[7] = ' ';
   }
   else if (strncmp(str, "STN", 3) == 0)
   {
      memmove(str + 8, str + 7, strlen(str) - 6);
      str[7] = ' ';
   }
}


// OBD info getter states
#define OBD_INFO_IDLE         0
#define OBD_INFO_START        1
#define OBD_INFO_TX_0100      2
#define OBD_INFO_WAIT_ATZ     3
#define OBD_INFO_ECU_TIMEOUT  4
#define OBD_INFO_WAIT_0100    5
#define OBD_INFO_WAIT_011C    6

#define OBD_INFO_MAX_RETRIES  3




