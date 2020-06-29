#ifndef __DRAW_RATE_H__
#define __DRAW_RATE_H__

// #include <string>
#include <M5StickC.h>
#include "DrawGadget.h"

class DrawRate : public DrawGadget, public TFT_eSprite
{
private:
    int16_t _rate = -1;
    int32_t _dx;
    int32_t _dy;
    String _progChr = "---";
    int32_t _progPos = 0;
    int32_t _progWidth;
    void _draw(PushImage pushImage);

public:
    DrawRate(TFT_eSPI *tft, int16_t x, int16_t y, int16_t width, int16_t height) : DrawGadget(x, y, width, height), TFT_eSprite(tft)
    {
        createSprite(width, height);
        setTextColor(TFT_WHITE);
        setTextDatum(MC_DATUM);
        _dx = width / 2;
        _dy = height / 2;
        _progWidth = width - 0;
    };
    bool updated();
    void progress(int32_t val);
    void rate(int16_t val);
};

#endif
