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

localparam RR_CMD_SIZE = 6,
localparam RR_CMD_NOP = 6'b000000,
localparam RR_CMD_INIT = 6'b000001,
localparam RR_CMD_X_INC = 6'b000010,
localparam RR_CMD_X_DEC = 6'b000100,
localparam RR_CMD_Y_INC = 6'b001000,
localparam RR_CMD_PUSH = 6'b010000,
localparam RR_CMD_POP = 6'b100000,