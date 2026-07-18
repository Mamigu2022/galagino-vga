// ps2_input.h — Soporte teclado PS/2 para TTGO VGA32 via FabGL
// Pines PS/2 hardwired en TTGO VGA32: CLK=GPIO33, DATA=GPIO32

#ifndef PS2_INPUT_H
#define PS2_INPUT_H

#include "config.h"

#ifdef KEYBOARD

#include <FabGL.h>

// El teclado PS/2 lo gestiona FabGL automáticamente
// cuando se inicializa con setResolution().
// Solo necesitamos leer las teclas con Keyboard.isVKDown().
extern fabgl::PS2Controller PS2Ctrl;
void ps2_input_update(void);
void ps2_input_init(void);

// Devuelve true si la tecla está pulsada ahora mismo
bool ps2_key_start(void);
bool ps2_key_coin(void);
bool ps2_key_left(void);
bool ps2_key_right(void);
bool ps2_key_up(void);
bool ps2_key_down(void);
bool ps2_key_fire(void);

// Mapeo de teclas (puedes cambiarlas aquí)
// Teclado numérico o cursor para joystick, Enter=fuego, 1=start, 5=moneda
#define PS2_KEY_LEFT   fabgl::VK_LEFT
#define PS2_KEY_RIGHT  fabgl::VK_RIGHT
#define PS2_KEY_UP     fabgl::VK_UP
#define PS2_KEY_DOWN   fabgl::VK_DOWN
#define PS2_KEY_FIRE   fabgl::VK_RETURN     // Enter
#define PS2_KEY_FIRE2  fabgl::VK_SPACE      // Espacio también dispara
#define PS2_KEY_START  fabgl::VK_1          // 1 = Start
#define PS2_KEY_COIN   fabgl::VK_5          // 5 = Coin (como MAME)
#define PS2_KEY_START2 fabgl::VK_F1         // F1 alternativo start
#define PS2_KEY_COIN2  fabgl::VK_F2         // F2 alternativo coin

#endif // VIDEO_BACKEND_VGA32
#endif // PS2_INPUT_H
