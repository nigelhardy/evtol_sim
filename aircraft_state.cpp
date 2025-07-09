#include "aircraft_state.h"
#include <stdexcept>
#include <iostream>

namespace evtol
{
    bool AircraftFrameData::transition_to(AircraftState new_state)
    {        
        AircraftState current_state = state;
        
        if (!AircraftStateMachine::is_valid_transition(current_state, new_state))
        {
            std::cerr << "ERROR: Invalid Transition State Machine Attempted" << std::endl;
            return false;
        }
        
        state = new_state;
        return true;
    }

    AircraftState AircraftFrameData::get_state() const
    {
        return state;
    }

    double AircraftFrameData::update_time_remaining(double delta_time_sec)
    {
        double current_time_left_sec = time_remaining_sec;
        double new_time = std::max(0.0, current_time_left_sec - delta_time_sec);
        time_remaining_sec = new_time;
        return new_time;
    }

    void AircraftFrameData::reset_for_activity(AircraftState new_state, double duration_sec)
    {        
        state = new_state;
        time_remaining_sec = duration_sec;
        fault_occurred = false;
        
        if (new_state == AircraftState::FLYING)
        {
            // Reset flight-specific data
            current_flight_time_hrs = duration_sec / 3600.0;  // Convert seconds to hours
            // Note: current_flight_distance will be set by calling code after reset
        }
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