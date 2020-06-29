#include <M5StickC.h>
#include "DeepSleep.h"

DeepSleep::DeepSleep()
{
    _after = 5 * 1000;
    _at = millis() + _after;
}

void DeepSleep::setup()
{
    pinMode(GPIO_NUM_37, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
}

void DeepSleep::sleep(unsigned long after)
{
    _after = after;
    _at = millis() + _after;
}

void DeepSleep::sleep()
{
    Serial.println("fade out");
    M5.Axp.ScreenBreath(8);
    delay(50);
    M5.Axp.ScreenBreath(7);
    delay(30);
    M5.Axp.ScreenBreath(6);

    Serial.println("waiting btn was released");
    // M5.update();
    while (M5.BtnA.isPressed())
    {
        M5.update();
        delay(10);
    }
    Serial.println("deep sleep");

    // M5.Axp.ScreenBreath(0);
    M5.Axp.SetLDO2(false);
    //M5.Axp.SetLDO3(false); LDO3 については下記のブロックでOFF

    // https://lang-ship.com/blog/work/m5stickc-power-saving/
    // https://lang-ship.com/blog/work/m5stickc-axp192-power/
    // ADC_OFF
    {
        Wire1.beginTransmission(0x34);
        Wire1.write(0x82);
        Wire1.write(0x00);
        Wire1.endTransmission();
        Wire1.beginTransmission(0x34);
        Wire1.write(0x83);
        Wire1.write(0x00);
        Wire1.endTransmission();
    }
    // EXTEN_10_OFF
    // 手持ちの M5StickC(電源 OFF 時に5V OUT も OFF される)の場合、
    // これで 5V OUT も OFF になるもよう.
    {
        Wire1.beginTransmission(0x34);
        Wire1.write(0x10);
        Wire1.endTransmission();
        Wire1.requestFrom(0x34, 1);
        uint8_t state = Wire1.read() & ~(1 << 2);
        Wire1.beginTransmission(0x34);
        Wire1.write(0x10);
        Wire1.write(state);
        Wire1.endTransmission();
    }
    // EXTEN_OFF
    {
        Wire1.beginTransmission(0x34);
        Wire1.write(0x12);
        Wire1.endTransmission();
        Wire1.requestFrom(0x34, 1);
        uint8_t state = Wire1.read() & ~(1 << 6);
        Wire1.beginTransmission(0x34);
        Wire1.write(0x12);
        Wire1.write(state);
        Wire1.endTransmission();
    }
    // LDO3_OFF
    {
        Wire1.beginTransmission(0x34);
        Wire1.write(0x12);
        Wire1.endTransmission();
        Wire1.requestFrom(0x34, 1);
        uint8_t state = Wire1.read() & ~(1 << 3);
        Wire1.beginTransmission(0x34);
        Wire1.write(0x12);
        Wire1.write(state);
        Wire1.endTransmission();
    }

    esp_deep_sleep_start();
}

void DeepSleep::bump()
{
    _at = millis() + _after;
}

void DeepSleep::tick()
{
    if (millis() > _at || M5.BtnA.pressedFor(1000 * 3))
    {
        sleep();
    }
}