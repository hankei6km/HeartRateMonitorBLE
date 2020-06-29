#include <M5StickC.h>
#include "DrawGadget.h"
#include "DrawRate.h"

void DrawRate::_draw(PushImage pushImage)
{
    if (_rate > -1)
    {
        // _sprite.fillSprite(TFT_DARKGREY);
        fillRoundRect(0, 0, DrawGadget::_width, DrawGadget::_height, 4, TFT_DARKGREEN);
        setTextFont(4);
        drawNumber(_rate, _dx, _dy);
    }
    else
    {
        // _sprite.fillSprite(TFT_RED);
        fillRoundRect(0, 0, DrawGadget::_width, DrawGadget::_height, 4, TFT_MAROON);
        // TODO: y は font の高さから算出.
        fillRect(0, _dy + 8, _progPos, 1, TFT_LIGHTGREY);
        setTextFont(1);
        drawString("C N T", _dx, _dy);
    }
    // _sprite.pushSprite(_x, _y);
    pushImage(_x, _y, DrawGadget::_width, DrawGadget::_height, (uint16_t *)TFT_eSprite::frameBuffer(1));
}

void DrawRate::progress(int32_t val)
{
    int16_t p = (_progWidth * val) / 100;
    if (_progPos != p)
    {
        _update();
    }
    _progPos = p;
}

void DrawRate::rate(int16_t val)
{
    if (_rate != val)
    {
        _update();
    }
    _rate = val;
}
