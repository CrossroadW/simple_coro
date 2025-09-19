#pragma once
#include <coroutine>
#include <cstdio>
#include <exception>

struct DummyCoro {
    struct promise_type {

        std::suspend_never initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void return_void() noexcept {}

        void unhandled_exception() {

             try {
                 std::rethrow_exception(std::current_exception());
             }catch (std::exception &e) {
                 spdlog::error("err: {}", e.what());
             }
        }

        DummyCoro get_return_object() noexcept {
            return DummyCoro{};
        }
    };

};
