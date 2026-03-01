// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

#ifndef VECI_HPP
#define VECI_HPP
#include <algorithm>
#include <array>
#include <cstdint>

namespace rr
{
// This is a configurable fixed point vector class
template <typename T, std::size_t VecSize, std::size_t DefaultShift = 0>
class Veci
{
public:
    static constexpr T One = (static_cast<T>(1) << DefaultShift);
    static constexpr T Half = One >> 1;
    static constexpr T Zero = 0;
    static constexpr T FracMax = One - 1;
    static constexpr std::size_t Shift = DefaultShift;
    using Type = T;

    Veci() { }
    Veci(const Veci<T, VecSize, DefaultShift>& val) { operator=(val.vec); }
    Veci(const std::array<T, VecSize>& val) { operator=(val); }
    Veci(const std::initializer_list<T> val) { std::copy(val.begin(), val.end(), vec.begin()); }
    ~Veci() { }

    Veci<T, VecSize, DefaultShift>& operator*=(T val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (vec[i] * val) >> DefaultShift;
        return *this;
    }

    Veci<T, VecSize, DefaultShift>& operator*=(const Veci<T, VecSize, DefaultShift>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (vec[i] * val[i]) >> DefaultShift;
        return *this;
    }

    template <std::size_t LocalShift = DefaultShift>
    void div(T val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (((static_cast<T>(vec[i]) << LocalShift) / static_cast<T>(val)));
    }

    template <std::size_t LocalShift = DefaultShift>
    void mul(T val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (static_cast<T>(vec[i]) * val) >> LocalShift;
    }

    template <std::size_t LocalShift = DefaultShift>
    void mul(const Veci<T, VecSize, DefaultShift>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (static_cast<T>(vec[i]) * val[i]) >> LocalShift;
    }

    Veci<T, VecSize, DefaultShift>& operator+=(const Veci<T, VecSize, DefaultShift>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] += val[i];
        return *this;
    }

    Veci<T, VecSize, DefaultShift>& operator-=(const Veci<T, VecSize, DefaultShift>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] -= val[i];
        return *this;
    }

    Veci<T, VecSize, DefaultShift> operator<<=(std::size_t val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] <<= val;
        return *this;
    }

    Veci<T, VecSize, DefaultShift> operator>>=(std::size_t val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] >>= val;
        return *this;
    }

    template <typename TV, std::size_t LocalShift = DefaultShift>
    static Veci<T, VecSize, DefaultShift> createFromVec(const TV& val)
    {
        Veci<T, VecSize, DefaultShift> vec;
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = static_cast<T>((val[i] * static_cast<float>(1ul << LocalShift)) + 0.5f);
        return vec;
    }

    template <typename TV, std::size_t LocalShift = DefaultShift>
    void fromVec(const TV& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = static_cast<T>((val[i] * static_cast<float>(1ul << LocalShift)) + 0.5f);
    }

    template <typename TV, std::size_t LocalShift = DefaultShift>
    static Veci<T, VecSize, DefaultShift> createFromVecToInt(const TV& val)
    {
        Veci<T, VecSize, DefaultShift> vec;
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = static_cast<T>((val[i] * (static_cast<float>(1ul << LocalShift) - 1.0f)) + 0.5f);
        return vec;
    }

    template <typename TV, std::size_t LocalShift = DefaultShift>
    void fromVecToInt(const TV& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = static_cast<T>((val[i] * (static_cast<float>(1ul << LocalShift) - 1.0f)) + 0.5f);
    }

    std::array<float, VecSize> toFloat() const
    {
        std::array<float, VecSize> result;
        for (std::size_t i = 0; i < VecSize; i++)
            result[i] = static_cast<float>(vec[i]) / static_cast<float>(FracMax);
        return result;
    }

    T& operator[](int index) { return vec[index]; }
    T operator[](int index) const { return vec[index]; }
    Veci<T, VecSize, DefaultShift>& operator=(const Veci<T, VecSize, DefaultShift>& val)
    {
        vec = val.vec;
        return *this;
    }
    Veci<T, VecSize, DefaultShift>& operator=(const std::array<T, VecSize>& val)
    {
        vec = val;
        return *this;
    }

    template <std::size_t LocalShift = DefaultShift>
    T dot(const Veci<T, VecSize, DefaultShift>& val) const
    {
        T retVal = 0;
        for (std::size_t i = 0; i < VecSize; i++)
            retVal += (static_cast<T>(vec[i]) * val[i]);
        return retVal >> LocalShift;
    }

    void clamp(const T low, const T high)
    {
        for (std::size_t i = 0; i < VecSize; i++)
        {
            vec[i] = std::clamp(vec[i], low, high);
        }
    }

    static Veci<T, VecSize, DefaultShift> interpolate(
        const Veci<T, VecSize, DefaultShift>& a,
        const Veci<T, VecSize, DefaultShift>& b,
        const T factor)
    {
        Veci<T, VecSize, DefaultShift> t;
        for (std::size_t i = 0; i < VecSize; i++)
            t[i] = a[i] + (((b[i] - a[i]) * factor) >> DefaultShift);
        return t;
    }

    const T* data() const
    {
        return vec.data();
    }

private:
    std::array<T, VecSize> vec {};
};

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator*(const Veci<T, VecSize, DefaultShift>& lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = (lhs[i] * rhs[i]) >> DefaultShift;
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator*(const T lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = (lhs * rhs[i]) >> DefaultShift;
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator*(const Veci<T, VecSize, DefaultShift>& lhs, const T rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = (lhs[i] * rhs) >> DefaultShift;
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator-(const Veci<T, VecSize, DefaultShift>& lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = lhs[i] - rhs[i];
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator+(const Veci<T, VecSize, DefaultShift>& lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = lhs[i] + rhs[i];
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline bool operator==(const rr::Veci<T, VecSize, DefaultShift>& lhs, const rr::Veci<T, VecSize, DefaultShift>& rhs)
{
    return std::equal(lhs.vec.begin(), lhs.vec.end(), rhs.vec.begin());
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline bool operator!=(const rr::Veci<T, VecSize, DefaultShift>& lhs, const rr::Veci<T, VecSize, DefaultShift>& rhs)
{
    return !(lhs == rhs);
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator&(const Veci<T, VecSize, DefaultShift> lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = lhs[i] & rhs[i];
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator|(const Veci<T, VecSize, DefaultShift>& lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = lhs[i] | rhs[i];
    return t;
}

template <typename T, std::size_t VecSize, std::size_t DefaultShift>
inline Veci<T, VecSize, DefaultShift> operator^(const Veci<T, VecSize, DefaultShift>& lhs, const Veci<T, VecSize, DefaultShift>& rhs)
{
    Veci<T, VecSize, DefaultShift> t;
    for (std::size_t i = 0; i < VecSize; i++)
        t[i] = lhs[i] ^ rhs[i];
    return t;
}

using VecInt = int32_t;
using Vec2i = Veci<VecInt, 2, 0>;
using Vec3i = Veci<VecInt, 3, 0>;
using Vec4i = Veci<VecInt, 4, 0>;

using Vec4iColorRGBA = Veci<int16_t, 4, 8>;
using Vec3iColorRGB = Veci<int16_t, 3, 8>;
using Vec1iColorR = Veci<int16_t, 1, 8>;

// Interpolation presets here have a direct impact on the image fidelity
// They try to get the maximum, but higher precision will increase the quality.
using Vec3iTexInterp = Veci<int32_t, 3, 28>;
using Vec4iColorInterp = Veci<int32_t, 4, 24>;
using Vec2iDepthInterp = Veci<int32_t, 2, 30>;
} // namespace rr
#endif // VECI_HPP
