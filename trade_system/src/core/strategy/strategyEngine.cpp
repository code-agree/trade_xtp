#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <boost/lockfree/queue.hpp>

class Strategy {
public:
    virtual void onMarketData(const MarketData& data) = 0;
    virtual ~Strategy() = default;
};

class StrategyEngine {
public:
    StrategyEngine(int num_threads) : running_(true) {
        for (int i = 0; i < num_threads; ++i) {
            threads_.emplace_back(&StrategyEngine::processData, this);
        }
    }

    ~StrategyEngine() {
        running_ = false;
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    void addStrategy(std::unique_ptr<Strategy> strategy) {
        strategies_.push_back(std::move(strategy));
    }

    void pushMarketData(const MarketData& data) {
        data_queue_.push(data);
    }

private:
    void processData() {
        while (running_) {
            MarketData data;
            if (data_queue_.pop(data)) {
                for (auto& strategy : strategies_) {
                    strategy->onMarketData(data);
                }
            }
        }
    }

    std::vector<std::unique_ptr<Strategy>> strategies_;
    boost::lockfree::queue<MarketData> data_queue_{1000};
    std::vector<std::thread> threads_;
    std::atomic<bool> running_;
};