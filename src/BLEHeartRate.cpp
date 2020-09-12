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

    // BLEDevice::setMTU は esp_ble_gattc_send_mtu_req で利用される値の準備、
    // esp_ble_gattc_send_mtu_req は GATT Client(centrarl) として接続されたときに
    // １回だけ実行される。
    // よって、今回は GATT server (peripheral) として動作するので実行しても効果はない。
    // と思われ。
    //
    // BLEServer::updatePeerMTU は、接続されている peer device からの ESP_GATTS_MTU_EVT の
    // 値を m_connectedServersMap に反映させるだけで、変更の要求を送信したりはしていない。
    // よって、こちらも onConnect あたりで実行しても効果はない。
    // (Web Bluetooth との接続で試してみたが、m_connectedServersMap の値を変更できても
    // 相手側では 20bytes しか受信されなかった)。
    // #if defined(LOGGING)
    //     BLEDevice::setMTU(HRM_DEV_LOG_MTU_SIZE);
    // #endif

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
#if defined(LOGGING)
    pServiceHrmDevLog = pServer->createService(HRM_DEV_LOG_SERVICE_UUID);
#endif

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

#if defined(LOGGING)
    _pCharacteristicHrmDevLogPrint = pServiceHrmDevLog->createCharacteristic(
        HRM_DEV_LOG_PRINT_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);
    _pCharacteristicHrmDevLogPrint->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    BLE2902 *logBle9202 = new BLE2902();
    logBle9202->setNotifications(true);
    logBle9202->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    _pCharacteristicHrmDevLogPrint->addDescriptor(logBle9202);
    // _pCharacteristicHrmDevLogPrint->setCallbacks(new _charLoggingCBs(_logPrint, 2)); // onNotify はコールされない。 _pCharacteristicHeartRate でセットするとコールされる。理由は不明(UUIDで切り替えてる?)

    _pCharacteristicHrmDevLogStart = pServiceHrmDevLog->createCharacteristic(
        HRM_DEV_LOG_START_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE_NR);
    // BLECharacteristic::PROPERTY_WRITE);
    _pCharacteristicHrmDevLogStart->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    // _pCharacteristicHrmDevLogStart->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_SIGNED);
    _pCharacteristicHrmDevLogStart->setCallbacks(new _charLoggingCBs(_logPrint, 3)); // BODY_SENSOR_LOCATION_CHAR_UUID で onNotification がコールされないと、onWrite がコールされない、ような気がする
#endif

    // pServiceHeartRate->start();
    // pServiceDeviceInformation->start();

    pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(DEVICE_INFORMATION_SERVICE_UUID);
    pAdvertising->addServiceUUID(HEART_RATE_SERVICE_UUID);
#if defined(LOGGING)
    pAdvertising->addServiceUUID(HRM_DEV_LOG_SERVICE_UUID);
#endif
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
#if defined(LOGGING)
    pServiceHrmDevLog->start();
#endif
    pAdvertising->start();
    _prevMillis = 0;
}

void BLEHeartRate::stop()
{
    pAdvertising->stop();
    pServiceDeviceInformation->stop();
    pServiceHeartRate->stop();
#if defined(LOGGING)
    pServiceHrmDevLog->stop();
#endif
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

#if defined(LOGGING)
bool BLEHeartRate::hrmLogPrintEnabled()
{
    return _logPrint;
}
void BLEHeartRate::hrmLogResetBuf()
{
    _logBufUpdated = false;
    memset(_logBuf, 0, HRM_DEV_LOG_MTU_SIZE);
}
void BLEHeartRate::hrmLogSetMillis(uint32_t *v)
{
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_MILLIS], v, 2);
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetGyro(float *x, float *y, float *z)
{
    int16_t t;
    t = (*x * 100.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_GYRO_X], &t, 2);
    t = (*y * 100.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_GYRO_Y], &t, 2);
    t = (*z * 100.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_GYRO_Z], &t, 2);
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetAcc(float *x, float *y, float *z)
{
    int16_t t;
    t = (*x * 1000.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_ACC_X], &t, 2);
    t = (*y * 1000.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_ACC_Y], &t, 2);
    t = (*z * 1000.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_ACC_Z], &t, 2);
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetAhrs(float *pitch, float *roll, float *yaw)
{
    int16_t t;
    t = (*pitch * 100.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_PITCH], &t, 2);
    t = (*roll * 100.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_ROLL], &t, 2);
    t = (*yaw * 100.0F);
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_YAW], &t, 2);
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetVal(int16_t *val)
{
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_VAL], val, 2);
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetBpm(int16_t *val)
{
    memcpy(&_logBuf[HRM_DEV_LOG_FMT_BPM], val, 2);
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetPeakP()
{
    _logBuf[HRM_DEV_LOG_FMT_EXT] = _logBuf[HRM_DEV_LOG_FMT_EXT] | 0x01;
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogSetPeakN()
{
    _logBuf[HRM_DEV_LOG_FMT_EXT] = _logBuf[HRM_DEV_LOG_FMT_EXT] | 0x02;
    _logBufUpdated = true;
}
void BLEHeartRate::hrmLogNotifyBuf()
{
    if (_logPrint && _logBufUpdated)
    {
        // Serial.printf("\nnoti:\n");
        _pCharacteristicHrmDevLogPrint->setValue(_logBuf, HRM_DEV_LOG_MTU_SIZE);
        _pCharacteristicHrmDevLogPrint->notify();
        // hrmLogResetBuf();
    }
}
#endif