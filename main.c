#include <SDL2/SDL.h>
extern SDL_Window *window;
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "globals.h"
#include "main_menu.h"
#include "error_handlers.h"
#include "options.h"
#include "serial.h"
#include "version.h"


#define WINDOW_TITLE   "ScanTool.net " SCANTOOL_VERSION_EX_STR


void write_log(const char *log_string)
{
   FILE *logfile = NULL;
   
   logfile = fopen(log_file_name, "a");
   if (logfile == NULL)
      fatal_error("Could not open log file for writing!");
   fprintf(logfile, log_string);
   fclose(logfile);
}


#ifdef LOG_COMMS
void write_comm_log(const char *marker, const char *data)
{
   FILE *logfile = NULL;

   logfile = fopen(comm_log_file_name, "a");
   if (logfile == NULL)
      fatal_error("Could not open comm log file for writing!");
   fprintf(logfile, "[%s]%s[/%s]\n", marker, data, marker);
   fclose(logfile);
}
#endif


static void init()
{
   char temp_buf[256];

   is_not_genuine_scan_tool = false;
   
   /* initialize some variables with default values */
   strcpy(options_file_name, "scantool.cfg");
   strcpy(data_file_name, "scantool.dat");
   strcpy(code_defs_file_name, "codes.dat");
   comport.status = NOT_OPEN;
   display_mode = 0;

   // Inicialização SDL2
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
      fatal_error("Erro ao inicializar SDL2!");
   }
   SDL_SetWindowTitle(window, WINDOW_TITLE);
   write_log("OK");

   /* load options from file, the defaults will be automatically substituted if file does not exist */
   write_log("\nLoading Preferences... ");
   set_config_file(options_file_name);
   /* if config file doesn't exist or is of an incorrect version */
   if (strcmp(get_config_string(NULL, "version", ""), SCANTOOL_VERSION_STR) != 0)
   {
      /* update config file */
      remove(options_file_name);
      set_config_file(options_file_name);
      set_config_string(NULL, "version", SCANTOOL_VERSION_STR);
      load_program_options();  // Load defaults
      save_program_options();
   }
   else
      load_program_options();
   write_log("OK");

   // Removido: carregamento de datafile, paleta, fonte, cores e mouse do Allegro

   write_log("\nInitializing Serial Module... ");
   serial_module_init();
   write_log("OK");

   sprintf(temp_buf, "\nOpening COM%i... ", comport.number + 1);
   write_log(temp_buf);
   /* try opening comport (comport.status will be set) */
   open_comport();
   switch (comport.status)
   {
      case READY:
         write_log("OK");
         // --- Identificação Honda CB500X 2023 ---
         {
            char vin_response[64] = {0};
            char mfr_response[64] = {0};
            int status;
            // Solicita VIN (PID 09 02)
            send_command("0902");
            status = read_comport(vin_response);
            // Solicita fabricante (PID 09 0A)
            send_command("090A");
            status = read_comport(mfr_response);
            // Verifica se é uma Honda CB500X 2023
            if (strstr(vin_response, "MLHPC64") != NULL || strstr(mfr_response, "HONDA") != NULL) {
               write_log("\nMotocicleta Honda CB500X 2023 detectada!");
               // Suporte ao protocolo Honda JOBD/ISO 9141-2
               // comport.protocol = PROTOCOL_ISO_9141_2; // Removido se não existir no struct
               write_log("\nProtocolo ISO 9141-2 (Honda JOBD) selecionado automaticamente.");
               // Aqui você pode ajustar variáveis globais, interface, sensores, etc.
            } else {
               write_log("\nVeículo conectado não é uma Honda CB500X 2023.");
            }
         }
         break;

      case NOT_OPEN:
         write_log("Error!");
         break;
         
      default:
         write_log("Unknown Status");
         break;
   }
}


static void shut_down()
{
   //clean up
   flush_config_file();
   write_log("\nShutting Down Serial Module... ");
   serial_module_shutdown();
   write_log("OK");
   // Removido: descarregamento de datafile e Allegro
}


int main()
{
   char temp_buf[64];
   time_t current_time;
   
   time(&current_time);  // get current time, and store it in current_time
   strcpy(temp_buf, ctime(&current_time));
   temp_buf[strlen(temp_buf)-1] = 0;
   
   strcpy(log_file_name, "log.txt");
   remove(log_file_name);
   write_log(temp_buf);
#ifdef LOG_COMMS
   strcpy(comm_log_file_name, "comm_log.txt");
   remove(comm_log_file_name);
   write_comm_log("START_TIME", temp_buf);
#endif

   sprintf(temp_buf, "\nVersion: %s for %s", SCANTOOL_VERSION_STR, SCANTOOL_PLATFORM_STR);
   write_log(temp_buf);

   write_log("\n\nInitializing All Modules...\n---------------------------");
   init(); // initialize everything

   write_log("\n\nDisplaying Main Menu...\n-----------------------");
   display_main_menu(); // dislpay main menu
   write_log("\nMain Menu Closed");

   write_log("\n\nShutting Down All Modules...\n----------------------------");
   shut_down(); // shut down

   return EXIT_SUCCESS;
}
