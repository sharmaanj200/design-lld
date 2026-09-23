#include <iostream>
#include <map>
#include <optional>
#include <utility>
#include <vector>
#include <chrono>

/*
we will assume there will be 5 levels and 90 parking spots at each with 30 assigned to each of the three vehicle type
*/


/*

1. a vehicle comes in -> check if a parking slot is available -----yes---> assign it and assign time. 
2. a vehicle leaves --> deregister the parking slot. ----> give the price 

*/

constexpr int levels = 5;
constexpr int spots = 30;

enum class VehicleType {
    Bike, Car, Truck
};

enum class CostType {
    Bike = 10, Car = 20, Truck = 30
};

class Vehicle;
class Ticket;

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

    std::optional<std::pair<int, int>> determineSpot()
    {
        for(int i=0; i<levels; i++)
        {
            for(int j=0; j<spots; j++)
            {
                if(!p_availableParkingSpots[i][j]) return std::pair<int, int>{i, j};
            }
        }
        return std::nullopt;
    }

    std::pair<int, int> assignSpot()
    {
        auto spot = determineSpot();
        if(spot.has_value()) {
            auto [l, s] = *spot;
            p_availableParkingSpots[l][s] = 1;
            return {l, s};
        }
        return {};
    }

    void deAssignSpot(int l, int s)
    {
        p_availableParkingSpots[l][s] = 0;
    }
};



class Vehicle {
    std::chrono::time_point <std::chrono::steady_clock> v_start, v_end;
    std::chrono::duration <double> v_duration;
    ParkingSpot* v_parkingspot;
    VehicleType v_type;
    int v_id;
    
public:
    Vehicle(VehicleType v, ParkingSpot* p) : v_type(v), v_parkingspot(p) {};
    
    int get_vehiclID() const {
        return v_id;
    }

    VehicleType getVehicleType() const
    {
        return v_type;
    }

    std::optional<std::pair<int, int>> register_vehicle()
    {
        v_start = std::chrono::high_resolution_clock::now();
        if(!v_parkingspot->determineSpot().has_value()) {
            return std::nullopt;
        } 
        else {
            return v_parkingspot->assignSpot();
        }
    }

    std::pair<int, int> deregister_vehicle(int l, int s)
    {
        // to determine the end time
        v_end = std::chrono::high_resolution_clock::now();
        v_duration = v_end - v_start;
        int time_taken = std::chrono::duration_cast<std::chrono::seconds>(v_duration).count();
        int hrs = time_taken / 3660;
        int mins = (time_taken % 3660) / 60;


        // to deregister and empty the parking slot
        v_parkingspot->deAssignSpot(l, s);
        return {hrs, mins};
    }

};


class Ticket {
    Vehicle* t_vehicle;
    int levelAssigned, spotAssigned;

public:
     Ticket(Vehicle* v): t_vehicle(v), levelAssigned(-1), spotAssigned(-1) {};
    

     void setLevelandSpot()
     {
        auto parkingData = t_vehicle->register_vehicle();
        if(parkingData.has_value()) {
            auto [level, spot] = *parkingData;
            this->levelAssigned = level;
            this->spotAssigned = spot;
        }
        else {
            std::cout << "No parking available for this vehicle type" << std::endl;
        }
     }

     int getCost()
     {
        VehicleType v = t_vehicle->getVehicleType();
        int cost = 0;
        if(v == VehicleType::Bike) cost = static_cast<int>(CostType::Bike);
        else if(v == VehicleType::Car) cost = static_cast<int>(CostType::Car);
        else cost = static_cast<int>(CostType::Truck);

        auto [hrs, min] = t_vehicle->deregister_vehicle(this->levelAssigned, this->spotAssigned);
        if(min > 30) hrs += 1;

        return (hrs * cost);
     }

};

// class ParkingLot {
    


// };


#include <thread>

int main()
{
    // ---------------------------------------
    // Create parking areas
    // ---------------------------------------
    ParkingSpot bikeParking(nullptr);
    ParkingSpot carParking(nullptr);
    ParkingSpot truckParking(nullptr);

    // ---------------------------------------
    // Create vehicles
    // ---------------------------------------
    Vehicle bike1(VehicleType::Bike, &bikeParking);
    Vehicle bike2(VehicleType::Bike, &bikeParking);

    Vehicle car1(VehicleType::Car, &carParking);
    Vehicle car2(VehicleType::Car, &carParking);

    Vehicle truck1(VehicleType::Truck, &truckParking);

    // ---------------------------------------
    // Create tickets
    // ---------------------------------------
    Ticket bikeTicket1(&bike1);
    Ticket bikeTicket2(&bike2);

    Ticket carTicket1(&car1);
    Ticket carTicket2(&car2);

    Ticket truckTicket1(&truck1);


    // =======================================
    // VEHICLES ENTER
    // =======================================

    std::cout << "\n===== VEHICLES ENTERING =====\n\n";

    std::cout << "Bike 1 entering...\n";
    bikeTicket1.setLevelandSpot();

    std::cout << "Bike 2 entering...\n";
    bikeTicket2.setLevelandSpot();

    std::cout << "Car 1 entering...\n";
    carTicket1.setLevelandSpot();

    std::cout << "Car 2 entering...\n";
    carTicket2.setLevelandSpot();

    std::cout << "Truck 1 entering...\n";
    truckTicket1.setLevelandSpot();


    // =======================================
    // SHOW CURRENT PARKING
    // =======================================

    std::cout << "\n===== CURRENT PARKING =====\n";

    std::cout << "Bike 1 -> Level/Spot assigned\n";
    std::cout << "Bike 2 -> Level/Spot assigned\n";
    std::cout << "Car 1  -> Level/Spot assigned\n";
    std::cout << "Car 2  -> Level/Spot assigned\n";
    std::cout << "Truck 1 -> Level/Spot assigned\n";


    // =======================================
    // SIMULATE PARKING TIME
    // =======================================

    std::cout << "\nVehicles are parked...\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));


    // =======================================
    // VEHICLE LEAVES
    // =======================================

    std::cout << "\n===== VEHICLES LEAVING =====\n\n";

    std::cout << "Bike 1 leaving...\n";
    int bikeCost = bikeTicket1.getCost();
    std::cout << "Bike 1 cost = " << bikeCost << "\n";

    std::cout << "\nCar 1 leaving...\n";
    int carCost = carTicket1.getCost();
    std::cout << "Car 1 cost = " << carCost << "\n";


    // =======================================
    // TEST WHETHER SPOT WAS FREED
    // =======================================

    std::cout << "\n===== TESTING FREED SPOT =====\n\n";

    Vehicle bike3(VehicleType::Bike, &bikeParking);
    Ticket bikeTicket3(&bike3);

    std::cout << "Bike 3 entering after Bike 1 left...\n";
    bikeTicket3.setLevelandSpot();


    // =======================================
    // TEST FULL PARKING
    // =======================================

    std::cout << "\n===== TESTING FULL PARKING =====\n\n";

    std::vector<Vehicle*> testCars;
    std::vector<Ticket*> testTickets;

    // Fill all remaining car spots
    for (int i = 0; i < levels * spots; i++)
    {
        Vehicle* car = new Vehicle(VehicleType::Car, &carParking);
        Ticket* ticket = new Ticket(car);

        std::cout << "Additional car " << i + 1 << " entering...\n";

        ticket->setLevelandSpot();

        testCars.push_back(car);
        testTickets.push_back(ticket);
    }


    // =======================================
    // TRY ONE MORE CAR
    // =======================================

    std::cout << "\nTrying to park one more car...\n";

    Vehicle extraCar(VehicleType::Car, &carParking);
    Ticket extraCarTicket(&extraCar);

    extraCarTicket.setLevelandSpot();


    // =======================================
    // CLEANUP
    // =======================================

    for (auto ticket : testTickets)
        delete ticket;

    for (auto car : testCars)
        delete car;

    return 0;
}