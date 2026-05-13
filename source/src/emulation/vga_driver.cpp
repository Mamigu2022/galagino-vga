#include "config.h"

#ifdef VIDEO_BACKEND_VGA32

#include "vga_driver.h"
#include <Arduino.h>
#include <FabGL.h>

// Definiciones globales
fabgl::VGAController* VGACtrl   = nullptr;

// IMPORTANTE: Cambiado a fabgl::RGB222* para que coincida con los punteros
static fabgl::RGB222* s_framebuf = nullptr;

static bool _frame_active = false;
static int  _vga_w        = 0;
static int  _vga_h        = 0;
static int  _margin_x     = 0;
static int  _margin_y     = 0;
static int  _draw_row_min = 0;
static int  _draw_row_max = 0;
static int  _draw_col_min = 0;
static int  _draw_col_max = 0;

// Función de conversión rápida
static inline fabgl::RGB222 rgb565_to_222(uint16_t p) {
  // __builtin_bswap16 es una instrucción directa del procesador
  uint16_t swapped = __builtin_bswap16(p);
  fabgl::RGB222 color;
  
  // Ahora extraemos los 2 bits más altos de cada canal del dato ya girado
  color.R = (swapped >> 14) & 0x03;
  color.G = (swapped >> 9)  & 0x03;
  color.B = (swapped >> 3)  & 0x03;
  return color;
}

void tft_init(void) {
  Serial.println("[VGA32] Iniciando sistema optimizado...");

  // Tamaño: 224 * 288 = 64512 bytes
  size_t buf_size = GAME_WIDTH * GAME_HEIGHT * sizeof(fabgl::RGB222);

  // Intentar alojar en RAM interna (mucho más rápida que PSRAM)
  s_framebuf = (fabgl::RGB222*) heap_caps_malloc(buf_size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  
  if (!s_framebuf) {
    Serial.println("[VGA32] RAM interna insuficiente, usando PSRAM...");
    s_framebuf = (fabgl::RGB222*) ps_malloc(buf_size);
  }

  if (s_framebuf) {
    memset(s_framebuf, 0, buf_size);
  }

  VGACtrl = new fabgl::VGAController();
  // Configuración de pines estándar VGA32 (ajústalos si tu placa es distinta)
  VGACtrl->begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_23, GPIO_NUM_15);
  
  // OPCIÓN CRÍTICA PARA VELOCIDAD: 
  // Si usas 640x480 el ESP32 no tendrá ciclos para el juego.
  // Usa 320x240 o 320x200 para que Galaga vaya fluido.
  VGACtrl->setResolution(QVGA_320x240_60Hz); 

  _vga_w = VGACtrl->getScreenWidth();
  _vga_h = VGACtrl->getScreenHeight();

  // Centrado de imagen
  _margin_x = (_vga_w - GAME_HEIGHT) / 2;
  _margin_y = (_vga_h - GAME_WIDTH)  / 2;

  _draw_col_min = max(0, -_margin_x);
  _draw_col_max = min(GAME_HEIGHT, _vga_w - _margin_x) - 1;
  _draw_row_min = max(0, -_margin_y);
  _draw_row_max = min(GAME_WIDTH,  _vga_h - _margin_y) - 1;

  fabgl::Canvas cv(VGACtrl);
  cv.setBrushColor(fabgl::Color::Black);
  cv.clear();

  Serial.println("[VGA32] Inicialización completada.");
}

void tft_start_frame(void) {
  _frame_active = true;
}

void tft_write_pixels(int x_start, int y_game, const uint16_t* pixels, int count) {
  if (!_frame_active || !s_framebuf) return;
  if (y_game < 0 || y_game >= GAME_HEIGHT) return;

  int col = y_game;
  for (int i = 0; i < count; i++) {
    int x = x_start + i;
    if (x < 0 || x >= GAME_WIDTH) continue;
    int row = GAME_WIDTH - 1 - x;
    
    // ASIGNACIÓN DIRECTA: Ya no hay error porque ambos son fabgl::RGB222
    s_framebuf[row * GAME_HEIGHT + col] = rgb565_to_222(pixels[i]);
  }
}

void tft_end_frame(void) {
  if (!_frame_active || !VGACtrl || !s_framebuf) return;
  _frame_active = false;

  // Volcado ultra-rápido por líneas
  int x_vga_start = _margin_x + _draw_col_min;
  int x_vga_end   = _margin_x + _draw_col_max;

  for (int row = _draw_row_min; row <= _draw_row_max; row++) {
    int y_vga = _margin_y + row;
    fabgl::RGB222* line_ptr = &s_framebuf[row * GAME_HEIGHT + _draw_col_min];
    
    // Volcamos la línea completa de una vez
    VGACtrl->writeScreen(fabgl::Rect(x_vga_start, y_vga, x_vga_end, y_vga), line_ptr);
  }
}

void tft_fill_rect(int x, int y, int w, int h, uint16_t color565) {
  if (!VGACanvas) return;
  
  fabgl::RGB222 c = rgb565_to_222(color565);
  
  // Para el Canvas necesitamos RGB888, así que expandimos los 2 bits a 8 (<< 6)
  VGACanvas->setBrushColor(fabgl::RGB888(c.R << 6, c.G << 6, c.B << 6));
  VGACanvas->fillRectangle(x, y, x + w - 1, y + h - 1);
}
#endif