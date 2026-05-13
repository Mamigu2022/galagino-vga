// video_vga32.h — Reemplaza video.h cuando VIDEO_BACKEND_VGA32 está activo
// Incluir en lugar de "emulation/video.h" en main.cpp

#ifndef VIDEO_VGA32_H
#define VIDEO_VGA32_H

#ifdef VIDEO_BACKEND_VGA32

#include "vga_driver.h"

// Clase Video compatible con la interfaz original
// pero que usa el driver VGA internamente
class Video {
public:
  Video() {}

  void begin(void) {
    tft_init();
  }

  void write(uint16_t *colors, uint32_t len) {
    // No se usa en VGA32 — tft_write_pixels() se llama directamente
    // desde updateAudioVideo() en main.cpp
    (void)colors; (void)len;
  }

  // flip/flipReset: en VGA la rotación es siempre la misma (90°)
  // estos métodos son no-op
  void flip(char flipY, char flipX) {
    (void)flipY; (void)flipX;
  }

  void flipReset(char flipY, char flipX) {
    (void)flipY; (void)flipX;
  }
};

#endif // VIDEO_BACKEND_VGA32
#endif // VIDEO_VGA32_H
