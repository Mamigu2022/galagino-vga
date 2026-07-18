#include "config.h"
#ifdef BT_GAMEPAD_INPUT

#include "ble_gamepad.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <nvs_flash.h>

#define CUSTOM_SERVICE_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CUSTOM_CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"

static volatile uint8_t s_buttons   = 0;
static volatile bool    s_connected = false;
static NimBLEClient* s_client    = nullptr;

static void parse_hid_report(const uint8_t* data, size_t len) {
  if (len < 3) return; 
  
  uint8_t state = 0;

  if (data[0] < 40)  state |= BUTTON_LEFT;
  if (data[0] > 200) state |= BUTTON_RIGHT;
  if (data[1] < 40)  state |= BUTTON_UP;
  if (data[1] > 200) state |= BUTTON_DOWN;

  uint8_t btns = data[2];
  if (btns & 0x01) state |= BUTTON_FIRE;  
  if (btns & 0x02) state |= BUTTON_FIRE;  
  if (btns & 0x04) state |= BUTTON_START; 
  if (btns & 0x08) state |= BUTTON_COIN;  

  s_buttons = state;
}

// Callback ultra-rápido sin Serial.print para evitar colapsos con FabGL/VGA
static void custom_notify_cb(NimBLERemoteCharacteristic* c, uint8_t* data, size_t len, bool isNotify) {
  parse_hid_report(data, len);
}

static bool subscribe_custom(NimBLEClient* client) {
  Serial.println("[GALAGINO-RX] Explorando tabla de canales...");
  
  NimBLERemoteService* pService = client->getService(CUSTOM_SERVICE_UUID);
  if (!pService) {
    Serial.println("[GALAGINO-RX] Error: No se localiza el servicio");
    return false;
  }

  NimBLERemoteCharacteristic* c = pService->getCharacteristic(CUSTOM_CHAR_UUID);
  if (c && c->canNotify()) {
    if (c->subscribe(true, custom_notify_cb)) { 
      Serial.println("[GALAGINO-RX] ¡CONEXIÓN ESTABLECIDA Y CANAL DE DATOS ABIERTO!");
      return true;
    }
  }
  return false;
}

class ClientCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* c) override { s_connected = true; }
  void onDisconnect(NimBLEClient* c, int reason) override {
    s_connected = false;
    s_buttons   = 0;
    Serial.printf("[GALAGINO-RX] Desconexión de radio. Buscando...\n");
    NimBLEDevice::getScan()->start(0, false, true);
  }
};
static ClientCallback s_client_cb;

static NimBLEAddress s_pending_addr("", 0);
static bool          s_pending_connect = false;

class ScanCallback : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* dev) override {
    if (!dev->isAdvertisingService(NimBLEUUID(CUSTOM_SERVICE_UUID))) return;
    Serial.println("[GALAGINO-RX] ¡Mando verificado en el aire!");
    s_pending_addr    = dev->getAddress();
    s_pending_connect = true;
    NimBLEDevice::getScan()->stop();
  }
};
static ScanCallback s_scan_cb;

void ble_gamepad_init(void) {
  ESP_ERROR_CHECK(nvs_flash_erase());
  nvs_flash_init();

  NimBLEDevice::init("Galagino-Receiver");
  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
  NimBLEDevice::setSecurityAuth(false, false, false);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(&s_scan_cb, false);
  scan->setActiveScan(true);
  scan->setInterval(120);
  scan->setWindow(60); 
  scan->start(0, false, true);
  Serial.println("[GALAGINO-RX] Escaneando frecuencia...");
}

void ble_gamepad_update(void) {
  if (!s_pending_connect) return;
  s_pending_connect = false;

  if (s_client) {
    NimBLEDevice::deleteClient(s_client);
    s_client = nullptr;
  }

  s_client = NimBLEDevice::createClient(s_pending_addr);
  s_client->setClientCallbacks(&s_client_cb, false);

  if (!s_client->connect()) {
    NimBLEDevice::deleteClient(s_client);
    s_client = nullptr;
    NimBLEDevice::getScan()->start(0, false, true);
    return;
  }

  if (!subscribe_custom(s_client)) {
    s_client->disconnect();
    NimBLEDevice::deleteClient(s_client);
    s_client = nullptr;
    NimBLEDevice::getScan()->start(0, false, true);
  }
  //vTaskDelay(pdMS_TO_TICKS(10));
}

uint8_t ble_gamepad_buttons(void)     { return s_buttons; }
bool    ble_gamepad_connected(void)   { return s_connected; }

#endif