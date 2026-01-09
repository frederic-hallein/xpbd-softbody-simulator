#pragma once

#include <chrono>

class Timer
{
public:
    Timer();

    void startFrame();
    float getDeltaTime() const { return m_deltaTime; };
    void capFrameRate(int targetFPS);

public:
    int frameDuration;

private:
    std::chrono::high_resolution_clock::time_point m_frameStart;
    std::chrono::high_resolution_clock::time_point m_lastFrame;
    float m_deltaTime;
};