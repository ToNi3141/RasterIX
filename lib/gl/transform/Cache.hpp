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

#ifndef CACHE_HPP
#define CACHE_HPP

#include <array>

namespace cache
{
template <typename T, std::size_t SIZE>
class Cache
{
public:
    Cache() = default;

    bool get(std::size_t index, T& entry)
    {
        VertexCacheEntry& cacheEntry = m_cache[index & (SIZE - 1)];
        if (cacheEntry.valid && cacheEntry.index == index)
        {
            entry = cacheEntry.entry;
            return true;
        }
        return false;
    }

    void put(std::size_t index, const T& entry)
    {
        VertexCacheEntry& cacheEntry = m_cache[index & (SIZE - 1)];
        cacheEntry.index = index;
        cacheEntry.entry = entry;
        cacheEntry.valid = true;
    }

    void reset()
    {
        for (VertexCacheEntry& cacheEntry : m_cache)
        {
            cacheEntry.valid = false;
        }
    }

private:
    struct VertexCacheEntry
    {
        std::size_t index = 0;
        T entry {};
        bool valid = false;
    };

    std::array<VertexCacheEntry, SIZE> m_cache {};
};
} // namespace cache

#endif // CACHE_HPP