#pragma once
#include "aircraft.h"
#include <random>

namespace evtol
{

    class AlphaAircraft : public Aircraft<AlphaAircraft>
    {
    public:
        AlphaAircraft(int id) : Aircraft<AlphaAircraft>(id) {}

        static const AircraftSpec &get_aircraft_spec()
        {
            static const AircraftSpec spec("Alpha", 120.0, 320.0, 0.6, 4, 0.25);
            return spec;
        }

        static constexpr AircraftType get_aircraft_type()
        {
            return AircraftType::ALPHA;
        }

    protected:
        double energy_consumption_per_mile() const override
        {
            return 1.6;
        }
    };

    class BetaAircraft : public Aircraft<BetaAircraft>
    {
    public:
        BetaAircraft(int id) : Aircraft<BetaAircraft>(id) {}

        static const AircraftSpec &get_aircraft_spec()
        {
            static const AircraftSpec spec("Beta", 100.0, 100.0, 0.2, 5, 0.10);
            return spec;
        }

        static constexpr AircraftType get_aircraft_type()
        {
            return AircraftType::BETA;
        }

    protected:
        double energy_consumption_per_mile() const override
        {
            return 1.5;
        }
    };

    class CharlieAircraft : public Aircraft<CharlieAircraft>
    {
    public:
        CharlieAircraft(int id) : Aircraft<CharlieAircraft>(id) {}

        static const AircraftSpec &get_aircraft_spec()
        {
            static const AircraftSpec spec("Charlie", 160.0, 220.0, 0.8, 3, 0.05);
            return spec;
        }

        static constexpr AircraftType get_aircraft_type()
        {
            return AircraftType::CHARLIE;
        }

    protected:
        double energy_consumption_per_mile() const override
        {
            return 2.2;
        }
    };

    class DeltaAircraft : public Aircraft<DeltaAircraft>
    {
    public:
        DeltaAircraft(int id) : Aircraft<DeltaAircraft>(id) {}

        static const AircraftSpec &get_aircraft_spec()
        {
            static const AircraftSpec spec("Delta", 90.0, 120.0, 0.62, 2, 0.22);
            return spec;
        }

        static constexpr AircraftType get_aircraft_type()
        {
            return AircraftType::DELTA;
        }

    protected:
        double energy_consumption_per_mile() const override
        {
            return 0.8;
        }
    };

    class EchoAircraft : public Aircraft<EchoAircraft>
    {
    public:
        EchoAircraft(int id) : Aircraft<EchoAircraft>(id) {}

        static const AircraftSpec &get_aircraft_spec()
        {
            static const AircraftSpec spec("Echo", 30.0, 150.0, 0.3, 2, 0.61);
            return spec;
        }

        static constexpr AircraftType get_aircraft_type()
        {
            return AircraftType::ECHO;
        }

    protected:
        double energy_consumption_per_mile() const override
        {
            return 5.8;
        }
    };

    template <typename... AircraftTypes>
    class AircraftFactory
    {
    private:
        static inline std::mt19937 rng{std::random_device{}()};
        static inline std::uniform_int_distribution<int> aircraft_dist{0, 4};

    public:
        static std::vector<std::unique_ptr<AircraftBase>> create_fleet(int size)
        {
            std::vector<std::unique_ptr<AircraftBase>> fleet;
            fleet.reserve(static_cast<size_t>(size));

            for (int i = 0; i < size; ++i)
            {
                int aircraft_type = aircraft_dist(rng);
                
                switch (aircraft_type)
                {
                case 0:
                    fleet.emplace_back(std::make_unique<AlphaAircraft>(i));
                    break;
                case 1:
                    fleet.emplace_back(std::make_unique<BetaAircraft>(i));
                    break;
                case 2:
                    fleet.emplace_back(std::make_unique<CharlieAircraft>(i));
                    break;
                case 3:
                    fleet.emplace_back(std::make_unique<DeltaAircraft>(i));
                    break;
                case 4:
                    fleet.emplace_back(std::make_unique<EchoAircraft>(i));
                    break;
                }
            }

            return fleet;
        }
    };

}