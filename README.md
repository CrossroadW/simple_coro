# simple_coro —— 基于 C++20 协程的轻量级异步库

`simple_coro` 利用 C++20 协程语法 (`co_await` / `co_return`)，
封装了异步编程的常见模式，帮助开发者摆脱回调地狱，让逻辑更加清晰、直观。

通过 thread pool/asio io_context实现同时处理多个连接
---


## examples

```c++
Task<void> hello() {
    spdlog::info("Hello from coroutine!");
    co_return;
}

Task<void> echo() {
    co_await hello();
    spdlog::info("Echo finished!");
}
```

```c++
Task<void> session(tcp::socket sock) {
    int msg_index = 0;
    for (;;) {
        const size_t max_length = 1024;
        char data[max_length]{};

        auto [error, length] =
            co_await async_read_some(sock, asio::buffer(data, max_length));

        if (error == asio::error::eof) {
             break; // 正常结束
        }
        if (error || std::string(data, length).starts_with("quit")) {
            break;
        }

        spdlog::info("msg: {}, data: {}", msg_index, std::string(data, length));

        auto [ec, _] = co_await async_write(sock, asio::buffer(data, length));
        if (ec) {
            spdlog::error("write error: {}", ec.message());
            break;
        }

        msg_index++;
    }

    boost::system::error_code ec;
    sock.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
    sock.close(ec);
}


Task<void> start_server(io_context &ioc, unsigned short port, Executor *pool) {
    std::error_code ec;
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port));

    for (;;) {   tcp::socket socket(ioc);
        auto accept_err = co_await async_accept(acceptor, socket);
        if (!accept_err) {
            spdlog::info("New client coming");
            session(std::move(socket)).setEx(pool).start([](auto &&) {});
        } else {
            //log err
        }
    }
}
```
