#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <queue>
#include <vector>
#include <boost/asio/io_context.hpp>

struct Executor {
    virtual ~Executor() = default;
    virtual void push(std::function<void()> fn) = 0;
};

struct ThreadPool : Executor {
    ThreadPool(ThreadPool &&) = delete;

    static constexpr int DEFAULT_THREADS = 4;

    explicit ThreadPool(int num = DEFAULT_THREADS) {
        for (int i = 0; i < num; i++) {
            ths.emplace_back(std::thread([this] {
                runLoop();
            }));
        }
    }

    void push(std::function<void()> fn) override {
        {
            std::lock_guard guard(mtx);
            queue.emplace(std::move(fn));
        }
        cv.notify_one();
    }

    void stop() {
        stop_ = true;
        cv.notify_all();
    }


    ~ThreadPool() {
        for (auto &th: ths) {
            if (th.joinable()) {
                th.join();
            }
        }
    }

    void runLoop() {
        while (!stop_) {
            std::function<void()> first;
            {
                std::unique_lock lock(mtx);
                cv.wait(lock, [this] {
                    return stop_ || queue.size();
                });
                if (stop_) {
                    break;
                }
                first = std::move(queue.front());
                queue.pop();
            }
            first();
        }
    }

    std::queue<std::function<void()>> queue;
    std::vector<std::thread> ths;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic_bool stop_ = false;
};

struct AsioExecutor : Executor {
    boost::asio::io_context &ioc;
    AsioExecutor(boost::asio::io_context &ctx): ioc(ctx) {}

    void push(std::function<void()> fn) override {
        ioc.post(fn);
    }
};
