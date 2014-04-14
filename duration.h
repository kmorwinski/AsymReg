#ifndef DURATION_H_
#define DURATION_H_

#include <chrono>

class Duration
{
public:
    Duration(std::chrono::high_resolution_clock::duration duration);
    Duration();

    double value() const;

    const char *unit() const;

private:
    enum TimeUnit {
        InvalidUnit = -1,
        SecondsUnit,
        MillisecondsUnit,
        MicrosecondsUnit
    };

    void detectUnit() const;

    std::chrono::high_resolution_clock::duration m_dur;
    mutable TimeUnit m_unit;
};

#endif // DURATION_H
