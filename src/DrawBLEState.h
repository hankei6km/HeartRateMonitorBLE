#ifndef __DRAW_BLE_ICON_H__
#define __DRAW_BLE_ICON_H__

#include <M5StickC.h>
#include "DrawGadget.h"

class DrawBLEState : public DrawGadget, public TFT_eSprite
{
public:
    DrawBLEState(TFT_eSPI *tft, int16_t x, int16_t y, int16_t width, int16_t height) : DrawGadget(x, y, width, height), TFT_eSprite(tft)
    {
        createSprite(width, height);
        setTextFont(1);
        setTextColor(TFT_WHITE);
        setTextDatum(MC_DATUM);
        _dx = width / 2;
        _dy = height / 2;
    };
    enum State
    {
        disconnected,
        connected,
        authenticate
    };
    void enable();
    void disable();
    void state(State s);

private:
    bool _enabled = false;
    State _state = State::disconnected;
    int32_t _dx = 0;
    int32_t _dy = 0;
    void _draw(PushImage pushImage);
};

#endif
