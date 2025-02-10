#pragma once

#include <iterator>
#include <vector>
#include <utility>
#include <vector>
#include <unordered_map>

namespace Se
{

/// Vector of vectors.
template <class T>
class MultiVector
{
public:
    /// Inner collection type.
    using InnerCollection = std::vector<T>;
    /// Outer collection type.
    using OuterCollection = std::vector<InnerCollection>;
    /// Index in multi-vector (pair of outer and inner indices).
    using Index = std::pair<unsigned, unsigned>;

    /// Iterator base.
    template <class OuterIterator, class InnerIterator>
    class BaseIterator : public std::iterator<std::forward_iterator_tag, T>
    {
        //using value_type = T;
    public:
        /// Construct default.
        BaseIterator() = default;

        /// Construct from outer range.
        BaseIterator(OuterIterator begin, OuterIterator end)
                : outer_(begin)
                , outerEnd_(end)
        {
            while (outer_ != outerEnd_ && outer_->empty())
                ++outer_;
            if (outer_ != outerEnd_)
            {
                inner_ = outer_->begin();
                innerEnd_ = outer_->end();
            }
        }

       /// Dereference. MultiVector<T>::ConstIterator
       T operator*() const { return *inner_; }

       /// Dereference.
       T* operator->() const { return &*inner_; }

        /// Advance by N.
        BaseIterator& operator+=(unsigned diff)
        {
            while (diff != 0)
            {
                // Advance inner iterator as much as possible
                const unsigned maxDiff = std::min(diff, static_cast<unsigned>(innerEnd_ - inner_));
                inner_ += maxDiff;
                diff -= maxDiff;

                // Advance outer iterator if necessary
                if (inner_ == innerEnd_)
                    fixInnerIterator();
            }
            return *this;
        }

        /// Advance by N.
        BaseIterator operator+(unsigned diff) const
        {
            auto iter = *this;
            iter += diff;
            return iter;
        }

        /// Pre-increment.
        BaseIterator& operator++()
        {
            ++inner_;
            if (inner_ == innerEnd_)
                fixInnerIterator();
            return *this;
        }

        /// Post-increment.
        BaseIterator operator++(int)
        {
            auto iter = *this;
            ++*this;
            return iter;
        }

        /// Compare equal.
        bool operator==(const BaseIterator& rhs) const
        {
            return (outer_ == outerEnd_ && rhs.outer_ == rhs.outerEnd_) || (outer_ == rhs.outer_ && inner_ == rhs.inner_);
        }

        /// Compare not equal.
        bool operator!=(const BaseIterator& rhs) const { return !(*this == rhs); }

    private:
        /// Fix inner iterator when it reaches end.
        void fixInnerIterator()
        {
            do ++outer_;
            while (outer_ != outerEnd_ && outer_->empty());

            if (outer_ != outerEnd_)
            {
                inner_ = outer_->begin();
                innerEnd_ = outer_->end();
            }
            else
            {
                inner_ = InnerIterator{};
                innerEnd_ = InnerIterator{};
            }
        }

        /// Outer iterator (current).
        OuterIterator outer_{};
        /// Outer iterator (end).
        OuterIterator outerEnd_{};
        /// Inner iterator (current).
        InnerIterator inner_{};
        /// Inner iterator (end).
        InnerIterator innerEnd_{};
    };

    /// Mutable iterator.
    using Iterator = BaseIterator<typename OuterCollection::iterator, typename InnerCollection::iterator>;
    /// Const iterator.
    using ConstIterator = BaseIterator<typename OuterCollection::const_iterator, typename InnerCollection::const_iterator>;

    /// Clear inner vectors. Reset outer vector to fixed size.
    void Clear(std::size_t outerSize)
    {
        outer_.resize(outerSize);
        for (auto& inner : outer_)
            inner.clear();
    }

    /// Emplace element at the back of specified outer vector.
    template <class ... Args>
    T& EmplaceBack(std::size_t outerIndex, Args&& ... args)
    {
        auto& inner = outer_[outerIndex];
        return inner.emplace_back(std::forward<Args>(args)...);
    }

    /// Push element into back of specified outer vector. Return index of added element.
    Index PushBack(std::size_t outerIndex, const T& value)
    {
        auto& inner = outer_[outerIndex];
        const std::size_t innerIndex = inner.size();
        inner.push_back(value);
        return { outerIndex, innerIndex };
    }

    /// Pop element from back of specified outer vector.
    void PopBack(std::size_t outerIndex)
    {
        auto& inner = outer_[outerIndex];
        inner.pop_back();
    }

    /// Return size.
    std::size_t Size() const
    {
        std::size_t size = 0;
        for (auto& inner : outer_)
            size += inner.size();
        return size;
    }

    /// Resize outer vector.
    void Resize(std::size_t outerSize) { outer_.resize(outerSize); }

    /// Copy content to vector.
    void CopyTo(InnerCollection& dest) const
    {
        dest.clear();
        for (const auto& inner : outer_)
            dest.append(inner);
    }

    /// Return element (mutable).
    T& operator[](const Index& index) { return outer_[index.first][index.second]; }

    /// Return element (mutable).
    const T& operator[](const Index& index) const { return outer_[index.first][index.second]; }

    /// Return outer collection (mutable).
    const OuterCollection& GetUnderlyingCollection() const { return outer_; }

    /// Return outer collection (const).
    OuterCollection& GetUnderlyingCollection() { return outer_; }

    /// Return begin iterator (mutable).
    Iterator Begin() { return Iterator{ outer_.begin(), outer_.end() }; }

    /// Return end iterator (mutable).
    Iterator End() { return Iterator{ outer_.end(), outer_.end() }; }

    /// Return begin iterator (const).
    ConstIterator Begin() const { return ConstIterator{ outer_.begin(), outer_.end() }; }

    /// Return end iterator (const).
    ConstIterator End() const { return ConstIterator{ outer_.end(), outer_.end() }; }

private:
    /// Internal collection.
    OuterCollection outer_;
};

/// Return begin iterator of const MultiVector.
template <class T>
typename MultiVector<T>::ConstIterator begin(const MultiVector<T>& c) { return c.Begin(); }
/// Return end iterator of const MultiVector.
template <class T>
typename MultiVector<T>::ConstIterator end(const MultiVector<T>& c) { return c.End(); }
/// Return begin iterator of mutable MultiVector.
template <class T>
typename MultiVector<T>::Iterator begin(MultiVector<T>& c) { return c.Begin(); }
/// Return end iterator of mutable MultiVector.
template <class T>
typename MultiVector<T>::Iterator end(MultiVector<T>& c) { return c.End(); }
/// Return size of MultiVector.
template <class T> std::size_t size(const MultiVector<T>& c) { return c.Size(); }

}