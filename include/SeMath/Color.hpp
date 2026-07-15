#pragma once

#include <SeMath/Vector4.hpp>

namespace Se
{

class String;

/// RGBA color.
class Color
{
public:

    /// Mask describing color channels.
    struct ChannelMask
    {
        /// Red channel mask. If zero, red channel is set to 0.
        unsigned r_;
        /// Green channel mask. If zero, green channel is set to 0.
        unsigned g_;
        /// Blue channel mask. If zero, blue channel is set to 0.
        unsigned b_;
        /// Alpha channel mask. If zero, alpha channel is set to 1.
        unsigned a_;
    };
    /// Mask for 0xAABBGGRR layout.
    static const ChannelMask ABGR;
    /// Mask for 0xAARRGGBB layout.
    static const ChannelMask ARGB;
    /// Mask for 0xRRGGBB layout.
    static const ChannelMask RGB;
    /// Mask for 0xRRGGBBAA layout.
    static const ChannelMask RGBA;

    /// Construct with default values (opaque white.)
    Color() noexcept :
        r_(1.0f), g_(1.0f), b_(1.0f), a_(1.0f)
    {
    }

    /// Copy-construct from another color.
    Color(const Color& color) noexcept = default;

    /// Construct from another color and modify the alpha.
    Color(const Color& color, float a) noexcept :
        r_(color.r_), g_(color.g_), b_(color.b_), a_(a)
    {
    }

    /// Construct from RGB values and set alpha fully opaque.
    Color(float r, float g, float b) noexcept :
        r_(r), g_(g), b_(b), a_(1.0f)
    {
    }

    /// Construct from RGB values and set alpha fully opaque.
    Color(const Vector3& v3, float a = 1.0f) noexcept :
            r_(v3.x_), g_(v3.y_), b_(v3.z_), a_(a)
    {
    }

    /// Construct from RGB values and set alpha fully opaque.
    Color(const Vector4& v4) noexcept :
            r_(v4.x_), g_(v4.y_), b_(v4.z_), a_(v4.w_)
    {
    }

    /// Construct from RGBA values.
    Color(float r, float g, float b, float a) noexcept :
        r_(r), g_(g), b_(b), a_(a)
    {
    }

    /// Construct from a float array.
    explicit Color(const float* data) noexcept :
        r_(data[0]), g_(data[1]), b_(data[2]), a_(data[3])
    {
    }

        /// Construct from 32-bit integer. Default format is 0xAABBGGRR.
    explicit Color(unsigned color, const ChannelMask& mask = ABGR) { FromUIntMask(color, mask); }

    /// Assign from another color.
    Color& operator =(const Color& rhs) noexcept = default;

    /// Test for equality with another color without epsilon.
    bool operator ==(const Color& rhs) const { return r_ == rhs.r_ && g_ == rhs.g_ && b_ == rhs.b_ && a_ == rhs.a_; }

    /// Test for inequality with another color without epsilon.
    bool operator !=(const Color& rhs) const { return r_ != rhs.r_ || g_ != rhs.g_ || b_ != rhs.b_ || a_ != rhs.a_; }

    /// Multiply with a scalar.
    Color operator *(float rhs) const { return Color(r_ * rhs, g_ * rhs, b_ * rhs, a_ * rhs); }

    /// Add a color.
    Color operator +(const Color& rhs) const { return Color(r_ + rhs.r_, g_ + rhs.g_, b_ + rhs.b_, a_ + rhs.a_); }

    /// Return negation.
    Color operator -() const { return Color(-r_, -g_, -b_, -a_); }

    /// Subtract a color.
    Color operator -(const Color& rhs) const { return Color(r_ - rhs.r_, g_ - rhs.g_, b_ - rhs.b_, a_ - rhs.a_); }

    /// Add-assign a color.
    Color& operator +=(const Color& rhs)
    {
        r_ += rhs.r_;
        g_ += rhs.g_;
        b_ += rhs.b_;
        a_ += rhs.a_;
        return *this;
    }

    /// Return float data.
    const float* Data() const { return &r_; }

    /// Return color packed to a 32-bit integer, with R component in the lowest 8 bits. Components are clamped to [0, 1] range.
    unsigned ToUInt() const;
    /// Return color packed to a 32-bit integer with arbitrary channel mask. Components are clamped to [0, 1] range.
    unsigned ToUIntMask(const ChannelMask& mask) const;
    /// Return HSL color-space representation as a Vector3; the RGB values are clipped before conversion but not changed in the process.
    Vector3 ToHSL() const;
    /// Return HSV color-space representation as a Vector3; the RGB values are clipped before conversion but not changed in the process.
    Vector3 ToHSV() const;
    /// Set RGBA values from packed 32-bit integer, with R component in the lowest 8 bits (format 0xAABBGGRR).
    void FromUInt(unsigned color);
    /// Set RGBA values from packed 32-bit integer with arbitrary channel mask.
    void FromUIntMask(unsigned color, const ChannelMask& mask);
    /// Set RGBA values from specified HSL values and alpha.
    void FromHSL(float h, float s, float l, float a = 1.0f);
    /// Set RGBA values from specified HSV values and alpha.
    void FromHSV(float h, float s, float v, float a = 1.0f);

    /// Return RGB as a three-dimensional vector.
    Vector3 ToVector3() const { return Vector3(r_, g_, b_); }

    /// Return RGBA as a four-dimensional vector.
    Vector4 ToVector4() const { return Vector4(r_, g_, b_, a_); }

    /// Return sum of RGB components.
    float SumRGB() const { return r_ + g_ + b_; }

    /// Return average value of the RGB channels.
    float Average() const { return (r_ + g_ + b_) / 3.0f; }

    /// Return the 'grayscale' representation of RGB values, as used by JPEG and PAL/NTSC among others.
    float Luma() const { return r_ * 0.299f + g_ * 0.587f + b_ * 0.114f; }

    /// Return the colorfulness relative to the brightness of a similarly illuminated white.
    float Chroma() const;
    /// Return hue mapped to range [0, 1.0).
    float Hue() const;
    /// Return saturation as defined for HSL.
    float SaturationHSL() const;
    /// Return saturation as defined for HSV.
    float SaturationHSV() const;

    /// Return value as defined for HSV: largest value of the RGB components. Equivalent to calling MinRGB().
    float Value() const { return MaxRGB(); }

    /// Convert single component of the color from gamma to linear space.
    static float ConvertGammaToLinear(float value)
    {
        if (value <= 0.04045f)
            return value / 12.92f;
        else if (value < 1.0f)
            return Pow((value + 0.055f) / 1.055f, 2.4f);
        else
            return Pow(value, 2.2f);
    }

    /// Convert single component of the color from linear to gamma space.
    static float ConvertLinearToGamma(float value)
    {
        if (value <= 0.0f)
            return 0.0f;
        else if (value <= 0.0031308f)
            return 12.92f * value;
        else if (value < 1.0f)
            return 1.055f * Pow(value, 0.4166667f) - 0.055f;
        else
            return Pow(value, 0.45454545f);
    }

    /// Convert color from gamma to linear space.
    Color GammaToLinear() const { return { ConvertGammaToLinear(r_), ConvertGammaToLinear(g_), ConvertGammaToLinear(b_), a_ }; }

    /// Convert color from linear to gamma space.
    Color LinearToGamma() const { return { ConvertLinearToGamma(r_), ConvertLinearToGamma(g_), ConvertLinearToGamma(b_), a_ }; }

    /// Return lightness as defined for HSL: average of the largest and smallest values of the RGB components.
    float Lightness() const;

    /// Stores the values of least and greatest RGB component at specified pointer addresses, optionally clipping those values to [0, 1] range.
    void Bounds(float* min, float* max, bool clipped = false) const;
    /// Return the largest value of the RGB components.
    float MaxRGB() const;
    /// Return the smallest value of the RGB components.
    float MinRGB() const;
    /// Return range, defined as the difference between the greatest and least RGB component.
    float Range() const;

    /// Clip to [0, 1.0] range.
    void Clip(bool clipAlpha = false);
    /// Inverts the RGB channels and optionally the alpha channel as well.
    void Invert(bool invertAlpha = false);
    /// Return linear interpolation of this color with another color.
    Color Lerp(const Color& rhs, float t) const;

    /// Return color with absolute components.
    Color Abs() const { return Color(Se::Abs(r_), Se::Abs(g_), Se::Abs(b_), Se::Abs(a_)); }

    /// Test for equality with another color with epsilon.
    bool Equals(const Color& rhs) const
    {
        return Se::Equals(r_, rhs.r_) && Se::Equals(g_, rhs.g_) && Se::Equals(b_, rhs.b_) && Se::Equals(a_, rhs.a_);
    }

    /// Return as string.
    String ToString() const {
        return cformat("%g %g %g %g", r_, g_, b_, a_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const { return ToUInt(); }

    /// Red value.
    float r_;
    /// Green value.
    float g_;
    /// Blue value.
    float b_;
    /// Alpha value.
    float a_;

    /// Opaque white color.
    static const Color WHITE;
    /// Opaque gray color.
    static const Color GRAY;
    /// Opaque black color.
    static const Color BLACK;
    /// Opaque red color.
    static const Color RED;
    /// Opaque green color.
    static const Color GREEN;
    /// Opaque blue color.
    static const Color BLUE;
    /// Opaque cyan color.
    static const Color CYAN;
    /// Opaque magenta color.
    static const Color MAGENTA;
    /// Opaque yellow color.
    static const Color YELLOW;
    /// Transparent black color (black with no alpha).
    static const Color TRANSPARENT_BLACK;
        /// Color-to-gray factors for color in gamma space.
    static const Color LUMINOSITY_GAMMA;
    /// Color-to-gray factors for color in linear space.
    static const Color LUMINOSITY_LINEAR;

protected:
    /// Return hue value given greatest and least RGB component, value-wise.
    float Hue(float min, float max) const;
    /// Return saturation (HSV) given greatest and least RGB component, value-wise.
    float SaturationHSV(float min, float max) const;
    /// Return saturation (HSL) given greatest and least RGB component, value-wise.
    float SaturationHSL(float min, float max) const;
    /// Calculate and set RGB values. Convenience function used by FromHSV and FromHSL to avoid code duplication.
    void FromHCM(float h, float c, float m);
};

/// Multiply Color with a scalar.
inline Color operator *(float lhs, const Color& rhs) { return rhs * lhs; }


/// Create Color from integer with ABGR mask.
inline Color operator"" _abgr(unsigned long long value) { return Color{ static_cast<unsigned>(value), Color::ABGR }; }

/// Create Color from integer with ARGB mask.
inline Color operator"" _argb(unsigned long long value) { return Color{ static_cast<unsigned>(value), Color::ARGB }; }

/// Create Color from integer with RGB mask.
inline Color operator"" _rgb(unsigned long long value) { return Color{ static_cast<unsigned>(value), Color::RGB }; }


//

inline unsigned Color::ToUInt() const
{
    auto r = (unsigned)Clamp(((int)(r_ * 255.0f)), 0, 255);
    auto g = (unsigned)Clamp(((int)(g_ * 255.0f)), 0, 255);
    auto b = (unsigned)Clamp(((int)(b_ * 255.0f)), 0, 255);
    auto a = (unsigned)Clamp(((int)(a_ * 255.0f)), 0, 255);
    return (a << 24u) | (b << 16u) | (g << 8u) | r;
}

inline unsigned Color::ToUIntMask(const ChannelMask& mask) const
{
    const auto max = static_cast<double>(M_MAX_UNSIGNED);
    const auto r = static_cast<unsigned>(Clamp(static_cast<double>(r_) * mask.r_, 0.0, max)) & mask.r_;
    const auto g = static_cast<unsigned>(Clamp(static_cast<double>(g_) * mask.g_, 0.0, max)) & mask.g_;
    const auto b = static_cast<unsigned>(Clamp(static_cast<double>(b_) * mask.b_, 0.0, max)) & mask.b_;
    const auto a = static_cast<unsigned>(Clamp(static_cast<double>(a_) * mask.a_, 0.0, max)) & mask.a_;
    return r | g | b | a;
}

inline Vector3 Color::ToHSL() const
{
    float min, max;
    Bounds(&min, &max, true);

    float h = Hue(min, max);
    float s = SaturationHSL(min, max);
    float l = (max + min) * 0.5f;

    return Vector3(h, s, l);
}

inline Vector3 Color::ToHSV() const
{
    float min, max;
    Bounds(&min, &max, true);

    float h = Hue(min, max);
    float s = SaturationHSV(min, max);
    float v = max;

    return Vector3(h, s, v);
}

inline void Color::FromUInt(unsigned color)
{
    a_ = ((color >> 24u) & 0xffu) / 255.0f;
    b_ = ((color >> 16u) & 0xffu) / 255.0f;
    g_ = ((color >> 8u)  & 0xffu) / 255.0f;
    r_ = ((color >> 0u)  & 0xffu) / 255.0f;
}

inline void Color::FromUIntMask(unsigned color, const ChannelMask& mask)
{
    // Channel offset is irrelevant during division, but double should be used to avoid precision loss.
    r_ = !mask.r_ ? 0.0f : static_cast<float>((color & mask.r_) / static_cast<double>(mask.r_));
    g_ = !mask.g_ ? 0.0f : static_cast<float>((color & mask.g_) / static_cast<double>(mask.g_));
    b_ = !mask.b_ ? 0.0f : static_cast<float>((color & mask.b_) / static_cast<double>(mask.b_));
    a_ = !mask.a_ ? 1.0f : static_cast<float>((color & mask.a_) / static_cast<double>(mask.a_));
}

inline void Color::FromHSL(float h, float s, float l, float a)
{
    float c;
    if (l < 0.5f)
        c = (1.0f + (2.0f * l - 1.0f)) * s;
    else
        c = (1.0f - (2.0f * l - 1.0f)) * s;

    float m = l - 0.5f * c;

    FromHCM(h, c, m);

    a_ = a;
}

inline void Color::FromHSV(float h, float s, float v, float a)
{
    float c = v * s;
    float m = v - c;

    FromHCM(h, c, m);

    a_ = a;
}

inline float Color::Chroma() const
{
    float min, max;
    Bounds(&min, &max, true);

    return max - min;
}

inline float Color::Hue() const
{
    float min, max;
    Bounds(&min, &max, true);

    return Hue(min, max);
}

inline float Color::SaturationHSL() const
{
    float min, max;
    Bounds(&min, &max, true);

    return SaturationHSL(min, max);
}

inline float Color::SaturationHSV() const
{
    float min, max;
    Bounds(&min, &max, true);

    return SaturationHSV(min, max);
}

inline float Color::Lightness() const
{
    float min, max;
    Bounds(&min, &max, true);

    return (max + min) * 0.5f;
}

inline void Color::Bounds(float* min, float* max, bool clipped) const
{
    assert(min && max);

    if (r_ > g_)
    {
        if (g_ > b_) // r > g > b
        {
            *max = r_;
            *min = b_;
        }
        else // r > g && g <= b
        {
            *max = r_ > b_ ? r_ : b_;
            *min = g_;
        }
    }
    else
    {
        if (b_ > g_) // r <= g < b
        {
            *max = b_;
            *min = r_;
        }
        else // r <= g && b <= g
        {
            *max = g_;
            *min = r_ < b_ ? r_ : b_;
        }
    }

    if (clipped)
    {
        *max = *max > 1.0f ? 1.0f : (*max < 0.0f ? 0.0f : *max);
        *min = *min > 1.0f ? 1.0f : (*min < 0.0f ? 0.0f : *min);
    }
}

inline float Color::MaxRGB() const
{
    if (r_ > g_)
        return (r_ > b_) ? r_ : b_;
    else
        return (g_ > b_) ? g_ : b_;
}

inline float Color::MinRGB() const
{
    if (r_ < g_)
        return (r_ < b_) ? r_ : b_;
    else
        return (g_ < b_) ? g_ : b_;
}

inline float Color::Range() const
{
    float min, max;
    Bounds(&min, &max);
    return max - min;
}

inline void Color::Clip(bool clipAlpha)
{
    r_ = (r_ > 1.0f) ? 1.0f : ((r_ < 0.0f) ? 0.0f : r_);
    g_ = (g_ > 1.0f) ? 1.0f : ((g_ < 0.0f) ? 0.0f : g_);
    b_ = (b_ > 1.0f) ? 1.0f : ((b_ < 0.0f) ? 0.0f : b_);

    if (clipAlpha)
        a_ = (a_ > 1.0f) ? 1.0f : ((a_ < 0.0f) ? 0.0f : a_);
}

inline void Color::Invert(bool invertAlpha)
{
    r_ = 1.0f - r_;
    g_ = 1.0f - g_;
    b_ = 1.0f - b_;

    if (invertAlpha)
        a_ = 1.0f - a_;
}

inline Color Color::Lerp(const Color& rhs, float t) const
{
    float invT = 1.0f - t;
    return Color(
        r_ * invT + rhs.r_ * t,
        g_ * invT + rhs.g_ * t,
        b_ * invT + rhs.b_ * t,
        a_ * invT + rhs.a_ * t
    );
}

inline float Color::Hue(float min, float max) const
{
    float chroma = max - min;

    // If chroma equals zero, hue is undefined
    if (chroma <= M_EPSILON)
        return 0.0f;

    // Calculate and return hue
    if (Se::Equals(g_, max))
        return (b_ + 2.0f * chroma - r_) / (6.0f * chroma);
    else if (Se::Equals(b_, max))
        return (4.0f * chroma - g_ + r_) / (6.0f * chroma);
    else
    {
        float r = (g_ - b_) / (6.0f * chroma);
        return (r < 0.0f) ? 1.0f + r : ((r >= 1.0f) ? r - 1.0f : r);
    }

}

inline float Color::SaturationHSV(float min, float max) const
{
    // Avoid div-by-zero: result undefined
    if (max <= M_EPSILON)
        return 0.0f;

    // Saturation equals chroma:value ratio
    return 1.0f - (min / max);
}

inline float Color::SaturationHSL(float min, float max) const
{
    // Avoid div-by-zero: result undefined
    if (max <= M_EPSILON || min >= 1.0f - M_EPSILON)
        return 0.0f;

    // Chroma = max - min, lightness = (max + min) * 0.5
    float hl = (max + min);
    if (hl <= 1.0f)
        return (max - min) / hl;
    else
        return (min - max) / (hl - 2.0f);

}

inline void Color::FromHCM(float h, float c, float m)
{
    if (h < 0.0f || h >= 1.0f)
        h -= floorf(h);

    float hs = h * 6.0f;
    float x = c * (1.0f - Se::Abs(fmodf(hs, 2.0f) - 1.0f));

    // Reconstruct r', g', b' from hue
    if (hs < 2.0f)
    {
        b_ = 0.0f;
        if (hs < 1.0f)
        {
            g_ = x;
            r_ = c;
        }
        else
        {
            g_ = c;
            r_ = x;
        }
    }
    else if (hs < 4.0f)
    {
        r_ = 0.0f;
        if (hs < 3.0f)
        {
            g_ = c;
            b_ = x;
        }
        else
        {
            g_ = x;
            b_ = c;
        }
    }
    else
    {
        g_ = 0.0f;
        if (hs < 5.0f)
        {
            r_ = x;
            b_ = c;
        }
        else
        {
            r_ = c;
            b_ = x;
        }
    }

    r_ += m;
    g_ += m;
    b_ += m;
}


inline const Color::ChannelMask Color::ABGR{ 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };
inline const Color::ChannelMask Color::ARGB{ 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };
inline const Color::ChannelMask Color::RGB{ 0x00ff0000, 0x0000ff00, 0x000000ff, 0 };
inline const Color::ChannelMask Color::RGBA{ 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff};
inline const Color Color::WHITE;
inline const Color Color::GRAY(0.5f, 0.5f, 0.5f);
inline const Color Color::BLACK(0.0f, 0.0f, 0.0f);
inline const Color Color::RED(1.0f, 0.0f, 0.0f);
inline const Color Color::GREEN(0.0f, 1.0f, 0.0f);
inline const Color Color::BLUE(0.0f, 0.0f, 1.0f);
inline const Color Color::CYAN(0.0f, 1.0f, 1.0f);
inline const Color Color::MAGENTA(1.0f, 0.0f, 1.0f);
inline const Color Color::YELLOW(1.0f, 1.0f, 0.0f);
inline const Color Color::TRANSPARENT_BLACK(0.0f, 0.0f, 0.0f, 0.0f);
inline const Color Color::LUMINOSITY_GAMMA(0.299f, 0.587f, 0.114f, 0.0f);
inline const Color Color::LUMINOSITY_LINEAR(0.2126f, 0.7152f, 0.0722f, 0.0f);

}
