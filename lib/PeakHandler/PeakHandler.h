#ifndef __PEAK_HANDLER_H__
#define __PEAK_HANDLER_H__
#include <functional>

template <typename T>
class PeakHandler
{
    std::function<void(T val)> _positiveHandler = [](T val) {};
    std::function<void(T val)> _negativeHandler = [](T val) {};

    T _prevVal = 0;
    int _prevDir = 0;
    int _prevPeak = 0;
    uint16_t _dirCnt = 0;
    T _peakNegative = 0;
    T _peakPositive = 0;
    T _thrNegative = 0;
    T _thrPositive = 0;
    T _thrDist = 3;

public:
    PeakHandler(){};

    PeakHandler(T initVal)
    {
        init(initVal);
    };

    T setThrDist(T dist)
    {
        T ret = _thrDist;
        _thrDist = dist;
        return ret;
    };

    void onPeakPositive(std::function<void(T val)> cb)
    {
        _positiveHandler = cb;
    };

    void onPeakNegative(std::function<void(T val)> cb)
    {
        _negativeHandler = cb;
    };

    void init(T val)
    {
        _prevVal = val;
        _peakPositive = val + (_thrDist + 1);
        _peakNegative = val - (_thrDist + 1);
        _thrPositive = _peakPositive - _thrDist;
        _thrNegative = _peakNegative + _thrDist;
        // _peakPositive = val;
        // _peakNegative = val;
    };

    bool put(T val)
    {
        bool ret = false;
        T dlt = val - _prevVal;
        int dir = (dlt == 0) ? 0 : abs(dlt) / dlt;
        if (dir != 0)
        {
            if (dir != _prevDir)
            {
                // １つ前の値を使うので、
                // ハンドラーの呼び出しは実際のピークよりも遅れる.
                if (dir < _prevDir && _prevVal >= _thrNegative)
                {
                    _peakPositive = _prevVal;
                    _thrPositive = _peakPositive - _thrDist;
                    if (_prevPeak != 1)
                    {
                        // 前回と同じ側のピークはハンドリングしない(情報の更新はする)
                        _positiveHandler(_prevVal);
                        ret = true;
                        _prevPeak = 1;
                    }
                }
                else if (dir > _prevDir && _prevVal <= _thrPositive)
                {
                    _peakNegative = _prevVal;
                    _thrNegative = _peakNegative + _thrDist;
                    if (_prevPeak != -1)
                    {
                        // 前回と同じ側のピークはハンドリングしない(情報の更新はする)
                        _negativeHandler(_prevVal);
                        ret = true;
                        _prevPeak = -1;
                    }
                }
            }
            _prevDir = dir;
        }
        _prevVal = val;
        return ret;
    };
};

#endif
