#include <string>
#include <functional>

#include <M5StickC.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "BLEHeartRate.h"

class dataBodyLocCb : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *pChar)
    {
        uint8_t buf[1];

        memset(buf, 0, sizeof(buf)); // バッファーを0クリア
        buf[0] = 0x02;
        pChar->setValue(buf, sizeof(buf)); // データーを書き込み
    }
};

void BLEHeartRate::init(std::string deviceName)
{
    bool &authenticated = _authenticated;
    std::function<void()> &onConnect = _onConnect;
    std::function<void()> &onDisconnect = _onDisconnect;
    std::function<void(uint32_t passKye)> &onPassKeyNotify = _onPassKeyNotify;
    std::function<void()> &onAuthentication = _onAuthentication;

    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new _securityCBs(
        authenticated,
        [&onPassKeyNotify](uint32_t passKey) {
            onPassKeyNotify(passKey);
            return;
        },
        [&onAuthentication]() {
            onAuthentication();
            return;
        }));

    BLEDevice::init(deviceName);

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new _serverCBs(
        [&onConnect]() {
            /// authenticated = true; ここではなく、securty の方で設定でする.
            onConnect();
            return;
        },
        [&onDisconnect, &authenticated]() {
            onDisconnect();
            authenticated = false;
            return;
        }));

    pServiceHeartRate = pServer->createService(HEART_RATE_SERVICE_UUID);
    pServiceDeviceInformation = pServer->createService(DEVICE_INFORMATION_SERVICE_UUID);

    _pCharacteristicHeartRate = pServiceHeartRate->createCharacteristic(
        HEART_RATE_MEASUREMENT_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);
    _pCharacteristicHeartRate->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED); // notify の設定は?
    BLE2902 *ble9202 = new BLE2902();
    ble9202->setNotifications(true);
    ble9202->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    _pCharacteristicHeartRate->addDescriptor(ble9202);

    pCharacteristicBodySensor = pServiceHeartRate->createCharacteristic(
        BODY_SENSOR_LOCATION_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ);
    pCharacteristicBodySensor->setCallbacks(new dataBodyLocCb());

    // pServiceHeartRate->start();
    // pServiceDeviceInformation->start();

    pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(DEVICE_INFORMATION_SERVICE_UUID);
    pAdvertising->addServiceUUID(HEART_RATE_SERVICE_UUID);
    // pAdvertising->start();

    BLESecurity *pSecurity = new BLESecurity();
#if defined(BLE_STATIC_PASS_KEY)
    // pSecurity->setStaticPIN(123456);
    // https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/BLESecurity.cpp
    uint32_t pass_key = BLE_STATIC_PASS_KEY; // 入力するときは 6 桁が必須のもよう(例. pass_key = 12345 で設定した場合、012345 と入力)
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &pass_key, sizeof(uint32_t));
#endif
    pSecurity->setCapability(ESP_IO_CAP_OUT); // OUT 必須ぽい？ NONEにすると PINの確認がされない。
    pSecurity->setKeySize();

    // pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

void BLEHeartRate::start()
{
    pServiceHeartRate->start();
    pServiceDeviceInformation->start();
    pAdvertising->start();
    _prevMillis = 0;
}

void BLEHeartRate::stop()
{
    pAdvertising->stop();
    pServiceDeviceInformation->stop();
    pServiceHeartRate->stop();
    _prevMillis = 0;
}

unsigned long BLEHeartRate::setHeartRateNotifyInterval(unsigned long ms)
{
    unsigned long ret = _heartRateNotifyInterval;
    _heartRateNotifyInterval = ms;
    return ret;
}

void BLEHeartRate::onConnect(std::function<void()> cb)
{
    _onConnect = cb;
}

void BLEHeartRate::onDisconnect(std::function<void()> cb)
{
    _onDisconnect = cb;
}

void BLEHeartRate::onPassKeyNotify(std::function<void(uint32_t passKey)> cb)
{
    _onPassKeyNotify = cb;
}

void BLEHeartRate::onAuthentication(std::function<void()> cb)
{
    _onAuthentication = cb;
}

void BLEHeartRate::notifyRate(std::function<uint8_t *()> cb)
{
    unsigned long m = millis();
    if (_authenticated && m > _prevMillis + _heartRateNotifyInterval)
    {
        uint8_t *buf = cb();
        _pCharacteristicHeartRate->setValue(buf, sizeof(buf)); // データーを書き込み
        _pCharacteristicHeartRate->notify();                   // 通知
        _prevMillis = m;
    }
}
