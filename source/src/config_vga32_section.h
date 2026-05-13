// config_vga32_section.h
// Añadir este bloque al final de config.h del proyecto,
// ANTES del #endif final:

#ifdef VIDEO_BACKEND_VGA32
  // Resolución VGA (cambiar según monitor)
  // VGA_320x240_60Hz — recomendado (cabe el juego rotado sin recorte)
  // VGA_640x480_60Hz — más calidad pero más lento
  #define VGA_RESOLUTION  VGA_320x240_60Hz

  // Dimensiones nativas del juego
  #define GAME_WIDTH   224
  #define GAME_HEIGHT  288

  // Pines botones libres en TTGO VGA32
  // (los pines TFT no se usan, liberan GPIOs)
  #undef  BTN_LEFT_PIN
  #undef  BTN_RIGHT_PIN
  #undef  BTN_DOWN_PIN
  #undef  BTN_UP_PIN
  #undef  BTN_FIRE_PIN
  #define BTN_LEFT_PIN   12
  #define BTN_RIGHT_PIN  13
  #define BTN_DOWN_PIN   39
  #define BTN_UP_PIN      2
  #define BTN_FIRE_PIN   36
  #define BTN_START_PIN  34
  #define BTN_COIN_PIN   35

  // Audio: GPIO25 via I2S (misma config que original)
  // Si hay conflicto con FabGL, la tarea audio_task lo gestiona
  #undef  SND_LEFT_CHANNEL
  #define SND_RIGHT_CHANNEL  // GPIO25 = canal derecho I2S0

  // Sin LED en VGA32 (pines ocupados)
  #undef LED_PIN

  // TFT_SPICLK debe estar definido para compilar video.cpp
  // aunque no se use — definir un valor dummy
  #ifndef TFT_SPICLK
    #define TFT_SPICLK 40000000
  #endif
#endif
