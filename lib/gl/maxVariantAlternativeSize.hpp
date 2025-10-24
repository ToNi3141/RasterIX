// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <variant>

template <typename Variant, std::size_t... I>
constexpr std::size_t maxVariantSizeImpl(std::index_sequence<I...>)
{
    return std::max({ sizeof(std::variant_alternative_t<I, Variant>)... });
}

template <typename Variant>
constexpr std::size_t maxVariantAlternativeSize()
{
    return maxVariantSizeImpl<Variant>(
        std::make_index_sequence<std::variant_size_v<Variant>> {});
}