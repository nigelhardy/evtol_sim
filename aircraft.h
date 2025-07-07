#pragma once
#include <memory>
#include <string>

namespace evtol
{

    enum class AircraftType
    {
        ALPHA,
        BETA,
        CHARLIE,
        DELTA,
        ECHO
    };

    struct AircraftSpec
    {
        const std::string manufacturer;
        const double cruise_speed_mph;
        const double battery_capacity_kwh;
        const double time_to_charge_hours;
        const int passenger_count;
        const double fault_probability_per_hour;

        constexpr AircraftSpec(const std::string &mfg, double speed, double battery,
                               double charge_time, int passengers, double fault_prob)
            : manufacturer(mfg), cruise_speed_mph(speed), battery_capacity_kwh(battery),
              time_to_charge_hours(charge_time), passenger_count(passengers),
              fault_probability_per_hour(fault_prob) {}
    };

    class AircraftBase
    {
    public:
        virtual ~AircraftBase() = default;
        virtual double get_flight_time_hours() const = 0;
        virtual double get_flight_distance_miles() const = 0;
        virtual bool check_fault_during_flight(double flight_time_hours) = 0;
        virtual void discharge_battery() = 0;
        virtual void charge_battery() = 0;
        virtual double get_battery_level() const = 0;
        virtual int get_id() const = 0;
        virtual AircraftType get_type() const = 0;
        virtual std::string get_manufacturer() const = 0;
        virtual const AircraftSpec &get_spec() const = 0;
        virtual int get_passenger_count() const = 0;
        virtual double get_charge_time_hours() const = 0;
    };

    template <typename Derived>
    class Aircraft : public AircraftBase
    {
    private:
        int aircraft_id_;
        double battery_level_;

    public:
        Aircraft(int id) : aircraft_id_(id), battery_level_(1.0) {}

        virtual ~Aircraft() = default;

        double get_flight_time_hours() const override { return 0.0; }
        double get_flight_distance_miles() const override { return 0.0; }
        bool check_fault_during_flight(double /*flight_time_hours*/) override { return false; }
        void discharge_battery() override {}
        void charge_battery() override {}
        double get_battery_level() const override { return battery_level_; }
        int get_id() const override { return aircraft_id_; }
        AircraftType get_type() const override { return AircraftType::ALPHA; }
        std::string get_manufacturer() const override { return get_spec().manufacturer; }
        const AircraftSpec &get_spec() const override
        {
            static AircraftSpec dummy_spec{"Dummy Manufacturer", 0.0, 0.0, 0.0, 0, 0.0};
            return dummy_spec;
        }
        int get_passenger_count() const override { return 0; }
        double get_charge_time_hours() const override { return 0.0; }

        virtual double energy_consumption_per_mile() const = 0;
    };

}