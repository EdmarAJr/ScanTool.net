// Includes e variáveis globais SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <string.h>
#include "globals.h"
#include "serial.h"
#include "options.h"
#include "error_handlers.h"
#include "sensors.h"
#include "custom_gui.h"
#include "reset.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;

// Função de alerta usando SDL2
int sdl2_alert(const char *msg1, const char *msg2, const char *msg3, const char *btn1, const char *btn2, int c1, int c2)
{
   int running = 1;
   int result = 0;
   while (running) {
      // Fundo
      sdl2_draw_box_generic(120, 120, 400, 180, 255, 255, 255);
      // Mensagens
      sdl2_draw_text_generic(msg1, 140, 140, 0, 0, 0);
      if (msg2) sdl2_draw_text_generic(msg2, 140, 170, 0, 0, 0);
      if (msg3) sdl2_draw_text_generic(msg3, 140, 200, 0, 0, 0);
      // Botões
      sdl2_draw_box_generic(180, 260, 100, 40, 0, 200, 0);
      sdl2_draw_text_generic(btn1, 200, 270, 255, 255, 255);
      sdl2_draw_box_generic(320, 260, 100, 40, 200, 180, 0);
      sdl2_draw_text_generic(btn2, 340, 270, 255, 255, 255);
      SDL_RenderPresent(renderer);
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            quit_sdl2();
            exit(0);
         }
         if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x;
            int my = event.button.y;
            if (mx >= 180 && mx <= 280 && my >= 260 && my <= 300) {
               result = 1;
               running = 0;
            }
            if (mx >= 320 && mx <= 420 && my >= 260 && my <= 300) {
               result = 2;
               running = 0;
            }
         }
         if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
               running = 0;
            }
         }
      }
      SDL_Delay(30);
   }
   return result;
}


void intake_air_temp_formula(int data, char *buf)
{
   sprintf(buf, "%i%c C", data-40, 0xB0);
}


void air_flow_rate_formula(int data, char *buf)
{
   sprintf(buf, "%.2f g/s", data*0.01);
}


void throttle_position_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data*100/255);
}


// **** New formulae added 3/11/2003: ****

// Fuel Pressure (guage): PID 0A
void fuel_pressure_formula(int data, char *buf)
{
   sprintf(buf, "%i kPa", data*3);
}


// Fuel Trim statuses: PID 06-09
void short_term_fuel_trim_formula(int data, char *buf)
{
   if (data > 0xFF)  // we're only showing bank 1 and 2 FT
      data >>= 8;

   sprintf(buf, (data == 128) ? "0.0%%" : "%+.1f%%", ((float)data - 128)*100/128);
}


void long_term_fuel_trim_formula(int data, char *buf)
{
   if (data > 0xFF)  // we're only showing bank 1 and 2 FT
      data >>= 8;

   sprintf(buf, (data == 128) ? "0.0%%" : "%+.1f%%", ((float)data - 128)*100/128);
}


// Commanded secondary air status: PID 12
void secondary_air_status_formula(int data, char *buf)
{
   data = data & 0x0700; // mask bits 0, 1, and 2

   if (data == 0x0100)
      sprintf(buf, "upstream of 1st cat. conv.");
   else if (data == 0x0200)
      sprintf(buf, "downstream of 1st cat. conv.");
   else if (data == 0x0400)
      sprintf(buf, "atmosphere / off");
   else
      sprintf(buf, "Not supported");
}

// Oxygen sensor voltages & short term fuel trims: PID 14-1B
// Format is bankX_sensor

void o2_sensor_formula(int data, char *buf)
{
   if ((data & 0xFF) == 0xFF)  // if the sensor is not used in fuel trim calculation,
      sprintf(buf, "%.3f V", (data >> 8)*0.005);
   else
      sprintf(buf, ((data & 0xFF) == 128) ? "%.3f V @ 0.0%% s.t. fuel trim" : "%.3f V @ %+.1f%% s.t. fuel trim", (data >> 8)*0.005, ((float)(data & 0xFF) - 128)*100/128);
}


//Power Take-Off Status: PID 1E
void pto_status_formula(int data, char *buf)
{
   if ((data & 0x01) == 0x01)
      sprintf(buf, "active");
   else
      sprintf(buf, "not active");	
}

// OBD requirement to which vehicle is designed: PID 1C
void obd_requirements_formula(int data, char *buf)
{
   switch (data)
   {
      case 0x01:
         sprintf(buf, "OBD-II (California ARB)");
         break;
      case 0x02:
         sprintf(buf, "OBD (Federal EPA)");
         break;
      case 0x03:
         sprintf(buf, "OBD and OBD-II");
         break;
      default:
         sprintf(buf, "Unknown");
         break;
   }
}


void frp_relative_formula(int data, char *buf)
{
   float kpa, psi;
   
   kpa = data*0.079;
   psi = kpa*0.145037738;

   sprintf(buf, "%.3f kPa", kpa);
}


void frp_widerange_formula(int data, char *buf)
{
   int kpa;
   float psi;

   kpa = data*10;
   psi = kpa*0.145037738;

   sprintf(buf, "%i kPa", kpa);
}


void o2_sensor_wrv_formula(int data, char *buf)
{
   float eq_ratio, o2_voltage; // equivalence ratio and sensor voltage
   
   eq_ratio = (float)(data >> 16)*0.0000305;  // data bytes A,B
   o2_voltage = (float)(data & 0xFFFF)*0.000122; // data bytes C,D
   
   sprintf(buf, "%.3f V, Eq. ratio: %.3f", o2_voltage, eq_ratio);
}


//Commanded EGR status: PID 2C
void commanded_egr_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data*100/255);
}

//EGR error: PID 2D
void egr_error_formula(int data, char *buf)
{
   sprintf(buf, (data == 128) ? "0.0%%" : "%+.1f%%", (float)(data-128)/255*100);
}


void evap_pct_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data/255*100);
}


void fuel_level_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data/255*100);
}


void warm_ups_formula(int data, char *buf)
{
   sprintf(buf, "%i", data);
}


void clr_distance_formula(int data, char *buf)
{
   sprintf(buf, "%i km", data);
}


void evap_vp_formula(int data, char *buf)
{
   if (system_of_measurements == METRIC)
      sprintf(buf, "%.2f Pa", data*0.25);
}


void baro_pressure_formula(int data, char *buf)
{
   if (system_of_measurements == METRIC)
      sprintf(buf, "%i kPa", data);
}


void o2_sensor_wrc_formula(int data, char *buf)
{
   float eq_ratio, o2_ma; // equivalence ratio and sensor current

   eq_ratio = (float)(data >> 16)*0.0000305;  // data bytes A,B
   o2_ma = ((float)(data & 0xFFFF) - 0x8000)*0.00390625; // data bytes C,D

   sprintf(buf, "%.3f mA, Eq. ratio: %.3f", o2_ma, eq_ratio);
}


void cat_temp_formula(int data, char *buf)
{
   float c, f;
   
   c = data*0.1 - 40; // degrees Celcius
   f = c*9/5 + 32;  // degrees Fahrenheit
   
   sprintf(buf, "%.1f%c C", c, 0xB0);
}


void ecu_voltage_formula(int data, char *buf)
{
   sprintf(buf, "%.3f V", data*0.001);
}


void abs_load_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data*100/255);
}


void eq_ratio_formula(int data, char *buf)
{
   sprintf(buf, "%.3f", data*0.0000305);
}


void relative_tp_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data*100/255);
}


void amb_air_temp_formula(int data, char *buf)
{
   int c = 0;
   c = data-40; // degrees Celcius
   sprintf(buf, "%i%c C", c, 0xB0);
}


void abs_tp_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data*100/255);
}


void tac_pct_formula(int data, char *buf)
{
   sprintf(buf, "%.1f%%", (float)data*100/255);
}


void mil_time_formula(int data, char *buf)
{
   sprintf(buf, "%i hrs %i min", data/60, data%60);
}


void clr_time_formula(int data, char *buf)
{
   sprintf(buf, "%i hrs %i min", data/60, data%60);
}



// Função placeholder para genuíno (SDL2)
void sdl2_genuine_proc() {
}
