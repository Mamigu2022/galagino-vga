#ifndef _CONFIG_H_
#define _CONFIG_H_

// game config
#define MASTER_ATTRACT_MENU_TIMEOUT  20000      // start games while sitting idle in menu for 20 seconds, undefine to disable
#define MASTER_ATTRACT_GAME_TIMEOUT  60000 * 5  // restart after 5 minutes 
//#define VIDEO_BACKEND_VGA32
// video config
//#define TFT_SPICLK  40000000    // 40 Mhz. Some displays cope with 80 Mhz
//#define TFT_SPICLK	80000000    // 80 Mhz. Some displays cope with 80 Mhz

// max possible video rate:
// 8*224 pixels = 8*224*16 = 28672 bits
// 2790 char rows per sec at 40Mhz = max 38 fps
#if TFT_SPICLK < 80000000
  #define VIDEO_HALF_RATE
#endif

// x and y offset of 224x288 pixels inside the 240x320 screen
#define TFT_X_OFFSET      8
#define TFT_Y_OFFSET      16

// led config
//#define LED_PIN           1 // pin used for optional WS2812 stripe
//#define LED_BRIGHTNESS 	  5 // range 0..255, 5 for CYD Handheld and 50 for Arcade Cab.

// audio config
//#define SND_DIFF   	 // set to output differential audio on GPIO25 _and_ inverted on GPIO26
#define SND_LEFT_CHANNEL // Use GPIO 26 for audio

// esp32 model config
//#define CHEAP_YELLOW_DISPLAY_CONF

#ifdef CHEAP_YELLOW_DISPLAY_CONF
  #define TFT_CS          15
  #define TFT_DC          2
  #define TFT_RST         -1
  #define TFT_BL          21   // some CYD use 27
  #define TFT_BL_LEVEL    HIGH  // backlight on with low or high signal
  //#define TFT_ILI9341 // define for ili9341, otherwise st7789
  //#define TFT_VFLIP   // define for upside down

  #define TFT_MISO 	      12
  #define TFT_MOSI 	      13
  #define TFT_SCLK 	      14
  #define TFT_MAC  	    0x20  // some CYD need this to rotate properly and have correct colors

  // Use the MCP23017 for input on the CYD
  #ifndef MCP23017_INPUT
  #define MCP23017_INPUT
  #endif
  #define MCP23017_SDA 22
  #define MCP23017_SCL 27
  #define MCP23017_ADDR 0x20
  #define MCP23017_LEFT_PIN   0
  #define MCP23017_RIGHT_PIN  1
  #define MCP23017_UP_PIN     2
  #define MCP23017_DOWN_PIN   3
  #define MCP23017_FIRE_PIN   4
  #define MCP23017_EXTRA_PIN  5
  #define MCP23017_COIN_PIN   6
  #define MCP23017_START_PIN  7
#endif

#ifndef CHEAP_YELLOW_DISPLAY_CONF
  #define TFT_CS          -1
  #define TFT_DC          -1
  #define TFT_RST         -1
  #define TFT_BL          -1      // don't set if backlight is hard wired
  #define TFT_BL_LEVEL    LOW     // backlight on with low or high signal
  #define TFT_ILI9341             // define for ili9341, otherwise st7789
  //#define TFT_VFLIP               // define for upside down

  #define TFT_MISO 	      -1
  #define TFT_MOSI 	      -1
  #define TFT_SCLK 	      -1

  // Pins used for buttons
  //#define BTN_START_PIN   0
  //#define BTN_COIN_PIN    21      // if this is not defined, then start will act as coin & start

  #ifndef NUNCHUCK_INPUT
    #define BTN_LEFT_PIN   34
    #define BTN_RIGHT_PIN  35
    #define BTN_DOWN_PIN   27
    #define BTN_UP_PIN     12
    #define BTN_FIRE_PIN   32
  #else
    #define NUNCHUCK_SDA  33
    #define NUNCHUCK_SCL  32
    #define NUNCHUCK_MOVE_THRESHOLD 30 // This is the dead-zone for where minor movements on the stick will not be considered valid movements
  #endif
#endif
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
  #define BTN_COIN_PIN   0

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

#endif // _CONFIG_H_
