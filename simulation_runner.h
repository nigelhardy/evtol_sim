#pragma once
#include <memory>
#include "simulation_interface.h"
#include "simulation_config.h"
#include "simulation_factory.h"

namespace evtol
{
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