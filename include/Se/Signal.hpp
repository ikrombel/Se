#pragma once

#include <type_traits>
#include <functional>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>
//#include <hash_set>

namespace Se
{

template <typename... Args>
class Signal {
public:

    using SlotFunc = std::function<void(Args...)>;
    using SlotId = std::size_t;
    struct Slot {
        SlotId id;
        SlotFunc func;
        void* target{nullptr};
        bool hasTarget{false};
    };

    /// Connect signal.
    SlotId connect(SlotFunc slot) {
        SlotId id{0};
//        printf("%s: 0x%X\n", typeid(SlotFunc).name(), typeid(SlotFunc).hash_code());
        hash_combine(id, typeid(SlotFunc).hash_code());
        hash_combine(id, ++idGen);
        slots_.push_back({id, slot});
//        printf("0x%X\n", id);
        return id;
    }

    SlotId connectTarget(void* target, SlotFunc slot) {

        if (!target)
            return 0;

        SlotId id{0};
//        printf("%s: 0x%X\n", typeid(SlotFunc).name(), typeid(SlotFunc).hash_code());
        hash_combine(id, typeid(SlotFunc).hash_code());
        hash_combine(id, ++idGen);
        slots_.push_back({id, slot, target, true});
//        printf("0x%X\n", id);
        return id;
    }

    bool disconnect(SlotId id) {
        auto it = std::find_if(slots_.begin(), slots_.end(), [id](const Slot& rslot){
            return rslot.id == id;
        });
        if (it == slots_.end())
            return false;

        slots_.erase(it);
        return true;
    }

    /// Invoke signal.
    void operator()(Args... args) {

        for (auto it = slots_.begin(); it != slots_.end(); ++it)
        {
            if (it->hasTarget && !it->target)
            {
                auto toDelete = it;
                it++;

                slots_.erase(toDelete);
            }
        }


        for (const auto& slot : slots_) {
            slot.func(args...);
        }
    }
    

    void disconnectAll() {
        slots_.clear();
    }

    bool empty() const {
        return slots_.empty();
    }

    std::size_t size() const {
        return slots_.size();
    }

private:
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    std::vector<Slot> slots_;
    std::size_t idGen{0};
};

template <typename... Args>
class SignalQueue : public Signal<Args...> {
public:
    using SlotFunc = typename Signal<Args...>::SlotFunc;
    using SlotId = typename Signal<Args...>::SlotId;

    void operator()(Args... args) {
        Signal<Args...>::operator()(args...);

        this->disconnectAll();
    }
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


template <typename... Args>
class SignalGuard
{
    SignalGuard() {}

    SignalGuard(Signal<Args...>& signal, typename Signal<Args...>::SlotFunc func)
    {
        connect(signal, func);
    }

    ~SignalGuard() {
        if (signal_)
            signal_->disconnect(id_);
    }

    void connect(Signal<Args...>& signal, typename Signal<Args...>::SlotFunc func)
    {
        signal_ = &signal;
        id_ = signal_->connect(func);
    }

private:
    Signal<Args...>* signal_{nullptr};
    typename Signal<Args...>::SlotId id_;
};

}
