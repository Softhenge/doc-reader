#pragma once

#include <chrono>
#include <iostream>

class Timer
{
public:
    Timer()
        : m_start{Clock::now()}
    {
    }

    ~Timer()
    {
        std::cout << std::endl
                  << "Time: "
                  << Duration(Clock::now() - m_start).count()
                  << std::endl;

    }

private:
    using Duration = std::chrono::duration<double>;
    using Clock = std::chrono::high_resolution_clock;

    std::chrono::time_point<Clock> m_start;
};
