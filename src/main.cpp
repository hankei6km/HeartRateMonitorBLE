#include <M5StickC.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// #include <nvs.h>
// #include <nvs_flash.h>

#include "movingAvg.h"

#include <HeartRate.h>
#include <PeakHandler.h>

#include "DeepSleep.h"
#include "BLEHeartRate.h"
#include "DrawGadget.h"
#include "DrawRate.h"
// #include "DrawBeatIcon.h"
#include "DrawBLEState.h"
#include "DrawPlotter.h"
#ifdef SCREEN_SHOT
#include "ScreenShot.h"
#endif

const int ANALOG_IN = 36;
const int LED_BUILTIN = 10;

unsigned long delayMS = 10; // 計測中(センサーの近くになにかある)で負荷があると loop 間隔は 20 から 30 ms になる。

// それなりに汎用
int16_t thrDist = 8;   // 波形ピーク検出、ピークから動的閾値までの距離。
movingAvg avgTemp(10); // およそ 25ms x 10 = 250ms 単位の平均となる(240bpm くらいまでは計測できる？)

// ワークアウト用
// int16_t thrDist = 5; // 運動中は波形の間隔は短く幅が狭くなる？ ノイズを拾いやすいが 1km ほどジョグしながらの計測でそれっぽい値は出る。
// movingAvg avgTemp(10); // およそ 25ms x 10 = 250ms 単位の平均となる(240bpm くらいまでは計測できる？)
//
// 160 bpm くらいでも良い用(波形が滑らかになるので計測しやすい)
// int16_t thrDist = 6;
// movingAvg avgTemp(15); // およそ 25ms x 15 = 300ms 単位の平均となる(160bpm くらいまでは計測できる？)

// 高 bpm にも対応する場合(波形の幅が狭いと計測できない)
// int16_t thrDist = 20;
// movingAvg avgTemp(5);

// テスト 2020-07-02 高 BPM 対応で閾値の距離を短く。平常時の計測だと細かい振れを拾ってしまう。
// int16_t thrDist = 5;
// movingAvg avgTemp(5);

HeartRate rate = HeartRate();
PeakHandler<int16_t> peak = PeakHandler<int16_t>();

DeepSleep deepSleep = DeepSleep();
BLEHeartRate bleHR = BLEHeartRate();

DrawRate drawRate = DrawRate(&M5.Lcd, 0, 0, 40, 38);
DrawBLEState drawBLEState = DrawBLEState(&M5.Lcd, 0, 42, 40, 38);
DrawPlotter drawPlotter = DrawPlotter(&M5.Lcd, 44, 0, 116, 80);

DrawGadget *gadgets[] = {&drawRate, &drawBLEState, &drawPlotter};
int16_t gadgetsLen = sizeof(gadgets) / sizeof(gadgets[0]);

#ifdef SCREEN_SHOT
ScreenShot screenShot = ScreenShot(&M5.Lcd);
#endif

// Preferences preferences;

void setup()
{
  M5.begin();

  // BLE で bondig したデバイスが NVS 保存されないような挙動をした
  // (bonding 後に M5Stick-C を deep sleep すると再度 PIN 入力を要求されるようになる).
  // 以下で回避できた。
  // なお Preferences::Clear() ではうまくいかなかった。
  // https://www.mgo-tec.com/blog-entry-trouble-shooting-esp32-wroom.html/2
  // int err;
  // err = nvs_flash_init();
  // Serial.printf("nvs_flash_init: %d\n", err);
  // err = nvs_flash_erase();
  // Serial.printf("nvs_flash_erase: %d\n", err);

  // これは常に -1 となる.
  // Serial.printf("bonded num: %d\n", esp_ble_get_bond_device_num());

  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED)
  {
    Serial.println("wakeup");
  }

  // power saving
  setCpuFrequencyMhz(80); // 80 -> BLE
  M5.Axp.ScreenBreath(9);

  // initialize board functions.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  analogReadResolution(12);

  // initialize ble.
  static std::string deviceName;
#if defined(BLE_DEVICE_NAME)
  deviceName = BLE_DEVICE_NAME;
#else
  deviceName = "hr_monitor";
#endif
  bleHR.init(deviceName);
  bleHR.setHeartRateNotifyInterval(1000 * 5);
  bleHR.onConnect([]() {
    deepSleep.sleep(1000 * 20); // connect 後はスリープしにくくする.
    Serial.println("-- connect");
    drawBLEState.state(DrawBLEState::State::connected);
  });
  bleHR.onPassKeyNotify([](uint32_t passKey) {
    Serial.printf("PIN(PassKey): %d\n", passKey);
  });
  bleHR.onAuthentication([]() {
    Serial.println("-- auth");
    drawBLEState.state(DrawBLEState::State::authenticate);
  });
  bleHR.onDisconnect([]() {
    deepSleep.sleep(1000 * 10); // disconnect 後はスリープしやすくする.
    Serial.println("-- disconnect");
    drawBLEState.state(DrawBLEState::State::disconnected);
  });
  bleHR.start();

  // initialize peak detection.
  avgTemp.begin();
  peak.setThrDist(thrDist);
  peak.onPeakPositive([](int16_t val) {
    digitalWrite(LED_BUILTIN, LOW);
    // drawBeatIcon.enable();
  });
  peak.onPeakNegative([](int16_t val) {
    digitalWrite(LED_BUILTIN, HIGH);
    // drawBeatIcon.disable();
    rate.beat(millis());
  });
  peak.init(analogRead(ANALOG_IN));

  // initialize screen.
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);

  // prepare deep sleep mode.
  deepSleep.setup();
  deepSleep.sleep(1000 * 10);

#ifdef SCREEN_SHOT
  // screen shot.
  screenShot.createSprite(160, 80);
#endif
}

bool updated()
{
  for (int16_t i = 0; i < gadgetsLen; i++)
  {
    if (gadgets[i]->updated())
    {
      return true;
    }
  }
  return false;
}

void draw(TFT_eSprite &sprite, bool force = false)
{
  for (int16_t i = 0; i < gadgetsLen; i++)
  {
    gadgets[i]->draw(sprite, force);
  }
}

void draw(TFT_eSPI &tft, bool force = false)
{
  for (int16_t i = 0; i < gadgetsLen; i++)
  {
    gadgets[i]->draw(tft, force);
  }
}

// static unsigned long prevTick = 0;
void loop()
{
  M5.update();
  if (M5.BtnB.wasPressed())
  {
#ifdef SCREEN_SHOT
    screenShot.setSwapBytes(true);
    draw(screenShot, true);
    screenShot.writeBmpBase64();
#endif
  }

  unsigned long now = millis();
  // Serial.printf("dur: %d\n", prevTick - now);
  // prevTick = now;

  int val;
  val = analogRead(ANALOG_IN);

  if (val < 4095) // 計測中(センサーの近くに何かある).
  {
    int avgVal = avgTemp.reading(val);
    peak.put(avgVal);
    // Serial.printf("val: %d\n", avgVal);

    // -- rate
    drawRate.progress(rate.filling());
    drawRate.rate(rate.rate(now));

    // -- plotter
    drawPlotter.plot(avgVal);

    deepSleep.bump(); // 計測中はスリープしない.
  }

  draw(M5.Lcd);

  // notify heart rate to central.
  int16_t r = rate.rate(now);

  uint8_t buf[2];
  bleHR.notifyRate([&buf, r]() {
    memset(buf, 0, sizeof(buf));
    // https://www.nttpc.co.jp/technology/IoT_health_2.html
    buf[0] = 0x00 | (r != -1 ? 0x06 : 0x04); // 8bit | 計測可能か?

    // 算出できていなくても 0 で送信する
    if (r != -1)
    {
      buf[1] = r; // 心拍数
    }
    else
    {
      buf[1] = 0;
    }
    // Serial.printf("notify - %d %d\n", buf[0], r);
    return buf;
  });

  // if (r != -1) // 算出できるサンプルがあるか?
  // {
  //   uint8_t buf[2];
  //   bleHR.notifyRate([&buf, r]() {
  //     memset(buf, 0, sizeof(buf));
  //     // https://www.nttpc.co.jp/technology/IoT_health_2.html
  //     buf[0] = 0x00 | (r != -1 ? 0x06 : 0x04); // 8bit | 計測可能か?
  //     buf[1] = r;                              // 心拍数
  //     Serial.printf("notify - %d %d\n", buf[0], r);
  //     return buf;
  //   });
  // }

  deepSleep.tick();

  delay(delayMS);
}
