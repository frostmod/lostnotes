#pragma once
#include <memory>
#include <array>
#include <cstdint>
#include <algorithm>

inline float poly_blep(float t, float w) {
    if (w <= 0.0f) return 0.0f;
    if (t < w) {
        t /= w;
        return t + t - t*t - 1.0f;
    } else if (t > 1.0f - w) {
        t = (t - 1.0f) / w;
        return t*t + t + t + 1.0f;
    }
    return 0.0f;
}

struct PolyBLEPSaw {
    inline float processSample(float phase, float dt) {
        float y = 2.f * phase - 1.f;
        y -= poly_blep(phase, dt);
        return y;
    }
};