#ifndef __DRAW_GADGET_H__
#define __DRAW_GADGET_H__

#include <functional>
#include <M5StickC.h>

using PushImage = std::function<void(int32_t, int32_t, int32_t, int32_t, const uint16_t *)>;
class DrawGadget
{
private:
    bool _updated = true;
    // virtual void _draw(PushImage pushImage);
    virtual void _draw(PushImage pushImage){};

protected:
    int16_t _x;
    int16_t _y;
    int16_t _width;
    int16_t _height;
    void _update();

public:
    DrawGadget(int16_t x, int16_t y, int16_t width, int16_t height)
    {
        _x = x;
        _y = y;
        _width = width;
        _height = height;
    };
    bool updated();
    void draw(TFT_eSPI &tft, bool force = false);
    void draw(TFT_eSprite &sprite, bool force = false);
};
#endif
