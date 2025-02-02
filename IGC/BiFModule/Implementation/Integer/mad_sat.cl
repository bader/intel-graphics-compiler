/*========================== begin_copyright_notice ============================

Copyright (c) 2016-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

============================= end_copyright_notice ===========================*/

#include "../include/BiF_Definitions.cl"
#include "../../Headers/spirv.h"
#include "../include/mul_hilo.cl"

INLINE
char2 __builtin_spirv_OpenCL_s_mad_sat_v2i8_v2i8_v2i8( char2 a,
                                                char2 b,
                                                char2 c )
{
    char2 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
char3 __builtin_spirv_OpenCL_s_mad_sat_v3i8_v3i8_v3i8( char3 a,
                                                char3 b,
                                                char3 c )
{
    char3 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
char4 __builtin_spirv_OpenCL_s_mad_sat_v4i8_v4i8_v4i8( char4 a,
                                                char4 b,
                                                char4 c )
{
    char4 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
char8 __builtin_spirv_OpenCL_s_mad_sat_v8i8_v8i8_v8i8( char8 a,
                                                char8 b,
                                                char8 c )
{
    char8 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
char16 __builtin_spirv_OpenCL_s_mad_sat_v16i8_v16i8_v16i8( char16 a,
                                                    char16 b,
                                                    char16 c )
{
    char16 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_s_mad_sat_i8_i8_i8(a.sf, b.sf, c.sf);
    return temp;
}


INLINE
uchar2 __builtin_spirv_OpenCL_u_mad_sat_v2i8_v2i8_v2i8( uchar2 a,
                                                 uchar2 b,
                                                 uchar2 c )
{
    uchar2 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
uchar3 __builtin_spirv_OpenCL_u_mad_sat_v3i8_v3i8_v3i8( uchar3 a,
                                                 uchar3 b,
                                                 uchar3 c )
{
    uchar3 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
uchar4 __builtin_spirv_OpenCL_u_mad_sat_v4i8_v4i8_v4i8( uchar4 a,
                                                 uchar4 b,
                                                 uchar4 c )
{
    uchar4 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
uchar8 __builtin_spirv_OpenCL_u_mad_sat_v8i8_v8i8_v8i8( uchar8 a,
                                                 uchar8 b,
                                                 uchar8 c )
{
    uchar8 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
uchar16 __builtin_spirv_OpenCL_u_mad_sat_v16i8_v16i8_v16i8( uchar16 a,
                                                     uchar16 b,
                                                     uchar16 c )
{
    uchar16 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_u_mad_sat_i8_i8_i8(a.sf, b.sf, c.sf);
    return temp;
}



INLINE
short2 __builtin_spirv_OpenCL_s_mad_sat_v2i16_v2i16_v2i16( short2 a,
                                                    short2 b,
                                                    short2 c )
{
    short2 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
short3 __builtin_spirv_OpenCL_s_mad_sat_v3i16_v3i16_v3i16( short3 a,
                                                    short3 b,
                                                    short3 c )
{
    short3 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
short4 __builtin_spirv_OpenCL_s_mad_sat_v4i16_v4i16_v4i16( short4 a,
                                                    short4 b,
                                                    short4 c )
{
    short4 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
short8 __builtin_spirv_OpenCL_s_mad_sat_v8i16_v8i16_v8i16( short8 a,
                                                    short8 b,
                                                    short8 c )
{
    short8 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
short16 __builtin_spirv_OpenCL_s_mad_sat_v16i16_v16i16_v16i16( short16 a,
                                                        short16 b,
                                                        short16 c )
{
    short16 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_s_mad_sat_i16_i16_i16(a.sf, b.sf, c.sf);
    return temp;
}



INLINE
ushort2 __builtin_spirv_OpenCL_u_mad_sat_v2i16_v2i16_v2i16( ushort2 a,
                                                     ushort2 b,
                                                     ushort2 c )
{
    ushort2 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
ushort3 __builtin_spirv_OpenCL_u_mad_sat_v3i16_v3i16_v3i16( ushort3 a,
                                                     ushort3 b,
                                                     ushort3 c )
{
    ushort3 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
ushort4 __builtin_spirv_OpenCL_u_mad_sat_v4i16_v4i16_v4i16( ushort4 a,
                                                     ushort4 b,
                                                     ushort4 c )
{
    ushort4 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
ushort8 __builtin_spirv_OpenCL_u_mad_sat_v8i16_v8i16_v8i16( ushort8 a,
                                                     ushort8 b,
                                                     ushort8 c )
{
    ushort8 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
ushort16 __builtin_spirv_OpenCL_u_mad_sat_v16i16_v16i16_v16i16( ushort16 a,
                                                         ushort16 b,
                                                         ushort16 c )
{
    ushort16 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_u_mad_sat_i16_i16_i16(a.sf, b.sf, c.sf);
    return temp;
}

INLINE
int2 __builtin_spirv_OpenCL_s_mad_sat_v2i32_v2i32_v2i32( int2 a,
                                                  int2 b,
                                                  int2 c )
{
    int2 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
int3 __builtin_spirv_OpenCL_s_mad_sat_v3i32_v3i32_v3i32( int3 a,
                                                  int3 b,
                                                  int3 c )
{
    int3 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
int4 __builtin_spirv_OpenCL_s_mad_sat_v4i32_v4i32_v4i32( int4 a,
                                                  int4 b,
                                                  int4 c )
{
    int4 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
int8 __builtin_spirv_OpenCL_s_mad_sat_v8i32_v8i32_v8i32( int8 a,
                                                  int8 b,
                                                  int8 c )
{
    int8 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
int16 __builtin_spirv_OpenCL_s_mad_sat_v16i32_v16i32_v16i32( int16 a,
                                                      int16 b,
                                                      int16 c )
{
    int16 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_s_mad_sat_i32_i32_i32(a.sf, b.sf, c.sf);
    return temp;
}



INLINE
uint2 __builtin_spirv_OpenCL_u_mad_sat_v2i32_v2i32_v2i32( uint2 a,
                                                   uint2 b,
                                                   uint2 c )
{
    uint2 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
uint3 __builtin_spirv_OpenCL_u_mad_sat_v3i32_v3i32_v3i32( uint3 a,
                                                   uint3 b,
                                                   uint3 c )
{
    uint3 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
uint4 __builtin_spirv_OpenCL_u_mad_sat_v4i32_v4i32_v4i32( uint4 a,
                                                   uint4 b,
                                                   uint4 c )
{
    uint4 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
uint8 __builtin_spirv_OpenCL_u_mad_sat_v8i32_v8i32_v8i32( uint8 a,
                                                   uint8 b,
                                                   uint8 c )
{
    uint8 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
uint16 __builtin_spirv_OpenCL_u_mad_sat_v16i32_v16i32_v16i32( uint16 a,
                                                       uint16 b,
                                                       uint16 c )
{
    uint16 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_u_mad_sat_i32_i32_i32(a.sf, b.sf, c.sf);
    return temp;
}

INLINE
long __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64( long a,
                                            long b,
                                            long c )
{
    long lo;
    long hi;
    hi = __builtin_spirv___intc_mul_hilo_i64_i64_p0i64(a, b, &lo);
    long result_lo = lo + c;
    if (c >= 0)
    {
        if (result_lo < lo)
            hi++;
    }
    else
    {
        if (result_lo > lo)
            hi--;
    }
    return  (hi == 0)  ?    result_lo    :
            (hi <  0)  ?    LONG_MIN :
                            LONG_MAX;
}

INLINE
long2 __builtin_spirv_OpenCL_s_mad_sat_v2i64_v2i64_v2i64( long2 a,
                                                   long2 b,
                                                   long2 c )
{
    long2 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
long3 __builtin_spirv_OpenCL_s_mad_sat_v3i64_v3i64_v3i64( long3 a,
                                                   long3 b,
                                                   long3 c )
{
    long3 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
long4 __builtin_spirv_OpenCL_s_mad_sat_v4i64_v4i64_v4i64( long4 a,
                                                   long4 b,
                                                   long4 c )
{
    long4 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
long8 __builtin_spirv_OpenCL_s_mad_sat_v8i64_v8i64_v8i64( long8 a,
                                                   long8 b,
                                                   long8 c )
{
    long8 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
long16 __builtin_spirv_OpenCL_s_mad_sat_v16i64_v16i64_v16i64( long16 a,
                                                       long16 b,
                                                       long16 c )
{
    long16 temp;
    temp.s0 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_s_mad_sat_i64_i64_i64(a.sf, b.sf, c.sf);
    return temp;
}

INLINE
ulong __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64( ulong a,
                                             ulong b,
                                             ulong c )
{
    ulong lo;
    ulong hi;
    hi = __builtin_spirv___intc_umul_hilo_i64_i64_p0i64(a, b, &lo);
    return (hi == 0) ? __builtin_spirv_OpenCL_u_add_sat_i64_i64(lo, c) :
    ULONG_MAX;
}

INLINE
ulong2 __builtin_spirv_OpenCL_u_mad_sat_v2i64_v2i64_v2i64( ulong2 a,
                                                    ulong2 b,
                                                    ulong2 c )
{
    ulong2 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    return temp;
}

INLINE
ulong3 __builtin_spirv_OpenCL_u_mad_sat_v3i64_v3i64_v3i64( ulong3 a,
                                                    ulong3 b,
                                                    ulong3 c )
{
    ulong3 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    return temp;
}

INLINE
ulong4 __builtin_spirv_OpenCL_u_mad_sat_v4i64_v4i64_v4i64( ulong4 a,
                                                    ulong4 b,
                                                    ulong4 c )
{
    ulong4 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s3, b.s3, c.s3);
    return temp;
}

INLINE
ulong8 __builtin_spirv_OpenCL_u_mad_sat_v8i64_v8i64_v8i64( ulong8 a,
                                                    ulong8 b,
                                                    ulong8 c )
{
    ulong8 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s7, b.s7, c.s7);
    return temp;
}

INLINE
ulong16 __builtin_spirv_OpenCL_u_mad_sat_v16i64_v16i64_v16i64( ulong16 a,
                                                        ulong16 b,
                                                        ulong16 c )
{
    ulong16 temp;
    temp.s0 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s0, b.s0, c.s0);
    temp.s1 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s1, b.s1, c.s1);
    temp.s2 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s2, b.s2, c.s2);
    temp.s3 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s3, b.s3, c.s3);
    temp.s4 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s4, b.s4, c.s4);
    temp.s5 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s5, b.s5, c.s5);
    temp.s6 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s6, b.s6, c.s6);
    temp.s7 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s7, b.s7, c.s7);
    temp.s8 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s8, b.s8, c.s8);
    temp.s9 = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.s9, b.s9, c.s9);
    temp.sa = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.sa, b.sa, c.sa);
    temp.sb = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.sb, b.sb, c.sb);
    temp.sc = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.sc, b.sc, c.sc);
    temp.sd = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.sd, b.sd, c.sd);
    temp.se = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.se, b.se, c.se);
    temp.sf = __builtin_spirv_OpenCL_u_mad_sat_i64_i64_i64(a.sf, b.sf, c.sf);
    return temp;
}

