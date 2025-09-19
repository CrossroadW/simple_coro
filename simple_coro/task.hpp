#pragma once
#include <coroutine>
#include <spdlog/spdlog.h>
#include "dummy.hpp"
#include <boost/asio/io_context.hpp>
#include "threadpool.hpp"
#include "utils.h"
#include "try.hpp"

namespace simple_coro {
template <typename T>
using CoroHandle = std::coroutine_handle<T>;

struct SuspendAlways {
    bool await_ready() noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        debug();
    }

    void await_resume() noexcept {}
};

template <typename>
struct Task;
template <typename>
struct SaveCallerAwaiter;
template <typename T>
struct promise;
template <typename T>
struct RescheduleTask;

struct FisnalAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    template <typename Promise>
    CoroHandle<void> await_suspend(std::coroutine_handle<Promise> coro) noexcept {
        debug();

        logicAssert(coro.promise().caller_ != nullptr, "coro.promise().caller_");
        return coro.promise().caller_;
    }


    void await_resume() noexcept {}
};

template <typename T>
struct promise_base {
    using Type = T;

    auto final_suspend() noexcept {
        return FisnalAwaiter{};
    }

    void setPool(Executor *pool) {
        debug();
        pool_ = pool;
    }


    std::coroutine_handle<> caller_{};
    Executor *pool_;
};

template <typename T>
struct RescheAwaiter {
    CoroHandle<promise<T>> coro_;
    RescheAwaiter(CoroHandle<promise<T>> h): coro_(h) {}

    RescheAwaiter(RescheAwaiter &&other) noexcept
        : coro_(std::exchange(other.coro_, nullptr)) {}

    ~RescheAwaiter() {
        if (coro_) {
            coro_.destroy();
        }
    }

    bool await_ready() const noexcept {
        return false;
    }

    template <typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) noexcept {
        debug();
        logicAssert(coro_ != nullptr, "coro is nullptr");
        logicAssert(coro_.promise().pool_ != nullptr, " coro_.promise().pool_ is nullptr");
        coro_.promise().caller_ = caller;
        coro_.promise().pool_->push([h = std::coroutine_handle<>(coro_)] {
            h.resume();
        });
    }

    Try<T> await_resume() noexcept {
        debug();
        if (coro_.promise()._exception != nullptr) {
            throw coro_.promise()._exception;
        }

        if constexpr (std::is_same_v<void, T>) {
            coro_.destroy();
            coro_ = nullptr;
            return Try<T>{};
        } else {
            Try<T> ret (coro_.promise().val_);
            coro_.destroy();
            coro_ = nullptr;
            return ret;
        }
    }
};

template <typename T>
struct SaveCallerAwaiter {
    CoroHandle<promise<T>> coro_;
    SaveCallerAwaiter(CoroHandle<promise<T>> h): coro_(h) {}

    SaveCallerAwaiter(SaveCallerAwaiter &&other) noexcept
        : coro_(std::exchange(other.coro_, nullptr)) {}

    ~SaveCallerAwaiter() {
        if (coro_) {
            coro_.destroy();
        }
    }

    bool await_ready() const noexcept {
        return false;
    }

    template <typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> caller) noexcept {
        debug();
        coro_.promise().caller_ = caller;

        return coro_;
    }

    Try<T> await_resume() noexcept {
        debug();
        if (coro_.promise()._exception != nullptr) {
            throw coro_.promise()._exception;
            // return Try<T>(coro_.promise()._exception);
        }
        if constexpr (std::is_same_v<void, T>) {
            coro_.destroy();
            coro_ = nullptr;
            return Try<void>{};
        } else {
            Try<T> val = std::move(coro_.promise().val_);
            coro_.destroy();
            coro_ = nullptr;
            return val;
        }
    }
};

template <typename T>
struct ValueAwaiter {
    CoroHandle<promise<T>> coro_;
    explicit ValueAwaiter(CoroHandle<promise<T>> h): coro_(h) {}

    ValueAwaiter(ValueAwaiter &&other) noexcept
        : coro_(std::exchange(other.coro_, nullptr)) {}

    ~ValueAwaiter() {
        if (coro_) {
            coro_.destroy();
        }
    }

    bool await_ready() const noexcept {
        return false;
    }

    template <typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> caller) noexcept {
        debug();
        coro_.promise().caller_ = caller;
        return coro_;
    }

    T await_resume() noexcept {
        debug();
        if (coro_.promise()._exception != nullptr) {
            throw coro_.promise()._exception;
            // return Try<T>(coro_.promise()._exception);
        }
        if constexpr (std::is_same_v<void, T>) {
            coro_.destroy();
            coro_ = nullptr;
            return;
        } else {
            T val = std::move(coro_.promise().val_);
            coro_.destroy();
            coro_ = nullptr;
            return val;
        }
    }
};


template <typename T>
struct promise : public promise_base<T> {
    Task<T> get_return_object();

    SuspendAlways initial_suspend() {
        return {};
    }

    template <typename U>
    auto &&await_transform(RescheduleTask<U> &&awaiter) {
        debug();
        return awaiter->coAwaitTry();
    }


    template <typename Awaitble>
    auto await_transform(Awaitble &&awaiter) {
        return std::forward<Awaitble>(awaiter);
    }

    template <typename U>
    auto await_transform(Task<U> &&awaiter) {
        // logicAssert(promise_base<T>::pool_ != nullptr,
        //             " pool_ != nullptr" + std::to_string(__LINE__));
        debug();
        return std::forward<Task<U>>(awaiter);
    }

    void return_value(T val) {
        debug();
        if (_exception) {
            std::rethrow_exception(_exception);
        }
        val_ = val;
    }


    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    T val_;
    std::exception_ptr _exception{nullptr};
};

template <>
struct promise<void> : promise_base<void> {
    Task<void> get_return_object();

    SuspendAlways initial_suspend() {
        return {};
    }

    template <typename U>
    auto &&await_transform(RescheduleTask<U> &&awaiter) {
        debug();
        return awaiter->coAwaitTry();
    }

    // template <typename Awaitble>
    // auto &&await_transform(Awaitble &&awaiter) {
    //     debug();
    //     return std::forward<Awaitble>(awaiter);
    // }


    template <typename U>
    auto &&await_transform(Task<U> &&awaiter) {
        // logicAssert(pool_ != nullptr, " pool_ != nullptr" + std::to_string(__LINE__));
        debug();
        return std::forward<Task<U>>(awaiter);
    }


    void return_void() {
        debug();
        if (_exception) {
            std::rethrow_exception(_exception);
        }
    }


    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    std::exception_ptr _exception{nullptr};
};

template <typename T>
struct RescheduleTask {
    using ValueType = typename Task<T>::ValueType;
    using Handle = typename Task<T>::Handle;
    explicit RescheduleTask(Handle h): coro_(h) {}

    RescheduleTask(RescheduleTask &&other) noexcept: coro_(
        std::exchange(other.coro_, nullptr)) {}


    auto coAwaitTry() {
        debug();
        return RescheAwaiter(std::exchange(coro_, nullptr));
    }

    template <typename F>
    void start(F &&callback) {
        debug();
        [](RescheduleTask lazy,
           std::decay_t<F> cb) mutable-> DummyCoro {
            cb(std::move(co_await lazy.coAwaitTry()));
        }(std::move(*this), std::forward<F>(callback));
    }

    Handle coro_;
};

template <typename T>
struct Task {
    using promise_type = promise<T>;
    using Handle = std::coroutine_handle<promise_type>;
    using ValueType = T;
    explicit Task(Handle h) : coro_(h) {}

    ~Task() {
        if (coro_) {
            coro_.destroy();
        }
    }

    RescheduleTask<T> setEx(Executor *ex) && {
        debug();
        coro_.promise().setPool(ex);
        return RescheduleTask<T>(std::exchange(coro_, nullptr));
    }

    Task(Task &&other) noexcept: coro_(other.coro_) {
        other.coro_ = nullptr;
    }

    auto coAwaitTry() {
        debug();
        return SaveCallerAwaiter(std::exchange(coro_, nullptr));
    }

    template <typename F>
    void start(F &&callback) {
        debug();
        [](Task<T> lazy,
           std::decay_t<F> cb) -> DummyCoro {
            cb(co_await lazy.coAwaitTry());
        }(std::move(*this), std::forward<F>(callback));
    }

    auto operator co_await() noexcept {
        debug();

        return ValueAwaiter<T>{std::exchange(coro_, nullptr)};
    }

    Handle coro_{};
};


template <typename T>
inline Task<T> promise<T>::get_return_object() {
    return Task<T>{std::coroutine_handle<promise>::from_promise(*this)};
}

inline Task<void> promise<void>::get_return_object() {
    return Task<void>{std::coroutine_handle<promise>::from_promise(*this)};
}


template <typename LazyType>
inline auto syncAwait(LazyType &&lazy) ->
    typename std::decay_t<LazyType>::ValueType {
    std::binary_semaphore cond{0};
    using ValueType = typename std::decay_t<LazyType>::ValueType;

    Try<ValueType> value;
    std::move(lazy)
        .start([&cond, &value](Try<ValueType> result) {
            debug();
            value = std::move(result);
            cond.release();
        });
    cond.acquire();
    return std::move(value).value();
}
}
