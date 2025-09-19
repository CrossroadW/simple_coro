#pragma once
#include <functional>
#include "task.hpp"
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

namespace simple_coro {
using namespace ::boost;

template <typename T>
struct AsioCallbackAwaiter {
    using CallbackFunction =
    std::function<void(std::coroutine_handle<>, std::function<void(T)>)>;

    AsioCallbackAwaiter(CallbackFunction &&callback): callback_(std::move(callback)) {}

    bool await_ready() noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) {
        debug();
        callback_(handle, [this](T t) {
            spdlog::trace("set value");
            result_ = std::move(t);
        });
    }


    T await_resume() noexcept {
        return result_;
    }

    T result_;
    CallbackFunction callback_;
};


inline Task<boost::system::error_code>
async_accept(asio::ip::tcp::acceptor &acceptor, asio::ip::tcp::socket &socket) {
    co_return co_await AsioCallbackAwaiter<boost::system::error_code>{
        [&acceptor,&socket](std::coroutine_handle<> handle,
                            std::function<void(boost::system::error_code)> set_val) {
            debug();
            acceptor.async_accept(socket, [set_val,handle](boost::system::error_code ec) {
                if (ec) {
                    spdlog::trace("accept err: {}", ec.message());
                }
                set_val(ec);
                handle.resume();
            });
        }};
}

template <typename Socket, typename AsioBuffer>
inline Task<std::pair<boost::system::error_code, size_t>>
async_read_some(Socket &socket, AsioBuffer &&buffer) noexcept {
    co_return co_await AsioCallbackAwaiter<std::pair<boost::system::error_code, size_t>>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            socket.async_read_some(
                std::move(buffer),
                [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
                    set_resume_value(std::make_pair(std::move(ec), size));
                    handle.resume();
                });
        }};
}

template <typename Socket, typename AsioBuffer>
inline Task<std::pair<boost::system::error_code, size_t>> async_write(
    Socket &socket, AsioBuffer &&buffer) noexcept {
    co_return co_await AsioCallbackAwaiter<std::pair<boost::system::error_code, size_t>>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            asio::async_write(
                socket, std::move(buffer),
                [handle, set_resume_value = std::move(set_resume_value)](
                auto ec, auto size) mutable {
                    set_resume_value(std::make_pair(std::move(ec), size));
                    handle.resume();
                });
        }};
}

inline Task<system::error_code> async_wait(asio::steady_timer &timer) noexcept {
    co_return co_await AsioCallbackAwaiter<system::error_code>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            timer.async_wait(
                [handle, set_resume_value = std::move(set_resume_value)]
            (system::error_code ec) mutable {
                    set_resume_value(ec);
                    handle.resume();
                });
        }};
}
}
