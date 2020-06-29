#include <M5StickC.h>
#include "DrawBLEState.h"

void DrawBLEState::_draw(PushImage pushImage)
{
    uint32_t bgColor = TFT_NAVY;
    String lbl = "B L E";
    switch (_state)
    {
    case State::authenticate:
        bgColor = TFT_BLUE;
        lbl = "C O M";
        break;
    case State::connected:
        bgColor = TFT_PURPLE;
        lbl = "A T H";
        break;
    case State::disconnected:
        lbl = "B L E";
        break;
    }
    fillRoundRect(0, 0, DrawGadget::_width, DrawGadget::_height, 4, bgColor);
    drawString(lbl, _dx, _dy);
    // _sprite.pushSprite(_x, _y);
    pushImage(_x, _y, DrawGadget::_width, DrawGadget::_height, (uint16_t *)frameBuffer(1));
}

void DrawBLEState::enable()
{
    _enabled = true;
    _update();
}

void DrawBLEState::disable()
{
    _enabled = false;
    _update();
}

void DrawBLEState::state(State s)
{
    _state = s;
    _update();
}