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

#ifndef DISPLAYLIST_HPP
#define DISPLAYLIST_HPP

#include <cstring>
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <std::size_t ALIGNMENT>
class GenericDisplayList
{
public:
    void setBuffer(tcb::span<uint8_t> buffer)
    {
        mem = buffer;
    }

    // Interface for writing the display list

    void* __restrict alloc(const std::size_t size)
    {
        if ((size + writePos) <= mem.size())
        {
            void* memPlace = &mem[writePos];
            writePos += size;
            return memPlace;
        }
        return nullptr;
    }

    template <typename GET_TYPE>
    GET_TYPE* __restrict create()
    {
        static constexpr std::size_t size = sizeOf<GET_TYPE>();
        return reinterpret_cast<GET_TYPE* __restrict>(alloc(size));
    }

    template <typename GET_TYPE>
    void remove()
    {
        static constexpr std::size_t size = sizeOf<GET_TYPE>();
        if (size <= writePos)
        {
            writePos -= size;
        }
    }

    template <typename GET_TYPE>
    static constexpr std::size_t sizeOf()
    {
        constexpr uint32_t mask = static_cast<uint32_t>(ALIGNMENT) - 1;
        return (sizeof(GET_TYPE) + mask) & ~mask;
    }

    void clear()
    {
        writePos = 0;
    }

    tcb::span<const uint8_t> getMemPtr() const
    {
        return { mem.data(), getSize() };
    }

    std::size_t getSize() const
    {
        return writePos;
    }

    std::size_t getFreeSpace() const
    {
        return mem.size() - writePos;
    }

    void initArea(const std::size_t start, const std::size_t size)
    {
        memset(&mem[start], 0, size);
    }

    std::size_t getCurrentWritePos() const
    {
        return writePos;
    }

    // Interface for reading the display list

    template <typename GET_TYPE>
    const GET_TYPE* __restrict lookAhead(const std::size_t token = 1) const
    {
        static constexpr std::size_t size = sizeOf<GET_TYPE>();
        if (((size * token) + readPos) <= writePos)
        {
            return reinterpret_cast<GET_TYPE* __restrict>(&mem[readPos + (size * (token - 1))]);
        }
        return nullptr;
    }

    template <typename GET_TYPE>
    const GET_TYPE* __restrict getNext()
    {
        static constexpr std::size_t size = sizeOf<GET_TYPE>();
        if ((size + readPos) <= writePos)
        {
            void* memPlace = &mem[readPos];
            readPos += size;
            return reinterpret_cast<GET_TYPE* __restrict>(memPlace);
        }
        return nullptr;
    }

    bool atEnd() const
    {
        return writePos <= readPos;
    }

    void resetGet()
    {
        readPos = 0;
    }

    void setCurrentSize(const std::size_t size)
    {
        writePos = size;
    }

private:
    tcb::span<uint8_t> mem;
    std::size_t writePos { 0 };
    std::size_t readPos { 0 };
    std::size_t sectionStartPos { 0 };
};

using DisplayList = GenericDisplayList<4>;

} // namespace rr::displaylist
#endif // DISPLAYLIST_HPP
