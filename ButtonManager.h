#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "Config.h"

// Define callback types
typedef std::function<void()> ButtonCallback;

struct ButtonConfig {
    uint8_t pin;
    unsigned long pressTime;
    bool pressed;
    bool handled;
    
    // Callbacks
    ButtonCallback onClick;
    ButtonCallback onLongPress;
    ButtonCallback onDoublePress;
    
    unsigned long releaseTime;
    int clickCount;
};

class ButtonManager {
public:
  void begin();
  void update();
  
  void setClickHandler(int btnIndex, ButtonCallback cb);
  void setLongPressHandler(int btnIndex, ButtonCallback cb);
  void setDoubleClickHandler(int btnIndex, ButtonCallback cb);

private:
  ButtonConfig buttons[4]; // 0=K1, 1=K2, 2=K3, 3=K4
  
  void handleButton(int index);
};

extern ButtonManager buttonManager;

#endif
