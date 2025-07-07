#pragma once
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <array>

namespace evtol
{
    class ChargerManager
    {
    private:
        std::queue<int> waiting_queue_;
        std::unordered_set<int> available_chargers_;
        std::unordered_map<int, int> aircraft_to_charger_map_;
        std::atomic<int> active_chargers_{0};
        mutable std::mutex queue_mutex_;
        mutable std::mutex charger_mutex_;

        const int NUM_CHARGERS = 3;

    public:
        ChargerManager()
        {
            for (int i = 0; i < NUM_CHARGERS; ++i)
            {
                available_chargers_.insert(i);
            }
        }

        bool request_charger(int aircraft_id)
        {
            std::lock_guard<std::mutex> lock(charger_mutex_);

            if (!available_chargers_.empty())
            {
                int charger_id = *available_chargers_.begin();
                available_chargers_.erase(charger_id);
                aircraft_to_charger_map_[aircraft_id] = charger_id;
                active_chargers_.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
            return false;
        }

        void release_charger(int aircraft_id)
        {
            std::lock_guard<std::mutex> lock(charger_mutex_);

            auto it = aircraft_to_charger_map_.find(aircraft_id);
            if (it != aircraft_to_charger_map_.end())
            {
                available_chargers_.insert(it->second);
                aircraft_to_charger_map_.erase(it);
                active_chargers_.fetch_sub(1, std::memory_order_relaxed);
            }
        }

        void add_to_queue(int aircraft_id)
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            waiting_queue_.push(aircraft_id);
        }

        int get_next_from_queue()
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            if (!waiting_queue_.empty())
            {
                int aircraft_id = waiting_queue_.front();
                waiting_queue_.pop();
                return aircraft_id;
            }
            return -1;
        }

        bool assign_charger(int aircraft_id)
        {
            return request_charger(aircraft_id);
        }

        int get_queue_size() const
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return static_cast<int>(waiting_queue_.size());
        }

        int get_active_chargers() const
        {
            return active_chargers_.load(std::memory_order_relaxed);
        }

        int get_available_chargers() const
        {
            std::lock_guard<std::mutex> lock(charger_mutex_);
            return static_cast<int>(available_chargers_.size());
        }

        constexpr int get_total_chargers() const
        {
            return NUM_CHARGERS;
        }
    };

}