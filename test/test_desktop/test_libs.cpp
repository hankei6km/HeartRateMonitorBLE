#include <string>
#include <unity.h>
#define private public
#include <SimplePlotterItems.h>
#include <HeartRate.h>
#include <PeakHandler.h>

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_plotter_items_init(void)
{
    PlotterItems<int16_t> items;
    TEST_ASSERT_EQUAL(0, items.size());
}

void test_plotter_items_append(void)
{
    PlotterItems<int16_t> items;

    items.append(10);
    TEST_ASSERT_EQUAL(1, items.size());
    TEST_ASSERT_EQUAL_INT16(10, items.front());

    items.append(20);
    TEST_ASSERT_EQUAL(2, items.size());
    TEST_ASSERT_EQUAL_INT16(10, items.front());
    TEST_ASSERT_EQUAL_INT16(20, items.back());
}

void test_plotter_items_lim(void)
{
    PlotterItems<int16_t> items;

    items.setLim(3);

    items.append(20);
    items.append(50);
    items.append(10);
    TEST_ASSERT_EQUAL_INT16(20, items.front());
    TEST_ASSERT_EQUAL_INT16(10, items.back());
    items.append(40);
    TEST_ASSERT_EQUAL_INT16(50, items.front());
    TEST_ASSERT_EQUAL_INT16(40, items.back());
    items.append(90);
    TEST_ASSERT_EQUAL_INT16(10, items.front());
    TEST_ASSERT_EQUAL_INT16(90, items.back());
}

void test_plotter_items_range_changed(void)
{
    PlotterItems<int16_t> items;

    items.setLim(3);

    TEST_ASSERT_EQUAL(true, items.append(10));
    TEST_ASSERT_EQUAL(true, items.append(50));
    TEST_ASSERT_EQUAL(false, items.append(20));
    TEST_ASSERT_EQUAL(true, items.append(60));
}

void test_plotter_items_low_high(void)
{
    PlotterItems<int16_t> items;

    items.setLim(3);

    items.append(10);
    items.append(50);
    items.append(20);

    TEST_ASSERT_EQUAL_INT16(10, items.low());
    TEST_ASSERT_EQUAL_INT16(50, items.high());

    items.append(60);
    TEST_ASSERT_EQUAL_INT16(20, items.low());
    TEST_ASSERT_EQUAL_INT16(60, items.high());

    items.append(5);
    TEST_ASSERT_EQUAL_INT16(5, items.low());
    TEST_ASSERT_EQUAL_INT16(60, items.high());

    items.append(90);
    TEST_ASSERT_EQUAL_INT16(5, items.low());
    TEST_ASSERT_EQUAL_INT16(90, items.high());
}

void test_plotter_items_range(void)
{
    auto range = PlotterItems<int16_t>::range(0, 0, 80);
    TEST_ASSERT_EQUAL_INT16(0, std::get<0>(range));
    TEST_ASSERT_EQUAL_INT16(80, std::get<1>(range));
    TEST_ASSERT_EQUAL_FLOAT(1.0, std::get<2>(range));

    range = PlotterItems<int16_t>::range(20, 20, 80);
    TEST_ASSERT_EQUAL_INT16(0, std::get<0>(range));
    TEST_ASSERT_EQUAL_INT16(40, std::get<1>(range));
    TEST_ASSERT_EQUAL_FLOAT(2.0, std::get<2>(range));

    range = PlotterItems<int16_t>::range(40, 60, 80);
    TEST_ASSERT_EQUAL_INT16(40, std::get<0>(range));
    TEST_ASSERT_EQUAL_INT16(60, std::get<1>(range));
    TEST_ASSERT_EQUAL_FLOAT(4.0, std::get<2>(range));

    range = PlotterItems<int16_t>::range(40, 290, 80);
    TEST_ASSERT_EQUAL_INT16(40, std::get<0>(range));
    TEST_ASSERT_EQUAL_INT16(290, std::get<1>(range));
    TEST_ASSERT_EQUAL_FLOAT(0.32, std::get<2>(range));

    range = PlotterItems<int16_t>::range(200, 200, 80);
    TEST_ASSERT_EQUAL_INT16(0, std::get<0>(range));
    TEST_ASSERT_EQUAL_INT16(400, std::get<1>(range));
    TEST_ASSERT_EQUAL_FLOAT(0.2, std::get<2>(range));
}

void test_heart_rate_init(void)
{
    HeartRate rate = HeartRate();
    TEST_ASSERT_EQUAL(-1, rate.rate(1 * 1000));
}

void test_heart_rate_effective(void)
{
    HeartRate rate = HeartRate();
    TEST_ASSERT_EQUAL(0, rate._effective(2 * 1000));
    rate._beats.push_back(1 * 1000);
    TEST_ASSERT_EQUAL(0, rate._effective(2 * 1000));
    TEST_ASSERT_EQUAL(-1, rate._effective(1.1 * 1000));
    TEST_ASSERT_EQUAL(1, rate._effective(3.1 * 1000));
}

void test_heart_rate_duration(void)
{
    HeartRate rate = HeartRate();
    TEST_ASSERT_EQUAL(0, rate.duration());
    rate._beats.push_back(1 * 1000);
    TEST_ASSERT_EQUAL(0, rate.duration());
    rate._beats.push_back(2 * 1000);
    TEST_ASSERT_EQUAL(1 * 1000, rate.duration());
    rate._beats.push_back(2.9 * 1000);
    TEST_ASSERT_EQUAL(1.9 * 1000, rate.duration());
}

void test_heart_rate_filling(void)
{
    HeartRate rate = HeartRate();
    TEST_ASSERT_EQUAL_INT16(0, rate.filling());
    rate._beats.push_back(1 * 1000);
    TEST_ASSERT_EQUAL_INT16(0, rate.filling());
    rate._beats.push_back(2 * 1000);
    TEST_ASSERT_EQUAL_INT16(20, rate.filling());
    rate._beats.push_back(2.9 * 1000);
    TEST_ASSERT_EQUAL_INT16(38, rate.filling());
}

void test_heart_rate_purge(void)
{
    HeartRate rate = HeartRate();
    // TODO: テスト追加
}

void test_heart_rate_beat(void)
{
    HeartRate rate = HeartRate();
    rate._purgeTrigger = 10 * 1000;

    TEST_ASSERT_TRUE(rate.beat(1 * 1000));
    TEST_ASSERT_EQUAL(1, rate._beats.size());
    TEST_ASSERT_FALSE(rate.beat(1.1 * 1000));
    TEST_ASSERT_EQUAL(1, rate._beats.size());
    for (int i = 2; i < 12; i++)
    {
        char buf[30];
        sprintf(buf, "beat: %d", i);
        TEST_ASSERT_TRUE_MESSAGE(rate.beat(i * 1000), buf);
        sprintf(buf, "beats.size: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(i, rate._beats.size(), buf);
    }
    TEST_ASSERT_TRUE_MESSAGE(rate.beat(12 * 1000), "beat: purge");
    TEST_ASSERT_EQUAL_MESSAGE(11, rate._beats.size(), "beats.size: purge");
}

void test_heart_rate_outiler_over(void)
{
    HeartRate rate = HeartRate();
    for (int i = 1; i < 7; i++)
    {
        rate.beat(i * 1000);
    }
    TEST_ASSERT_FALSE(rate.beat(9 * 1000));
    TEST_ASSERT_EQUAL(9, rate._beats.size());
    TEST_ASSERT_EQUAL(9 * 1000, rate._beats.back());
    TEST_ASSERT_FALSE(rate.beat(20 * 1000));
    TEST_ASSERT_EQUAL(0, rate._beats.size());
}

void test_heart_rate_rate(void)
{
    HeartRate rate = HeartRate();

    TEST_ASSERT_EQUAL(-1, rate.rate(1 * 1000));
    for (int i = 1; i < 7; i++)
    {
        rate.beat(i * 1000);
    }
    TEST_ASSERT_EQUAL(5 * 1000, rate.duration());
    TEST_ASSERT_EQUAL(60, rate.rate(6.0 * 1000));

    rate._reset();
    for (int i = 1; i < 20; i++)
    {
        rate.beat(i * 1500);
    }
    TEST_ASSERT_EQUAL(9 * 1000, rate.duration()); // 1500 * 6 < 1000 * 10 < 1500 *7
    TEST_ASSERT_EQUAL(40, rate.rate(19 * 1500));
}

void test_peak_handler_positive(void)
{
    int16_t posVal = 0;
    int16_t negVal = 0;
    int posCnt = 0;
    int negCnt = 0;
    PeakHandler<int16_t> peak = PeakHandler<int16_t>();
    peak.init(100);
    peak.onPeakPositive([&posVal, &posCnt](int16_t val) {
        posVal = val;
        posCnt++;
    });
    peak.onPeakNegative([&negVal, &negCnt](int16_t val) {
        negVal = val;
        negCnt++;
    });

    TEST_ASSERT_FALSE(peak.put(100));
    TEST_ASSERT_TRUE(peak.put(50));
    TEST_ASSERT_EQUAL_INT16(100, posVal);
    TEST_ASSERT_EQUAL(1, posCnt);
    TEST_ASSERT_EQUAL_INT16(0, negVal);
    TEST_ASSERT_EQUAL(0, negCnt);
}

void test_peak_handler_negative(void)
{
    int16_t posVal = 0;
    int16_t negVal = 0;
    int posCnt = 0;
    int negCnt = 0;
    PeakHandler<int16_t> peak = PeakHandler<int16_t>();
    peak.init(100);
    peak.onPeakPositive([&posVal, &posCnt](int16_t val) {
        posVal = val;
        posCnt++;
    });
    peak.onPeakNegative([&negVal, &negCnt](int16_t val) {
        negVal = val;
        negCnt++;
    });

    TEST_ASSERT_FALSE(peak.put(100));
    TEST_ASSERT_TRUE(peak.put(150));
    TEST_ASSERT_EQUAL_INT16(0, posVal);
    TEST_ASSERT_EQUAL(0, posCnt);
    TEST_ASSERT_EQUAL_INT16(100, negVal);
    TEST_ASSERT_EQUAL(1, negCnt);
}

void test_peak_handler_thr_dist(void)
{
    {
        PeakHandler<int16_t> peak = PeakHandler<int16_t>();
        peak.init(100);
        TEST_ASSERT_TRUE_MESSAGE(peak.put(150), "handle negative:default");
        TEST_ASSERT_TRUE_MESSAGE(peak.put(100), "handle positive:defailt");
    }

    {
        PeakHandler<int16_t> peak = PeakHandler<int16_t>();
        peak.init(100);
        peak.setThrDist(300);
        TEST_ASSERT_TRUE_MESSAGE(peak.put(150), "handle negative:300"); // こ時点では閾値は古い値のまま
        TEST_ASSERT_FALSE_MESSAGE(peak.put(100), "handle positive:300");
    }
}

void test_peak_handler_freq(void)
{
    int16_t posVal[] = {0, 0, 0};
    int16_t negVal[] = {0, 0, 0};
    int posCnt = 0;
    int negCnt = 0;
    PeakHandler<int16_t> peak = PeakHandler<int16_t>();
    peak.init(100);
    peak.onPeakPositive([&posVal, &posCnt](int16_t val) {
        TEST_ASSERT_LESS_THAN_INT_MESSAGE(sizeof(posVal) / sizeof(posVal[0]), posCnt, "overflow pos");
        posVal[posCnt] = val;
        posCnt++;
    });
    peak.onPeakNegative([&negVal, &negCnt](int16_t val) {
        TEST_ASSERT_LESS_THAN_INT_MESSAGE(sizeof(negVal) / sizeof(posVal[0]), negCnt, "overflow neg");
        negVal[negCnt] = val;
        negCnt++;
    });

    peak.put(150);
    peak.put(140);
    peak.put(142);
    peak.put(130); // 直前の値は negative との幅が狭いので positive とされない
    peak.put(144); // 直前の値は negative が連続しているので、negative としてハンドルされない(negativeとして閾値の更新などはされている)
    peak.put(100); // 直前の値は negative との幅が広いので positive とされる
    peak.put(110);
    peak.put(90);

    int16_t posEx[] = {150, 144, 110};
    TEST_ASSERT_EQUAL_INT16_ARRAY_MESSAGE(posEx, posVal, 3, "array pos");
    int16_t negEx[] = {100, 140, 100};
    TEST_ASSERT_EQUAL_INT16_ARRAY_MESSAGE(negEx, negVal, 3, "array neg");
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_plotter_items_init);
    RUN_TEST(test_plotter_items_append);
    RUN_TEST(test_plotter_items_lim);
    RUN_TEST(test_plotter_items_range_changed);
    RUN_TEST(test_plotter_items_low_high);
    RUN_TEST(test_plotter_items_range);

    RUN_TEST(test_heart_rate_init);
    RUN_TEST(test_heart_rate_effective);
    RUN_TEST(test_heart_rate_duration);
    RUN_TEST(test_heart_rate_filling);
    RUN_TEST(test_heart_rate_purge);
    RUN_TEST(test_heart_rate_beat);
    RUN_TEST(test_heart_rate_outiler_over);
    RUN_TEST(test_heart_rate_rate);

    RUN_TEST(test_peak_handler_positive);
    RUN_TEST(test_peak_handler_negative);
    RUN_TEST(test_peak_handler_thr_dist);
    RUN_TEST(test_peak_handler_freq);

    UNITY_END();

    return 0;
}
