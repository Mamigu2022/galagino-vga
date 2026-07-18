// vga_driver.h — Driver VGA para TTGO VGA32 (FabGL)
// Port de Galagino PlatformIO → TTGO VGA32

#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#ifdef VIDEO_BACKEND_VGA32

#include <Arduino.h>
#include <FabGL.h>

// Punteros globales
extern fabgl::VGAController* VGACtrl;
//extern fabgl::Canvas*        VGACanvas;

#ifndef GAME_WIDTH
  #define GAME_WIDTH  224
#endif
#ifndef GAME_HEIGHT
  #define GAME_HEIGHT 288
#endif

// Resolución VGA — ajustar según monitor
// QVGA_320x240_60Hz: 320×240, cabe el juego rotado (224px alto)
#ifndef VGA_RESOLUTION
  #define VGA_RESOLUTION QVGA_320x240_60Hz
#endif

void tft_init(void);
void tft_start_frame(void);
void tft_write_pixels(int x_start, int y_game, const uint16_t* pixels, int count);
void tft_end_frame(void);
//void tft_fill_rect(int x, int y, int w, int h, uint16_t color565);

static inline uint16_t rgb_to_565(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

#endif // VIDEO_BACKEND_VGA32
#endif // VGA_DRIVER_H