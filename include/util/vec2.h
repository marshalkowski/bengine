#pragma once

#include <cmath>

struct vec2 {
	float x;
	float y;

	vec2() : x(0), y(0) {}

	vec2(float xVal, float yVal) : x(xVal), y(yVal) {}

    float Length() const {
        return std::sqrt(x * x + y * y);
    }

    // Returns the normalized vector
    vec2 Normalized() const {
        float len = Length();
        if (len == 0.0f) return { 0.0f, 0.0f }; // Avoid divide-by-zero
        return { x / len, y / len };
    }

    vec2 operator+(const vec2& other) const {
        return { x + other.x, y + other.y };
    }

    void operator+=(const vec2& other) {
        x += other.x;
        y += other.y;
    }

    vec2 operator*(float other) const {
        return { x * other, y * other };
    }
};