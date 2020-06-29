#include "DrawPlotter.h"

void DrawPlotter::_draw(PushImage pushImage)
{
    // _plotter.pushSprite(_x, _y);
    pushImage(_x, _y, DrawGadget::_width, DrawGadget::_height, (uint16_t *)frameBuffer(1));
}

bool DrawPlotter::plot(int16_t val)
{
    bool ret = SimplePlotter::plot(val);
    _update();
    return ret;
}