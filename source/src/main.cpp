/*
 * Galagino - Galaga arcade for ESP32 and Platformio
 * VGA32 port — basado en DynaMight1124/galagino
 */
#include <Arduino.h>
#include "esp_task_wdt.h"
#include "config.h"
#include "machines.h"
#include "machines/machineBase.h"
#include "emulation/audio.h"
#include "emulation/input.h"
#include "emulation/menu.h"
#include "emulation/emulation.h"
#ifdef LED_PIN
  #include "emulation/led.h"
#endif
#ifdef BT_GAMEPAD_INPUT
  #include "emulation/ble_gamepad.h"
#endif
#ifdef VIDEO_BACKEND_VGA32
  #include "emulation/vga_driver.h"
  #include "emulation/video_vga32.h"
#else
  #include "emulation/video.h"
#endif

#ifdef KEYBOARD
  #include "emulation/ps2_input.h"
#endif

//SET_LOOP_TASK_STACK_SIZE(16384);

// ── Globales — declarados como punteros para evitar construcción
// estática antes de que el heap de Bluepad32 esté listo ────────
machineBase *machines[MAX_MACHINES];
signed char  machinesCount = 0;
machineBase *currentMachine;
struct sprite_S *sprite_buffer;
unsigned short  *frame_buffer;
unsigned char   *memory;

// Objetos principales — punteros, se crean en setup()
static Audio *audioPtr;
static Video *videoPtr;
static Input *inputPtr;
static Menu  *menuPtr;

// Referencias globales para compatibilidad con el resto del código
#define audio (*audioPtr)
#define video (*videoPtr)
#define input (*inputPtr)
#define menu  (*menuPtr)

#ifdef LED_PIN
  Led led = Led();
#endif

void updateAudioVideo(void);
void renderRow(short row, bool isMenu);
void onDoAttractReset();
void onVolumeUpDown(bool up, bool down);
void onDoReset();
bool doReset = false;

void setup() {
  //esp_task_wdt_init(10, false);
  // ── Crear objetos en heap ANTES de cualquier otra cosa ──────
  // Esto evita construcción estática que crashea con Bluepad32
  audioPtr = new Audio();
  videoPtr = new Video();
  inputPtr = new Input();
  menuPtr  = new Menu();

  Serial.begin(115200);
  delay(500);
  Serial.println("Galagino VGA32 BT");

  setCpuFrequencyMhz(240);
  Serial.printf("CPU: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

  // ── Inicializar máquinas ─────────────────────────────────────
  initMachines();
  Serial.printf("Machines: %d\n", machinesCount);

    
    // Verificar PSRAM antes de usarla
    Serial.printf("PSRAM found: %s\n", psramFound() ? "SI" : "NO");
    Serial.printf("PSRAM size: %d\n", ESP.getPsramSize());
    
    if (!psramFound()) {
        Serial.println("ERROR: PSRAM no detectada, comprobando heap normal...");
        Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
        // Fallback a malloc normal si no hay PSRAM
        frame_buffer  = (unsigned short*)malloc(224 * 8 * 2);
        sprite_buffer = (sprite_S*)malloc(128 * sizeof(sprite_S));
        memory        = (uint8_t*)malloc(RAMSIZE);
    } else {
        frame_buffer  = (unsigned short*)ps_malloc(224 * 8 * 2);
        sprite_buffer = (sprite_S*)ps_malloc(128 * sizeof(sprite_S));
        memory        = (uint8_t*)ps_malloc(RAMSIZE);
    }

  if (!frame_buffer || !sprite_buffer || !memory) {
    Serial.println("ERROR: ps_malloc falló");
    while(1) delay(100);
  }
  Serial.printf("Buffers OK, heap: %d\n", ESP.getFreeHeap());

  currentMachine = machines[0];
  for (int i = 0; i < machinesCount; i++)
    machines[i]->init(inputPtr, frame_buffer, sprite_buffer, memory);
  Serial.println("Machines init OK");

  // ── Vídeo ────────────────────────────────────────────────────
  video.begin();
  Serial.println("Video OK");

  // ── Audio ────────────────────────────────────────────────────
  audio.init();
  audio.start(currentMachine);
  Serial.println("Audio OK");

  // ── Teclado PS/2 (solo sin BT) ──────────────────────────────
#ifdef KEYBOARD
  #ifndef BT_GAMEPAD_INPUT
    ps2_input_init();
    Serial.println("PS2 OK");
  #endif
#endif

  // ── Input (incluye init de Bluepad32 si BT_GAMEPAD_INPUT) ───
  input.init(machinesCount == 1);
  input.onVolumeUpDown(onVolumeUpDown);
  input.onDoReset(onDoReset);
  input.onDoAttractReset(onDoAttractReset);
  Serial.println("Input OK");

  menu.init(inputPtr, machines, machinesCount, frame_buffer);
  Serial.println("Menu OK");

#ifdef LED_PIN
  led.init();
#endif

  delay(500);
  Serial.printf("Free heap final: %d\n", ESP.getFreeHeap());
  Serial.println("Setup completo");
}

void loop(void) {
  // Esperar a que la emulación esté lista
  //static bool waited = false;
  /*if (!waited) {
    while(!emulation_ready) vTaskDelay(10);
    waited = true;
  }*/
 #ifdef BT_GAMEPAD_INPUT
  ble_gamepad_update();
 #endif 
  updateAudioVideo();

#ifdef LED_PIN
  led.update(machines, menu.machineIndexPreselection(), menu.machineIndexSelected());
#endif
}
uint32_t fps_timer = 0;
uint32_t frame_count = 0;
uint32_t fps_actuales = 0;
void updateAudioVideo(void) {
  uint32_t t0 = micros();
  bool isMenu = menu.machineIndexIsMenu();

  if (isMenu) {
    menu.handle();
  } else {
    if (menu.startMachine()) {
      currentMachine = machines[menu.machineIndexSelected()];
      audio.start(currentMachine);
      //video.flip(currentMachine->videoFlipY(), currentMachine->videoFlipX());
      emulation_start();
    }
    currentMachine->prepare_frame();
  }

  if (doReset || menu.attract_gameTimeout()) {
    emulation_stop();
    //video.flipReset(currentMachine->videoFlipY(), currentMachine->videoFlipX());
    menu.show_menu();
    doReset = false;
  }

#ifdef VIDEO_BACKEND_VGA32
  // ── VGA32: Renderizado dinámico adaptativo ──────────────────
  bool videoHalfRate = currentMachine->useVideoHalfRate() && !isMenu;
  static bool skipFrame = false;
  if (!videoHalfRate) {
    // Juegos ligeros: Renderizan todos los frames
    tft_start_frame();
    for (int c = 0; c < 36; c++) {
      renderRow(c, isMenu);
      for (int row = 0; row < 8; row++) {
        tft_write_pixels(0, c * 8 + row, frame_buffer + (row * 224), 224);
      }
      audio.transmit();
    }
    tft_end_frame();
  } else {
    // Juegos pesados (Star Force): Renderizado entrelazado/alterno por ráfagas
    // Esto duplica inmediatamente la velocidad sacrificando frames visuales intermedios
    
    skipFrame = !skipFrame;

    tft_start_frame();
    for (int c = 0; c < 36; c++) {
      if (!skipFrame) {
        renderRow(c, isMenu);
        for (int row = 0; row < 8; row++) {
          tft_write_pixels(0, c * 8 + row, frame_buffer + (row * 224), 224);
        }
      }
      // El audio y la lógica del juego DEBEN transmitirse siempre para no congelarse
      audio.transmit();
    }
    tft_end_frame();
  }

  /*// Sincronizador dinámico de hilos para dar aire al Bluetooth
    unsigned long t1 = (micros() - t0) / 1000;
static int dbg_counter = 0;
if (++dbg_counter >= 60) {
  dbg_counter = 0;
  Serial.printf("[VIDEO] render+audio: %lums\n", t1);
} */
  
  emulation_notifyGive();

#else
  // ── TFT SPI original ──────────────────────────────────────
  bool videoHalfRate = true;
#ifndef VIDEO_HALF_RATE
  videoHalfRate = currentMachine->useVideoHalfRate() && !isMenu;
#endif

  if (!videoHalfRate) {
    for (int c = 0; c < 36; c += 6) {
      for (int i = 0; i < 6; i++) {
        renderRow(c + i, isMenu); 
        video.write(frame_buffer, 224 * 8);
      }
      audio.transmit();
    }
    unsigned long t1 = (micros() - t0) / 1000;
    if (t1 < 16) vTaskDelay(pdMS_TO_TICKS(16 - t1));
    else         vTaskDelay(1);
    emulation_notifyGive();
  } else {
    for (int half = 0; half < 2; half++) {
      for (int c = 18 * half; c < 18 * (half + 1); c += 3) {
        renderRow(c + 0, isMenu); video.write(frame_buffer, 224 * 8);
        renderRow(c + 1, isMenu); video.write(frame_buffer, 224 * 8);
        renderRow(c + 2, isMenu); video.write(frame_buffer, 224 * 8);
        audio.transmit();
      }
      unsigned long t1 = (micros() - t0) / 1000;
      if (t1 < (half ? 33 : 16)) vTaskDelay(pdMS_TO_TICKS((half ? 33 : 16) - t1));
      else if (half)              vTaskDelay(1);
      emulation_notifyGive();
    }
  }
#endif // VIDEO_BACKEND_VGA32
#if 0
frame_count++;
  if (millis() - fps_timer >= 1000) { // Cada vez que pasa 1 segundo (1000 ms)
    fps_actuales = frame_count;
    frame_count = 0;
    fps_timer = millis();
    
    // Imprime el resultado en el monitor serie de Arduino
    Serial.println(fps_actuales);
  }
 #endif
}

void renderRow(short row, bool isMenu) {
  if (isMenu) {
    menu.render_row(row);
  } else {
    // OPTIMIZACIÓN: Solo limpiar si el core gráfico del juego no llena el buffer por completo.
    // Si tu juego redibuja todas las líneas de fondo, puedes comentar el memset para ganar velocidad.
    memset(frame_buffer, 0, 3584);// 2 * 224 * 8);
    currentMachine->render_row(row);
  }
}

void onVolumeUpDown(bool up, bool down) { audio.volumeUpDown(up, down); }
void onDoAttractReset() { menu.attract_resetTimer(); }
void onDoReset() {
  if (!menu.machineIndexIsMenu()) doReset = true;
}
