#pragma once
#ifdef BT_GAMEPAD_INPUT

#include <stdint.h>
#include <stdbool.h>

// ── Máscaras de botones ───────────────────────────────────────
#define BUTTON_LEFT   0x01
#define BUTTON_RIGHT  0x02
#define BUTTON_UP     0x04
#define BUTTON_DOWN   0x08
#define BUTTON_FIRE   0x10
#define BUTTON_START  0x20
#define BUTTON_COIN   0x40
#define BUTTON_EXTRA  0x80

// ── API pública ───────────────────────────────────────────────
void    ble_gamepad_init(void);
void    ble_gamepad_update(void);
uint8_t ble_gamepad_buttons(void);
bool    ble_gamepad_connected(void);

#endif // BT_GAMEPAD_INPUT
