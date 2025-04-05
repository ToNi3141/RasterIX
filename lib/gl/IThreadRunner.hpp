// Rasterix
// https://github.com/ToNi3141/Rasterix
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

#ifndef ITHREADRUNNER_HPP
#define ITHREADRUNNER_HPP

#include <functional>

namespace rr
{

class IThreadRunner
{
public:
    virtual void wait() = 0;
    virtual void run(const std::function<bool()>& operation) = 0;
};

} // namespace rr

#endif // ITHREADRUNNER_HPP