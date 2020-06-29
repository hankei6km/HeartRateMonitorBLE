#ifdef ARDUINO // native test 時にコンパイルされないようにする(M5StickC.hが存在しない)
#include <list>
#include <tuple>
#include "M5StickC.h"

#include "SimplePlotter.h"

// SimplePlotter::SimplePlotter()
// {
//     // TFT_eSprite(&M5.Lcd);
// }

// SimplePlotter::TFT_eSprite(tft), SimplePlotter(TFT_eSPI *tft)
// {
//     // TFT_eSprite(tft);
// }

void *SimplePlotter::createSprite(int16_t w, int16_t h, uint8_t frames)
{
    _width = w;
    _height = h;
    _updateItemsLim();
    _range[1] = w;
    return TFT_eSprite::createSprite(w, h, frames);
}

void SimplePlotter::fillPlotter(uint32_t color)
{
    _plotterColor = color;
    TFT_eSprite::fillSprite(color);
}

// void SimplePlotter::fillSprite(uint32_t color)
// {
//     TFT_eSprite::fillSprite(color);
// }

void SimplePlotter::setPixelColor(uint32_t color)
{
    _pixelColor = color;
    _updateItemsLim();
}

bool SimplePlotter::plot(int16_t val)
{
    _updateItems(val);

    TFT_eSprite::fillRect(0, 0, _width, _height, _plotterColor);

    auto iteBegin = _items.begin();
    int16_t x = _width;
    for (auto ite = _items.end(); x >= 0 && ite != iteBegin; ite--, x = x - _scrollOffset)
    {
        auto prev = ite;
        prev--;
        const int16_t y1 = _height - ((*ite - _range[0]) * _scale);
        const int16_t y2 = _height - ((*prev - _range[0]) * _scale);
        TFT_eSprite::drawLine(x, y1, x - _scrollOffset, y2, _pixelColor);
    }

    return false;
}

std::tuple<int16_t, int16_t> SimplePlotter::range()
{
    return std::tuple<int16_t, int16_t>(_range[0], _range[1]);
}

void SimplePlotter::_updateItemsLim()
{
    _items.setLim(_width / _scrollOffset);
}

void SimplePlotter::_updateItems(int16_t val)
{
    _items.append(val);
    auto range = _items.range(_items.low(), _items.high(), _height);
    _range[0] = std::get<0>(range);
    _range[1] = std::get<1>(range);
    _scale = std::get<2>(range);
}

#endif