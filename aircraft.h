#pragma once
#include <memory>
#include <string>
#include <random>
#include <cmath>

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
    struct FlightStats
    {
        double total_flight_time_hours = 0.0;
        double total_distance_miles = 0.0;
        double total_charging_time_hours = 0.0;
        double total_waiting_time_hours = 0.0;
        int total_faults = 0;
        double total_passenger_miles = 0.0;
        int flight_count = 0;
        int charge_count = 0;

        // Partial activities (when simulation ends mid-activity)
        double partial_flight_time_hours = 0.0;
        double partial_distance_miles = 0.0;
        double partial_charging_time_hours = 0.0;
        double partial_passenger_miles = 0.0;
        int partial_flight_count = 0;
        int partial_charge_count = 0;

        FlightStats() = default;
        FlightStats(const FlightStats &other)
            : total_flight_time_hours(other.total_flight_time_hours),
              total_distance_miles(other.total_distance_miles),
              total_charging_time_hours(other.total_charging_time_hours),
              total_waiting_time_hours(other.total_waiting_time_hours),
              total_faults(other.total_faults),
              total_passenger_miles(other.total_passenger_miles),
              flight_count(other.flight_count),
              charge_count(other.charge_count),
              partial_flight_time_hours(other.partial_flight_time_hours),
              partial_distance_miles(other.partial_distance_miles),
              partial_charging_time_hours(other.partial_charging_time_hours),
              partial_passenger_miles(other.partial_passenger_miles),
              partial_flight_count(other.partial_flight_count),
              partial_charge_count(other.partial_charge_count) {}

        FlightStats &operator=(const FlightStats &other)
        {
            if (this != &other)
            {
                total_flight_time_hours = other.total_flight_time_hours;
                total_distance_miles = other.total_distance_miles;
                total_charging_time_hours = other.total_charging_time_hours;
                total_waiting_time_hours = other.total_waiting_time_hours;
                total_faults = other.total_faults;
                total_passenger_miles = other.total_passenger_miles;
                flight_count = other.flight_count;
                charge_count = other.charge_count;
                partial_flight_time_hours = other.partial_flight_time_hours;
                partial_distance_miles = other.partial_distance_miles;
                partial_charging_time_hours = other.partial_charging_time_hours;
                partial_passenger_miles = other.partial_passenger_miles;
                partial_flight_count = other.partial_flight_count;
                partial_charge_count = other.partial_charge_count;
            }
            return *this;
        }

        void add_flight(double flight_time, double distance, int passengers)
        {
            total_flight_time_hours += flight_time;
            total_distance_miles += distance;
            total_passenger_miles += passengers * distance;
            flight_count++;
        }

        void add_charge_session(double charge_time)
        {
            total_charging_time_hours += charge_time;
            charge_count++;
        }

        void add_charge_session(double charge_time, double waiting_time)
        {
            total_charging_time_hours += charge_time;
            total_waiting_time_hours += waiting_time;
            charge_count++;
        }

        void add_waiting_time(double waiting_time)
        {
            total_waiting_time_hours += waiting_time;
        }

        void add_fault()
        {
            total_faults++;
        }

        void add_partial_flight(double flight_time, double distance, int passengers)
        {
            partial_flight_time_hours += flight_time;
            partial_distance_miles += distance;
            partial_passenger_miles += passengers * distance;
            partial_flight_count++;

            // adding partial to total
            total_flight_time_hours += flight_time;
            total_distance_miles += distance;
            total_passenger_miles += passengers * distance;
            flight_count++;
        }

        void add_partial_charge(double charge_time)
        {
            partial_charging_time_hours += charge_time;
            partial_charge_count++;

            // adding partial to total
            total_charging_time_hours += charge_time;
            charge_count++;
        }

        double avg_flight_time() const
        {
            return flight_count > 0 ? total_flight_time_hours / flight_count : 0.0;
        }

        double avg_distance() const
        {
            return flight_count > 0 ? total_distance_miles / flight_count : 0.0;
        }

        double avg_charging_time() const
        {
            return charge_count > 0 ? total_charging_time_hours / charge_count : 0.0;
        }

        double avg_waiting_time() const
        {
            return charge_count > 0 ? total_waiting_time_hours / charge_count : 0.0;
        }

        double avg_total_charge_time() const
        {
            return charge_count > 0 ? (total_charging_time_hours + total_waiting_time_hours) / charge_count : 0.0;
        }

        double total_charge_time_including_waiting() const
        {
            return total_charging_time_hours + total_waiting_time_hours;
        }
    };

    struct AircraftSpec
    {
        const std::string manufacturer;
        const double cruise_speed_mph;
        const double battery_capacity_kwh;
        const double time_to_charge_hours;
        const int passenger_count;
        const double fault_probability_per_hour;
        // out of scope for this project, but leaving energy usage in class to be dynamic (in theory)

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
        virtual double check_fault_during_flight(double flight_time_hours) = 0;
        virtual void discharge_battery() = 0;
        virtual void charge_battery() = 0;
        virtual double get_battery_level() const = 0;
        virtual int get_id() const = 0;
        virtual AircraftType get_type() const = 0;
        virtual std::string get_manufacturer() const = 0;
        virtual const AircraftSpec &get_spec() const = 0;
        virtual int get_passenger_count() const = 0;
        virtual double get_charge_time_hours() const = 0;
        virtual bool is_faulty() const = 0;
        virtual void set_faulty(bool faulty) = 0;
    };

    template <typename Derived>
    class Aircraft : public AircraftBase
    {
    private:
        static inline std::mt19937 rng{std::random_device{}()};
        static inline std::uniform_real_distribution<double> fault_dist{0.0, 1.0};

    public:
        Aircraft(int id) : aircraft_id_(id), battery_level_(1.0), is_faulty_(false) {}

        virtual ~Aircraft() = default;

        const AircraftSpec &get_spec() const override
        {
            return static_cast<const Derived *>(this)->get_aircraft_spec();
        }

        double get_flight_time_hours() const override
        {
            return battery_level_ * get_spec().battery_capacity_kwh /
                   (get_spec().cruise_speed_mph * energy_consumption_per_mile());
        }

        double get_flight_distance_miles() const override
        {
            return get_flight_time_hours() * get_spec().cruise_speed_mph;
        }

        double check_fault_during_flight(double flight_time_hours) override
        {
            double fault_rate = get_spec().fault_probability_per_hour;
            if (fault_rate <= 0.0)
                return -1.0; // never faults

            // Simple probability check for fault during this flight
            double flight_fault_probability = fault_rate * flight_time_hours;
            if (fault_dist(rng) < flight_fault_probability)
            {
                // Fault occurs - randomly pick a time during the flight
                // this could be more sophisticated, but should work for our purposes
                return fault_dist(rng) * flight_time_hours;
            }
            return -1.0;
        }

        void discharge_battery() override
        {
            battery_level_ = 0.0;
        }

        void charge_battery() override
        {
            battery_level_ = 1.0;
        }

        double get_battery_level() const override
        {
            return battery_level_;
        }

        int get_id() const override
        {
            return aircraft_id_;
        }

        AircraftType get_type() const override
        {
            return static_cast<const Derived *>(this)->get_aircraft_type();
        }

        std::string get_manufacturer() const override
        {
            return get_spec().manufacturer;
        }

        int get_passenger_count() const override
        {
            return get_spec().passenger_count;
        }

        double get_charge_time_hours() const override
        {
            return get_spec().time_to_charge_hours;
        }

        bool is_faulty() const override
        {
            return is_faulty_;
        }

        void set_faulty(bool faulty) override
        {
            is_faulty_ = faulty;
        }

    protected:
        virtual double energy_consumption_per_mile() const = 0;

    private:
        int aircraft_id_;
        double battery_level_;
        bool is_faulty_;
    };
}