/*=====================================================================*
 *                   Copyright (C) 2012 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __CAST_H_

#    ifdef __cplusplus
#        define cast_uint32_t static_cast<uint32_t>
#    else
#        define cast_uint32_t (uint32_t)
#    endif

#    define LOGTEN_TWO 3.321928

#endif // __CAST_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __SSE_H_
#    define __SSE_H_

#    ifdef __SSE2__

#        include <emmintrin.h>

#        ifdef __cplusplus
namespace {
#        endif // __cplusplus

    typedef __m128 v4sf;
    typedef __m128i v4si;

#        define v4si_to_v4sf _mm_cvtepi32_ps
#        define v4sf_to_v4si _mm_cvttps_epi32

#        define v4sfl(x) ((const v4sf){(x), (x), (x), (x)})
#        define v2dil(x) ((const v4si){(x), (x)})
#        define v4sil(x) v2dil((((long long) (x)) << 32) | (x))

    typedef union {
        v4sf f;
        float array[4];
    } v4sfindexer;
#        define v4sf_index(_findx, _findi)                                                                             \
            ({                                                                                                         \
                v4sfindexer _findvx = {_findx};                                                                        \
                _findvx.array[_findi];                                                                                 \
            })
    typedef union {
        v4si i;
        int array[4];
    } v4siindexer;
#        define v4si_index(_iindx, _iindi)                                                                             \
            ({                                                                                                         \
                v4siindexer _iindvx = {_iindx};                                                                        \
                _iindvx.array[_iindi];                                                                                 \
            })

    typedef union {
        v4sf f;
        v4si i;
    } v4sfv4sipun;
#        define v4sf_fabs(x)                                                                                           \
            ({                                                                                                         \
                v4sfv4sipun vx;                                                                                        \
                vx.f = x;                                                                                              \
                vx.i &= v4sil(0x7FFFFFFF);                                                                             \
                vx.f;                                                                                                  \
            })

#        ifdef __cplusplus
} // end namespace
#        endif // __cplusplus

#    endif // __SSE2__

#endif // __SSE_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_EXP_H_
#    define __FAST_EXP_H_

#    include <stdint.h>

// Underflow of exponential is common practice in numerical routines,
// so handle it here.

inline static float fastpow2(float p) {
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp  = (p < -126) ? -126.0f : p;
    int w        = clipp;
    float z      = clipp - w + offset;
    union {
        uint32_t i;
        float f;
    } v = {cast_uint32_t((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))};

    return v.f;
}

inline static float fastexp(float p) {
    return fastpow2(1.442695040f * p);
}

inline static float fasterpow2(float p) {
    float clipp = (p < -126) ? -126.0f : p;
    union {
        uint32_t i;
        float f;
    } v = {cast_uint32_t((1 << 23) * (clipp + 126.94269504f))};
    return v.f;
}

inline static float fasterexp(float p) {
    return fasterpow2(1.442695040f * p);
}

#    ifdef __SSE2__

inline static v4sf vfastpow2(v4sf const p) {
    v4sf ltzero = _mm_cmplt_ps(p, v4sfl(0.0f));
    v4sf offset = _mm_and_ps(ltzero, v4sfl(1.0f));
    v4sf lt126  = _mm_cmplt_ps(p, v4sfl(-126.0f));
    v4sf clipp  = _mm_or_ps(_mm_andnot_ps(lt126, p), _mm_and_ps(lt126, v4sfl(-126.0f)));
    v4si w      = v4sf_to_v4si(clipp);
    v4sf z      = clipp - v4si_to_v4sf(w) + offset;

    v4sf const c_121_2740838 = v4sfl(121.2740575f);
    v4sf const c_27_7280233  = v4sfl(27.7280233f);
    v4sf const c_4_84252568  = v4sfl(4.84252568f);
    v4sf const c_1_49012907  = v4sfl(1.49012907f);
    union {
        v4si i;
        v4sf f;
    } v = {
        v4sf_to_v4si(v4sfl(1 << 23) * (clipp + c_121_2740838 + c_27_7280233 / (c_4_84252568 - z) - c_1_49012907 * z))};

    return v.f;
}

inline static v4sf vfastexp(v4sf const p) {
    v4sf const c_invlog_2 = v4sfl(1.442695040f);

    return vfastpow2(c_invlog_2 * p);
}

inline static v4sf vfasterpow2(v4sf const p) {
    v4sf const c_126_94269504 = v4sfl(126.94269504f);
    v4sf lt126                = _mm_cmplt_ps(p, v4sfl(-126.0f));
    v4sf clipp                = _mm_or_ps(_mm_andnot_ps(lt126, p), _mm_and_ps(lt126, v4sfl(-126.0f)));
    union {
        v4si i;
        v4sf f;
    } v = {v4sf_to_v4si(v4sfl(1 << 23) * (clipp + c_126_94269504))};
    return v.f;
}

inline static v4sf vfasterexp(v4sf const p) {
    v4sf const c_invlog_2 = v4sfl(1.442695040f);

    return vfasterpow2(c_invlog_2 * p);
}

#    endif //__SSE2__

#endif // __FAST_EXP_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_LOG_H_
#    define __FAST_LOG_H_

#    include <stdint.h>

inline static float fastlog2(float x) {
    union {
        float f;
        uint32_t i;
    } vx = {x};
    union {
        uint32_t i;
        float f;
    } mx    = {(vx.i & 0x007FFFFF) | 0x3f000000};
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    return (y - 124.22551499f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f)) / LOGTEN_TWO;
}

inline static float fastlog(float x) {
    return 0.69314718f * fastlog2(x);
}

inline static float fasterlog2(float x) {
    union {
        float f;
        uint32_t i;
    } vx    = {x};
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    float tmp = y - 126.94269504f;

    float l10 = tmp / 0.30103;

    std::cerr << "x: " << x << " log2: " << tmp << " log10: " << l10 << std::endl;

    return y - 126.94269504f;
}

inline static float fasterlog(float x) {
    //  return 0.69314718f * fasterlog2 (x);

    union {
        float f;
        uint32_t i;
    } vx    = {x};
    float y = vx.i;
    y *= 8.2629582881927490e-8f;
    return y - 87.989971088f;
}

#    ifdef __SSE2__

inline static v4sf vfastlog2(v4sf x) {
    union {
        v4sf f;
        v4si i;
    } vx = {x};
    union {
        v4si i;
        v4sf f;
    } mx;
    mx.i   = (vx.i & v4sil(0x007FFFFF)) | v4sil(0x3f000000);
    v4sf y = v4si_to_v4sf(vx.i);
    y *= v4sfl(1.1920928955078125e-7f);

    v4sf const c_124_22551499 = v4sfl(124.22551499f);
    v4sf const c_1_498030302  = v4sfl(1.498030302f);
    v4sf const c_1_725877999  = v4sfl(1.72587999f);
    v4sf const c_0_3520087068 = v4sfl(0.3520887068f);

    return y - c_124_22551499 - c_1_498030302 * mx.f - c_1_725877999 / (c_0_3520087068 + mx.f);
}

inline static v4sf vfastlog(v4sf x) {
    v4sf const c_0_69314718 = v4sfl(0.69314718f);

    return c_0_69314718 * vfastlog2(x);
}

inline static v4sf vfasterlog2(v4sf x) {
    union {
        v4sf f;
        v4si i;
    } vx   = {x};
    v4sf y = v4si_to_v4sf(vx.i);
    y *= v4sfl(1.1920928955078125e-7f);

    v4sf const c_126_94269504 = v4sfl(126.94269504f);

    return y - c_126_94269504;
}

inline static v4sf vfasterlog(v4sf x) {
    //  const v4sf c_0_69314718 = v4sfl (0.69314718f);
    //
    //  return c_0_69314718 * vfasterlog2 (x);

    union {
        v4sf f;
        v4si i;
    } vx   = {x};
    v4sf y = v4si_to_v4sf(vx.i);
    y *= v4sfl(8.2629582881927490e-8f);

    v4sf const c_87_989971088 = v4sfl(87.989971088f);

    return y - c_87_989971088;
}

#    endif // __SSE2__

#endif // __FAST_LOG_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_ERF_H_
#    define __FAST_ERF_H_

#    include <math.h>
#    include <stdint.h>

// fasterfc: not actually faster than erfcf(3) on newer machines!
// ... although vectorized version is interesting
//     and fastererfc is very fast

inline static float fasterfc(float x) {
    static float const k = 3.3509633149424609f;
    static float const a = 0.07219054755431126f;
    static float const b = 15.418191568719577f;
    static float const c = 5.609846028328545f;

    union {
        float f;
        uint32_t i;
    } vc        = {c * x};
    float xsq   = x * x;
    float xquad = xsq * xsq;

    vc.i |= 0x80000000;

    return 2.0f / (1.0f + fastpow2(k * x)) - a * x * (b * xquad - 1.0f) * fasterpow2(vc.f);
}

inline static float fastererfc(float x) {
    static float const k = 3.3509633149424609f;

    return 2.0f / (1.0f + fasterpow2(k * x));
}

// fasterf: not actually faster than erff(3) on newer machines!
// ... although vectorized version is interesting
//     and fastererf is very fast

inline static float fasterf(float x) {
    return 1.0f - fasterfc(x);
}

inline static float fastererf(float x) {
    return 1.0f - fastererfc(x);
}

inline static float fastinverseerf(float x) {
    static float const invk = 0.30004578719350504f;
    static float const a    = 0.020287853348211326f;
    static float const b    = 0.07236892874789555f;
    static float const c    = 0.9913030456864257f;
    static float const d    = 0.8059775923760193f;

    float xsq = x * x;

    return invk * fastlog2((1.0f + x) / (1.0f - x)) + x * (a - b * xsq) / (c - d * xsq);
}

inline static float fasterinverseerf(float x) {
    static float const invk = 0.30004578719350504f;

    return invk * fasterlog2((1.0f + x) / (1.0f - x));
}

#    ifdef __SSE2__

inline static v4sf vfasterfc(v4sf x) {
    v4sf const k = v4sfl(3.3509633149424609f);
    v4sf const a = v4sfl(0.07219054755431126f);
    v4sf const b = v4sfl(15.418191568719577f);
    v4sf const c = v4sfl(5.609846028328545f);

    union {
        v4sf f;
        v4si i;
    } vc;
    vc.f = c * x;
    vc.i |= v4sil(0x80000000);

    v4sf xsq   = x * x;
    v4sf xquad = xsq * xsq;

    return v4sfl(2.0f) / (v4sfl(1.0f) + vfastpow2(k * x)) - a * x * (b * xquad - v4sfl(1.0f)) * vfasterpow2(vc.f);
}

inline static v4sf vfastererfc(v4sf const x) {
    v4sf const k = v4sfl(3.3509633149424609f);

    return v4sfl(2.0f) / (v4sfl(1.0f) + vfasterpow2(k * x));
}

inline static v4sf vfasterf(v4sf x) {
    return v4sfl(1.0f) - vfasterfc(x);
}

inline static v4sf vfastererf(v4sf const x) {
    return v4sfl(1.0f) - vfastererfc(x);
}

inline static v4sf vfastinverseerf(v4sf x) {
    v4sf const invk = v4sfl(0.30004578719350504f);
    v4sf const a    = v4sfl(0.020287853348211326f);
    v4sf const b    = v4sfl(0.07236892874789555f);
    v4sf const c    = v4sfl(0.9913030456864257f);
    v4sf const d    = v4sfl(0.8059775923760193f);

    v4sf xsq = x * x;

    return invk * vfastlog2((v4sfl(1.0f) + x) / (v4sfl(1.0f) - x)) + x * (a - b * xsq) / (c - d * xsq);
}

inline static v4sf vfasterinverseerf(v4sf x) {
    v4sf const invk = v4sfl(0.30004578719350504f);

    return invk * vfasterlog2((v4sfl(1.0f) + x) / (v4sfl(1.0f) - x));
}

#    endif //__SSE2__

#endif // __FAST_ERF_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_GAMMA_H_
#    define __FAST_GAMMA_H_

#    include <stdint.h>

/* gamma/digamma functions only work for positive inputs */

inline static float fastlgamma(float x) {
    float logterm = fastlog(x * (1.0f + x) * (2.0f + x));
    float xp3     = 3.0f + x;

    return -2.081061466f - x + 0.0833333f / xp3 - logterm + (2.5f + x) * fastlog(xp3);
}

inline static float fasterlgamma(float x) {
    return -0.0810614667f - x - fasterlog(x) + (0.5f + x) * fasterlog(1.0f + x);
}

inline static float fastdigamma(float x) {
    float twopx   = 2.0f + x;
    float logterm = fastlog(twopx);

    return (-48.0f + x * (-157.0f + x * (-127.0f - 30.0f * x))) / (12.0f * x * (1.0f + x) * twopx * twopx) + logterm;
}

inline static float fasterdigamma(float x) {
    float onepx = 1.0f + x;

    return -1.0f / x - 1.0f / (2 * onepx) + fasterlog(onepx);
}

#    ifdef __SSE2__

inline static v4sf vfastlgamma(v4sf x) {
    v4sf const c_1_0         = v4sfl(1.0f);
    v4sf const c_2_0         = v4sfl(2.0f);
    v4sf const c_3_0         = v4sfl(3.0f);
    v4sf const c_2_081061466 = v4sfl(2.081061466f);
    v4sf const c_0_0833333   = v4sfl(0.0833333f);
    v4sf const c_2_5         = v4sfl(2.5f);

    v4sf logterm = vfastlog(x * (c_1_0 + x) * (c_2_0 + x));
    v4sf xp3     = c_3_0 + x;

    return -c_2_081061466 - x + c_0_0833333 / xp3 - logterm + (c_2_5 + x) * vfastlog(xp3);
}

inline static v4sf vfasterlgamma(v4sf x) {
    v4sf const c_0_0810614667 = v4sfl(0.0810614667f);
    v4sf const c_0_5          = v4sfl(0.5f);
    v4sf const c_1            = v4sfl(1.0f);

    return -c_0_0810614667 - x - vfasterlog(x) + (c_0_5 + x) * vfasterlog(c_1 + x);
}

inline static v4sf vfastdigamma(v4sf x) {
    v4sf twopx   = v4sfl(2.0f) + x;
    v4sf logterm = vfastlog(twopx);

    return (v4sfl(-48.0f) + x * (v4sfl(-157.0f) + x * (v4sfl(-127.0f) - v4sfl(30.0f) * x))) /
               (v4sfl(12.0f) * x * (v4sfl(1.0f) + x) * twopx * twopx) +
           logterm;
}

inline static v4sf vfasterdigamma(v4sf x) {
    v4sf const c_1_0 = v4sfl(1.0f);
    v4sf const c_2_0 = v4sfl(2.0f);
    v4sf onepx       = c_1_0 + x;

    return -c_1_0 / x - c_1_0 / (c_2_0 * onepx) + vfasterlog(onepx);
}

#    endif //__SSE2__

#endif // __FAST_GAMMA_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_HYPERBOLIC_H_
#    define __FAST_HYPERBOLIC_H_

#    include <stdint.h>

inline static float fastsinh(float p) {
    return 0.5f * (fastexp(p) - fastexp(-p));
}

inline static float fastersinh(float p) {
    return 0.5f * (fasterexp(p) - fasterexp(-p));
}

inline static float fastcosh(float p) {
    return 0.5f * (fastexp(p) + fastexp(-p));
}

inline static float fastercosh(float p) {
    return 0.5f * (fasterexp(p) + fasterexp(-p));
}

inline static float fasttanh(float p) {
    return -1.0f + 2.0f / (1.0f + fastexp(-2.0f * p));
}

inline static float fastertanh(float p) {
    return -1.0f + 2.0f / (1.0f + fasterexp(-2.0f * p));
}

#    ifdef __SSE2__

inline static v4sf vfastsinh(v4sf const p) {
    v4sf const c_0_5 = v4sfl(0.5f);

    return c_0_5 * (vfastexp(p) - vfastexp(-p));
}

inline static v4sf vfastersinh(v4sf const p) {
    v4sf const c_0_5 = v4sfl(0.5f);

    return c_0_5 * (vfasterexp(p) - vfasterexp(-p));
}

inline static v4sf vfastcosh(v4sf const p) {
    v4sf const c_0_5 = v4sfl(0.5f);

    return c_0_5 * (vfastexp(p) + vfastexp(-p));
}

inline static v4sf vfastercosh(v4sf const p) {
    v4sf const c_0_5 = v4sfl(0.5f);

    return c_0_5 * (vfasterexp(p) + vfasterexp(-p));
}

inline static v4sf vfasttanh(v4sf const p) {
    v4sf const c_1 = v4sfl(1.0f);
    v4sf const c_2 = v4sfl(2.0f);

    return -c_1 + c_2 / (c_1 + vfastexp(-c_2 * p));
}

inline static v4sf vfastertanh(v4sf const p) {
    v4sf const c_1 = v4sfl(1.0f);
    v4sf const c_2 = v4sfl(2.0f);

    return -c_1 + c_2 / (c_1 + vfasterexp(-c_2 * p));
}

#    endif //__SSE2__

#endif // __FAST_HYPERBOLIC_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_LAMBERT_W_H_
#    define __FAST_LAMBERT_W_H_

#    include <stdint.h>

// these functions compute the upper branch aka W_0

inline static float fastlambertw(float x) {
    static float const threshold = 2.26445f;

    float c = (x < threshold) ? 1.546865557f : 1.0f;
    float d = (x < threshold) ? 2.250366841f : 0.0f;
    float a = (x < threshold) ? -0.737769969f : 0.0f;

    float logterm    = fastlog(c * x + d);
    float loglogterm = fastlog(logterm);

    float minusw     = -a - logterm + loglogterm - loglogterm / logterm;
    float expminusw  = fastexp(minusw);
    float xexpminusw = x * expminusw;
    float pexpminusw = xexpminusw - minusw;

    return (2.0f * xexpminusw - minusw * (4.0f * xexpminusw - minusw * pexpminusw)) /
           (2.0f + pexpminusw * (2.0f - minusw));
}

inline static float fasterlambertw(float x) {
    static float const threshold = 2.26445f;

    float c = (x < threshold) ? 1.546865557f : 1.0f;
    float d = (x < threshold) ? 2.250366841f : 0.0f;
    float a = (x < threshold) ? -0.737769969f : 0.0f;

    float logterm    = fasterlog(c * x + d);
    float loglogterm = fasterlog(logterm);

    float w    = a + logterm - loglogterm + loglogterm / logterm;
    float expw = fasterexp(-w);

    return (w * w + expw * x) / (1.0f + w);
}

inline static float fastlambertwexpx(float x) {
    static float const k = 1.1765631309f;
    static float const a = 0.94537622168f;

    float logarg = fmaxf(x, k);
    float powarg = (x < k) ? a * (x - k) : 0;

    float logterm = fastlog(logarg);
    float powterm = fasterpow2(powarg); // don't need accuracy here

    float w    = powterm * (logarg - logterm + logterm / logarg);
    float logw = fastlog(w);
    float p    = x - logw;

    return w * (2.0f + p + w * (3.0f + 2.0f * p)) / (2.0f - p + w * (5.0f + 2.0f * w));
}

inline static float fasterlambertwexpx(float x) {
    static float const k = 1.1765631309f;
    static float const a = 0.94537622168f;

    float logarg = fmaxf(x, k);
    float powarg = (x < k) ? a * (x - k) : 0;

    float logterm = fasterlog(logarg);
    float powterm = fasterpow2(powarg);

    float w    = powterm * (logarg - logterm + logterm / logarg);
    float logw = fasterlog(w);

    return w * (1.0f + x - logw) / (1.0f + w);
}

#    ifdef __SSE2__

inline static v4sf vfastlambertw(v4sf x) {
    v4sf const threshold = v4sfl(2.26445f);

    v4sf under = _mm_cmplt_ps(x, threshold);
    v4sf c     = _mm_or_ps(_mm_and_ps(under, v4sfl(1.546865557f)), _mm_andnot_ps(under, v4sfl(1.0f)));
    v4sf d     = _mm_and_ps(under, v4sfl(2.250366841f));
    v4sf a     = _mm_and_ps(under, v4sfl(-0.737769969f));

    v4sf logterm    = vfastlog(c * x + d);
    v4sf loglogterm = vfastlog(logterm);

    v4sf minusw     = -a - logterm + loglogterm - loglogterm / logterm;
    v4sf expminusw  = vfastexp(minusw);
    v4sf xexpminusw = x * expminusw;
    v4sf pexpminusw = xexpminusw - minusw;

    return (v4sfl(2.0f) * xexpminusw - minusw * (v4sfl(4.0f) * xexpminusw - minusw * pexpminusw)) /
           (v4sfl(2.0f) + pexpminusw * (v4sfl(2.0f) - minusw));
}

inline static v4sf vfasterlambertw(v4sf x) {
    v4sf const threshold = v4sfl(2.26445f);

    v4sf under = _mm_cmplt_ps(x, threshold);
    v4sf c     = _mm_or_ps(_mm_and_ps(under, v4sfl(1.546865557f)), _mm_andnot_ps(under, v4sfl(1.0f)));
    v4sf d     = _mm_and_ps(under, v4sfl(2.250366841f));
    v4sf a     = _mm_and_ps(under, v4sfl(-0.737769969f));

    v4sf logterm    = vfasterlog(c * x + d);
    v4sf loglogterm = vfasterlog(logterm);

    v4sf w    = a + logterm - loglogterm + loglogterm / logterm;
    v4sf expw = vfasterexp(-w);

    return (w * w + expw * x) / (v4sfl(1.0f) + w);
}

inline static v4sf vfastlambertwexpx(v4sf x) {
    v4sf const k     = v4sfl(1.1765631309f);
    v4sf const a     = v4sfl(0.94537622168f);
    v4sf const two   = v4sfl(2.0f);
    v4sf const three = v4sfl(3.0f);
    v4sf const five  = v4sfl(5.0f);

    v4sf logarg = _mm_max_ps(x, k);
    v4sf powarg = _mm_and_ps(_mm_cmplt_ps(x, k), a * (x - k));

    v4sf logterm = vfastlog(logarg);
    v4sf powterm = vfasterpow2(powarg); // don't need accuracy here

    v4sf w    = powterm * (logarg - logterm + logterm / logarg);
    v4sf logw = vfastlog(w);
    v4sf p    = x - logw;

    return w * (two + p + w * (three + two * p)) / (two - p + w * (five + two * w));
}

inline static v4sf vfasterlambertwexpx(v4sf x) {
    v4sf const k = v4sfl(1.1765631309f);
    v4sf const a = v4sfl(0.94537622168f);

    v4sf logarg = _mm_max_ps(x, k);
    v4sf powarg = _mm_and_ps(_mm_cmplt_ps(x, k), a * (x - k));

    v4sf logterm = vfasterlog(logarg);
    v4sf powterm = vfasterpow2(powarg);

    v4sf w    = powterm * (logarg - logterm + logterm / logarg);
    v4sf logw = vfasterlog(w);

    return w * (v4sfl(1.0f) + x - logw) / (v4sfl(1.0f) + w);
}

#    endif // __SSE2__

#endif // __FAST_LAMBERT_W_H_

/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_POW_H_
#    define __FAST_POW_H_

#    include <stdint.h>

inline static float fastpow(float x, float p) {
    return fastpow2(p * fastlog2(x));
}

inline static float fasterpow(float x, float p) {
    return fasterpow2(p * fasterlog2(x));
}

#    ifdef __SSE2__

inline static v4sf vfastpow(v4sf const x, v4sf const p) {
    return vfastpow2(p * vfastlog2(x));
}

inline static v4sf vfasterpow(v4sf const x, v4sf const p) {
    return vfasterpow2(p * vfasterlog2(x));
}

#    endif //__SSE2__

#endif // __FAST_POW_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_SIGMOID_H_
#    define __FAST_SIGMOID_H_

#    include <stdint.h>

inline static float fastsigmoid(float x) {
    return 1.0f / (1.0f + fastexp(-x));
}

inline static float fastersigmoid(float x) {
    return 1.0f / (1.0f + fasterexp(-x));
}

#    ifdef __SSE2__

inline static v4sf vfastsigmoid(v4sf const x) {
    v4sf const c_1 = v4sfl(1.0f);

    return c_1 / (c_1 + vfastexp(-x));
}

inline static v4sf vfastersigmoid(v4sf const x) {
    v4sf const c_1 = v4sfl(1.0f);

    return c_1 / (c_1 + vfasterexp(-x));
}

#    endif //__SSE2__

#endif // __FAST_SIGMOID_H_
/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef __FAST_TRIG_H_
#    define __FAST_TRIG_H_

#    include <stdint.h>

// http://www.devmaster.net/forums/showthread.php?t=5784
// fast sine variants are for x \in [ -\pi, pi ]
// fast cosine variants are for x \in [ -\pi, pi ]
// fast tangent variants are for x \in [ -\pi / 2, pi / 2 ]
// "full" versions of functions handle the entire range of inputs
// although the range reduction technique used here will be hopelessly
// inaccurate for |x| >> 1000
//
// WARNING: fastsinfull, fastcosfull, and fasttanfull can be slower than
// libc calls on older machines (!) and on newer machines are only
// slighly faster.  however:
//   * vectorized versions are competitive
//   * faster full versions are competitive

inline static float fastsin(float x) {
    static float const fouroverpi   = 1.2732395447351627f;
    static float const fouroverpisq = 0.40528473456935109f;
    static float const q            = 0.78444488374548933f;
    union {
        float f;
        uint32_t i;
    } p = {0.20363937680730309f};
    union {
        float f;
        uint32_t i;
    } r = {0.015124940802184233f};
    union {
        float f;
        uint32_t i;
    } s = {-0.0032225901625579573f};

    union {
        float f;
        uint32_t i;
    } vx          = {x};
    uint32_t sign = vx.i & 0x80000000;
    vx.i          = vx.i & 0x7FFFFFFF;

    float qpprox   = fouroverpi * x - fouroverpisq * x * vx.f;
    float qpproxsq = qpprox * qpprox;

    p.i |= sign;
    r.i |= sign;
    s.i ^= sign;

    return q * qpprox + qpproxsq * (p.f + qpproxsq * (r.f + qpproxsq * s.f));
}

inline static float fastersin(float x) {
    static float const fouroverpi   = 1.2732395447351627f;
    static float const fouroverpisq = 0.40528473456935109f;
    static float const q            = 0.77633023248007499f;
    union {
        float f;
        uint32_t i;
    } p = {0.22308510060189463f};

    union {
        float f;
        uint32_t i;
    } vx          = {x};
    uint32_t sign = vx.i & 0x80000000;
    vx.i &= 0x7FFFFFFF;

    float qpprox = fouroverpi * x - fouroverpisq * x * vx.f;

    p.i |= sign;

    return qpprox * (q + p.f * qpprox);
}

inline static float fastsinfull(float x) {
    static float const twopi    = 6.2831853071795865f;
    static float const invtwopi = 0.15915494309189534f;

    int k      = x * invtwopi;
    float half = (x < 0) ? -0.5f : 0.5f;
    return fastsin((half + k) * twopi - x);
}

inline static float fastersinfull(float x) {
    static float const twopi    = 6.2831853071795865f;
    static float const invtwopi = 0.15915494309189534f;

    int k      = x * invtwopi;
    float half = (x < 0) ? -0.5f : 0.5f;
    return fastersin((half + k) * twopi - x);
}

inline static float fastcos(float x) {
    static float const halfpi           = 1.5707963267948966f;
    static float const halfpiminustwopi = -4.7123889803846899f;
    float offset                        = (x > halfpi) ? halfpiminustwopi : halfpi;
    return fastsin(x + offset);
}

inline static float fastercos(float x) {
    static float const twooverpi = 0.63661977236758134f;
    static float const p         = 0.54641335845679634f;

    union {
        float f;
        uint32_t i;
    } vx = {x};
    vx.i &= 0x7FFFFFFF;

    float qpprox = 1.0f - twooverpi * vx.f;

    return qpprox + p * qpprox * (1.0f - qpprox * qpprox);
}

inline static float fastcosfull(float x) {
    static float const halfpi = 1.5707963267948966f;
    return fastsinfull(x + halfpi);
}

inline static float fastercosfull(float x) {
    static float const halfpi = 1.5707963267948966f;
    return fastersinfull(x + halfpi);
}

inline static float fasttan(float x) {
    static float const halfpi = 1.5707963267948966f;
    return fastsin(x) / fastsin(x + halfpi);
}

inline static float fastertan(float x) {
    return fastersin(x) / fastercos(x);
}

inline static float fasttanfull(float x) {
    static float const twopi    = 6.2831853071795865f;
    static float const invtwopi = 0.15915494309189534f;

    int k      = x * invtwopi;
    float half = (x < 0) ? -0.5f : 0.5f;
    float xnew = x - (half + k) * twopi;

    return fastsin(xnew) / fastcos(xnew);
}

inline static float fastertanfull(float x) {
    static float const twopi    = 6.2831853071795865f;
    static float const invtwopi = 0.15915494309189534f;

    int k      = x * invtwopi;
    float half = (x < 0) ? -0.5f : 0.5f;
    float xnew = x - (half + k) * twopi;

    return fastersin(xnew) / fastercos(xnew);
}

#    ifdef __SSE2__

inline static v4sf vfastsin(v4sf const x) {
    v4sf const fouroverpi   = v4sfl(1.2732395447351627f);
    v4sf const fouroverpisq = v4sfl(0.40528473456935109f);
    v4sf const q            = v4sfl(0.78444488374548933f);
    v4sf const p            = v4sfl(0.20363937680730309f);
    v4sf const r            = v4sfl(0.015124940802184233f);
    v4sf const s            = v4sfl(-0.0032225901625579573f);

    union {
        v4sf f;
        v4si i;
    } vx      = {x};
    v4si sign = vx.i & v4sil(0x80000000);
    vx.i &= v4sil(0x7FFFFFFF);

    v4sf qpprox   = fouroverpi * x - fouroverpisq * x * vx.f;
    v4sf qpproxsq = qpprox * qpprox;
    union {
        v4sf f;
        v4si i;
    } vy;
    vy.f = qpproxsq * (p + qpproxsq * (r + qpproxsq * s));
    vy.i ^= sign;

    return q * qpprox + vy.f;
}

inline static v4sf vfastersin(v4sf const x) {
    v4sf const fouroverpi   = v4sfl(1.2732395447351627f);
    v4sf const fouroverpisq = v4sfl(0.40528473456935109f);
    v4sf const q            = v4sfl(0.77633023248007499f);
    v4sf const plit         = v4sfl(0.22308510060189463f);
    union {
        v4sf f;
        v4si i;
    } p = {plit};

    union {
        v4sf f;
        v4si i;
    } vx      = {x};
    v4si sign = vx.i & v4sil(0x80000000);
    vx.i &= v4sil(0x7FFFFFFF);

    v4sf qpprox = fouroverpi * x - fouroverpisq * x * vx.f;

    p.i |= sign;

    return qpprox * (q + p.f * qpprox);
}

inline static v4sf vfastsinfull(v4sf const x) {
    v4sf const twopi    = v4sfl(6.2831853071795865f);
    v4sf const invtwopi = v4sfl(0.15915494309189534f);

    v4si k = v4sf_to_v4si(x * invtwopi);

    v4sf ltzero = _mm_cmplt_ps(x, v4sfl(0.0f));
    v4sf half   = _mm_or_ps(_mm_and_ps(ltzero, v4sfl(-0.5f)), _mm_andnot_ps(ltzero, v4sfl(0.5f)));

    return vfastsin((half + v4si_to_v4sf(k)) * twopi - x);
}

inline static v4sf vfastersinfull(v4sf const x) {
    v4sf const twopi    = v4sfl(6.2831853071795865f);
    v4sf const invtwopi = v4sfl(0.15915494309189534f);

    v4si k = v4sf_to_v4si(x * invtwopi);

    v4sf ltzero = _mm_cmplt_ps(x, v4sfl(0.0f));
    v4sf half   = _mm_or_ps(_mm_and_ps(ltzero, v4sfl(-0.5f)), _mm_andnot_ps(ltzero, v4sfl(0.5f)));

    return vfastersin((half + v4si_to_v4sf(k)) * twopi - x);
}

inline static v4sf vfastcos(v4sf const x) {
    v4sf const halfpi           = v4sfl(1.5707963267948966f);
    v4sf const halfpiminustwopi = v4sfl(-4.7123889803846899f);
    v4sf lthalfpi               = _mm_cmpnlt_ps(x, halfpi);
    v4sf offset                 = _mm_or_ps(_mm_and_ps(lthalfpi, halfpiminustwopi), _mm_andnot_ps(lthalfpi, halfpi));
    return vfastsin(x + offset);
}

inline static v4sf vfastercos(v4sf x) {
    v4sf const twooverpi = v4sfl(0.63661977236758134f);
    v4sf const p         = v4sfl(0.54641335845679634);

    v4sf vx     = v4sf_fabs(x);
    v4sf qpprox = v4sfl(1.0f) - twooverpi * vx;

    return qpprox + p * qpprox * (v4sfl(1.0f) - qpprox * qpprox);
}

inline static v4sf vfastcosfull(v4sf const x) {
    v4sf const halfpi = v4sfl(1.5707963267948966f);
    return vfastsinfull(x + halfpi);
}

inline static v4sf vfastercosfull(v4sf const x) {
    v4sf const halfpi = v4sfl(1.5707963267948966f);
    return vfastersinfull(x + halfpi);
}

inline static v4sf vfasttan(v4sf const x) {
    v4sf const halfpi = v4sfl(1.5707963267948966f);
    return vfastsin(x) / vfastsin(x + halfpi);
}

inline static v4sf vfastertan(v4sf const x) {
    return vfastersin(x) / vfastercos(x);
}

inline static v4sf vfasttanfull(v4sf const x) {
    v4sf const twopi    = v4sfl(6.2831853071795865f);
    v4sf const invtwopi = v4sfl(0.15915494309189534f);

    v4si k = v4sf_to_v4si(x * invtwopi);

    v4sf ltzero = _mm_cmplt_ps(x, v4sfl(0.0f));
    v4sf half   = _mm_or_ps(_mm_and_ps(ltzero, v4sfl(-0.5f)), _mm_andnot_ps(ltzero, v4sfl(0.5f)));
    v4sf xnew   = x - (half + v4si_to_v4sf(k)) * twopi;

    return vfastsin(xnew) / vfastcos(xnew);
}

inline static v4sf vfastertanfull(v4sf const x) {
    v4sf const twopi    = v4sfl(6.2831853071795865f);
    v4sf const invtwopi = v4sfl(0.15915494309189534f);

    v4si k = v4sf_to_v4si(x * invtwopi);

    v4sf ltzero = _mm_cmplt_ps(x, v4sfl(0.0f));
    v4sf half   = _mm_or_ps(_mm_and_ps(ltzero, v4sfl(-0.5f)), _mm_andnot_ps(ltzero, v4sfl(0.5f)));
    v4sf xnew   = x - (half + v4si_to_v4sf(k)) * twopi;

    return vfastersin(xnew) / vfastercos(xnew);
}

#    endif //__SSE2__

#endif // __FAST_TRIG_H_
