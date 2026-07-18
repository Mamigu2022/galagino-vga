#include "input.h"

// PS/2 solo en entornos sin BT
#ifndef BT_GAMEPAD_INPUT
  #ifndef BT_GAMEPAD_INPUT_BLUEPAD
    #include "ps2_input.h"
  #endif
#endif

#ifdef BT_GAMEPAD_INPUT
  #include "ble_gamepad.h"
#endif

#ifdef BT_GAMEPAD_INPUT_BLUEPAD
  extern "C" unsigned char bt_input_get_state(void);
  extern "C" void          bt_input_setup(void);
#endif

void Input::init(char SingleMachine) {
  singleMachine = SingleMachine;
  virtual_coin_state = 0;
#ifdef MCP23017_INPUT
  mcp.setup();
#elif defined(NUNCHUCK_INPUT)
  nunchuck.setup();
#elif defined(BT_GAMEPAD_INPUT_BLUEPAD)
  bt_input_setup();
#elif defined(BT_GAMEPAD_INPUT)
  ble_gamepad_init();
#else
  pinMode(BTN_START_PIN, INPUT_PULLUP);
  #ifdef BTN_COIN_PIN
    pinMode(BTN_COIN_PIN, INPUT_PULLUP);
  #endif
  pinMode(BTN_LEFT_PIN,  INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN,  INPUT_PULLUP);
  pinMode(BTN_UP_PIN,    INPUT_PULLUP);
  pinMode(BTN_FIRE_PIN,  INPUT_PULLUP);
#endif

  char inputs = buttons_get();
  if (inputs & BUTTON_FIRE) {
    printf("Demo Sounds switched off\n");
    switchDemoSoundsOff = 1;
    firePressedAtStart = 1;
  }
}

char Input::demoSoundsOff() {
  return switchDemoSoundsOff;
}

unsigned char Input::buttons_get(void) {
  unsigned char input_states = 0;

#ifdef MCP23017_INPUT
  input_states = mcp.getInput();
#elif defined(NUNCHUCK_INPUT)
  input_states = nunchuck.getInput();
#elif defined(BT_GAMEPAD_INPUT_BLUEPAD)
  input_states = bt_input_get_state();
#elif defined(BT_GAMEPAD_INPUT)

  input_states = ble_gamepad_buttons();
  if(input_states==BUTTON_COIN) virtual_coin_state = 1;
#else
  #ifdef BTN_COIN_PIN
    input_states = (ps2_key_coin() ? BUTTON_EXTRA : 0);
  #else
    input_states = (!digitalRead(BTN_START_PIN)) ? BUTTON_EXTRA : 0;
  #endif
  input_states |=
  #ifdef KEYBOARD
    (ps2_key_left()  ? BUTTON_LEFT  : 0) |
    (ps2_key_right() ? BUTTON_RIGHT : 0) |
    (ps2_key_up()    ? BUTTON_UP    : 0) |
    (ps2_key_down()  ? BUTTON_DOWN  : 0) |
    (ps2_key_fire()  ? BUTTON_FIRE  : 0);
    ps2_input_update();
  #else
    (digitalRead(BTN_LEFT_PIN)  ? 0 : BUTTON_LEFT)  |
    (digitalRead(BTN_RIGHT_PIN) ? 0 : BUTTON_RIGHT) |
    (digitalRead(BTN_UP_PIN)    ? 0 : BUTTON_UP)    |
    (digitalRead(BTN_DOWN_PIN)  ? 0 : BUTTON_DOWN)  |
    (digitalRead(BTN_FIRE_PIN)  ? 0 : BUTTON_FIRE);
  #endif
#endif

  bool extraPhysicalPressed = (input_states & BUTTON_EXTRA);
  unsigned char startAndCoinState = 0;

  switch(virtual_coin_state) {
    case 0:
      if(extraPhysicalPressed) {
        virtual_coin_state = 1;
        virtual_coin_timer = millis();
      }
      break;
    case 1:
      if(millis() - virtual_coin_timer > 100) {
        virtual_coin_state = 2;
        virtual_coin_timer = millis();
      }
      break;
    case 2:
      if(millis() - virtual_coin_timer > 500) {
        virtual_coin_state = 3;
        virtual_coin_timer = millis();
      }
      break;
    case 3:
      if(millis() - virtual_coin_timer > 100) {
        virtual_coin_state = 4;
        virtual_coin_timer = millis();
      }
      break;
    case 4:
      if(!extraPhysicalPressed)
        virtual_coin_state = 0;
      break;
  }

  startAndCoinState = ((virtual_coin_state != 1) ? 0 : BUTTON_COIN) |
                      (((virtual_coin_state != 3) && (virtual_coin_state != 4)) ? 0 : BUTTON_START);

#ifdef MCP23017_INPUT
  startAndCoinState |= (input_states & (BUTTON_START | BUTTON_COIN));
#elif !defined(BT_GAMEPAD_INPUT) && !defined(BT_GAMEPAD_INPUT_BLUEPAD) && \
      !defined(NUNCHUCK_INPUT) && defined(BTN_COIN_PIN)
  startAndCoinState |=
  #ifdef KEYBOARD
    (ps2_key_start() ? BUTTON_START : 0) |
    (ps2_key_coin()  ? BUTTON_COIN  : 0) |
  #endif
    (digitalRead(BTN_START_PIN) ? BUTTON_START : 0) |
    (digitalRead(BTN_COIN_PIN)  ? BUTTON_COIN  : 0);
#endif

  input_states &= ~(BUTTON_START | BUTTON_COIN);

  if (extraPhysicalPressed && _volume_callback) {
    _volume_callback(input_states & BUTTON_UP, input_states & BUTTON_DOWN);
    if ((input_states & BUTTON_UP) | (input_states & BUTTON_DOWN))
      reset_timer = 0;
  }

  if (!singleMachine) {
    bool buttonExtraRisingEdge = extraPhysicalPressed && !(input_states_last & BUTTON_EXTRA);
    bool buttonUpRisingEdge    = (input_states & BUTTON_UP)   && !(input_states_last & BUTTON_UP);
    bool buttonDownRisingEdge  = (input_states & BUTTON_DOWN) && !(input_states_last & BUTTON_DOWN);

    if (buttonUpRisingEdge | buttonDownRisingEdge | buttonExtraRisingEdge) {
      if (_doAttractReset_callback)
        _doAttractReset_callback();
    }

    if(extraPhysicalPressed) {
      if(!reset_timer)
        reset_timer = millis();
      if(millis() - reset_timer > 3000) {
        reset_timer = millis();
        if (_doReset_callback)
          _doReset_callback();
      }
    } else {
      reset_timer = 0;
    }

    input_states_last = (input_states & ~BUTTON_EXTRA) | (extraPhysicalPressed ? BUTTON_EXTRA : 0);
  }

  if (firePressedAtStart && input_states & BUTTON_FIRE) {
    printf("Wait for release fire button...\n");
    input_states = 0;
  } else {
    firePressedAtStart = false;
  }

  return (input_states & ~BUTTON_EXTRA) | startAndCoinState;
}

Input &Input::onVolumeUpDown(THandlerVolume fn) {
  _volume_callback = fn;
  return *this;
}

Input &Input::onDoReset(THandlerDoReset fn) {
  _doReset_callback = fn;
  return *this;
}

Input &Input::onDoAttractReset(THandlerDoAttractReset fn) {
  _doAttractReset_callback = fn;
  return *this;
}