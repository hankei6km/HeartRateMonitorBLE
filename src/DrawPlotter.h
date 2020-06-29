#ifndef __DRAW_PLOTTER_H__
#define __DRAW_PLOTTER_H__

#include <M5StickC.h>
#include <SimplePlotter.h>
#include "DrawGadget.h"

class DrawPlotter : public DrawGadget, public SimplePlotter
{
private:
    void _draw(PushImage pushImage);

public:
    DrawPlotter(TFT_eSPI *tft, int16_t x, int16_t y, int16_t width, int16_t height) : DrawGadget(x, y, width, height), SimplePlotter(tft)
    {
        createSprite(width, height);
        fillPlotter(TFT_DARKCYAN);
        setPixelColor(TFT_WHITE);
    };
    bool plot(int16_t val);
};

#endif
