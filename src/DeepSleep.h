#ifndef __DEEP_SLEEP_H__
#define __DEEP_SLEEP_H__

class DeepSleep
{
private:
    unsigned long _after;
    unsigned long _at;

public:
    DeepSleep();
    void setup();
    void sleep(unsigned long after);
    void sleep();
    void bump();
    void tick();
};

#endif
