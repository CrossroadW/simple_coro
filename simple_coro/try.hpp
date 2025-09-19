#pragma once

#include <source_location>
#include <optional>
#include  <variant>

namespace simple_coro {
template <class R>
struct Try {
    using Storage = std::variant<std::monostate, R, std::exception_ptr>;

    Try() = default;
    ~Try() = default ;

    Try(Try&&) noexcept = default;
    Try& operator=(Try&&) noexcept = default;

    template <class... Args>
     explicit Try(Args&&... args) {
        result_.template emplace<R>(std::forward<Args>(args)...);
    }

    // 设置值
    template <class... Args>
    void emplace(Args&&... args) {
        result_.template emplace<R>(std::forward<Args>(args)...);
    }
    void setException(std::exception_ptr e) {
        result_ = e;
    }
    explicit operator bool() const noexcept {
        return std::holds_alternative<R>(result_);
    }

    R& value() {
        if (std::holds_alternative<R>(result_)) {
            return std::get<R>(result_);
        }
        if (std::holds_alternative<std::exception_ptr>(result_)) {
            std::rethrow_exception(std::get<std::exception_ptr>(result_));
        }
        throw std::runtime_error("no value");
    }

    Storage result_;
};
template <>
struct Try<void> {
    Try() noexcept = default;
    ~Try() = default;

    // 默认可拷贝/可移动（exception_ptr 是可拷贝的）
    Try(Try&&) noexcept = default;
    Try& operator=(Try&&) noexcept = default;

    // 通过异常构造（表示失败）
    explicit Try(std::exception_ptr eptr) noexcept: exception_(std::move(eptr)) {}

    explicit operator bool() const noexcept { return exception_ == nullptr; }

    // 如果有异常则重新抛出，否则什么都不做（void）
    void value() {
        if (exception_) {
            std::rethrow_exception(exception_);
        }
    }

    // 设置为成功（清空异常）
    void clear() noexcept { exception_ = nullptr; }

    // 设置异常（表示失败）
    void setException(std::exception_ptr eptr) noexcept { exception_ = std::move(eptr); }

    // 取出异常（不会抛），并清空内部异常
    std::exception_ptr take() noexcept {
        auto tmp = exception_;
        exception_ = nullptr;
        return tmp;
    }

private:
    std::exception_ptr exception_; // nullptr 表示成功
};
inline void simple_coro_debug(std::source_location source = std::source_location::current()) {
    spdlog::trace("{}:{}:{}", source.file_name(), source.line(), source.column());
}
#ifndef NDEBUG
#define debug() simple_coro_debug();
#else
#define debug()
#endif
}
