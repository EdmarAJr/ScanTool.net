*********************************************************************
   ScanTool.net OBD-II Software v1.20 for ElmScan and OBDLink devices
   Copyright (C) 2010 ScanTool.net LLC, All Rights Reserved
*********************************************************************

======================================
============= Disclaimer =============
======================================

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

======================================
============ Introduction ============
======================================

ScanTool.net OBD-II Software for ElmScan is free software that allows you to 
use your computer and an inexpensive hardware interface to read information 
from your car's computer. Current version allows you to read trouble codes and 
see their descriptions, clear the codes and turn off the "Check Engine" light, 
and display real-time sensor data such as RPM, Engine Load, Vehicle Speed, 
Coolant Temperature, and Timing Advance.

For more information about the ElmScan OBD-II interface, please visit 
our website, located at http://www.ScanTool.net.


======================================
===== Requisitos Mínimos (2025) ======
======================================

Linux:
   - Processador x86 ou ARM
   - 128Mb RAM
   - Linux Ubuntu 24.04 ou superior
   - SDL2 e SDL2_ttf instalados
   - Display 800x480 ou superior
   - Porta serial (USB/RS232)

======================================
========== Compilação ================
======================================

1. Instale as dependências:
   - SDL2: sudo apt install libsdl2-dev
   - SDL2_ttf: sudo apt install libsdl2-ttf-dev
2. Compile com:
   gcc *.c -o scantool -lSDL2 -lSDL2_ttf
3. Execute com:
   ./scantool

======================================
========== Novidades 2025 ============
======================================

- Migração completa do sistema gráfico para SDL2/SDL2_ttf
- Compatível com Linux (Windows/Allegro removido)
- Sistema de medidas apenas métrico
- Interface adaptada para motocicleta Honda CB500X
- Diálogos e botões redesenhados com SDL2
- Remoção total de código legado Allegro/DIALOG
- Serial adaptado para POSIX/termios


======================================
========== Troubleshooting ===========
======================================

If you are having problems running the software, please visit our
Support Page located at http://www.scantool.net/support


======================================
========== Histórico de Versões ======
======================================

   v2.00  -  (2025) Migração total para SDL2/SDL2_ttf, compatibilidade Linux, sistema métrico, interface Honda CB500X, serial POSIX, remoção de Allegro/DIALOG, diálogos SDL2
   v1.21  -  Baud rate padrão 115.2k
   v1.20  -  Seleção dinâmica de porta COM, correções Bluetooth, erros UART ELM327, impressão de versão de firmware, bugfixes
   v1.15  -  Suporte ElmScan 5 Compact, OBDLink/OBDLink CI, CFG por exclusão, correção de sensores desabilitados, CPU 100%, bugfixes
   v1.14  -  Novos baud rates, validação scan tool
   v1.13  -  Correções de fórmulas, taxa de atualização, DTCs, bugfixes
   v1.12  -  Descrições de erro ELM327, respostas CAN DTC, warning Clear Codes, bugfixes
   v1.11  -  Correção Display Codes crash, erro porta COM
   v1.10  -  Baud rate switching, ELM327, detecção protocolo, pending DTCs, OBD Info, Main Menu, About, bugfixes
   v1.09  -  Correção 7F KWP2000, número de códigos, DTCs múltiplos
   v1.08  -  Correção ECUs padding 0
   v1.07  -  Novos sensores SAE J1979, Sensor Data, código, COM 5-8, codes.dat atualizado, info sistema
   v1.06  -  Correção RS232, fórmula US, novos sensores, layout Sensor Data, bugfixes
   v1.04  -  Biblioteca serial atualizada, bugfixes, suporte multiplataforma
   v1.03  -  Correção sensores ELM323
   v1.02  -  Bugfixes
   v1.01  -  Bugfixes
   v1.00  -  Lançamento inicial
