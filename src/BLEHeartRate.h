#ifndef __BLE_HEART_RATE_H__
#define __BLE_HEART_RATE_H__

#include <string>
#include <functional>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_INFORMATION_SERVICE_UUID "180A"
#define HEART_RATE_SERVICE_UUID "180D"

#define BODY_SENSOR_LOCATION_CHAR_UUID "2A38"
#define HEART_RATE_MEASUREMENT_CHAR_UUID "2A37"

class BLEHeartRate
{
private:
    BLEService *pServiceHeartRate;
    BLEService *pServiceDeviceInformation;

    BLECharacteristic *_pCharacteristicHeartRate;
    BLECharacteristic *pCharacteristicBodySensor;

    BLEAdvertising *pAdvertising;

    std::function<void()> _onConnect = []() {};
    std::function<void()> _onDisconnect = []() {};
    std::function<void(uint32_t)> _onPassKeyNotify = [](uint32_t passKey) {};
    std::function<void()> _onAuthentication = []() {};

    bool _authenticated;
    unsigned long _prevMillis;
    unsigned long _heartRateNotifyInterval = 1000 * 1;

    class _serverCBs : public BLEServerCallbacks
    {
    private:
        std::function<void()> _connectHandler;
        std::function<void()> _disconnectHandler;

    public:
        _serverCBs(
            std::function<void()> connectHandler,
            std::function<void()> disconnectHandler)
            : _connectHandler(connectHandler),
              _disconnectHandler(disconnectHandler){};

        void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
        {
            _connectHandler();
        };

        void onDisconnect(BLEServer *pServer)
        {
            _disconnectHandler();
        }
    };
    class _securityCBs : public BLESecurityCallbacks
    {
    private:
        bool &_authenticated;
        std::function<void(uint32_t passKey)> _passKeyNotifyHandler;
        std::function<void()> _authenticationHandler;

    public:
        _securityCBs(bool &authenticated,
                     std::function<void(uint32_t passKey)> passKeyNotifyHandler,
                     std::function<void()> authenticationHandler) : _authenticated(authenticated),
                                                                    _passKeyNotifyHandler(passKeyNotifyHandler),
                                                                    _authenticationHandler(authenticationHandler){};

        bool onConfirmPIN(uint32_t pin)
        {
            // Serial.println("onConfirmPIN");
            return true;
        }

        uint32_t onPassKeyRequest()
        {
            // Serial.println("onPassKeyRequest");
            // これはコールされない?
            return 123456;
        }

        void onPassKeyNotify(uint32_t pass_key)
        {
            // Serial.println("onPassKeyNotify");
            // Serial.printf("PassKey PIN: %d\n", pass_key);
            _passKeyNotifyHandler(pass_key);
        }

        bool onSecurityRequest()
        {
            // Serial.println("onSecurityRequest");
            return true;
        }

        void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
        {
            if (cmpl.success)
            {
                // uint16_t length;
                // esp_ble_gap_get_whitelist_size(&length);
                // Serial.printf("size: %d\n", length);
                _authenticated = true;
                _authenticationHandler();
            }
        }
    };

public:
    BLEHeartRate(){};
    void init(std::string deviceName);
    unsigned long setHeartRateNotifyInterval(unsigned long ms);
    void start();
    void stop();

    void onConnect(std::function<void()> cb);
    void onDisconnect(std::function<void()> cb);
    void onPassKeyNotify(std::function<void(uint32_t passKey)> cb);
    void onAuthentication(std::function<void()> cb);

    void notifyRate(std::function<uint8_t *()> cb);
};

#endif
