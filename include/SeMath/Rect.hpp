#pragma once

#include <SeMath/Vector4.hpp>

namespace Se
{

/// Two-dimensional bounding rectangle.
class Rect
{
public:
    /// Construct an undefined rect.
    Rect() noexcept :
        min_(M_INFINITY, M_INFINITY),
        max_(-M_INFINITY, -M_INFINITY)
    {
    }

    /// Construct from minimum and maximum vectors.
    Rect(const Vector2& min, const Vector2& max) noexcept :
        min_(min),
        max_(max)
    {
    }

    /// Construct from coordinates.
    Rect(float left, float top, float right, float bottom) noexcept :
        min_(left, top),
        max_(right, bottom)
    {
    }

    /// Construct from a Vector4.
    explicit Rect(const Vector4& vector) noexcept :
        min_(vector.x_, vector.y_),
        max_(vector.z_, vector.w_)
    {
    }

    /// Construct from a float array.
    explicit Rect(const float* data) noexcept :
        min_(data[0], data[1]),
        max_(data[2], data[3])
    {
    }

    /// Copy-construct from another rect.
    Rect(const Rect& rect) noexcept = default;

    /// Assign from another rect.
    Rect& operator =(const Rect& rhs) noexcept = default;

    /// Test for equality with another rect.
    bool operator ==(const Rect& rhs) const { return min_ == rhs.min_ && max_ == rhs.max_; }

    /// Test for inequality with another rect.
    bool operator !=(const Rect& rhs) const { return min_ != rhs.min_ || max_ != rhs.max_; }

    /// Add another rect to this one inplace.
    Rect& operator +=(const Rect& rhs)
    {
        min_ += rhs.min_;
        max_ += rhs.max_;
        return *this;
    }

    /// Subtract another rect from this one inplace.
    Rect& operator -=(const Rect& rhs)
    {
        min_ -= rhs.min_;
        max_ -= rhs.max_;
        return *this;
    }

    /// Divide by scalar inplace.
    Rect& operator /=(float value)
    {
        min_ /= value;
        max_ /= value;
        return *this;
    }

    /// Multiply by scalar inplace.
    Rect& operator *=(float value)
    {
        min_ *= value;
        max_ *= value;
        return *this;
    }

    /// Divide by scalar.
    Rect operator /(float value) const
    {
        return Rect(min_ / value, max_ / value);
    }

    /// Multiply by scalar.
    Rect operator *(float value) const
    {
        return Rect(min_ * value, max_ * value);
    }

    /// Add another rect.
    Rect operator +(const Rect& rhs) const
    {
        return Rect(min_ + rhs.min_, max_ + rhs.max_);
    }

    /// Subtract another rect.
    Rect operator -(const Rect& rhs) const
    {
        return Rect(min_ - rhs.min_, max_ - rhs.max_);
    }

    /// Define from another rect.
    void Define(const Rect& rect)
    {
        min_ = rect.min_;
        max_ = rect.max_;
    }

    /// Define from minimum and maximum vectors.
    void Define(const Vector2& min, const Vector2& max)
    {
        min_ = min;
        max_ = max;
    }

    /// Define from a point.
    void Define(const Vector2& point)
    {
        min_ = max_ = point;
    }

    /// Merge a point.
    void Merge(const Vector2& point)
    {
        if (point.x_ < min_.x_)
            min_.x_ = point.x_;
        if (point.x_ > max_.x_)
            max_.x_ = point.x_;
        if (point.y_ < min_.y_)
            min_.y_ = point.y_;
        if (point.y_ > max_.y_)
            max_.y_ = point.y_;
    }

    /// Merge a rect.
    void Merge(const Rect& rect)
    {
        if (rect.min_.x_ < min_.x_)
            min_.x_ = rect.min_.x_;
        if (rect.min_.y_ < min_.y_)
            min_.y_ = rect.min_.y_;
        if (rect.max_.x_ > max_.x_)
            max_.x_ = rect.max_.x_;
        if (rect.max_.y_ > max_.y_)
            max_.y_ = rect.max_.y_;
    }

    /// Clear to undefined state.
    void Clear()
    {
        min_ = Vector2(M_INFINITY, M_INFINITY);
        max_ = Vector2(-M_INFINITY, -M_INFINITY);
    }

    /// Clip with another rect.
    void Clip(const Rect& rect);

    /// Return true if this rect is defined via a previous call to Define() or Merge().
    bool Defined() const
    {
        return min_.x_ != M_INFINITY;
    }

    /// Return center.
    Vector2 Center() const { return (max_ + min_) * 0.5f; }

    /// Return size.
    Vector2 Size() const { return max_ - min_; }

    /// Return half-size.
    Vector2 HalfSize() const { return (max_ - min_) * 0.5f; }

    /// Test for equality with another rect with epsilon.
    bool Equals(const Rect& rhs) const { return min_.Equals(rhs.min_) && max_.Equals(rhs.max_); }

    /// Test whether a point is inside.
    Intersection IsInside(const Vector2& point) const
    {
        if (point.x_ < min_.x_ || point.y_ < min_.y_ || point.x_ > max_.x_ || point.y_ > max_.y_)
            return OUTSIDE;
        else
            return INSIDE;
    }

    /// Test if another rect is inside, outside or intersects.
    Intersection IsInside(const Rect& rect) const
    {
        if (rect.max_.x_ < min_.x_ || rect.min_.x_ > max_.x_ || rect.max_.y_ < min_.y_ || rect.min_.y_ > max_.y_)
            return OUTSIDE;
        else if (rect.min_.x_ < min_.x_ || rect.max_.x_ > max_.x_ || rect.min_.y_ < min_.y_ || rect.max_.y_ > max_.y_)
            return INTERSECTS;
        else
            return INSIDE;
    }

    /// Return float data.
    const float* Data() const { return &min_.x_; }

    /// Return as a vector.
    Vector4 ToVector4() const { return Vector4(min_.x_, min_.y_, max_.x_, max_.y_); }

    /// Return as string.
    String ToString() const {
        return cformat("%g %g %g %g", min_.x_, min_.y_, max_.x_, max_.y_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const
    {
        return min_.ToHash() * 37 + max_.ToHash();
    }

    /// Return left-top corner position.
    Vector2 Min() const { return min_; }

    /// Return right-bottom corner position.
    Vector2 Max() const { return max_; }

    /// Return left coordinate.
    float Left() const { return min_.x_; }

    /// Return top coordinate.
    float Top() const { return min_.y_; }

    /// Return right coordinate.
    float Right() const { return max_.x_; }

    /// Return bottom coordinate.
    float Bottom() const { return max_.y_; }

    /// Minimum vector.
    Vector2 min_;
    /// Maximum vector.
    Vector2 max_;

    /// Rect in the range (-1, -1) - (1, 1)
    static const Rect FULL;
    /// Rect in the range (0, 0) - (1, 1)
    static const Rect POSITIVE;
    /// Zero-sized rect.
    static const Rect ZERO;
};

/// Two-dimensional bounding rectangle with integer values.
class IntRect {
public:
    /// Construct a zero rect.
    IntRect() noexcept:
            left_(0),
            top_(0),
            right_(0),
            bottom_(0) {
    }

    /// Construct from minimum and maximum vectors.
    IntRect(const IntVector2 &min, const IntVector2 &max) noexcept:
            left_(min.x_),
            top_(min.y_),
            right_(max.x_),
            bottom_(max.y_) {
    }

    /// Construct from coordinates.
    IntRect(int left, int top, int right, int bottom) noexcept:
            left_(left),
            top_(top),
            right_(right),
            bottom_(bottom) {
    }

    /// Construct from an int array.
    explicit IntRect(const int *data) noexcept:
            left_(data[0]),
            top_(data[1]),
            right_(data[2]),
            bottom_(data[3]) {
    }

    /// Test for equality with another rect.
    bool operator==(const IntRect &rhs) const {
        return left_ == rhs.left_ && top_ == rhs.top_ && right_ == rhs.right_ && bottom_ == rhs.bottom_;
    }

    /// Test for inequality with another rect.
    bool operator!=(const IntRect &rhs) const {
        return left_ != rhs.left_ || top_ != rhs.top_ || right_ != rhs.right_ || bottom_ != rhs.bottom_;
    }

    /// Add another rect to this one inplace.
    IntRect &operator+=(const IntRect &rhs) {
        left_ += rhs.left_;
        top_ += rhs.top_;
        right_ += rhs.right_;
        bottom_ += rhs.bottom_;
        return *this;
    }

    /// Subtract another rect from this one inplace.
    IntRect &operator-=(const IntRect &rhs) {
        left_ -= rhs.left_;
        top_ -= rhs.top_;
        right_ -= rhs.right_;
        bottom_ -= rhs.bottom_;
        return *this;
    }

    /// Divide by scalar inplace.
    IntRect &operator/=(float value) {
        left_ = static_cast<int>(left_ / value);
        top_ = static_cast<int>(top_ / value);
        right_ = static_cast<int>(right_ / value);
        bottom_ = static_cast<int>(bottom_ / value);
        return *this;
    }

    /// Multiply by scalar inplace.
    IntRect &operator*=(float value) {
        left_ = static_cast<int>(left_ * value);
        top_ = static_cast<int>(top_ * value);
        right_ = static_cast<int>(right_ * value);
        bottom_ = static_cast<int>(bottom_ * value);
        return *this;
    }

    /// Divide by scalar.
    IntRect operator/(float value) const {
        return {
                static_cast<int>(left_ / value), static_cast<int>(top_ / value),
                static_cast<int>(right_ / value), static_cast<int>(bottom_ / value)
        };
    }

    /// Multiply by scalar.
    IntRect operator*(float value) const {
        return {
                static_cast<int>(left_ * value), static_cast<int>(top_ * value),
                static_cast<int>(right_ * value), static_cast<int>(bottom_ * value)
        };
    }

    /// Add another rect.
    IntRect operator+(const IntRect &rhs) const {
        return {
                left_ + rhs.left_, top_ + rhs.top_,
                right_ + rhs.right_, bottom_ + rhs.bottom_
        };
    }

    /// Subtract another rect.
    IntRect operator-(const IntRect &rhs) const {
        return {
                left_ - rhs.left_, top_ - rhs.top_,
                right_ - rhs.right_, bottom_ - rhs.bottom_
        };
    }

    /// Return size.
    IntVector2 Size() const { return IntVector2(Width(), Height()); }

    /// Return width.
    int Width() const { return right_ - left_; }

    /// Return height.
    int Height() const { return bottom_ - top_; }

    /// Test whether a point is inside.
    Intersection IsInside(const IntVector2 &point) const {
        if (point.x_ < left_ || point.y_ < top_ || point.x_ >= right_ || point.y_ >= bottom_)
            return OUTSIDE;
        else
            return INSIDE;
    }

    /// Clip with another rect.  Since IntRect does not have an undefined state
    /// like Rect, return (0, 0, 0, 0) if the result is empty.
    void Clip(const IntRect &rect);

    /// Merge a rect.  If this rect was empty, become the other rect.  If the
    /// other rect is empty, do nothing.
    void Merge(const IntRect &rect);

    /// Return integer data.
    const int *Data() const { return &left_; }

    /// Return as string.
    String ToString() const {
        return cformat("%d %d %d %d", left_, top_, right_, bottom_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const {
        return (unsigned)left_ * 31 * 31 * 31 +
        (unsigned)right_ * 31 * 31 +
        (unsigned)top_ * 31 +
        (unsigned)bottom_;
    }


    /// Return left-top corner position.
    IntVector2 Min() const { return {left_, top_}; }

    /// Return right-bottom corner position.
    IntVector2 Max() const { return {right_, bottom_}; }

    /// Return left coordinate.
    int Left() const { return left_; }

    /// Return top coordinate.
    int Top() const { return top_; }

    /// Return right coordinate.
    int Right() const { return right_; }

    /// Return bottom coordinate.
    int Bottom() const { return bottom_; }

    union {
        struct {
            /// Left coordinate.
            int left_;
            /// Top coordinate.
            int top_;
            /// Right coordinate.
            int right_;
            /// Bottom coordinate.
            int bottom_;
        };
        int data[4];
    };

    /// Zero-sized rect.
    static const IntRect ZERO;
};


/// Iterator that iterates through all elements of IntRect row-by-row.
class IntRectIterator
{
public:
    /// Construct valid. Iterators with different rectangles are incompatible.
    IntRectIterator(const IntRect& rect, const IntVector2& index)
        : rect_(rect)
        , index_(index)
    {
    }

    /// Pre-increment.
    IntRectIterator& operator++()
    {
        ++index_.x_;
        if (index_.x_ >= rect_.right_)
        {
            ++index_.y_;
            index_.x_ = rect_.left_;

            if (index_.y_ >= rect_.bottom_)
                index_.y_ = rect_.bottom_;
        }
        return *this;
    }

    /// Post-increment.
    IntRectIterator operator++(int)
    {
        IntRectIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    /// Compare for equality.
    bool operator ==(const IntRectIterator& rhs) const { assert(rect_ == rhs.rect_); return index_ == rhs.index_; }

    /// Compare for non-equality.
    bool operator !=(const IntRectIterator& rhs) const { return !(*this == rhs); }

    /// Dereference.
    const IntVector2& operator *() const { return index_; }

    /// Dereference.
    const IntVector2* operator ->() const { return &index_; }

private:
    /// Iterated rectangle.
    IntRect rect_;
    /// Current index within rectangle.
    IntVector2 index_;
};

/// Return begin iterator of IntRect.
inline IntRectIterator begin(const IntRect& rect) { return IntRectIterator(rect, rect.Min()); }

/// Return end iterator of IntRect.
inline IntRectIterator end(const IntRect& rect) { return IntRectIterator(rect, IntVector2(rect.left_, rect.bottom_)); }

inline const String ToString(const IntRect& value)
{
    return cformat("IntRect( L:%d, T:%d, R:%d, B:%d)",
            value.left_, value.top_, value.right_, value.bottom_);
}

inline const String ToString(const Rect& value) {
    return cformat("IntRect( L:%g, T:%g, R:%g, B:%g)",
            value.Left(), value.Top(), value.Right(), value.Bottom());
}

inline const Rect Rect::FULL(-1.0f, -1.0f, 1.0f, 1.0f);
inline const Rect Rect::POSITIVE(0.0f, 0.0f, 1.0f, 1.0f);
inline const Rect Rect::ZERO(0.0f, 0.0f, 0.0f, 0.0f);

inline const IntRect IntRect::ZERO(0, 0, 0, 0);

inline void IntRect::Clip(const IntRect& rect)
{
    if (rect.left_ > left_)
        left_ = rect.left_;
    if (rect.right_ < right_)
        right_ = rect.right_;
    if (rect.top_ > top_)
        top_ = rect.top_;
    if (rect.bottom_ < bottom_)
        bottom_ = rect.bottom_;

    if (left_ >= right_ || top_ >= bottom_)
        *this = IntRect();
}

inline void IntRect::Merge(const IntRect& rect)
{
    if (Width() <= 0 || Height() <= 0)
    {
        *this = rect;
    }
    else if (rect.Width() > 0 && rect.Height() > 0)
    {
        if (rect.left_ < left_)
            left_ = rect.left_;
        if (rect.top_ < top_)
            top_ = rect.top_;
        if (rect.right_ > right_)
            right_ = rect.right_;
        if (rect.bottom_ > bottom_)
            bottom_ = rect.bottom_;
    }
}

inline void Rect::Clip(const Rect& rect)
{
    if (rect.min_.x_ > min_.x_)
        min_.x_ = rect.min_.x_;
    if (rect.max_.x_ < max_.x_)
        max_.x_ = rect.max_.x_;
    if (rect.min_.y_ > min_.y_)
        min_.y_ = rect.min_.y_;
    if (rect.max_.y_ < max_.y_)
        max_.y_ = rect.max_.y_;

    if (min_.x_ > max_.x_ || min_.y_ > max_.y_)
    {
        min_ = Vector2(M_INFINITY, M_INFINITY);
        max_ = Vector2(-M_INFINITY, -M_INFINITY);
    }
}

}