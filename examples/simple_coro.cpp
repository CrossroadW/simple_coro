#include <spdlog/spdlog.h>
#include <simple_coro/task.hpp>
#include <simple_coro/threadpool.hpp>
using namespace simple_coro;

Task<void> hello() {
    spdlog::info("hello");
    co_return;
}

Task<int> echo() {
    spdlog::info("echo start");
    co_await hello();
    spdlog::info("echo end");
    co_return 1;
}


int main() try {
    using namespace std::chrono_literals;

    ThreadPool p;
    spdlog::set_level(spdlog::level::trace);
    auto coro = echo().setEx(&p);

    auto r = syncAwait(coro);
    spdlog::info("result: {}", r);
    std::this_thread::sleep_for(2s);
    p.stop();
} catch (std::exception &e) {
    spdlog::error("err: {}", e.what());
}
