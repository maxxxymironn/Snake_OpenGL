#pragma once

template <typename T>
struct vec2 {
    T x{}, y{};
    
    vec2() = default;
    constexpr vec2(T n): x(n), y(n) {}
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
struct vec3 {
    T x{}, y{}, z{};

    vec3() = default;
    vec3(const T n): x(n), y(n), z(n) {}
    vec3(const T x, const T y, const T z): x(x), y(y), z(z) {}
    
    vec3(const vec3& other): x(other.x), y(other.y), z(other.z) {}
    vec3(const vec2<T>& other, const T z): x(other.x), y(other.y), z(z) {}

    vec3& operator=(const vec3& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    bool operator==(const vec3 other) { return x == other.x && y == other.y && z == other.z;}
    bool operator!=(const vec3 other) { return !(this == other); }
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

    bool operator==(const vec4& other) const {
        return this->x == other.x && this->y == other.y 
            && this->z == other.z && this->w == other.w;
    }

    bool operator!=(const vec4& other) const {
        return !(*this == other);
    }

    vec4& operator-=(const vec4& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    vec4& operator-=(const T n) {
        x -= n;
        y -= n;
        z -= n;
        w -= n;
        return *this;
    }

    vec4& operator*=(const T& n) {
        x *= n;
        y *= n;
        z *= n;
        w *= n;
        return *this;
    }
};

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
inline vec2<T> operator*(vec2<T> lv, const vec2<T>& rv) {
    return lv *= rv;
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

template <typename T>
inline vec4<T> operator-(vec4<T> lVec, const vec4<T>& rVec) {
    lVec -= rVec;
    return lVec;
}

template <typename T>
inline vec4<T> operator-(vec4<T> vec, const T& n) {
    vec -= n;
    return vec;
}

template <typename T>
inline vec4<T> operator*(vec4<T> vec, const T& n) {
    vec *= n;
    return vec;
}

using vec2i = vec2<int>;
using vec2f = vec2<float>;
using vec3i = vec3<int>;
using vec3f = vec3<float>;
using vec4i = vec4<int>;
using vec4f = vec4<float>;