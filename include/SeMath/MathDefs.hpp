#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244) // Conversion from 'double' to 'float'
#pragma warning(disable:4702) // unreachable code
#endif

//#include <GFrost/Math/Random.h>

#include <cstdlib>
#include <cmath>
#include <limits>
#include <type_traits>
#include <cstring>

namespace Se
{

#undef M_PI
static const float M_PI = 3.14159265358979323846264338327950288f;
static const float M_PI2 = 2.0f*3.14159265358979323846264338327950288f;
static const float M_HALF_PI = M_PI * 0.5f;
static const int M_MIN_INT = 0x80000000;
static const int M_MAX_INT = 0x7fffffff;
static const unsigned M_MIN_UNSIGNED = 0x00000000;
static const unsigned M_MAX_UNSIGNED = 0xffffffff;

static const float M_EPSILON = 0.000001f;
static const float M_LARGE_EPSILON = 0.00005f;
static const float M_MIN_NEARCLIP = 0.01f;
static const float M_MAX_FOV = 160.0f;
static const float M_LARGE_VALUE = 100000000.0f;
static const float M_INFINITY = (float)HUGE_VAL;
static const float M_DEGTORAD = M_PI / 180.0f;
static const float M_DEGTORAD_2 = M_PI / 360.0f;    // M_DEGTORAD / 2.f
static const float M_RADTODEG = 1.0f / M_DEGTORAD;

/// Intersection test result.
enum Intersection
{
    OUTSIDE,
    INTERSECTS,
    INSIDE
};

/// Check whether two floating point values are equal within accuracy.
template <class T>
inline bool Equals(T lhs, T rhs, float eps = M_EPSILON) { return lhs + eps >= rhs && lhs - eps <= rhs; }

/// Linear interpolation between two values.
template <class T, class U>
inline T Lerp(T lhs, T rhs, U t) { return lhs * (1.0 - t) + rhs * t; }

/// Inverse linear interpolation between two values.
template <class T>
inline T InverseLerp(T lhs, T rhs, T x) { return (x - lhs) / (rhs - lhs); }

/// Return the smaller of two values.
template <class T, class U>
inline T Min(T lhs, U rhs) { return lhs < rhs ? lhs : rhs; }

/// Return the larger of two values.
template <class T, class U>
inline T Max(T lhs, U rhs) { return lhs > rhs ? lhs : rhs; }

/// Return absolute value of a value
template <class T>
inline T Abs(T value) { return value >= 0.0 ? value : -value; }

/// Return the sign of a float (-1, 0 or 1.)
template <class T>
inline T Sign(T value) { return value > 0.0 ? 1.0 : (value < 0.0 ? -1.0 : 0.0); }

/// Convert degrees to radians.
template <class T>
inline T ToRadians(const T degrees) { return M_DEGTORAD * degrees; }

/// Convert radians to degrees.
template <class T>
inline T ToDegrees(const T radians) { return M_RADTODEG * radians; }

/// Return a representation of the specified floating-point value as a single format bit layout.
inline unsigned FloatToRawIntBits(float value)
{
    unsigned u = *((unsigned*)&value);
    return u;
}

/// @brief Convert a double-precision floating-point value to its raw integer bit representation.
/// @param value The double value to convert.
/// @return A pair of unsigned integers representing the low and high bits of the double value.
inline std::pair<unsigned, unsigned> DoubleToRawIntBits(double value)
{
    unsigned long long u = *((unsigned long long*)&value);
    return { static_cast<unsigned>(u & 0xFFFFFFFF), static_cast<unsigned>(u >> 32) };
}

/// Check whether a floating point value is NaN.
template <class T> inline bool IsNaN(T value) { return std::isnan(value); }

/// Check whether a floating point value is positive or negative infinity.
template <class T> inline bool IsInf(T value) { return std::isinf(value); }

/// Clamp a number to a range.
template <class T>
inline T Clamp(T value, T min, T max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}
template <class T>
inline T Lerp(T v0, T v1, T k) {
    return v0 + (v1 - v0) * k;
}

/// Per-component clamp of vector.
template <class T>
inline T VectorClamp(const T& value, const T& min, const T& max)
{
    return VectorMax(min, VectorMin(value, max));
}


/// Smoothly damp between values.
template <class T>
inline T SmoothStep(T lhs, T rhs, T t)
{
    t = Clamp((t - lhs) / (rhs - lhs), T(0.0), T(1.0)); // Saturate t
    return t * t * (3.0 - 2.0 * t);
}



/// Return sine of an angle in degrees.
template <class T> inline T Sin(T angle) { return sin(angle * M_DEGTORAD); }

/// Return cosine of an angle in degrees.
template <class T> inline T Cos(T angle) { return cos(angle * M_DEGTORAD); }

/// Return tangent of an angle in degrees.
template <class T> inline T Tan(T angle) { return tan(angle * M_DEGTORAD); }

/// Return arc sine in degrees.
template <class T> inline T Asin(T x) { return M_RADTODEG * asin(Clamp(x, T(-1.0), T(1.0))); }

/// Return arc cosine in degrees.
template <class T> inline T Acos(T x) { return M_RADTODEG * acos(Clamp(x, T(-1.0), T(1.0))); }

/// Return arc tangent in degrees.
template <class T> inline T Atan(T x) { return M_RADTODEG * atan(x); }

/// Return arc tangent of y/x in degrees.
template <class T> inline T Atan2(T y, T x) { return M_RADTODEG * atan2(y, x); }

/// Return X in power Y.
template <class T> inline T Pow(T x, T y) { return pow(x, y); }

/// Return natural logarithm of X.
template <class T> inline T Ln(T x) { return log(x); }

/// Return square root of X.
template <class T> inline T Sqrt(T x) { return sqrt(x); }

/// Return floating-point remainder of X/Y.
template <class T> inline T Mod(T x, T y) { return fmod(x, y); }

/// Return fractional part of passed value in range [0, 1).
template <class T> inline T Fract(T value) { return value - floor(value); }

/// Round value down.
template <class T> inline T Floor(T x) { return floor(x); }

/// Round value down to nearest number that can be represented as i*y, where i is integer.
template <class T> inline T SnapFloor(T x, T y) { return floor(x / y) * y; }

/// Round value down. Returns integer value.
template <class T> inline int FloorToInt(T x) { return static_cast<int>(floor(x)); }

/// Round value to nearest integer.
template <class T> inline T Round(T x) { return round(x); }

/// Return constant for exponential smoothing.
template <class T> inline T ExpSmoothing(T constant, T timeStep) { return constant ? T(1) - Clamp(Pow(T(2), -timeStep * constant), T(0), T(1)) : T(1); }


/// Compute average value of the range.
template <class Iterator> inline auto Average(Iterator begin, Iterator end) -> typename std::decay<decltype(*begin)>::type
{
    using T = typename std::decay<decltype(*begin)>::type;

    T average{};
    unsigned size{};
    for (Iterator it = begin; it != end; ++it)
    {
        average += *it;
        ++size;
    }

    return size != 0 ? average / size : average;
}

/// Round value to nearest number that can be represented as i*y, where i is integer.
template <class T> inline T SnapRound(T x, T y) { return round(x / y) * y; }

/// Round value to nearest integer.
template <class T> inline int RoundToInt(T x) { return static_cast<int>(round(x)); }

/// Round value to nearest multiple.
template <class T> inline T RoundToNearestMultiple(T x, T multiple)
{
    T mag = Abs(x);
    multiple = Abs(multiple);
    T remainder = Mod(mag, multiple);
    if (remainder >= multiple / 2)
        return (FloorToInt<T>(mag / multiple) * multiple + multiple)*Sign(x);
    else
        return (FloorToInt<T>(mag / multiple) * multiple)*Sign(x);
}

/// Round value up.
template <class T> inline T Ceil(T x) { return ceil(x); }

/// Round value up.
template <class T> inline int CeilToInt(T x) { return static_cast<int>(ceil(x)); }

/// Check whether an unsigned integer is a power of two.
inline bool IsPowerOfTwo(unsigned value)
{
    return !(value & (value - 1));
}

/// Round up to next power of two.
inline unsigned NextPowerOfTwo(unsigned value)
{
    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    --value;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return ++value;
}

/// Round up or down to the closest power of two.
inline unsigned ClosestPowerOfTwo(unsigned value)
{
    const unsigned next = NextPowerOfTwo(value);
    const unsigned prev = next >> 1u;
    return (value - prev) > (next - value) ? next : prev;
}

/// Return log base two or the MSB position of the given value.
inline unsigned LogBaseTwo(unsigned value)
{
    // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious
    unsigned ret = 0;
    while (value >>= 1)     // Unroll for more speed...
        ++ret;
    return ret;
}

/// Count the number of set bits in a mask.
inline unsigned CountSetBits(unsigned value)
{
    // Brian Kernighan's method
    unsigned count = 0;
    for (count = 0; value; count++)
        value &= value - 1;
    return count;
}

// /// Update a hash with the given 8-bit value using the SDBM algorithm.
// inline unsigned SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6u) + (hash << 16u) - hash; }

// /// Return a random float between 0.0 (inclusive) and 1.0 (exclusive.)
// inline float Random() { return Rand() / 32768.0f; }

// /// Return a random float between 0.0 and range, inclusive from both ends.
// inline float Random(float range) { return Rand() * range / 32767.0f; }

// /// Return a random float between min and max, inclusive from both ends.
// inline float Random(float min, float max) { return Rand() * (max - min) / 32767.0f + min; }

// /// Return a random integer between 0 and range - 1.
// inline int Random(int range) { return (int)(Random() * range); }

// /// Return a random integer between min and max - 1.
// inline int Random(int min, int max) { auto range = (float)(max - min); return (int)(Random() * range) + min; }

// /// Return a random normal distributed number with the given mean value and variance.
// inline float RandomNormal(float meanValue, float variance) { return RandStandardNormal() * sqrtf(variance) + meanValue; }

/// Convert float to half float. From https://gist.github.com/martinkallman/5049614
inline unsigned short FloatToHalf(float value)
{
    unsigned inu = FloatToRawIntBits(value);
    unsigned t1 = inu & 0x7fffffffu;         // Non-sign bits
    unsigned t2 = inu & 0x80000000u;         // Sign bit
    unsigned t3 = inu & 0x7f800000u;         // Exponent

    t1 >>= 13;                              // Align mantissa on MSB
    t2 >>= 16;                              // Shift sign bit into position

    t1 -= 0x1c000;                          // Adjust bias

    t1 = (t3 < 0x38800000) ? 0 : t1;        // Flush-to-zero
    t1 = (t3 > 0x47000000) ? 0x7bff : t1;   // Clamp-to-max
    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    return (unsigned short)t1;
}

/// Convert half float to float. From https://gist.github.com/martinkallman/5049614
inline float HalfToFloat(unsigned short value)
{
    unsigned t1 = value & 0x7fffu;           // Non-sign bits
    unsigned t2 = value & 0x8000u;           // Sign bit
    unsigned t3 = value & 0x7c00u;           // Exponent

    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position

    t1 += 0x38000000;                       // Adjust bias

    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    float out;
    *((unsigned*)&out) = t1;
    return out;
}

/// Calculate both sine and cosine, with angle in degrees.
inline void SinCos(float angle, float& sin, float& cos) {
    float angleRadians = angle * M_DEGTORAD;
#if defined(HAVE_SINCOSF)
    sincosf(angleRadians, &sin, &cos);
#elif defined(HAVE___SINCOSF)
    __sincosf(angleRadians, &sin, &cos);
#else
    sin = sinf(angleRadians);
    cos = cosf(angleRadians);
#endif
}

// namespace Math {
//  float itof(int v);
//  int ftoi(float v);
//  int ftos(float v);
//  int roundf(float v);

//  double itod(int v);
//  int dtoi(double v);

//   float ltof(long long v);
//  long long ftol(float v);

//  double ltod(long long v);
//  long long dtol(double v);

// // nearest power of two
//   int npot(int v);
//   int udiv(int x,int y);

// // memory
//  void prefetch(const void *ptr);
//  void memset(void *dest,int c,size_t size);
//  void memcpy(void *dest,const void *src,size_t size);
//  int memcmp(const void *src_0,const void *src_1,size_t size);
// }

    inline float rcpf(float v) {
        return 1.0f / v;
    }

inline int Compare(float v0,float v1) {
    float v = Abs(v0 - v1);
    return (v < M_EPSILON);
}

inline int Compare(float v0, float v1, float epsilon) {
    float v = Abs(v0 - v1);
    return (v < (Abs(v0) + Abs(v1) + 1.0f) * epsilon);
}

inline int Compare(double v0,double v1) {
    double v = Abs(v0 - v1);
    return (v < M_EPSILON);
}

inline int Compare(double v0,double v1,double epsilon) {
    double v = Abs(v0 - v1);
    return (v < (Abs(v0) + Abs(v1) + 1.0) * epsilon);
}

inline float Saturate(float v) {
    if(v < 0.0f) return 0.0f;
    if(v > 1.0f) return 1.0f;
    return v;
}







//    inline int compare(const dvec2 &v0,const dvec2 &v1) {
//        return (compare(v0.x,v1.x) && compare(v0.y,v1.y));
//    }
//
//    inline int compare(const dvec2 &v0,const dvec2 &v1,double epsilon) {
//        return (compare(v0.x,v1.x,epsilon) && compare(v0.y,v1.y,epsilon));
//    }
//
//    inline int compare(const dvec3 &v0,const dvec3 &v1) {
//        return (compare(v0.x,v1.x) && compare(v0.y,v1.y) && compare(v0.z,v1.z));
//    }
//
//    inline int compare(const dvec3 &v0,const dvec3 &v1,double epsilon) {
//        return (compare(v0.x,v1.x,epsilon) && compare(v0.y,v1.y,epsilon) && compare(v0.z,v1.z,epsilon));
//    }
//
//    inline int compare(const dvec4 &v0,const dvec4 &v1) {
//        return (compare(v0.x,v1.x) && compare(v0.y,v1.y) && compare(v0.z,v1.z) && compare(v0.w,v1.w));
//    }
//
//    inline int compare(const dvec4 &v0,const dvec4 &v1,double epsilon) {
//        return (compare(v0.x,v1.x,epsilon) && compare(v0.y,v1.y,epsilon) && compare(v0.z,v1.z,epsilon) && compare(v0.w,v1.w,epsilon));
//    }
//
//    inline int compare(const mat2 &m0,const mat2 &m1) {
//        return (compare(m0.m00,m1.m00) && compare(m0.m10,m1.m10)) &&
//               compare(m0.m01,m1.m01) && compare(m0.m11,m1.m11);
//    }
//
//    inline int compare(const mat2 &m0,const mat2 &m1,float epsilon) {
//        return (compare(m0.m00,m1.m00,epsilon) && compare(m0.m10,m1.m10,epsilon)) &&
//               compare(m0.m01,m1.m01,epsilon) && compare(m0.m11,m1.m11,epsilon);
//    }

struct Half {
    inline Half() { }
    Half(const Half &h) : h(h.h) { }
    explicit Half(int h) : h((unsigned short)h) { }
    explicit Half(float v) { SetFloat(v); }

    void SetFloat(float v)
    {
        unsigned int i; //IntFloat(v).ui;
        memcpy(&i, &v, sizeof(i));
        unsigned int e = (i >> 23) & 0x00ff;
        unsigned int m = i & 0x007fffff;
        if(e <= 127 - 15) {
            h = ((m | 0x00800000) >> (127 - 14 - e)) >> 13;
        } else {
            h = (i >> 13) & 0x3fff;
        }
        h |= (i >> 16) & 0xc000;
    }

    float GetFloat() const {
        unsigned int f = (h << 16) & 0x80000000;
        unsigned int em = h & 0x7fff;
        if(em > 0x03ff) {
            f |= (em << 13) + ((127 - 15) << 23);
        } else {
            unsigned int m = em & 0x03ff;
            if(m != 0) {
                unsigned int e = (em >> 10) & 0x1f;
                while((m & 0x0400) == 0) {
                    m <<= 1;
                    e--;
                }
                m &= 0x3ff;
                f |= ((e + (127 - 14)) << 23) | (m << 13);
            }
        }
        float out;
        memcpy(&out, &f, sizeof(f));
        return out; //IntFloat(f).f;
    }

    unsigned short h;
};

}



#ifdef _MSC_VER
#pragma warning(pop)
#endif
