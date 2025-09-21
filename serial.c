#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "globals.h"
#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>


int fd = -1;

#define TX_TIMEOUT_MULTIPLIER    0
#define TX_TIMEOUT_CONSTANT      1000

#define SERIAL_PORT "/dev/ttyUSB0"




// Funções de timer removidas. Use usleep() diretamente onde necessário.



void serial_module_init() {
   // Inicialização simples, sem Allegro
}



void serial_module_shutdown() {
   close_comport();
}



int open_comport() {
    struct termios options;
    if (comport.status == READY)
        close_comport();
    fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        comport.status = NOT_OPEN;
        return -1;
    }
    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);
   cfsetispeed(&options, B38400); // Exemplo: 38400 baud
   cfsetospeed(&options, B38400);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);
    comport.status = READY;
    return 0;
}


void close_comport()
{
   if (comport.status == READY && fd != -1) {
      close(fd);
      fd = -1;
   }
   comport.status = NOT_OPEN;
}


void send_command(const char *command) {
   char tx_buf[32];
   ssize_t bytes_written;
   sprintf(tx_buf, "%s\r", command);  // Adiciona CR
   bytes_written = write(fd, tx_buf, strlen(tx_buf));
   if (bytes_written != (ssize_t)strlen(tx_buf)) {
#ifdef LOG_COMMS
      log_comm("TX ERROR", tx_buf);
#endif
      return;
   }
#ifdef LOG_COMMS
   write_comm_log("TX", tx_buf);
#endif
}


int read_comport(char *response) {
   char *prompt_pos = NULL;
   ssize_t bytes_read = 0;
   int i, j;
   response[0] = '\0';
   bytes_read = read(fd, response, 255);
   if (bytes_read <= 0)
      return EMPTY;
   // Remove extraneous 0s
   for (i = 0, j = 0; i < bytes_read; i++)
      if (response[i] > 0)
         response[j++] = response[i];
   response[j] = 0;
   prompt_pos = strchr(response, '>');
   if (prompt_pos != NULL) {
#ifdef LOG_COMMS
      write_comm_log("RX", response);
#endif
      *prompt_pos = '\0';
      return PROMPT;
   } else if (strlen(response) == 0) {
      return EMPTY;
   } else {
#ifdef LOG_COMMS
      write_comm_log("RX", response);
#endif
      return DATA;
   }
}


int find_valid_response(char *buf, char *response, const char *filter, char **stop)
{
   char *in_ptr = response;
   char *out_ptr = buf;

   buf[0] = 0;

   while (*in_ptr)
   {
      if (strncmp(in_ptr, filter, strlen(filter)) == 0)
      {
         while (*in_ptr && *in_ptr != SPECIAL_DELIMITER) // copy valid response into buf
         {
            *out_ptr = *in_ptr;
            in_ptr++;
            out_ptr++;
         }
         *out_ptr = 0;  // terminate string
         if (*in_ptr == SPECIAL_DELIMITER)
            in_ptr++;
         break;
      }
      else
      {
         // skip to the next delimiter
         while (*in_ptr && *in_ptr != SPECIAL_DELIMITER)
            in_ptr++;
         if (*in_ptr == SPECIAL_DELIMITER)  // skip the delimiter
            in_ptr++;
      }
   }

   if (stop)
      *stop = in_ptr;

   if (strlen(buf) > 0)
      return 1;
   else
      return 0;
}

// DO NOT TRANSLATE ANY STRINGS IN THIS FUNCTION!
int process_response(const char *cmd_sent, char *msg_received)
{
   int i = 0;
   char *msg = msg_received;
   int echo_on = 1; //echo status
   int is_hex_num = 1;
   char temp_buf[80];

   if (cmd_sent)
   {
      for(i = 0; cmd_sent[i]; i++)
      {
         if (cmd_sent[i] != *msg)    // if the characters are not the same,
         {
            echo_on = 0;  // echo desligado
            break;            // break out of the loop
         }
         msg++;
      }

   if (echo_on == 1)  //if echo is on
      {
         send_command("ate0"); // turn off the echo
         start_serial_timer(AT_TIMEOUT);
         // wait for chip response or timeout
         while ((read_comport(temp_buf) != PROMPT) && !serial_time_out)
            ;
         stop_serial_timer();
         if (!serial_time_out)
         {
            send_command("atl0"); // turn off linefeeds
            start_serial_timer(AT_TIMEOUT);
            // wait for chip response or timeout
            while ((read_comport(temp_buf) != PROMPT) && !serial_time_out)
               ;
            stop_serial_timer();
         }
      }
      else //if echo is off
         msg = msg_received;
   }

   while(*msg && (*msg <= ' '))
      msg++;

   if (strncmp(msg, "SEARCHING...", 12) == 0)
      msg += 13;
   else if (strncmp(msg, "BUS INIT: OK", 12) == 0)
      msg += 13;
   else if (strncmp(msg, "BUS INIT: ...OK", 15) == 0)
      msg += 16;

   for(i = 0; *msg; msg++) //loop to copy data
   {
      if (*msg > ' ')  // if the character is not a special character or space
      {
         if (*msg == '<') // Detect <DATA_ERROR
         {
            if (strncmp(msg, "<DATA ERROR", 10) == 0)
               return DATA_ERROR2;
            else
               return RUBBISH;
         }
         msg_received[i] = *msg; // rewrite response
         if (!isxdigit(*msg) && *msg != ':')
            is_hex_num = 0;
         i++;
      }
      else if (((*msg == '\n') || (*msg == '\r')) && (msg_received[i-1] != SPECIAL_DELIMITER)) // if the character is a CR or LF
         msg_received[i++] = SPECIAL_DELIMITER; // replace CR with SPECIAL_DELIMITER
   }
   
   if (i > 0)
      if (msg_received[i-1] == SPECIAL_DELIMITER)
         i--;
   msg_received[i] = '\0'; // terminate the string

   if (is_hex_num)
      return HEX_DATA;

   if (strcmp(msg_received, "NODATA") == 0)
      return ERR_NO_DATA;
   if (strcmp(msg_received + strlen(msg_received) - 15, "UNABLETOCONNECT") == 0)
      return UNABLE_TO_CONNECT;
   if (strcmp(msg_received + strlen(msg_received) - 7, "BUSBUSY") == 0)
      return BUS_BUSY;
   if (strcmp(msg_received + strlen(msg_received) - 9, "DATAERROR") == 0)
      return DATA_ERROR;
   if (strcmp(msg_received + strlen(msg_received) - 8, "BUSERROR") == 0 ||
       strcmp(msg_received + strlen(msg_received) - 7, "FBERROR") == 0)
      return BUS_ERROR;
   if (strcmp(msg_received + strlen(msg_received) - 8, "CANERROR") == 0)
      return CAN_ERROR;
   if (strcmp(msg_received + strlen(msg_received) - 10, "BUFFERFULL") == 0)
      return BUFFER_FULL;
   if (strncmp(msg_received, "BUSINIT:", 8) == 0)
   {
      if (strcmp(msg_received + strlen(msg_received) - 5, "ERROR") == 0)
         return BUS_INIT_ERROR;
      else
         return SERIAL_ERROR;
   }
   if (strcmp(msg_received, "?") == 0)
      return UNKNOWN_CMD;
   if (strncmp(msg_received, "ELM320", 6) == 0)
      return INTERFACE_ELM320;
   if (strncmp(msg_received, "ELM322", 6) == 0)
      return INTERFACE_ELM322;
   if (strncmp(msg_received, "ELM323", 6) == 0)
      return INTERFACE_ELM323;
   if (strncmp(msg_received, "ELM327", 6) == 0)
      return INTERFACE_ELM327;
   if (strncmp(msg_received, "OBDLink", 7) == 0 ||
       strncmp(msg_received, "STN1000", 7) == 0 ||
       strncmp(msg_received, "STN11", 5) == 0)
      return INTERFACE_OBDLINK;
   if (strncmp(msg_received, "SCANTOOL.NET", 12) == 0)
      return STN_MFR_STRING;
   if (strcmp(msg_received, "OBDIItoRS232Interpreter") == 0)
      return ELM_MFR_STRING;
   
   return RUBBISH;
}


int display_error_message(int error, int retry)
{
   char buf[32];
   
   switch (error)
   {
      case BUS_ERROR:
         return alert("Bus Error: OBDII bus is shorted to Vbatt or Ground.", NULL, NULL, (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case BUS_BUSY:
         return alert("OBD Bus Busy. Try again.", NULL, NULL, (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case BUS_INIT_ERROR:
         return alert("OBD Bus Init Error. Check connection to the vehicle,", "make sure the vehicle is OBD-II compliant,", "and ignition is ON.", (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case UNABLE_TO_CONNECT:
         return alert("Unable to connect to OBD bus.", "Check connection to the vehicle. Make sure", "the vehicle is OBD-II compliant, and ignition is ON.", (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case CAN_ERROR:
         return alert("CAN Error.", "Check connection to the vehicle. Make sure", "the vehicle is OBD-II compliant, and ignition is ON.", (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case DATA_ERROR:
      case DATA_ERROR2:
         return alert("Data Error: there has been a loss of data.", "You may have a bad connection to the vehicle,", "check the cable and try again.", (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case BUFFER_FULL:
         return alert("Hardware data buffer overflow.", NULL, NULL, (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      case SERIAL_ERROR:
      case UNKNOWN_CMD:
      case RUBBISH:
         return alert("Serial Link Error: please check connection", "between computer and scan tool.", NULL, (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);

      default:
         sprintf(buf, "Unknown error occured: %i", error);
         return alert(buf, NULL, NULL, (retry) ? "Retry" : "OK", (retry) ? "Cancel" : NULL, 0, 0);
   }
}


const char *get_protocol_string(int interface_type, int protocol_id)
{
   switch (interface_type)
   {
      case INTERFACE_ELM320:
         return "SAE J1850 PWM (41.6 kBit/s)";
      case INTERFACE_ELM322:
         return "SAE J1850 VPW (10.4 kBit/s)";
      case INTERFACE_ELM323:
         return "ISO 9141-2 / ISO 14230-4 (KWP2000)";
      case INTERFACE_ELM327:
      case INTERFACE_OBDLINK:
         switch (protocol_id)
         {
            case 0:
               return "N/A";
            case 1:
               return "SAE J1850 PWM (41.6 kBit/s)";
            case 2:
               return "SAE J1850 VPW (10.4 kBit/s)";
            case 3:
               return "ISO 9141-2";
            case 4:
               return "ISO 14230-4 KWP2000 (5-baud init)";
            case 5:
               return "ISO 14230-4 KWP2000 (fast init)";
            case 6:
               return "ISO 15765-4 CAN (11-bit ID, 500 kBit/s)";
            case 7:
               return "ISO 15765-4 CAN (29-bit ID, 500 kBit/s)";
            case 8:
               return "ISO 15765-4 CAN (11-bit ID, 250 kBit/s)";
            case 9:
               return "ISO 15765-4 CAN (29-bit ID, 250 kBit/s)";
         }
   }
   
   return "unknown";
}
