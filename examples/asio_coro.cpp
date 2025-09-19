#include <spdlog/spdlog.h>
#include <simple_coro/asio_coro.hpp>
#include <thread>
using namespace simple_coro;
using namespace boost::asio;
using ip::tcp;


Task<void> session(tcp::socket sock) {
    int msg_index = 0;
    for (;;) {
        const size_t max_length = 1024;
        char data[max_length];

        auto [error, length] =
            co_await async_read_some(sock, asio::buffer(data, max_length));

        if (error == asio::error::eof) {
            spdlog::info("Remote client closed at message index: {}", msg_index);
            break; // 正常结束
        }
        if (error) {
            spdlog::error("read error: {}", error.message());
            break;
        }
        if (std::string(data, length).starts_with("quit")) {
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

    spdlog::info("Finished msgindex: {}", msg_index);
    co_return;
}


Task<void> start_server(io_context &ioc, unsigned short port, Executor *pool) {
    std::error_code ec;

    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port));

    asio::steady_timer timer(ioc);



    for (;;) {   tcp::socket socket(ioc);
        auto accept_err = co_await async_accept(acceptor, socket);

        if (!accept_err) {
            spdlog::info("New client coming");
            session(std::move(socket)).setEx(pool).start([](auto &&) {});

        } else {
            spdlog::error("Accept failed, error: {}", accept_err.message());
        }
    }
    spdlog::info("Server loop stopped");
}

int main(int argc, char *argv[]) {
    // spdlog::set_level(spdlog::level::trace);
    asio::io_context io_context;
    AsioExecutor pool{io_context};

    asio::io_context::work work(io_context);
    std::thread thd([&io_context] {
        io_context.run();
    });

    syncAwait(start_server(io_context, 8888, &pool));
    thd.join();
}
