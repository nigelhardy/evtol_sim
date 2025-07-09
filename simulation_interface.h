#pragma once
#include <memory>
#include "charger_manager.h"
#include "statistics_engine.h"

namespace evtol
{
    enum class SimulationMode
    {
        EVENT_DRIVEN,
        FRAME_BASED
    };

    /**
     * Abstract base interface for simulation engines
     * Provides polymorphic behavior for different simulation strategies
     */
    class ISimulationEngine
    {
    public:
        virtual ~ISimulationEngine() = default;

        /**
         * Run the simulation with the given fleet and charger manager
         * @param charger_mgr Reference to charger manager
         * @param fleet Reference to aircraft fleet
         */
        template <typename Fleet>
        void run_simulation(ChargerManager &charger_mgr, Fleet &fleet)
        {
            run_simulation_impl(charger_mgr, static_cast<void *>(&fleet));
        }

        /**
         * Get the current simulation time in hours
         * @return Current simulation time
         */
        virtual double get_current_time() const = 0;

        /**
         * Get the simulation duration in hours
         * @return Simulation duration
         */
        virtual double get_duration() const = 0;

        /**
         * Check if simulation is currently running
         * @return True if simulation is running
         */
        virtual bool is_running() const = 0;

    protected:
        /**
         * Implementation-specific simulation runner
         * @param charger_mgr Reference to charger manager
         * @param fleet_ptr Pointer to fleet (type-erased)
         */
        virtual void run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr) = 0;
    };

    /**
     * Base class for simulation engines with common functionality
     */
    class SimulationEngineBase : public ISimulationEngine
    {
    protected:
        double current_time_hours_;
        double simulation_duration_hours_;
        StatisticsCollector &stats_collector_;
        bool is_running_;

    public:
        SimulationEngineBase(StatisticsCollector &stats, double duration_hours = 3.0)
            : current_time_hours_(0.0), simulation_duration_hours_(duration_hours), stats_collector_(stats), is_running_(false)
        {
        }

        double get_current_time() const override { return current_time_hours_; }
        double get_duration() const override { return simulation_duration_hours_; }
        bool is_running() const override { return is_running_; }
    };
}