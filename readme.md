# ScanTool.net OBD-II Software (SDL2/Linux)

ScanTool.net OBD-II Software é um programa livre para diagnóstico OBD-II, agora adaptado para motocicletas Honda (CB500X) e ambiente Linux. Permite ler códigos de erro, visualizar descrições, limpar códigos e exibir dados de sensores em tempo real (RPM, carga do motor, velocidade, temperatura do líquido de arrefecimento, avanço de ignição, etc.).

## Principais atualizações
- **Migração completa do sistema gráfico para SDL2/SDL2_ttf** (substituindo Allegro)
- **Compatível com Linux** (compilação via gcc)
- **Sistema de medidas apenas métrico**
- **Interface adaptada para motocicleta Honda CB500X**
- **Diálogos e botões redesenhados com SDL2**
- **Remoção total de código legado Allegro/DIALOG**
- **Serial adaptado para POSIX/termios**

## Como compilar (Linux)
1. Instale as dependências:
	- SDL2: `sudo apt install libsdl2-dev`
	- SDL2_ttf: `sudo apt install libsdl2-ttf-dev`
2. Compile com:
	```bash
	gcc *.c -o scantool -lSDL2 -lSDL2_ttf
	```
3. Execute com:
	```bash
	./scantool
	```

## Observações
- O código foi totalmente limpo e modernizado para SDL2.
- Não há mais suporte para Windows/Allegro.
- O sistema de unidades é exclusivamente métrico.
- Foco em motocicleta Honda CB500X, mas pode ser adaptado para outros veículos OBD-II.

## Créditos
Projeto original: ScanTool.net
Migração SDL2/Linux: EdmarAJr