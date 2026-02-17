// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

#ifndef ARRAY_TO_PTR_ARRAY_HPP
#define ARRAY_TO_PTR_ARRAY_HPP

#include <array>
#include <cstddef>
#include <utility>

namespace rr
{

namespace detail
{

    template <typename PtrType, typename T, std::size_t N, std::size_t... Is>
    constexpr std::array<PtrType*, N> arrayToPtrArrayImpl(std::array<T, N>& arr, std::index_sequence<Is...>)
    {
        return { { &arr[Is]... } };
    }

} // namespace detail

/// Converts an array of objects to an array of pointers to those objects.
/// @tparam PtrType The pointer type to use (defaults to T, can be a base class)
/// @tparam T The element type of the source array
/// @tparam N The size of the array
/// @param arr The source array
/// @return An array of pointers to each element in the source array
template <typename PtrType = void, typename T, std::size_t N>
constexpr auto arrayToPtrArray(std::array<T, N>& arr)
{
    using ResultPtrType = std::conditional_t<std::is_void_v<PtrType>, T, PtrType>;
    return detail::arrayToPtrArrayImpl<ResultPtrType>(arr, std::make_index_sequence<N> {});
}

} // namespace rr

#endif // ARRAY_TO_PTR_ARRAY_HPP
