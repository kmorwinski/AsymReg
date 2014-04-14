#include "duration.h"

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;

Duration::Duration(std::chrono::high_resolution_clock::duration duration)
    : m_dur(duration),
      m_unit(InvalidUnit)
{}

Duration::Duration()
    : m_dur(0),
      m_unit(InvalidUnit)
{}

void Duration::detectUnit() const
{
    if (m_unit != InvalidUnit)
        return;

    m_unit = MicrosecondsUnit;
    auto v = duration_cast<microseconds>(m_dur).count();
    if (v > 1000) {
        m_unit = MillisecondsUnit;
        v = duration_cast<milliseconds>(m_dur).count();
        if (v > 1000)
            m_unit = SecondsUnit;
    }
}

double Duration::value() const
{
    detectUnit();
    switch (m_unit) {
    case SecondsUnit:
        return duration_cast<milliseconds>(m_dur).count() / 1000.;
    case MillisecondsUnit:
        return duration_cast<microseconds>(m_dur).count() / 1000.;
    case MicrosecondsUnit:
        return double(duration_cast<microseconds>(m_dur).count());
    }

    return 0.0;
}

const char *Duration::unit() const
{
    detectUnit();
    switch (m_unit) {
    case SecondsUnit:
        return "s";
    case MillisecondsUnit:
        return "ms";
    case MicrosecondsUnit:
        return "µs";
    }

    return "";
}
