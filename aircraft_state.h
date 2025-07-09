#pragma once
#include <atomic>
#include <mutex>

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
        // TODO no longer multi-threaded, so not necessary here
        std::atomic<AircraftState> state{AircraftState::IDLE};
        std::atomic<double> time_remaining_sec{0.0};      // Time left in current activity
        std::atomic<double> current_flight_time_hrs{0.0}; // Current flight duration
        std::atomic<double> current_flight_distance{0.0}; // Current flight distance
        std::atomic<bool> fault_occurred{false};
        std::atomic<int> charger_id{-1};                       // ID of assigned charger (-1 if none)
        std::atomic<double> waiting_start_time{0.0};           // Time when waiting started
        std::atomic<double> accumulated_waiting_time_sec{0.0}; // Total waiting time for current charge cycle

        // Delete copy constructor and assignment operator
        AircraftFrameData(const AircraftFrameData &) = delete;
        AircraftFrameData &operator=(const AircraftFrameData &) = delete;

        // Default constructor and move operations
        AircraftFrameData() = default;
        AircraftFrameData(AircraftFrameData &&other) noexcept
            : state(other.state.load()), time_remaining_sec(other.time_remaining_sec.load()), current_flight_time_hrs(other.current_flight_time_hrs.load()), current_flight_distance(other.current_flight_distance.load()), fault_occurred(other.fault_occurred.load()), charger_id(other.charger_id.load()), waiting_start_time(other.waiting_start_time.load()), accumulated_waiting_time_sec(other.accumulated_waiting_time_sec.load())
        {
        }

        AircraftFrameData &operator=(AircraftFrameData &&other) noexcept
        {
            if (this != &other)
            {
                state.store(other.state.load());
                time_remaining_sec.store(other.time_remaining_sec.load());
                current_flight_time_hrs.store(other.current_flight_time_hrs.load());
                current_flight_distance.store(other.current_flight_distance.load());
                fault_occurred.store(other.fault_occurred.load());
                charger_id.store(other.charger_id.load());
                waiting_start_time.store(other.waiting_start_time.load());
                accumulated_waiting_time_sec.store(other.accumulated_waiting_time_sec.load());
            }
            return *this;
        }

        /**
         * Safely transition to a new state
         * @param new_state The new state to transition to
         * @return True if transition was successful
         */
        bool transition_to(AircraftState new_state);

        /**
         * Get current state safely
         * @return Current state
         */
        AircraftState get_state() const;

        /**
         * Update time remaining atomically
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