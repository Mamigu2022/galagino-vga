
// bt_input.cpp — Gamepad BT usando BluePad32
// Compatible con FabGL + ESP32 (core 0 para BT, core 1 para VGA)


#include <Arduino.h>
#include "bt_input.h"
#ifdef BT_GAMEPAD_INPUT_BLUEPAD
#include "C:/Users/migue/.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32/include/bluepad32_arduino/include/Bluepad32.h"


// ── Mascaras Galagino ─────────────────────────────────────────
#define G_BUTTON_LEFT  0x01
#define G_BUTTON_RIGHT 0x02
#define G_BUTTON_UP    0x04
#define G_BUTTON_DOWN  0x08
#define G_BUTTON_FIRE  0x10
#define G_BUTTON_EXTRA 0x80

// Umbral ejes analogicos (-512 a +511 en BluePad32)
#define BT_AXIS_THRESHOLD 100

// ── Estado compartido ─────────────────────────────────────────
static volatile unsigned char _state = 0;
static volatile bool _connected = false;
static bool _initialized = false;

static GamepadPtr _gamepad = nullptr;

// ── Callbacks BluePad32 ───────────────────────────────────────
static void onConnectedGamepad(GamepadPtr gp) {
    _gamepad = gp;
    _connected = true;
    Serial.println("[BP32] Mando conectado");
    GamepadProperties props = gp->getProperties();
    Serial.printf("[BP32] Modelo: %s, BDA: %s\n",
        gp->getModelName().c_str(),
        props.btaddr);
}

static void onDisconnectedGamepad(GamepadPtr gp) {
    _gamepad = nullptr;
    _connected = false;
    _state = 0;
    Serial.println("[BP32] Mando desconectado");
}

// ── Tarea BluePad32 ───────────────────────────────────────────
static void btTask(void* p) {
    while (true) {
        // update() procesa los eventos BT pendientes
        bool updated = BP32.update();

        if (updated && _gamepad && _gamepad->isConnected()) {
            unsigned char result = 0;

            // ── D-pad ─────────────────────────────────────────
            if (_gamepad->dpad() & DPAD_LEFT)  result |= G_BUTTON_LEFT;
            if (_gamepad->dpad() & DPAD_RIGHT) result |= G_BUTTON_RIGHT;
            if (_gamepad->dpad() & DPAD_UP)    result |= G_BUTTON_UP;
            if (_gamepad->dpad() & DPAD_DOWN)  result |= G_BUTTON_DOWN;

            // ── Stick izquierdo ───────────────────────────────
            int ax = _gamepad->axisX();
            int ay = _gamepad->axisY();
            if (ax < -BT_AXIS_THRESHOLD) result |= G_BUTTON_LEFT;
            if (ax >  BT_AXIS_THRESHOLD) result |= G_BUTTON_RIGHT;
            if (ay < -BT_AXIS_THRESHOLD) result |= G_BUTTON_UP;
            if (ay >  BT_AXIS_THRESHOLD) result |= G_BUTTON_DOWN;

            // ── Botones de accion ─────────────────────────────
            if (_gamepad->a() || _gamepad->b())   result |= G_BUTTON_FIRE;
            if (_gamepad->x() || _gamepad->y())   result |= G_BUTTON_FIRE;
            if (_gamepad->l1() || _gamepad->r1()) result |= G_BUTTON_FIRE;
            // Select/Start → EXTRA (moneda/inicio en Galagino)
            if (_gamepad->miscSelect())            result |= G_BUTTON_EXTRA;
            if (_gamepad->miscStart())             result |= G_BUTTON_EXTRA;

            _state = result;
        }

        // BluePad32 necesita ser llamado frecuentemente
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ── API C pura ────────────────────────────────────────────────
extern "C" void bt_input_setup(void) {
    Serial.println("[BP32] Iniciando BluePad32...");

    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    // Olvida mandos emparejados anteriormente para empezar limpio
    // Comenta esta linea si quieres reconexion automatica
    BP32.forgetBluetoothKeys();

    // Tarea BT en core 0 — FabGL usa core 1 para VGA
    xTaskCreatePinnedToCore(
        btTask,
        "btTask",
        4096,    // stack generoso para BluePad32
        NULL,
        1,       // prioridad alta para no perder eventos
        NULL,
        0        
    );

    _initialized = true;
    Serial.println("[BP32] Listo. Pon el mando en modo pairing.");
}

extern "C" unsigned char bt_input_get_state(void) {
    if (!_initialized || !_connected) return 0;
    return _state;
}
#endif // BT_GAMEPAD_INPUT_BLUEPAD
// ble_gamepad.cpp — Cliente HID BLE con NimBLE 2.5.x (h2zero)
