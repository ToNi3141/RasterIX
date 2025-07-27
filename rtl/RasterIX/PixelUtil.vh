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

`ifndef PIXEL_UTIL_VH
`define PIXEL_UTIL_VH

`define Saturate(FuncName, ElementWidth) \
    function [ElementWidth - 1 : 0] FuncName; \
        input [ElementWidth * 2 : 0] subpixel; \
        FuncName = (subpixel[ElementWidth * 2]) ? {ElementWidth{1'b1}} \
                                                : subpixel[(ElementWidth * 2) - 1 : ElementWidth]; \
    endfunction

// Gets an signed integer S1.X and clamps and satureates it to a S0.Y number.
`define ReduceAndSaturateSigned(FuncName, SubPixelWidthIn, SubPixelWidthOut) \
    function automatic [SubPixelWidthOut - 1 : 0] FuncName; \
        input [SubPixelWidthIn - 1 : 0] subpixel; \
        localparam Diff = SubPixelWidthIn - SubPixelWidthOut; \
        if (subpixel[SubPixelWidthIn - 1]) // check sign \
        begin \
            // In the negative case, clamp to 'b10000... \
            FuncName = (&subpixel[SubPixelWidthOut - 1 +: Diff])    ? { subpixel[SubPixelWidthIn - 1], subpixel[0 +: SubPixelWidthOut - 1] } \
                                                                    : { subpixel[SubPixelWidthIn - 1], { (SubPixelWidthOut - 1){1'b0} } }; \
        end \
        else \
        begin \
            // In the positive case, clamp to 'b01111... \
            FuncName = (|subpixel[SubPixelWidthOut - 1 +: Diff])    ? { subpixel[SubPixelWidthIn - 1], { (SubPixelWidthOut - 1){1'b1} } } \
                                                                    : { subpixel[SubPixelWidthIn - 1], subpixel[0 +: SubPixelWidthOut - 1] }; \
        end \
    endfunction


// Expands the size of a signed integer.
`define ExpandSigned(FuncName, SubPixelWidthIn, SubPixelWidthOut) \
    function [SubPixelWidthOut - 1 : 0] FuncName; \
        input [SubPixelWidthIn - 1 : 0] subpixel; \
        FuncName = { { (SubPixelWidthOut - SubPixelWidthIn){subpixel[SubPixelWidthIn - 1]} }, subpixel}; \
    endfunction

// Casts a signed integer to an unsigned by removing the sign.
// In case the integer is negative, it clamps it to zero.
`define SaturateCastSignedToUnsigned(FuncName, SubPixelWidthSigned) \
    function [SubPixelWidthSigned - 2 : 0] FuncName; // with removed sign \
        input [SubPixelWidthSigned - 1 : 0] subpixel; \
        if (subpixel[SubPixelWidthSigned - 1]) // check sign \
        begin \
            // in case the value is negative, clamp to zero \
            FuncName = 0; \
        end \
        else \
        begin \
            // In the positive case, just remove the sign \
            FuncName = subpixel[0 +: SubPixelWidthSigned - 1]; \
        end \
    endfunction

// Expands a pixel from a given bit width to a new bit width.
// For instance, given is a vector with 4 bit elements and the new width is 6 bit.
// Assume a[3 : 0] and b[5 : 0].  
// a ElementWidth is 2 bit and b NewElementWidth is 3 bit.
// The number of elements is 2.
// Then the result will be:
// b = { a[3 : 2], a[3 : 3], a[1 : 0], a[1 : 1] }
`define Expand(FuncName, ElementWidth, NewElementWidth, NumberOfElements) \
    function automatic [(NewElementWidth * NumberOfElements) - 1 : 0] FuncName; \
        input [(ElementWidth * NumberOfElements) - 1 : 0] pixel; \
        localparam PIXEL_WIDTH = ElementWidth * NumberOfElements; \
        localparam DIFF_SUB_PIXEL_WIDTH = NewElementWidth - ElementWidth; \
        integer i; \
        if (ElementWidth == NewElementWidth) \
        begin \
            FuncName[0 +: PIXEL_WIDTH] = pixel[0 +: PIXEL_WIDTH];  \
        end \
        else \
        begin \
            for (i = 0; i < NumberOfElements; i = i + 1) \
            begin \
                FuncName[i * NewElementWidth +: NewElementWidth] = {  \
                    pixel[i * ElementWidth +: ElementWidth],  \
                    // Mirror the subpixel in the reminder. \
                    // Assume, expanded sub pixel width is 6 bit and sub pixel width is 4 bit \
                    // Then the expanded sub pixel width will ec[5 : 0] = { c[3 : 0], c[3 : 2] } \
                    pixel[(i * ElementWidth) + (ElementWidth - DIFF_SUB_PIXEL_WIDTH) +: DIFF_SUB_PIXEL_WIDTH] \
                }; \
            end \
        end \
    endfunction

`define Reduce(FuncName, NewElementWidth, ElementWidth, NumberOfElements) \
    function [(NewElementWidth * NumberOfElements) - 1 : 0] FuncName; \
        input [(ElementWidth * NumberOfElements) - 1 : 0] pixel; \
        localparam PIXEL_WIDTH = NewElementWidth * NumberOfElements; \
        localparam DIFF_SUB_PIXEL_WIDTH = ElementWidth - NewElementWidth; \
        integer i; \
        if (NewElementWidth == ElementWidth) \
        begin \
            FuncName[0 +: PIXEL_WIDTH] = pixel[0 +: PIXEL_WIDTH];  \
        end \
        else \
        begin \
            for (i = 0; i < NumberOfElements; i = i + 1) \
            begin \
                FuncName[i * NewElementWidth +: NewElementWidth] = {  \
                    pixel[(ElementWidth * i) + DIFF_SUB_PIXEL_WIDTH +: NewElementWidth]  \
                }; \
            end \
        end \
    endfunction

// Used to reduce a vector.
// ElementWidth: The width of a vector element.
// NumberOfElements: The number of elements the vector contains.
// Offset: Index of the first element which will be removed. If this is 0, the first element is removed. If it is 1, the second element will be removed.
// N: Every very n't element will be removed beginning at the offset. If N is 2, every other element will be removed. If N is 4, one component from an RGBA color can be removed.
// NewNumberOfElements: The size of the vector after the conversion.
`define ReduceVec(FuncName, ElementWidth, NumberOfElements, Offset, N, NewNumberOfElements) \
    function [(ElementWidth * NewNumberOfElements) - 1 : 0] FuncName; \
        input [(ElementWidth * NumberOfElements) - 1 : 0] pixel; \
        integer i, j, k; \
        begin \
            k = 0; \
            for (i = 0; i < NumberOfElements; ) \
            begin \
                if (i >= Offset) \
                begin \
                    for (j = 0; j < N; j = j + 1) \
                    begin \
                        if (j == 0) \
                        begin \
                            i = i + 1; \
                        end \
                        else \
                        begin \
                            FuncName[k * ElementWidth +: ElementWidth] = pixel[i * ElementWidth +: ElementWidth]; \
                            k = k + 1; \
                            i = i + 1; \
                        end \
                    end \
                end \
                else \
                begin \
                    FuncName[k * ElementWidth +: ElementWidth] = pixel[i * ElementWidth +: ElementWidth]; \
                    k = k + 1; \
                    i = i + 1; \
                end \
            end \
        end \
    endfunction

// Used to expand a vector.
// ElementWidth: The width of a vector element.
// NumberOfElements: The number of elements the vector contains.
// Offset: Index of the first element which will be inserted. If this is 0, the first element is inserted at position 0.
// N: At every n't position there will be an element inserted.
// NewNumberOfElements: Size of the vector after all elements have been inserted.
// value: The value of the inserted element.
`define ExpandVec(FuncName, ElementWidth, NumberOfElements, Offset, N, NewNumberOfElements) \
    function [(ElementWidth * NewNumberOfElements) - 1 : 0] FuncName; \
        input [(ElementWidth * NumberOfElements) - 1 : 0] pixel; \
        input [ElementWidth - 1 : 0] value; \
        integer i, j, k; \
        begin \
            k = 0; \
            for (i = 0; i < NumberOfElements; ) \
            begin \
                if (i >= Offset) \
                begin \
                    for (j = 0; j < N; j = j + 1) \
                    begin \
                        if (j == 0) \
                        begin \
                            FuncName[k * ElementWidth +: ElementWidth] = value; \
                            k = k + 1; \
                        end \
                        else \
                        begin \
                            FuncName[k * ElementWidth +: ElementWidth] = pixel[i * ElementWidth +: ElementWidth]; \
                            k = k + 1; \
                            i = i + 1; \
                        end \
                    end \
                end \
                else \
                begin \
                    FuncName[k * ElementWidth +: ElementWidth] = pixel[i * ElementWidth +: ElementWidth]; \
                    k = k + 1; \
                    i = i + 1; \
                end \
            end \
        end \
    endfunction

`define XXX2RGB565(FuncName, ElementWidth, NumberOfPixels) \
    function [(16 * NumberOfPixels) - 1 : 0] FuncName; \
        input [(ElementWidth * 3 * NumberOfPixels) - 1 : 0] pixels; \
        integer i; \
        for (i = 0; i < NumberOfPixels; i = i + 1) \
        begin \
            FuncName[(i * 16) + 0  +: 5] = pixels[(i * ElementWidth * 3) +                      (ElementWidth - 5) +: ElementWidth - (ElementWidth - 5)]; \
            FuncName[(i * 16) + 5  +: 6] = pixels[(i * ElementWidth * 3) + ElementWidth +       (ElementWidth - 6) +: ElementWidth - (ElementWidth - 6)]; \
            FuncName[(i * 16) + 11 +: 5] = pixels[(i * ElementWidth * 3) + (ElementWidth * 2) + (ElementWidth - 5) +: ElementWidth - (ElementWidth - 5)]; \
        end \
    endfunction

`define RGB5652XXX(FuncName, ElementWidth, NumberOfPixels) \
    function [(ElementWidth * 3 * NumberOfPixels) - 1 : 0] FuncName; \
        input [(16 * NumberOfPixels) - 1 : 0] pixels; \
        integer i; \
        for (i = 0; i < NumberOfPixels; i = i + 1) \
        begin \
            FuncName[(i * ElementWidth * 3)                      +: ElementWidth] = { pixels[(i * 16) + 0  +: 5], pixels[(i * 16) + 0  + (ElementWidth - 6) +: (ElementWidth - 5)] }; \
            FuncName[(i * ElementWidth * 3) + ElementWidth       +: ElementWidth] = { pixels[(i * 16) + 5  +: 6], pixels[(i * 16) + 5  + (ElementWidth - 5) +: (ElementWidth - 6)] }; \
            FuncName[(i * ElementWidth * 3) + (ElementWidth * 2) +: ElementWidth] = { pixels[(i * 16) + 11 +: 5], pixels[(i * 16) + 11 + (ElementWidth - 6) +: (ElementWidth - 5)] }; \
        end \
    endfunction

`endif // PIXEL_UTIL_VH