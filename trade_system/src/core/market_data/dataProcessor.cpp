#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <atomic>
#include <thread>
#include <vector>

class MarketDataProcessor {
public:
    MarketDataProcessor(int port) : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        startAccept();
    }

    void run() {
        std::vector<std::thread> threads;
        for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
            threads.emplace_back([this] { io_context_.run(); });
        }
        for (auto& thread : threads) {
            thread.join();
        }
    }

private:
    void startAccept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
            if (!ec) {
                std::make_shared<Connection>(std::move(*socket))->start();
            }
            startAccept();
        });
    }

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        explicit Connection(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

        void start() {
            readHeader();
        }

    private:
        void readHeader() {
            auto self(shared_from_this());
            boost::asio::async_read(socket_, boost::asio::buffer(header_), [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    processHeader();
                    readBody();
                }
            });
        }

        void readBody() {
            auto self(shared_from_this());
            boost::asio::async_read(socket_, boost::asio::buffer(body_), [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    processBody();
                    readHeader();
                }
            });
        }

        void processHeader() {
            // Process header data
        }

        void processBody() {
            // Process body data and push to queue
            data_queue_.push(body_);
        }

        boost::asio::ip::tcp::socket socket_;
        std::array<char, 8> header_;
        std::vector<char> body_;
    };

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::lockfree::queue<std::vector<char>> data_queue_{1000};
};