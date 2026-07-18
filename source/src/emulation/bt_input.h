#pragma once

// bt_input.h — API unificada de entrada BT para Galagino
// Soporta: NimBLE HID (BT_GAMEPAD_INPUT)
//          BluePad32  (BT_GAMEPAD_INPUT_BLUEPAD)

#ifdef BT_GAMEPAD_INPUT_BLUEPAD

#ifdef __cplusplus
extern "C" {
#endif

void          bt_input_setup(void);
unsigned char bt_input_get_state(void);

#ifdef __cplusplus
}
#endif

#endif // BT_GAMEPAD_INPUT_BLUEPAD