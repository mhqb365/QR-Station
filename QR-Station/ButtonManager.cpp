#include "ButtonManager.h"

ButtonManager buttonManager;

void ButtonManager::begin() {
  uint8_t pins[] = {BUTTON1, BUTTON2, BUTTON3, BUTTON4};
  for(int i=0; i<4; i++) {
    buttons[i].pin = pins[i];
    pinMode(buttons[i].pin, INPUT_PULLUP);
    buttons[i].pressed = false;
    buttons[i].pressTime = 0;
    buttons[i].handled = false;
    buttons[i].clickCount = 0;
    buttons[i].releaseTime = 0;
  }
}

void ButtonManager::update() {
  for(int i=0; i<4; i++) {
    handleButton(i);
  }
}

void ButtonManager::handleButton(int i) {
  int reading = digitalRead(buttons[i].pin);
  unsigned long now = millis();

  if (reading == LOW) { // Button Pressed
    if (!buttons[i].pressed) {
      buttons[i].pressed = true;
      buttons[i].pressTime = now;
      buttons[i].handled = false;
    } else {
      // Check for long press (e.g. > 2000ms)
      if (!buttons[i].handled && (now - buttons[i].pressTime > 2000)) {
        if (buttons[i].onLongPress) {
            buttons[i].onLongPress();
        }
        buttons[i].handled = true; // Mark as handled so we don't trigger click on release
      }
    }
  } else { // Button Released
    if (buttons[i].pressed) {
        // Just released
        if (!buttons[i].handled) {
            // It was a short press
            // Logic for double click
            // If we have double click handler, we need to wait a bit
            if (buttons[i].onDoublePress) {
                if (now - buttons[i].releaseTime < 400 && buttons[i].clickCount > 0) {
                     buttons[i].onDoublePress();
                     buttons[i].clickCount = 0;
                } else {
                    buttons[i].clickCount++;
                    buttons[i].releaseTime = now;
                }
            } else {
                // Simple click
                if (buttons[i].onClick) buttons[i].onClick();
            }
        }
        buttons[i].pressed = false;
        buttons[i].handled = false;
    } else {
        // Idle state, check for delayed single click if double click is enabled
        if (buttons[i].onDoublePress && buttons[i].clickCount > 0 && (now - buttons[i].releaseTime >= 400)) {
            if (buttons[i].onClick) buttons[i].onClick();
            buttons[i].clickCount = 0;
        }
    }
  }
}

void ButtonManager::setClickHandler(int btnIndex, ButtonCallback cb) {
    if(btnIndex >=0 && btnIndex < 4) buttons[btnIndex].onClick = cb;
}

void ButtonManager::setLongPressHandler(int btnIndex, ButtonCallback cb) {
    if(btnIndex >=0 && btnIndex < 4) buttons[btnIndex].onLongPress = cb;
}

void ButtonManager::setDoubleClickHandler(int btnIndex, ButtonCallback cb) {
    if(btnIndex >=0 && btnIndex < 4) buttons[btnIndex].onDoublePress = cb;
}

void ButtonManager::setHandled(int btnIndex) {
    if(btnIndex >=0 && btnIndex < 4) buttons[btnIndex].handled = true;
}
