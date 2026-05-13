// main_vga32_patch.cpp
// =====================
// Instrucciones para parchear main.cpp para VGA32.
// NO es un archivo compilable — es una guía de cambios.
//
// CAMBIO 1: includes al principio de main.cpp
// --------------------------------------------
// Añadir después de los includes existentes:
//
//   #ifdef VIDEO_BACKEND_VGA32
//   #include "vga_driver.h"
//   #endif
//
// El include de "emulation/video.h" se mantiene pero video.h
// debe detectar VIDEO_BACKEND_VGA32 (ver CAMBIO 2).
//
//
// CAMBIO 2: en emulation/video.h
// --------------------------------
// Rodear todo el contenido con:
//
//   #ifndef VIDEO_BACKEND_VGA32
//   ... contenido original ...
//   #endif
//
// Y añadir al final, dentro del guard:
//
//   #ifdef VIDEO_BACKEND_VGA32
//   #include "../../include/video_vga32.h"
//   #endif
//
//
// CAMBIO 3: en emulation/video.cpp
// ----------------------------------
// Rodear todo el contenido con:
//
//   #ifndef VIDEO_BACKEND_VGA32
//   ... contenido original ...
//   #endif
//
//
// CAMBIO 4: setup() en main.cpp
// ------------------------------
// Añadir SET_LOOP_TASK_STACK_SIZE antes de setup():
//
//   SET_LOOP_TASK_STACK_SIZE(16384);
//
// En setup(), el orden debe ser:
//   1. video.begin()  ← ya llama a tft_init()
//   2. audio.init() + audio.start()
//   3. xTaskCreatePinnedToCore(audio_task, ...)  ← tarea audio
//   4. xTaskCreatePinnedToCore(emulation_task, ...)  ← si existe
//
//
// CAMBIO 5: updateAudioVideo() — reemplazar el bucle de vídeo
// ------------------------------------------------------------
// Sustituir los dos bloques (videoHalfRate y !videoHalfRate)
// por el bloque VGA32 cuando está activo:
//
//   #ifdef VIDEO_BACKEND_VGA32
//     tft_start_frame();
//     for(int c = 0; c < 36; c++) {
//       renderRow(c, isMenu);
//       for(int row = 0; row < 8; row++)
//         tft_write_pixels(0, c*8+row, frame_buffer + row*224, 224);
//     }
//     tft_end_frame();
//     emulation_notifyGive();
//   #else
//     ... código original TFT ...
//   #endif
//
//
// CAMBIO 6: audio_task dedicada
// ------------------------------
// Añadir junto a las otras variables globales:
//
//   #ifdef VIDEO_BACKEND_VGA32
//   TaskHandle_t audiotask = nullptr;
//   volatile bool emulation_ready = false;
//
//   void audio_task(void *p) {
//     while(!emulation_ready) vTaskDelay(10);
//     TickType_t last = xTaskGetTickCount();
//     while(1) {
//       audio.transmit();
//       vTaskDelayUntil(&last, pdMS_TO_TICKS(3));
//     }
//   }
//   #endif
//
// En setup(), crear la tarea:
//   #ifdef VIDEO_BACKEND_VGA32
//   xTaskCreatePinnedToCore(audio_task, "audio", 4096, NULL, 4, &audiotask, 0);
//   #endif
//
// Y en emulation_start() o donde se inicie la emulación:
//   emulation_ready = true;
//
//
// CAMBIO 7: loop() — esperar emulation_ready
// -------------------------------------------
//   void loop(void) {
//     #ifdef VIDEO_BACKEND_VGA32
//     static bool waited = false;
//     if(!waited) { while(!emulation_ready) vTaskDelay(10); waited = true; }
//     #endif
//     updateAudioVideo();
//     ...
//   }
//
// FIN DE INSTRUCCIONES
