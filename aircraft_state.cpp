#include "aircraft_state.h"
#include <stdexcept>

namespace evtol
{
    bool AircraftFrameData::transition_to(AircraftState new_state)
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        AircraftState current_state = state.load();
        
        if (!AircraftStateMachine::is_valid_transition(current_state, new_state))
        {
            return false;
        }
        
        state.store(new_state);
        return true;
    }

    AircraftState AircraftFrameData::get_state() const
    {
        return state.load();
    }

    double AircraftFrameData::update_time_remaining(double delta_time)
    {
        double current_time = time_remaining.load();
        double new_time = std::max(0.0, current_time - delta_time);
        time_remaining.store(new_time);
        return new_time;
    }

    void AircraftFrameData::reset_for_activity(AircraftState new_state, double duration)
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        state.store(new_state);
        time_remaining.store(duration);
        fault_occurred.store(false);
        
        if (new_state == AircraftState::FLYING)
        {
            // Reset flight-specific data
            current_flight_time.store(duration / 3600.0);  // Convert seconds to hours
            current_flight_distance.store(0.0);  // Will be set by calling code
        }
    }

    bool AircraftStateMachine::update_frame(double delta_time)
    {
        AircraftState current_state = frame_data_.get_state();
        
        // Update time remaining
        double new_time = frame_data_.update_time_remaining(delta_time);
        
        // Check for state transitions based on time completion
        if (new_time <= 0.0)
        {
            switch (current_state)
            {
                case AircraftState::FLYING:
                    // Flight completed, but don't automatically transition
                    // Let the simulation engine handle this
                    return false;
                    
                case AircraftState::CHARGING:
                    // Charging completed, but don't automatically transition
                    // Let the simulation engine handle this
                    return false;
                    
                default:
                    return false;
            }
        }
        
        return false;
    }

    bool AircraftStateMachine::is_valid_transition(AircraftState from_state, AircraftState to_state)
    {
        // Define valid state transitions
        switch (from_state)
        {
            case AircraftState::IDLE:
                return to_state == AircraftState::FLYING;
                
            case AircraftState::FLYING:
                return to_state == AircraftState::CHARGING || 
                       to_state == AircraftState::WAITING_FOR_CHARGER ||
                       to_state == AircraftState::FAULT;
                
            case AircraftState::CHARGING:
                return to_state == AircraftState::IDLE ||
                       to_state == AircraftState::FAULT;
                
            case AircraftState::WAITING_FOR_CHARGER:
                return to_state == AircraftState::CHARGING ||
                       to_state == AircraftState::FAULT;
                
            case AircraftState::FAULT:
                // Aircraft with fault stays in fault state
                return false;
                
            default:
                return false;
        }
    }

    const char* AircraftStateMachine::state_to_string(AircraftState state)
    {
        switch (state)
        {
            case AircraftState::IDLE:
                return "IDLE";
            case AircraftState::FLYING:
                return "FLYING";
            case AircraftState::CHARGING:
                return "CHARGING";
            case AircraftState::WAITING_FOR_CHARGER:
                return "WAITING_FOR_CHARGER";
            case AircraftState::FAULT:
                return "FAULT";
            default:
                return "UNKNOWN";
        }
    }
}