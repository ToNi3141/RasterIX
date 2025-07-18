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

// Scissor function
function [0 : 0] scissorFunc;
    input                       enable;
    input [X_BIT_WIDTH - 1 : 0] startX;
    input [Y_BIT_WIDTH - 1 : 0] startY;
    input [X_BIT_WIDTH - 1 : 0] endX;
    input [Y_BIT_WIDTH - 1 : 0] endY;
    input [X_BIT_WIDTH - 1 : 0] screenX;
    input [Y_BIT_WIDTH - 1 : 0] screenY;
    begin
        scissorFunc = !enable || ((screenX >= startX) && (screenX < endX) && (screenY >= startY) && (screenY < endY));
    end
endfunction