/*
 * Galagino - Galaga arcade for ESP32 and Platformio
 * VGA32 port — basado en DynaMight1124/galagino
 */
#include <Arduino.h>
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

// ── Backend de vídeo ─────────────────────────────────────────
#ifdef VIDEO_BACKEND_VGA32
  #include "emulation/vga_driver.h"
  // La clase Video se define en video_vga32.h (stub compatible)
  #include "emulation/video_vga32.h"
  #include "emulation/ps2_input.h"
#else
  #include "emulation/video.h"
#endif

// Stack aumentado para compatibilidad con FabGL
SET_LOOP_TASK_STACK_SIZE(16384);

signed char machinesCount = (signed char)(sizeof(machines) / sizeof(machines[0]));
machineBase *currentMachine;
struct sprite_S *sprite_buffer;
unsigned short *frame_buffer;
unsigned char *memory;

Audio audio = Audio();
Video video = Video();
Input input = Input();
Menu  menu  = Menu();
#ifdef LED_PIN
  Led led = Led();
#endif

// ── Variables y tareas VGA32 ─────────────────────────────────
/*volatile bool emulation_ready = false;

#ifdef VIDEO_BACKEND_VGA32
TaskHandle_t audiotask = nullptr;

void audio_task(void *p) {
  Serial.println("[AUDIO] Esperando emulacion...");
  while(!emulation_ready) vTaskDelay(10);
  Serial.println("[AUDIO] Iniciando tarea de audio");
  TickType_t last = xTaskGetTickCount();
  while(1) {
    audio.transmit();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(3));
  }
}
#endif*/

void updateAudioVideo(void);
void renderRow(short row, bool isMenu);
void onDoAttractReset();
void onVolumeUpDown(bool up, bool down);
void onDoReset();
bool doReset = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Galagino VGA32");

  Serial.print("ESP-IDF "); Serial.println(ESP_IDF_VERSION, HEX);

#ifdef WORKAROUND_I2S_APLL_PROBLEM
  Serial.println("I2S APLL workaround active");
#endif

  setCpuFrequencyMhz(240);
  Serial.printf("CPU: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

  // Allocate buffers
  frame_buffer  = (unsigned short*)malloc(224 * 8 * 2);
  sprite_buffer = (sprite_S*)malloc(128 * sizeof(sprite_S));
  memory        = (uint8_t*)malloc(RAMSIZE);

  if (!frame_buffer || !sprite_buffer || !memory) {
    Serial.println("ERROR: malloc falló");
    while(1) delay(100);
  }
  Serial.printf("frame_buffer @ %p\n", frame_buffer);

  currentMachine = machines[0];
  for (int i = 0; i < machinesCount; i++)
    machines[i]->init(&input, frame_buffer, sprite_buffer, memory);

  // ── Inicializar vídeo PRIMERO ────────────────────────────
  video.begin();  // llama a tft_init() en VGA32

  // ── Audio ────────────────────────────────────────────────
  audio.init();
  audio.start(currentMachine);

#ifdef VIDEO_BACKEND_VGA32
  
  ps2_input_init();
  // Tarea de audio dedicada (evita conflicto timing con vídeo)
  //emulation_ready = true;
  //xTaskCreatePinnedToCore(audio_task, "audio", 4096, NULL, 2, &audiotask, 1);
#endif

  input.init(machinesCount == 1);
  input.onVolumeUpDown(onVolumeUpDown);
  input.onDoReset(onDoReset);
  input.onDoAttractReset(onDoAttractReset);

  menu.init(&input, machines, machinesCount, frame_buffer);

#ifdef LED_PIN
  led.init();
#endif

  // Delay inicial para evitar race condition con emulación
  // (se reemplaza por emulation_ready en emulation_start)
  delay(500);

  Serial.printf("Free heap final: %d\n", ESP.getFreeHeap());
}

void loop(void) {
  // Esperar a que la emulación esté lista
  //static bool waited = false;
  /*if (!waited) {
    while(!emulation_ready) vTaskDelay(10);
    waited = true;
  }*/

  updateAudioVideo();

#ifdef LED_PIN
  led.update(machines, menu.machineIndexPreselection(), menu.machineIndexSelected());
#endif
}

void updateAudioVideo(void) {
  uint32_t t0 = micros();
  bool isMenu = menu.machineIndexIsMenu();
  //emulation_ready=true;
  //Serial.println("menu");
  if (isMenu) {
    menu.handle();
  } else {
    if (menu.startMachine()) {
      currentMachine = machines[menu.machineIndexSelected()];
      audio.start(currentMachine);
      video.flip(currentMachine->videoFlipY(), currentMachine->videoFlipX());
      emulation_start();
    }
    currentMachine->prepare_frame();
  }

  if (doReset || menu.attract_gameTimeout()) {
    emulation_stop();
    video.flipReset(currentMachine->videoFlipY(), currentMachine->videoFlipX());
    menu.show_menu();
    doReset = false;
  }

#ifdef VIDEO_BACKEND_VGA32
  // ── VGA32: renderizar frame completo ──────────────────────
  tft_start_frame();
  for (int c = 0; c < 36; c++) {
    renderRow(c, isMenu);
    for (int row = 0; row < 8; row++)
      tft_write_pixels(0, c * 8 + row, frame_buffer + row * 224, 224);
    // Audio gestionado por audio_task independiente
    audio.transmit();
  }
  
  tft_end_frame();
  emulation_notifyGive();
vTaskDelay(1);
#else
  // ── TFT SPI original ──────────────────────────────────────
  bool videoHalfRate = true;
#ifndef VIDEO_HALF_RATE
  videoHalfRate = currentMachine->useVideoHalfRate() && !isMenu;
#endif

  if (!videoHalfRate) {
    for (int c = 0; c < 36; c += 6) {
      for (int i = 0; i < 6; i++) {
        renderRow(c + i, isMenu); video.write(frame_buffer, 224 * 8);
      }
      audio.transmit();
    }
    unsigned long t1 = (micros() - t0) / 1000;
    if (t1 < 16) vTaskDelay(16 - t1);
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
      if (t1 < (half ? 33 : 16)) vTaskDelay((half ? 33 : 16) - t1);
      else if (half)              vTaskDelay(1);
      emulation_notifyGive();
    }
  }

#endif // VIDEO_BACKEND_VGA32
}

void renderRow(short row, bool isMenu) {
  if (isMenu) {
    menu.render_row(row);
  } else {
    memset(frame_buffer, 0, 2 * 224 * 8);
    currentMachine->render_row(row);
  }
}

void onVolumeUpDown(bool up, bool down) { audio.volumeUpDown(up, down); }
void onDoAttractReset() { menu.attract_resetTimer(); }
void onDoReset() {
  if (!menu.machineIndexIsMenu()) doReset = true;
}
