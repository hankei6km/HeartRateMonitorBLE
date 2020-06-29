#include <M5StickC.h>
#include "DrawGadget.h"

void DrawGadget::_update()
{
    _updated = true;
}

bool DrawGadget::updated()
{
    return _updated;
}

void DrawGadget::draw(TFT_eSPI &tft, bool force)
{
    if (force)
    {
        _draw([&tft](int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *d) {
            tft.pushImage(x, y, w, h, d);
        });
    }
    else if (updated())
    {
        _draw([&tft](int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *d) {
            tft.pushImage(x, y, w, h, d);
        });
        _updated = false;
    }
}

void DrawGadget::draw(TFT_eSprite &sprite, bool force)
{
    if (force)
    {
        _draw([&sprite](int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *d) {
            sprite.pushImage(x, y, w, h, d);
        });
    }
    else if (updated())
    {
        _draw([&sprite](int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *d) {
            sprite.pushImage(x, y, w, h, d);
        });
        _updated = false;
    }
}