#pragma once

template <typename T>
struct vec2 {
    T x{}, y{};

    vec2() = default;
    vec2(T x): x(x), y(x) {}
    constexpr vec2(T x, T y): x(x), y(y) {}
    
    template <typename U>
    vec2(const vec2<U>& other)
        : x(static_cast<T>(other.x)),
          y(static_cast<T>(other.y)) {}

    bool operator==(const vec2& other) const {
        return this->x == other.x && this->y == other.y;
    }

    bool operator!=(const vec2& other) const {
        return !(*this == other);
    }

    template <typename U>
    vec2& operator=(const vec2<U>& other) {
        x = static_cast<T>(other.x);
        y = static_cast<T>(other.y);
        return *this;
    }

    vec2& operator-=(const vec2& other) {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    vec2& operator+=(const vec2& other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    vec2& operator*=(const vec2& other) {
        this->x *= other.x;
        this->y *= other.y;
        return *this;
    }

    vec2& operator/=(const vec2& other) {
        this->x /= other.x;
        this->y /= other.y;
        return *this;
    }

    vec2& operator*=(const T& n) {
        this->x *= n;
        this->y *= n;
        return *this;
    }

    vec2& operator/=(const T& n) {
        this->x /= n;
        this->y /= n;
        return *this;
    }

    vec2 operator-() { return vec2(-x, -y); }
};

template <typename T>
struct vec4 {
    T x{}, y{}, z{}, w{};

    vec4() = default;
    vec4(T x): x(x), y(x), z(x), w(x) {}
    constexpr vec4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) {}
    vec4(const vec2<T>& v1, const vec2<T>& v2): x(v1.x), y(v2.y), z(v2.x), w(v2.y) {}

    template <typename U>
    vec4(const vec4<U>& other)
        : x(static_cast<T>(other.x)),
          y(static_cast<T>(other.y)),
          z(static_cast<T>(other.z)),
          w(static_cast<T>(other.w)) {}

    template <typename U>
    vec4& operator=(const vec4<U>& other) {
        x = static_cast<T>(other.x);
        y = static_cast<T>(other.y);
        z = static_cast<T>(other.z);
        w = static_cast<T>(other.w);
        return *this;
    }
};

// template <typename T>
// inline bool operator==(const vec2<T>& left, const vec2<T>& right) {
//     return left.x == right.x && left.y == right.y;
// }

template <typename T>
inline vec2<T> operator-(vec2<T> lVec, const vec2<T>& rVec) {
    lVec -= rVec;
    return lVec;
}

template <typename T>
inline vec2<T> operator+(vec2<T> lVec, const vec2<T>& rVec) {
    lVec += rVec;
    return lVec;
}

template <typename T>
inline vec2<T> operator/(vec2<T> lVec, const vec2<T>& rVec) {
    lVec /= rVec;
    return lVec;
}

template <typename T>
inline vec2<T> operator*(vec2<T> vec, const T& n) {
    vec.x *= n;
    vec.y *= n;
    return vec;
}

template <typename T>
inline vec2<T> operator/(vec2<T> vec, const T& n) {
    vec.x /= n;
    vec.y /= n;
    return vec;
}

using vec2i = vec2<int>;
using vec2f = vec2<float>;
using vec4i = vec4<int>;
using vec4f = vec4<float>;