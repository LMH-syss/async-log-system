#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity = 0)
        : capacity_(capacity) {}

    bool push(T item) {
        return pushBlocking(std::move(item));
    }

    bool pushBlocking(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this] {
            return stopped_ || capacity_ == 0 || queue_.size() < capacity_;
        });

        if (stopped_) {
            return false;
        }

        queue_.push(std::move(item));
        lock.unlock();
        notEmpty_.notify_one();
        return true;
    }

    bool tryPush(T item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_ || (capacity_ > 0 && queue_.size() >= capacity_)) {
                return false;
            }

            queue_.push(std::move(item));
        }
        notEmpty_.notify_one();
        return true;
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [this] {
            return stopped_ || !queue_.empty();
        });

        if (queue_.empty()) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();
        lock.unlock();
        notFull_.notify_one();
        return true;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void setCapacity(std::size_t capacity) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            capacity_ = capacity;
        }
        notFull_.notify_all();
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::queue<T> queue_;
    std::size_t capacity_{0};
    bool stopped_{false};
};
