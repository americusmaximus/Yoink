/*
Copyright (c) 2024 - 2025 Americus Maximus

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

RADEXPFUNC void RADEXPLINK ExpandBink(void* yp1, void* yp2, void* ap1, void* ap2, void* mp, u32 decompwidth, u32 decompheight, u32 width, u32 height, u32 param_10, u32 key, u32 param_12, u32 flags, u32 type); // TODO
RADEXPFUNC void RADEXPLINK ExpandBundleSizes(struct BUNDLEPOINTERS* pointers, u32 width);
RADEXPFUNC void RADEXPLINK YUV_blit_16a1bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16a1bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16a4bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16a4bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_16bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24bpp_mask(void* dest, u32 destx, u32 desty, s32 destpitch, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24rbpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_24rbpp_mask(void* dest, u32 destx, u32 desty, s32 destpitch, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32abpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32abpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rabpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rabpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rbpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_32rbpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_UYVY(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_UYVY_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_YUY2(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_YUY2_mask(void* dest, u32 destx, u32 desty, s32 destpitch, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12); // TODO
RADEXPFUNC void RADEXPLINK YUV_blit_YV12(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13); // TODO
RADEXPFUNC void RADEXPLINK YUV_init(u32 flags);