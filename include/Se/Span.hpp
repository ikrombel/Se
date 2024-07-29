#pragma once
#include <cstddef>
#include <stdexcept>

namespace Se {

template <typename T>
class span {
public:
    // Constructor
    span(T* data, std::size_t size) : data_(data), size_(size) {}

    // Get the data pointer
    T* data() const { return data_; }

    // Get the size of the span
    std::size_t size() const { return size_; }

    // Get the first element of the span
    T& front() const {
        if (size_ == 0) {
            throw std::out_of_range("Span is empty");
        }
        return *data_;
    }

    // Get the last element of the span
    T& back() const {
        if (size_ == 0) {
            throw std::out_of_range("Span is empty");
        }
        return *(data_ + size_ - 1);
    }

    // Get an iterator to the beginning of the span
    T* begin() const { return data_; }

    // Get an iterator to the end of the span
    T* end() const { return data_ + size_; }

    // Indexing operator
    T& operator[](size_t index) {
        assert(index < size_);
        return data_[index];
    }

    // Const indexing operator
    const T& operator[](size_t index) const {
        assert(index < size_);
        return data_[index];
    }

private:
    T* data_;
    std::size_t size_;
};

}