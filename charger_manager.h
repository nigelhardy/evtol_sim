#pragma once
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <array>
#include <vector>

namespace evtol
{
    class ChargerManager
    {
    private:
        std::queue<int> waiting_queue_;
        std::unordered_set<int> available_chargers_;
        std::unordered_map<int, int> aircraft_to_charger_map_;
        int active_chargers_ = 0;

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
            if (!available_chargers_.empty())
            {
                int charger_id = *available_chargers_.begin();
                available_chargers_.erase(charger_id);
                aircraft_to_charger_map_[aircraft_id] = charger_id;
                active_chargers_++;
                return true;
            }
            return false;
        }

        void release_charger(int aircraft_id)
        {
            auto it = aircraft_to_charger_map_.find(aircraft_id);
            if (it != aircraft_to_charger_map_.end())
            {
                available_chargers_.insert(it->second);
                aircraft_to_charger_map_.erase(it);
                active_chargers_--;
            }
        }

        void add_to_queue(int aircraft_id)
        {
            waiting_queue_.push(aircraft_id);
        }

        int get_next_from_queue()
        {
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
            return static_cast<int>(waiting_queue_.size());
        }

        int get_active_chargers() const
        {
            return active_chargers_;
        }

        int get_available_chargers() const
        {
            return static_cast<int>(available_chargers_.size());
        }

        constexpr int get_total_chargers() const
        {
            return NUM_CHARGERS;
        }

        // Additional methods for frame-based simulation
        int get_num_chargers() const
        {
            return NUM_CHARGERS;
        }

        bool is_charger_occupied(int charger_id) const
        {
            return available_chargers_.find(charger_id) == available_chargers_.end();
        }

        int get_aircraft_at_charger(int charger_id) const
        {
            for (const auto& pair : aircraft_to_charger_map_)
            {
                if (pair.second == charger_id)
                {
                    return pair.first;
                }
            }
            return -1;  // No aircraft at this charger
        }

        int get_charger_id(int aircraft_id) const
        {
            auto it = aircraft_to_charger_map_.find(aircraft_id);
            return (it != aircraft_to_charger_map_.end()) ? it->second : -1;
        }

        std::vector<int> get_waiting_queue() const
        {
            std::vector<int> queue_copy;
            std::queue<int> temp_queue = waiting_queue_;  // Copy the queue
            while (!temp_queue.empty())
            {
                queue_copy.push_back(temp_queue.front());
                temp_queue.pop();
            }
            return queue_copy;
        }
    };

}