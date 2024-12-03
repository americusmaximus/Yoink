/*
Copyright (c) 2024 Americus Maximus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "State.hxx"

RADEXPFUNC void RADEXPLINK YUV_blit_16a1bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16a1bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16a4bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16a4bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24rbpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24rbpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32abpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32abpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rabpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rabpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rbpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rbpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_UYVY(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_UYVY_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_YUY2(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_YUY2_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_YV12(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_init(U32 flags);