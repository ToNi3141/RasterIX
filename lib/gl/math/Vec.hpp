// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef VEC_HPP
#define VEC_HPP
#include <array>
#include <cstdint>
#include <functional>
#include <math.h>

namespace rr
{
template <std::size_t VecSize>
class Vec
{
public:
    Vec() { }
    Vec(const Vec<VecSize>& val) { vec = val.vec; }
    Vec(const Vec<VecSize>&& val) { vec = std::move(val.vec); }
    Vec(const std::initializer_list<float> val) { std::copy(val.begin(), val.end(), vec.begin()); }
    Vec(const std::array<float, VecSize>& val) { vec = val; }
    Vec(const float* val) { operator=(val); }
    ~Vec() { }

    void init()
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = 0.0f;
    }

    void initHomogeneous()
    {
        if constexpr (VecSize < 1)
        {
            vec[0] = 1.0f;
            return;
        }
        for (std::size_t i = 0; i < VecSize - 1; i++)
            vec[i] = 0.0f;
        vec[VecSize - 1] = 1.0f;
    }

    static Vec<VecSize> createHomogeneous()
    {
        if constexpr (VecSize < 1)
        {
            Vec<VecSize> vec;
            vec[0] = 1.0f;
            return vec;
        }

        Vec<VecSize> vec;
        for (std::size_t i = 0; i < VecSize - 1; i++)
            vec.vec[i] = 0.0f;
        vec.vec[VecSize - 1] = 1.0f;
        return vec;
    }

    template <typename T, std::size_t S>
    static Vec<VecSize> createFromArray(const T* arr)
    {
        Vec<VecSize> vec;
        static_assert(S <= VecSize);
        for (std::size_t i = 0; i < S; i++)
            vec[i] = static_cast<float>(arr[i]);
        return vec;
    }

    template <typename T>
    void fromArray(const T* arr, const std::size_t size)
    {
        const std::size_t len = (std::min)(size, VecSize); // Put std::min in parenthesis to increase compatibility with msvc
        for (std::size_t i = 0; i < len; i++)
            vec[i] = arr[i];
    }

    Vec<VecSize>& operator*=(const float val)
    {
        mul(val);
        return *this;
    }

    Vec<VecSize>& operator*=(const Vec<VecSize>& val)
    {
        mul(val);
        return *this;
    }

    void perspectiveDivide()
    {
        const float inv = 1.0f / vec[VecSize - 1];
        if constexpr (VecSize < 1)
        {
            vec[0] = inv;
            return;
        }
        vec[VecSize - 1] = inv;
        for (std::size_t i = 0; i < VecSize - 1; i++)
            vec[i] = vec[i] * inv;
    }

    void div(float val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] / val;
    }

    void mul(float val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * val;
    }

    void mul(const Vec<VecSize>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * val[i];
    }

    Vec<VecSize>& operator+=(const Vec<VecSize>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] += val[i];
        return *this;
    }

    Vec<VecSize>& operator-=(const Vec<VecSize>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] -= val[i];
        return *this;
    }

    Vec<VecSize>& operator-=(float val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] -= val;
        return *this;
    }

    float& operator[](int index) { return vec[index]; }
    float operator[](int index) const { return vec[index]; }
    Vec<VecSize>& operator=(const Vec<VecSize>& val)
    {
        vec = val.vec;
        return *this;
    }
    Vec<VecSize>& operator=(const Vec<VecSize>&& val)
    {
        vec = std::move(val.vec);
        return *this;
    }
    Vec<VecSize>& operator=(const std::array<float, VecSize>& val)
    {
        vec = val;
        return *this;
    }
    Vec<VecSize>& operator=(const float* val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = val[i];
        return *this;
    }

    float dot(const Vec<VecSize>& val) const
    {
        float retVal = 0;
        for (std::size_t i = 0; i < VecSize; i++)
            retVal += vec[i] * val[i];
        return retVal;
    }

    void normalize()
    {
        float tmp = 0;
        for (std::size_t i = 0; i < VecSize; i++)
            tmp += vec[i] * vec[i];

        tmp = sqrtf(tmp);
        if (tmp == 0.0f)
            return;

        tmp = 1.0f / tmp;
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * tmp;
    }

    float dist(const Vec<VecSize>& val) const
    {
        float tmp = 0;
        for (std::size_t i = 0; i < VecSize; i++)
            tmp += (vec[i] - val[i]) * (vec[i] - val[i]);
        return sqrtf(tmp);
    }

    void unit()
    {
        float tmp = 0;
        for (std::size_t i = 0; i < VecSize; i++)
            tmp += vec[i] * vec[i];

        tmp = sqrtf(tmp);
        if (tmp == 0.0f)
            return;

        tmp = 1.0f / tmp;
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * tmp;
    }

    void cross(const Vec<VecSize>& val)
    {
        const Vec<VecSize> tmp { *this };
        for (std::size_t i = 0; i < VecSize; i++)
        {
            vec[i] = (tmp[(i + 1) % VecSize] * val[(i + 2) % VecSize]) - (tmp[(i + 2) % VecSize] * val[(i + 1) % VecSize]);
        }
    }

    const float* data() const
    {
        return vec.data();
    }

    float* data()
    {
        return vec.data();
    }

    template <std::size_t S>
    friend bool operator==(const rr::Vec<S>& lhs, const rr::Vec<S>& rhs);

private:
    std::array<float, VecSize> vec {};
};

template <std::size_t S, std::size_t T>
inline Vec<T> operator*(const Vec<S>& lhs, const Vec<T>& rhs)
{
    Vec<T> t;
    for (std::size_t i = 0; i < T; i++)
        t[i] = lhs[i] * rhs[i];
    return t;
}

template <std::size_t T>
inline Vec<T> operator*(const float lhs, const Vec<T>& rhs)
{
    Vec<T> t;
    for (std::size_t i = 0; i < T; i++)
        t[i] = lhs * rhs[i];
    return t;
}

template <std::size_t T>
inline Vec<T> operator*(const Vec<T>& lhs, const float rhs)
{
    Vec<T> t;
    for (std::size_t i = 0; i < T; i++)
        t[i] = lhs[i] * rhs;
    return t;
}

template <std::size_t T>
inline Vec<T> operator-(const Vec<T>& lhs, const Vec<T>& rhs)
{
    Vec<T> t;
    for (std::size_t i = 0; i < T; i++)
        t[i] = lhs[i] - rhs[i];
    return t;
}

template <std::size_t T>
inline Vec<T> operator+(const Vec<T>& lhs, const Vec<T>& rhs)
{
    Vec<T> t;
    for (std::size_t i = 0; i < T; i++)
        t[i] = lhs[i] + rhs[i];
    return t;
}

template <std::size_t T>
inline bool operator==(const rr::Vec<T>& lhs, const rr::Vec<T>& rhs)
{
    return std::equal(lhs.vec.begin(), lhs.vec.end(), rhs.vec.begin());
}

template <std::size_t T>
inline bool operator!=(const rr::Vec<T>& lhs, const rr::Vec<T>& rhs)
{
    return !(lhs == rhs);
}

using Vec1 = Vec<1>; // Is basically a scalar but simplifies some code which uses templates
using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;

} // namespace rr
#endif // VEC_HPP
