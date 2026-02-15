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
    static constexpr T One = static_cast<T>(1) << DefaultShift;
    static constexpr T Half = One >> 1;
    static constexpr T Zero = 0;
    using Type = T;

    Veci() { }
    Veci(const Veci<T, VecSize, DefaultShift>& val) { operator=(val.vec); }
    Veci(const std::array<T, VecSize>& val) { operator=(val); }
    Veci(const std::initializer_list<T> val) { std::copy(val.begin(), val.end(), vec.begin()); }
    ~Veci() { }

    Veci<T, VecSize, DefaultShift>& operator*=(T val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * val;
        return *this;
    }

    Veci<T, VecSize, DefaultShift>& operator*=(const Veci<T, VecSize, DefaultShift>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * val[i];
        return *this;
    }

    template <std::size_t shift>
    void div(T val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (((static_cast<int64_t>(vec[i]) << shift) / static_cast<int64_t>(val)));
    }

    template <std::size_t shift>
    void mul(T val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (static_cast<int64_t>(vec[i]) * val) >> shift;
    }

    template <std::size_t shift>
    void mul(const Veci<T, VecSize, DefaultShift>& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (static_cast<int64_t>(vec[i]) * val[i]) >> shift;
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

    template <typename TV, std::size_t Shift = DefaultShift>
    static Veci<T, VecSize, DefaultShift> createFromVec(const TV& val)
    {
        Veci<T, VecSize, DefaultShift> vec;
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (val[i] * static_cast<float>(1ul << Shift)) + 0.5f;
        return vec;
    }

    template <typename TV, std::size_t Shift = DefaultShift>
    void fromVec(const TV& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (val[i] * static_cast<float>(1ul << Shift)) + 0.5f;
    }

    template <typename TV, std::size_t Shift = DefaultShift>
    static Veci<T, VecSize, DefaultShift> createFromVecToInt(const TV& val)
    {
        Veci<T, VecSize, DefaultShift> vec;
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (val[i] * (static_cast<float>(1ul << Shift) - 1.0f)) + 0.5f;
        return vec;
    }

    template <typename TV, std::size_t Shift = DefaultShift>
    void fromVecToInt(const TV& val)
    {
        for (std::size_t i = 0; i < VecSize; i++)
            vec[i] = (val[i] * (static_cast<float>(1ul << Shift) - 1.0f)) + 0.5f;
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

    template <std::size_t Shift = DefaultShift>
    int64_t dot(const Veci<T, VecSize, DefaultShift>& val) const
    {
        int64_t retVal = 0;
        for (std::size_t i = 0; i < VecSize; i++)
            retVal += (static_cast<int64_t>(vec[i]) * val[i]);
        return retVal >> Shift;
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

using Vec4ui8 = Veci<uint8_t, 4, 0>;
using Vec4i16 = Veci<int16_t, 4, 8>;
} // namespace rr
#endif // VECI_HPP
