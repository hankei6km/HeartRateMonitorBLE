#ifndef __HEART_RATE_H__
#define __HEART_RATE_H__

#include <list>

class HeartRate
{
private:
    std::list<unsigned long> _beats; // milli()
    unsigned long _rateDuration = 5 * 1000;
    unsigned long _effectiveIntervalMin = 0.3 * 1000;
    unsigned long _effectiveIntervalMax = 2 * 1000;
    unsigned long _purgeTrigger = 10 * 1000;
    unsigned long _outlierLim = 2;
    unsigned long _outlierCnt = 0;

    unsigned long duration()
    {
        if (_beats.size() <= 1) // 0 のときに .back() 等を実行すると落ちる?
        {
            return 0;
        }
        return (_beats.back() - _beats.front());
    };
    unsigned long duration(unsigned long t)
    {
        if (_beats.size() == 0)
        {
            return 0;
        }
        return (t - _beats.front());
    };
    /**
     * t (時刻)が有効な範囲におさまっているか?
     *  0 : おさまっている.
     * -1 : 短く外れている.
     *  1 : 長く外れている.
     **/
    int8_t _effective(unsigned long t)
    {
        if (_beats.size() == 0)
        {
            return 0;
        }
        unsigned long b = _beats.back();
        if (t <= b + _effectiveIntervalMin)
        {
            return -1;
        }
        else if (b + _effectiveIntervalMax <= t)
        {
            return 1;
        }
        return 0;
    };
    void _purge(unsigned long t)
    {
        while (duration(t) > _purgeTrigger)
        {
            _beats.erase(_beats.begin());
        }
    };
    void _reset()
    {
        _beats.resize(0);
        _outlierCnt = 0;
    };

public:
    bool beat(unsigned long t)
    {
        bool ret = false;
        int8_t e = _effective(t);

        // 古い値を切り離す
        _purge(t);

        // 有効な値か?
        if (e == 0)
        {
            _beats.push_back(t);
            _outlierCnt = 0;
            ret = true;
        }
        else if (e < 0)
        {
            // 短い間隔での外れ値は無視する
            _outlierCnt = 0;
        }
        else
        {
            // 長い間隔での外れ値.
            _outlierCnt++;
            if (_outlierCnt >= _outlierLim)
            {
                // 複数回外れ値だったらリセット
                _reset();
            }
            else
            {
                if (duration() >= _rateDuration)
                {
                    // 算出できるサンプルがあれば平均で補完する
                    unsigned long avg = duration() / (unsigned long)(_beats.size() - 1);
                    for (unsigned long b = _beats.back() + avg; b <= t; b = b + avg)
                    {
                        _beats.push_back(b);
                    }
                }
            }
        }
        return ret;
    };
    int16_t rate(unsigned long t)
    {
        _purge(t);
        unsigned long d = duration();
        if (d < _rateDuration)
        {
            return -1;
        }
        return 60000.0 * (unsigned long)(_beats.size() - 1) / d;
    };
    int16_t filling()
    {
        int16_t ret = (duration() * 100) / _rateDuration;
        if (ret > 100)
        {
            return 100;
        }
        return ret;
    };
    int16_t filling(unsigned long t)
    {
        int16_t ret = (duration(t) * 100) / _rateDuration;
        if (ret > 100)
        {
            return 100;
        }
        return ret;
    };
};

#endif
