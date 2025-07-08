#pragma once
#include <memory>
#include "simulation_interface.h"
#include "simulation_config.h"
#include "simulation_engine.h"
#include "frame_based_simulation.h"

namespace evtol
{
    /**
     * Wrapper for the existing event-driven simulation engine
     * to implement the ISimulationEngine interface
     */
    class EventDrivenSimulationEngine : public SimulationEngineBase
    {
    private:
        std::unique_ptr<SimulationEngine> engine_;

    public:
        EventDrivenSimulationEngine(StatisticsCollector &stats, double duration_hours = 3.0)
            : SimulationEngineBase(stats, duration_hours), engine_(std::make_unique<SimulationEngine>(stats, duration_hours))
        {
        }

    protected:
        void run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr) override
        {
            is_running_ = true;

            // Type-erase back to template - this is a limitation of the current design
            // In a real implementation, we might use std::function or type erasure
            // For now, we'll use a simple approach
            auto *fleet = static_cast<std::vector<std::unique_ptr<AircraftBase>> *>(fleet_ptr);
            engine_->run_simulation(charger_mgr, *fleet);

            is_running_ = false;
        }
    };

    /**
     * Factory for creating simulation engines
     */
    class SimulationFactory
    {
    public:
        /**
         * Create a simulation engine based on configuration
         * @param config Simulation configuration
         * @param stats Statistics collector
         * @return Unique pointer to simulation engine
         */
        static std::unique_ptr<ISimulationEngine> create_engine(
            const SimulationConfig &config,
            StatisticsCollector &stats)
        {
            switch (config.mode)
            {
            case SimulationMode::EVENT_DRIVEN:
                return std::make_unique<EventDrivenSimulationEngine>(stats, config.simulation_duration_hours);

            case SimulationMode::FRAME_BASED:
                return std::make_unique<FrameBasedSimulationEngine>(stats, config);

            default:
                throw std::invalid_argument("Unknown simulation mode");
            }
        }

        /**
         * Create a complete simulation setup
         * @param config Simulation configuration
         * @param stats Statistics collector
         * @return engine
         */
        static std::unique_ptr<ISimulationEngine>
        create_simulation_setup(const SimulationConfig &config, StatisticsCollector &stats)
        {
            return create_engine(config, stats);
        }
    };

    /**
     * High-level simulation runner that encapsulates the factory pattern
     */
    class SimulationRunner
    {
    private:
        SimulationConfig config_;
        StatisticsCollector &stats_collector_;
        std::unique_ptr<ISimulationEngine> engine_;

    public:
        SimulationRunner(StatisticsCollector &stats, const SimulationConfig &config = SimulationConfig{})
            : config_(config), stats_collector_(stats)
        {
            engine_ = SimulationFactory::create_simulation_setup(config_, stats_collector_);
        }

        /**
         * Run the simulation with the given fleet and charger manager
         * @param charger_mgr Reference to charger manager
         * @param fleet Reference to aircraft fleet
         */
        template <typename Fleet>
        void run_simulation(ChargerManager &charger_mgr, Fleet &fleet)
        {
            if (!engine_)
            {
                throw std::runtime_error("Simulation engine not initialized");
            }

            engine_->run_simulation(charger_mgr, fleet);
        }

        /**
         * Get the current simulation engine
         * @return Pointer to current engine
         */
        ISimulationEngine *get_engine() { return engine_.get(); }

        /**
         * Update configuration and recreate engine
         * @param new_config New configuration
         */
        void update_config(const SimulationConfig &new_config)
        {
            config_ = new_config;
            engine_ = SimulationFactory::create_simulation_setup(config_, stats_collector_);
        }

        /**
         * Get current configuration
         * @return Current configuration
         */
        const SimulationConfig &get_config() const { return config_; }

        // Control methods (delegate to engine)
        void stop()
        {
            if (engine_)
                engine_->stop();
        }


        // Status methods
        bool is_running() const { return engine_ && engine_->is_running(); }
        double get_current_time() const { return engine_ ? engine_->get_current_time() : 0.0; }
        double get_duration() const { return engine_ ? engine_->get_duration() : 0.0; }
    };
}