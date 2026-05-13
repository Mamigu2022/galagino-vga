// ps2_input.cpp — Implementación teclado PS/2 para TTGO VGA32

#include "config.h"

#ifdef VIDEO_BACKEND_VGA32

#include "ps2_input.h"
#include <FabGL.h>

// FabGL gestiona el PS/2 a través del VGAController.
// El Keyboard se inicializa automáticamente con PS2Controller.
// En TTGO VGA32: KB_CLK=GPIO33, KB_DATA=GPIO32

fabgl::PS2Controller PS2Ctrl;
void ps2_input_update(void) {
  auto keyboard = PS2Ctrl.keyboard();
  // Vaciamos todos los eventos acumulados para que la cola no se llene
  while (keyboard->virtualKeyAvailable()) {
    keyboard->getNextVirtualKey();
  }
}

void ps2_input_init(void) {
  // Inicializar PS/2 con teclado en puerto 0 (pines 33/32 del VGA32)
  // FabGL ya sabe los pines del VGA32 si usas el preset correcto
  PS2Ctrl.begin(PS2Preset::KeyboardPort0);
  Serial.println("[PS2] Teclado PS/2 iniciado (CLK=33, DATA=32)");
}

static inline bool key_down(fabgl::VirtualKey vk) {
  return PS2Ctrl.keyboard()->isVKDown(vk);
}

bool ps2_key_left(void)  { return key_down(PS2_KEY_LEFT); }
bool ps2_key_right(void) { return key_down(PS2_KEY_RIGHT); }
bool ps2_key_up(void)    { return key_down(PS2_KEY_UP); }
bool ps2_key_down(void)  { return key_down(PS2_KEY_DOWN); }
bool ps2_key_fire(void)  { return key_down(PS2_KEY_FIRE)  || key_down(PS2_KEY_FIRE2); }
bool ps2_key_start(void) { return key_down(PS2_KEY_START) || key_down(PS2_KEY_START2); }
bool ps2_key_coin(void)  { return key_down(PS2_KEY_COIN)  || key_down(PS2_KEY_COIN2); }

#endif // VIDEO_BACKEND_VGA32
