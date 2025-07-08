#include "frame_based_simulation.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace evtol
{
    FrameBasedSimulationEngine::FrameBasedSimulationEngine(StatisticsCollector &stats, const SimulationConfig& config)
        : SimulationEngineBase(stats, config.simulation_duration_hours)
        , config_(config)
        , frame_time_seconds_(config.frame_time_seconds)
    {        
        // Validate configuration
        if (!config_.validate())
        {
            throw std::invalid_argument("Invalid simulation configuration");
        }
    }

    FrameBasedSimulationEngine::~FrameBasedSimulationEngine()
    {
        if (is_running_)
        {
            stop();
        }
        
        stop_worker_threads();
    }

    void FrameBasedSimulationEngine::stop()
    { 
        // Wake up any waiting threads
        frame_cv_.notify_all();
    }

    void FrameBasedSimulationEngine::run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr)
    {
        // Type-erase back to template - cast to the expected fleet type
        auto* fleet = static_cast<std::vector<std::unique_ptr<AircraftBase>>*>(fleet_ptr);
        run_frame_based_simulation(charger_mgr, *fleet);
    }

    void FrameBasedSimulationEngine::start_worker_threads()
    {
        if (config_.num_threads <= 1)
        {
            return;
        }
        
        worker_threads_.clear();
        worker_threads_.reserve(static_cast<size_t>(config_.num_threads));
        
        for (int i = 0; i < config_.num_threads; ++i)
        {
            worker_threads_.emplace_back(&FrameBasedSimulationEngine::worker_thread_main, this, i);
        }
    }

    void FrameBasedSimulationEngine::stop_worker_threads()
    {
        if (worker_threads_.empty())
        {
            return;
        }
        
        // Signal all threads to stop
        is_running_ = false;
        frame_cv_.notify_all();
        
        // Wait for all threads to finish
        for (auto& thread : worker_threads_)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        
        worker_threads_.clear();
    }

    void FrameBasedSimulationEngine::worker_thread_main(int /* thread_id */)
    {
        while (is_running_)
        {
            std::unique_lock<std::mutex> lock(frame_mutex_);
            
            // Wait for frame start signal
            frame_cv_.wait(lock, [this] { return !is_running_ || threads_completed_.load() == -1; });
            
            if (!is_running_)
            {
                break;
            }
            
            lock.unlock();
            
            // Process assigned aircraft subset
            // Note: This is a simplified implementation - in practice you'd need to pass
            // the fleet and charger manager to the worker threads through shared data
            
            // Signal frame completion
            signal_frame_complete();
        }
    }

    void FrameBasedSimulationEngine::wait_for_all_threads()
    {
        if (worker_threads_.empty())
        {
            return;
        }
        
        // Wait for all threads to complete their work
        while (threads_completed_.load() < config_.num_threads)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        
        // Reset for next frame
        threads_completed_.store(0);
    }

    void FrameBasedSimulationEngine::signal_frame_start()
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        threads_completed_.store(-1);  // Signal that frame has started
        frame_cv_.notify_all();
    }

    void FrameBasedSimulationEngine::signal_frame_complete()
    {
        threads_completed_.fetch_add(1);
    }

    void FrameBasedSimulationEngine::wait_for_frame_time()
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_frame_time_).count();
        
        double target_frame_time_ms = (frame_time_seconds_ * 1000.0);
        
        if (elapsed < target_frame_time_ms)
        {
            auto sleep_time = std::chrono::milliseconds(static_cast<int>(target_frame_time_ms - elapsed));
            std::this_thread::sleep_for(sleep_time);
        }
        
        last_frame_time_ = std::chrono::high_resolution_clock::now();
    }

    double FrameBasedSimulationEngine::get_frame_delta_time()
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_frame_time_);
        return elapsed.count() / 1000.0;  // Convert to seconds
    }

    // Need to check the existing charger manager interface to make sure these methods exist
    // For now, I'll implement placeholder methods that would need to be added to ChargerManager
    
    void FrameBasedSimulationEngine::handle_partial_flight(std::unique_ptr<AircraftBase> &aircraft, AircraftFrameData &frame_data)
    {
        // Calculate how much of the flight was completed
        double total_flight_time = frame_data.current_flight_time;
        double remaining_time_seconds = frame_data.time_remaining;
        double completed_flight_time = total_flight_time - (remaining_time_seconds / 3600.0); // Convert to hours
        
        // Calculate partial distance
        double partial_distance = (completed_flight_time / total_flight_time) * frame_data.current_flight_distance;
        
        if (config_.enable_detailed_logging)
        {
            log_event("Processing partial flight for aircraft " + std::to_string(aircraft->get_id()) + 
                     " (flew " + std::to_string(completed_flight_time) + "h/" + std::to_string(total_flight_time) + 
                     "h, " + std::to_string(partial_distance) + "/" + std::to_string(frame_data.current_flight_distance) + " miles)");
        }
        
        // Record partial flight statistics
        stats_collector_.record_partial_flight(aircraft->get_type(), completed_flight_time, partial_distance, aircraft->get_passenger_count());
    }

    void FrameBasedSimulationEngine::handle_partial_charging(std::unique_ptr<AircraftBase> &aircraft, AircraftFrameData &frame_data)
    {
        // Calculate how much charging was completed
        double total_charge_time = aircraft->get_charge_time_hours();
        double remaining_time_seconds = frame_data.time_remaining;
        double completed_charge_time = total_charge_time - (remaining_time_seconds / 3600.0); // Convert to hours
        
        if (config_.enable_detailed_logging)
        {
            log_event("Processing partial charge for aircraft " + std::to_string(aircraft->get_id()) + 
                     " (charged " + std::to_string(completed_charge_time) + "h/" + std::to_string(total_charge_time) + 
                     "h, waited: " + std::to_string(frame_data.accumulated_waiting_time / 3600.0) + "h)");
        }
        
        // Record partial charging statistics
        stats_collector_.record_partial_charge(aircraft->get_type(), completed_charge_time);
    }

    std::string FrameBasedSimulationEngine::aircraft_type_to_string(AircraftType type)
    {
        static const char* aircraft_type_names[] = {
            "Alpha", "Beta", "Charlie", "Delta", "Echo"
        };
        
        int index = static_cast<int>(type);
        if (index >= 0 && index < 5)
        {
            return std::string(aircraft_type_names[index]);
        }
        return "Unknown";
    }
}