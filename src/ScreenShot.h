#ifndef __SCREEN_SHOT_H__
#define __SCREEN_SHOT_H__

#include <M5StickC.h>
#include <SpriteToBmp.h>

class ScreenShot : public TFT_eSprite
{
    static void writeRec(const char *s)
    {
        Serial.printf("==== BMP:%s\n", s);
    };

public:
    ScreenShot(TFT_eSPI *tft) : TFT_eSprite(tft){};
    void writeBmpBase64()
    {
        SpriteToBmp::toBmpBase64(*this, [](uint8_t *b, size_t s) {
            if (b != NULL)
            {
                ScreenShot::writeRec((const char *)b);
            }
            else
            {
                ScreenShot::writeRec("---- cut ----");
            }
        });
    };
};

#endif
