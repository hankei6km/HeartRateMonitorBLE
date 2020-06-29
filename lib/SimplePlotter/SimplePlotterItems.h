#ifndef __SIMPLE_PLOTTER_ITEMS_H__
#define __SIMPLE_PLOTTER_ITEMS_H__

#include <list>
#include <set>
#include <tuple>

template <typename T>
class PlotterItems : public std::list<T>
{
public:
    PlotterItems() : std::list<T>(){};
    uint16_t setLim(const uint16_t lim)
    {
        uint16_t r = _lim;
        _lim = lim;
        return r;
    };
    T low()
    {
        return _low;
    }
    T high()
    {
        return _high;
    }
    bool append(const T v)
    {
        bool ret = false;
        const size_t itemsSize = std::list<T>::size();
        if (_lim > 0 && itemsSize >= _lim)
        {
            auto b = std::list<T>::begin();
            std::list<T>::erase(b);
            _rangeItems.erase(_rangeItems.find(*b));
        }
        std::list<T>::push_back(v);
        _rangeItems.insert(v);

        T l = *_rangeItems.begin();
        T h = *_rangeItems.rbegin();
        if (_low != l)
        {
            ret = true;
        }
        _low = l;
        if (_high != h)
        {
            ret = true;
        }
        _high = h;

        return ret;
    };
    static std::tuple<T, T, float> range(T low, T high, uint16_t pixels)
    {
        T d = high - low;
        if (d == 0)
        {
            if (low == 0)
            {
                return std::tuple<T, T, float>(0, (T)pixels, 1.0);
            }
            return std::tuple<T, T, float>(0, low * 2, (float)pixels / (low * 2));
        }
        return std::tuple<T, T, float>(low, high, (float)pixels / d);
    };

private:
    std::multiset<T> _rangeItems;
    uint16_t _lim = 0;
    T _range[2] = {0, 0};
    T _low = 0;
    T _high = 0;
};

#endif
