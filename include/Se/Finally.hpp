#pragma once
#include <iostream>

namespace Se {

template <typename F>
class Finally {
    static_assert(!std::is_lvalue_reference_v<F>, "GFrost::Finally requires the callable is passed as an rvalue reference.");

    F functor_;
    bool engaged_ = false;
public:
    template <typename Func>
    Finally(Func func) : functor_(std::move(func)), engaged_(true) {}

    Finally(Finally&& other) : functor_(std::move(other.functor_)), engaged_(other.engaged_) {
			other.Dismiss();
	}

    ~Finally() { Execute(); }

    
    inline void Dismiss() { engaged_ = false; }

    inline void Execute()
    {
        if (engaged_)
            functor_();

        Dismiss();
    }

    Finally(const Finally&) = delete;
    Finally& operator =(const Finally&) = delete;
    Finally& operator =(Finally&&) = delete;
};

template <typename F>
Finally<F> MakeFinally(F&& f)
{
    return { std::forward<F>(f) };
}

}