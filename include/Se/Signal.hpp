#pragma once

#include <type_traits>
#include <functional>
#include <utility>
#include <vector>
#include <map>

namespace Se
{

template <typename... Args>
class Signal {
public:
    using Slot = std::function<void(Args...)>;

    /// Connect signal.
    void connect(Slot slot) {
        slots_.push_back(slot);
    }

    /// Invoke signal.
    void operator()(Args... args) {
        for (const auto& slot : slots_) {
            slot(args...);
        }
    }

    bool empty() const {
        return slots_.empty();
    }

    std::size_t size() const {
        return slots_.size();
    }

private:
    std::vector<Slot> slots_;
};

class EventSystem {
public:

    template <typename... Args, typename F>
    void registerEvent(int type, F&& f) {
        auto signal = getSignal<Args...>(type);
        auto fn = std::function{std::forward<F>(f)};
        signal->connect(fn);
    }

    template <typename... Args>
    void emitEvent(int type, Args... args) {
        auto signal = getSignal<Args...>(type);
        if (signal) {
            (*signal)(args...);
        }
    }

private:
    template <typename... Args>
    Signal<Args...>* getSignal(int type) {
        auto it = signals_.find(type);
        if (it == signals_.end()) {
            auto signal = new Signal<Args...>();
            signals_[type] = signal;
            return signal;
        }
        return reinterpret_cast<Signal<Args...>*>((void(*))it->second);
    }

    std::map<int, void*> signals_;
};


}
