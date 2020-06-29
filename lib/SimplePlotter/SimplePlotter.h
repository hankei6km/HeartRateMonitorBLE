#ifndef __SIMPLE_PLOTTER_H__
#define __SIMPLE_PLOTTER_H__

#include <list>
#include <set>
#include <tuple>
#include "M5StickC.h"
#include "SimplePlotterItems.h"

class SimplePlotter : public TFT_eSprite
{
public:
    SimplePlotter(TFT_eSPI *tft) : TFT_eSprite(tft){};
    // SimplePlotter() = default;

    void *createSprite(int16_t w, int16_t h, uint8_t frames = 1);
    void fillPlotter(uint32_t color);

    void setPixelColor(uint32_t color);

    bool plot(int16_t val);
    std::tuple<int16_t, int16_t> range();

private:
    int16_t _width = 160;
    int16_t _height = 80;
    int16_t _labelWidth = 30;
    uint32_t _plotterColor = TFT_DARKCYAN;
    uint32_t _pixelColor = TFT_RED;

    float _scale = 1.0;
    int16_t _range[2] = {0, 0};
    int16_t _scrollOffset = 1;

    PlotterItems<int16_t> _items;

    void _updateItemsLim();
    void _updateItems(int16_t val);
};

#endif
