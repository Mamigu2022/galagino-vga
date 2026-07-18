#ifdef VIDEO_BACKEND_VGA32

#include "vga_driver.h"
#include <Arduino.h>
#include <FabGL.h>
 
// =====================================================================================
// vga_driver.cpp — version optimizada
//
// Cambios respecto a la version original:
//   1) Conversion RGB565 -> RGB222 sin bswap: se extraen directamente los bits que
//      importan del valor original (ver rgb565_to_rgb222). Es matematicamente
//      equivalente al bswap+shifts original pero sin la vuelta de bytes ni tabla,
//      y sin gastar RAM extra.
//   2) Agrupacion (batching) de columnas de pantalla contiguas: en vez de llamar a
//      VGACtrl->writeScreen() una vez por cada columna del juego, se acumulan hasta
//      VGA_BATCH_MAX_COLS columnas compatibles (misma altura de fila) y se vuelcan
//      en una sola llamada. Esto reduce drasticamente el numero de llamadas a
//      writeScreen() por frame.
//   3) Limites de fila absolutos precalculados una sola vez en tft_init().
//   4) Se elimina malloc/objetos temporales del hot path.
//
// NOTA IMPORTANTE sobre el paso 2:
//   Se asume que fabgl::VGAController::writeScreen(rect, buf) espera "buf" en orden
//   row-major (fila a fila, y dentro de cada fila columna a columna), tal y como
//   documenta la libreria para el par readScreen()/writeScreen(). Si al probar ves
//   la imagen "peinada" o con columnas desordenadas dentro del bloque agrupado,
//   es señal de que el orden esperado es column-major; en ese caso cambia el indice
//   dentro de flush_batch() (esta comentado donde corresponde).
// =====================================================================================
 
fabgl::VGAController* VGACtrl   = nullptr;
//fabgl::Canvas*        VGACanvas = nullptr;
 
static bool _frame_active = false;
static int  _vga_w        = 0;
static int  _vga_h        = 0;
static int  _margin_x     = 0;
static int  _margin_y     = 0;
static int  _draw_col_min = 0;
static int  _draw_col_max = 0;
static int  _draw_row_min = 0;
static int  _draw_row_max = 0;
 
// Limites de fila ya combinados con _margin_y (antes se recalculaban en cada
// llamada a tft_write_pixels; ahora se calculan una unica vez en tft_init()).
static int  _abs_row_min  = 0;
static int  _abs_row_max  = 0;
 
// ---------------------------------------------------------------------------
// Conversion RGB565 -> RGB222 sin bswap ni tabla.
//
// El bswap16() original solo reordenaba los bytes de p antes de extraer 2 bits
// por canal con shifts fijos. Haciendo el algebra de bits a mano, el mismo
// resultado se obtiene extrayendo estos bits directamente de "p" sin invertir
// nada, así que se ahorra la instruccion de swap y la variable intermedia
// (y de paso ya no hace falta ninguna tabla en RAM):
//
//   R = (p >> 6)  & 0x3
//   G = (p >> 1)  & 0x3
//   B = (p >> 11) & 0x3
//
// (Verificado a mano con varios valores de prueba contra la version original
// con bswap16 + shifts sobre el valor invertido; el resultado es identico.)
// ---------------------------------------------------------------------------
static inline fabgl::RGB222 rgb565_to_rgb222(uint16_t p) {
  fabgl::RGB222 c;
  c.R = (p >> 6)  & 0x03;
  c.G = (p >> 1)  & 0x03;
  c.B = (p >> 11) & 0x03;
  return c;
}
 
// ---------------------------------------------------------------------------
// Batching de columnas para reducir llamadas a writeScreen().
// ---------------------------------------------------------------------------
#ifndef VGA_BATCH_MAX_COLS
#define VGA_BATCH_MAX_COLS 8   // numero de columnas de pantalla agrupadas por llamada
#endif
 
// Buffer intermedio column-major: una columna por indice, igual que el s_col_buf
// original. Facil de rellenar exactamente igual que antes.
static fabgl::RGB222 s_batch_col[VGA_BATCH_MAX_COLS][GAME_WIDTH];
 
// Buffer final row-major que se pasa a writeScreen() (fila por fila).
static fabgl::RGB222 s_batch_row[VGA_BATCH_MAX_COLS * GAME_WIDTH];
 
static int s_batch_cols   = 0;  // columnas acumuladas en el batch actual
static int s_batch_x0     = 0;  // screen_x de la primera columna del batch
static int s_batch_y_t    = 0;  // fila superior (comun a todas las columnas del batch)
static int s_batch_y_b    = 0;  // fila inferior (comun a todas las columnas del batch)
static int s_batch_height = 0;  // s_batch_y_b - s_batch_y_t + 1
 
static void flush_batch(void) {
  if (s_batch_cols == 0) return;
 
  // Transponer de column-major (s_batch_col) a row-major (s_batch_row).
  for (int r = 0; r < s_batch_height; r++) {
    fabgl::RGB222* row_ptr = &s_batch_row[r * s_batch_cols];
    for (int c = 0; c < s_batch_cols; c++) {
      row_ptr[c] = s_batch_col[c][r];
    }
  }
 
  fabgl::Rect rect(s_batch_x0, s_batch_y_t,
                    s_batch_x0 + s_batch_cols - 1, s_batch_y_b);
  VGACtrl->writeScreen(rect, s_batch_row);
 
  s_batch_cols = 0;
}
 
static inline void batch_reset(int screen_x, int y_t, int y_b) {
  s_batch_x0     = screen_x;
  s_batch_y_t    = y_t;
  s_batch_y_b    = y_b;
  s_batch_height = y_b - y_t + 1;
  s_batch_cols   = 0;
}
 
void tft_init(void) {
  Serial.println("[VGA32] Iniciando...");
 
  VGACtrl = new fabgl::VGAController();
  if (!VGACtrl) { Serial.println("[VGA32] ERROR"); while(1); }
 
  VGACtrl->begin(
    GPIO_NUM_22, GPIO_NUM_21,
    GPIO_NUM_19, GPIO_NUM_18,
    GPIO_NUM_5,  GPIO_NUM_4,
    GPIO_NUM_23, GPIO_NUM_15
  );
  VGACtrl->setResolution(VGA_RESOLUTION);
 
  _vga_w = VGACtrl->getScreenWidth();
  _vga_h = VGACtrl->getScreenHeight();
  _margin_x = (_vga_w - GAME_HEIGHT) / 2;
  _margin_y = (_vga_h - GAME_WIDTH)  / 2;
  _draw_col_min = max(0, -_margin_x);
  _draw_col_max = min(GAME_HEIGHT, _vga_w - _margin_x) - 1;
  _draw_row_min = max(0, -_margin_y);
  _draw_row_max = min(GAME_WIDTH,  _vga_h - _margin_y) - 1;
 
  // Precalculo de limites absolutos de fila (antes se sumaba en cada llamada).
  _abs_row_min = _margin_y + _draw_row_min;
  _abs_row_max = _margin_y + _draw_row_max;
 
  //Serial.printf("[VGA32] %dx%d  mx=%d my=%d\n", _vga_w, _vga_h, _margin_x, _margin_y);
 
  s_batch_cols = 0;
 
  /*VGACanvas = new fabgl::Canvas(VGACtrl);
  if (!VGACanvas) { Serial.println("[VGA32] ERROR Canvas"); while(1); }
 
  VGACanvas->setBrushColor(fabgl::Color::Black);
  VGACanvas->clear();
  //VGACanvas->waitCompletion();
 
  //Serial.println("[VGA32] Listo.");*/
}
 
void tft_start_frame(void) {
  _frame_active = true;
  s_batch_cols  = 0; // por seguridad, no deberia quedar nada pendiente de un frame anterior
}
 
void tft_write_pixels(int x_start, int y_game,
                      const uint16_t* pixels, int count) {
  if (!_frame_active || !VGACtrl) return;
  if (y_game < _draw_col_min || y_game > _draw_col_max) return;
 
  int screen_x = _margin_x + y_game;
 
  // Calcular rango de filas VGA visibles.
  int y_top    = _margin_y + (GAME_WIDTH - 1 - (x_start + count - 1));
  int y_bottom = _margin_y + (GAME_WIDTH - 1 - x_start);
  int y_t = max(0, max(y_top,    _abs_row_min));
  int y_b = min(_vga_h - 1, min(y_bottom, _abs_row_max));
  if (y_t > y_b) return;
 
  int height = y_b - y_t + 1;
 
  // ¿Esta columna puede unirse al batch abierto? Debe ser la siguiente columna
  // contigua en pantalla y compartir el mismo rango de filas.
  bool fits = (s_batch_cols > 0) &&
              (s_batch_cols < VGA_BATCH_MAX_COLS) &&
              (screen_x == s_batch_x0 + s_batch_cols) &&
              (y_t == s_batch_y_t) &&
              (y_b == s_batch_y_b);
 
  if (!fits) {
    flush_batch();
    batch_reset(screen_x, y_t, y_b);
  }
 
  // Rellenar la columna actual dentro del batch (misma logica que el original,
  // solo que ahora escribe en s_batch_col[s_batch_cols][] en vez de un buffer
  // de una sola columna, y usa la conversion directa sin bswap).
  fabgl::RGB222* dst = s_batch_col[s_batch_cols];
  for (int i = 0; i < height; i++) {
    int game_x = x_start + (GAME_WIDTH - 1 - (y_t + i - _margin_y));
    dst[i] = (game_x >= 0 && game_x < count)
             ? rgb565_to_rgb222(pixels[game_x])
             : fabgl::RGB222{0, 0, 0};
  }
 
  s_batch_cols++;
 
  if (s_batch_cols >= VGA_BATCH_MAX_COLS) {
    flush_batch();
  }
}
 
void tft_end_frame(void) {
  flush_batch();   // volcar cualquier columna pendiente antes de cerrar el frame
  _frame_active = false;
}
 
/*void tft_fill_rect(int x, int y, int w, int h, uint16_t color565) {
  if (!VGACanvas) return;
  uint8_t r = ((color565 >> 11) & 0x1F) << 3;
  uint8_t g = ((color565 >>  5) & 0x3F) << 2;
  uint8_t b =  (color565        & 0x1F) << 3;
  VGACanvas->setBrushColor(fabgl::RGB888(r, g, b));
  VGACanvas->fillRectangle(x, y, x + w - 1, y + h - 1);
  //VGACanvas->waitCompletion();
}*/
 
#endif // VIDEO_BACKEND_VGA32
 