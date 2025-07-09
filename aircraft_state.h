#pragma once

namespace evtol
{
    /**
     * Aircraft state for frame-based simulation
     */
    enum class AircraftState
    {
        IDLE,                // Ready to fly
        FLYING,              // Currently in flight
        CHARGING,            // Currently charging
        WAITING_FOR_CHARGER, // Needs charger but none available
        FAULT                // Aircraft has a fault
    };

    /**
     * Frame-based data for aircraft state management
     */
    struct AircraftFrameData
    {
        AircraftState state{AircraftState::IDLE};
        double time_remaining_sec{0.0};      // Time left in current activity
        double current_flight_time_hrs{0.0}; // Current flight duration
        double current_flight_distance{0.0}; // Current flight distance
        bool fault_occurred{false};
        int charger_id{-1};                       // ID of assigned charger (-1 if none)
        double waiting_start_time{0.0};           // Time when waiting started
        double accumulated_waiting_time_sec{0.0}; // Total waiting time for current charge cycle

        // Use default copy/move semantics for single-threaded operation
        AircraftFrameData() = default;
        AircraftFrameData(const AircraftFrameData &) = default;
        AircraftFrameData &operator=(const AircraftFrameData &) = default;
        AircraftFrameData(AircraftFrameData &&) = default;
        AircraftFrameData &operator=(AircraftFrameData &&) = default;

        /**
         * Safely transition to a new state
         * @param new_state The new state to transition to
         * @return True if transition was successful
         */
        bool transition_to(AircraftState new_state);

        /**
         * Get current state
         * @return Current state
         */
        AircraftState get_state() const;

        /**
         * Update time remaining
         * @param delta_time Time to subtract from remaining time
         * @return New time remaining
         */
        double update_time_remaining(double delta_time);

        /**
         * Reset for new activity
         * @param new_state New state to transition to
         * @param duration Duration of the new activity
         */
        void reset_for_activity(AircraftState new_state, double duration);
    };

    /**
     * State machine for aircraft state transitions
     */
    class AircraftStateMachine
    {
    private:
        AircraftFrameData &frame_data_;

    public:
        AircraftStateMachine(AircraftFrameData &frame_data)
            : frame_data_(frame_data) {}

        /**
         * Check if aircraft can transition to new state
         * @param from_state Current state
         * @param to_state Desired state
         * @return True if transition is valid
         */
        static bool is_valid_transition(AircraftState from_state, AircraftState to_state);

        /**
         * Get string representation of state
         * @param state The state to convert
         * @return String representation
         */
        static const char *state_to_string(AircraftState state);
    };
}