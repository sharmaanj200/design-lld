#include <iostream>
#include <map>

/*
we will assume there will be 5 levels and 90 parking spots at each with 30 assigned to each of the three vehicle type
*/


constexpr int levels = 5;
constexpr int spots = 30;

enum VehicleType {
    Bike, Car, Truck
};

enum CostType {
    C_BIKE = 10, C_Car = 20, C_Truck = 30
};


class Vehicle {
    VehicleType v_type;
    int v_id;

public:
    Vehicle(VehicleType v) : v_type(v) {};
    
    int get_vehiclID() const {
        return v_id;
    }

};


class ParkingSpot{
    using levelMap = std::map<int, std::vector<bool>>;
    levelMap p_availableParkingSpots;
    Vehicle* p_vehicle;

public:
    ParkingSpot(Vehicle* v) : p_vehicle(v) {
        for(int i=0; i<levels; i++)
        {
            p_availableParkingSpots[i].resize(spots, 0);
        }
    };

    std::pair<int, int> determineSpot()
    {
        for(int i=0; i<levels; i++)
        {
            for(int j=0; j<spots; j++)
            {
                if(!p_availableParkingSpots[i][j]) return {i, j};
            }
        }
        return {};
    }

    std::pair<int, int> assignSpot() 
    {
        auto [l, s] = determineSpot();
        p_availableParkingSpots[l][s] = 1;
        return {l, s};
    }
};


class Ticket {
    std::chrono::time_point <std::chrono::steady_clock> t_start, t_end;
    std::chrono::duration <double> t_duration;
    Vehicle t_vehicle;
    ParkingSpot t_parkingspot;

public:
    void register_vehicle()
    {
        t_start = std::chrono::high_resolution_clock::now();
    }

    std::pair<int, int> deregister_vehicle()
    {
        t_end = std::chrono::high_resolution_clock::now();
        t_duration = t_end - t_start;
        int time_taken = std::chrono::duration_cast<std::chrono::seconds>(t_duration).count();
        int hrs = time_taken / 3660;
        int mins = (time_taken % 3660) / 60;
        return {hrs, mins};
    }
};



class ParkingLot {
    Vehicle* m_vehicle;

};


int main()
{

}