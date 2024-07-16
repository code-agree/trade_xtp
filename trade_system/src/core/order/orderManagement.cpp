#include <unordered_map>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

struct Order {
    std::string id;
    std::string symbol;
    double price;
    int quantity;
    std::string status;
};

class OrderManagementSystem {
public:
    OrderManagementSystem(int num_threads) : running_(true) {
        for (int i = 0; i < num_threads; ++i) {
            threads_.emplace_back(&OrderManagementSystem::processOrders, this);
        }
    }

    ~OrderManagementSystem() {
        running_ = false;
        cv_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    void submitOrder(const Order& order) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            order_queue_.push(order);
        }
        cv_.notify_one();
    }

    void cancelOrder(const std::string& order_id) {
        std::unique_lock<std::shared_mutex> lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it != orders_.end()) {
            it->second.status = "CANCELLED";
        }
    }

    Order getOrderStatus(const std::string& order_id) {
        std::shared_lock<std::shared_mutex> lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it != orders_.end()) {
            return it->second;
        }
        return Order{};
    }

private:
    void processOrders() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !order_queue_.empty() || !running_; });

            while (!order_queue_.empty()) {
                Order order = order_queue_.front();
                order_queue_.pop();
                lock.unlock();

                // Process the order (in a real system, this would involve interacting with exchanges)
                order.status = "FILLED";  // Assume all orders are filled immediately for this example

                {
                    std::unique_lock<std::shared_mutex> orders_lock(orders_mutex_);
                    orders_[order.id] = order;
                }

                lock.lock();
            }
        }
    }

    std::unordered_map<std::string, Order> orders_;
    std::queue<Order> order_queue_;
    std::vector<std::thread> threads_;
    std::atomic<bool> running_;
    std::mutex queue_mutex_;
    std::shared_mutex orders_mutex_;
    std::condition_variable cv_;
};