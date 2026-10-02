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
1. The parking lot should have multiple levels, each level with a certain number of parking spots.
2. The parking lot should support different types of vehicles, such as cars, motorcycles, and trucks.
3. Each parking spot should be able to accommodate a specific type of vehicle.
4. The system should assign a parking spot to a vehicle upon entry and release it when the vehicle exits.
5. The system should track the availability of parking spots and provide real-time information to customers.
6. The system should handle multiple entry and exit points and support concurrent access.
*/





/*

Vehicle
- id
- vtype

Ticket
- id
- vehicle
- arrivalTime

ParkingLevel
- level
- list<spots>
- availableSpots

IParkingStrategy (interface)
- void getSpot(vehicle)

ICostStrategy (interface)
- void getCost(ticket, departTime)


ParkingLot
- levels
- parkingStrategy
- costStrategy
+ park(vehicle): Ticket
+ exit(ticket, departTime): Money

*/

//use singleton
class IdGenerator {
    static int nextId;

public:
    static int generate() {
        return nextId++;
    }
};

int IdGenerator::nextId = 1;

using dt = std::chrono::time_point <std::chrono::steady_clock>;
constexpr int levels = 5;
constexpr int spots = 30;

enum class VehicleType {
    Bike, Car, Truck
};

enum class CostType {
    Bike = 10, Car = 20, Truck = 30
};


class Vehicle {
public:
    std::string v_id;
    VehicleType v_type;
};

class ParkingSpotStrategy {
public:
    virtual int getNumberofSpotsPerVehicle() = 0;
    virtual VehicleType getType() = 0;
    virtual ~ParkingSpotStrategy() = default;
};

class BikeParking : public ParkingSpotStrategy {
public:
    int getNumberofSpotsPerVehicle() override
    {
        return spots / 3;
    }

    VehicleType getType() override {
        return VehicleType::Bike;
    }
};

class CarParking : public ParkingSpotStrategy {
public:
    int getNumberofSpotsPerVehicle() override
    {
        return spots / 3;
    }

    VehicleType getType() override {
        return VehicleType::Car;
    }
};

class TruckParking : public ParkingSpotStrategy {
public:
    int getNumberofSpotsPerVehicle() override
    {
        return spots / 3;
    }

    VehicleType getType() override {
        return VehicleType::Truck;
    }
};


class ParkingSpot {
public:
    VehicleType vehicle;
    int level;
    bool occupied;

    ParkingSpot(int l, bool o, VehicleType v): level(l), occupied(o), vehicle(v) {};

};


class ParkingLevel {
public:
    int level;
    int availableSpots;
    std::vector<ParkingSpot> avlspots;

    ParkingLevel(int l) : level(l) {
        std::unique_ptr<ParkingSpotStrategy> parking;

        parking = std::make_unique<BikeParking>();
        int n_bike = parking->getNumberofSpotsPerVehicle();
        for(int i=0; i<n_bike; i++)
        {
            avlspots.emplace_back(ParkingSpot(level, false, VehicleType::Bike));
        }

        parking = std::make_unique<CarParking>();
        int n_car = parking->getNumberofSpotsPerVehicle();
        for(int i=0; i<n_car; i++)
        {
            avlspots.emplace_back(ParkingSpot(level, false, VehicleType::Car));
        }

        parking = std::make_unique<TruckParking>();
        int n_truck = parking->getNumberofSpotsPerVehicle();
        for(int i=0; i<n_truck; i++)
        {
            avlspots.emplace_back(ParkingSpot(level, false, VehicleType::Truck));
        }

        availableSpots = n_bike + n_car + n_truck;
    };
};


class Ticket {
public:
    std::string t_id;
    std::string vehicle_id;
    VehicleType t_type;
    int level;
    int spot;
    dt t_start;
};


class ParkingStrategy {
public:
    virtual int fillSpot(VehicleType vehicle ,ParkingLevel& p) = 0;
    virtual ~ParkingStrategy() = default;
};


class FCFS : public ParkingStrategy {
public:
    int fillSpot(VehicleType vehicle, ParkingLevel& p) override {

        if (p.availableSpots == 0)
            return -1;

        for (int i = 0; i < p.avlspots.size(); i++)
        {
            ParkingSpot& spot = p.avlspots[i];

            if (!spot.occupied && spot.vehicle == vehicle)
            {
                spot.occupied = true;
                spot.level = p.level;
                p.availableSpots--;

                return i;
            }
        }

        return -1;
    }
};


class CostStrategy {
public:
    virtual int getCost(int hrs) = 0;
    virtual ~CostStrategy() = default;
};


class BikeCost : public CostStrategy {
    int cost = 0;
public:
    int getCost(int hrs) override {
        return static_cast<int>(CostType::Bike) * hrs;
    }
};


class CarCost : public CostStrategy {
    int cost = 0;
public:
    int getCost(int hrs) override {
        return static_cast<int>(CostType::Car) * hrs;
    }
};


class TruckCost : public CostStrategy {
    int cost = 0;
public:
    int getCost(int hrs) override {
        return static_cast<int>(CostType::Truck) * hrs;
    }
};


struct location {
    int level;
    int spot;
};

//singleton
//manages all the levels parking capacity together and assigns and deassigns spots.
class LevelManager {
    std::vector<ParkingLevel> p_level;
    std::unique_ptr<ParkingStrategy> p_strategy;
public:
    LevelManager() {
        for(int i=0; i<levels; i++)
        {
            p_level.emplace_back(ParkingLevel(i));
        }
        p_strategy = std::make_unique<FCFS>();
    }

    location park_vehicle(VehicleType v)
    {
        for(int i=0; i<p_level.size(); i++)
        {
            int s = p_strategy->fillSpot(v, p_level[i]);
            if(s != -1) return {i, s};  //level, spot
        }

        return {-1, -1};
    }


    void unpark_vehicle(const Ticket& t)
    {
        int l = t.level;
        int s = t.spot;
        ParkingLevel& level = p_level[l];

        ParkingSpot& spot = level.avlspots[s];
        if (!spot.occupied)
            throw std::runtime_error("Spot already vacant");
            
        level.availableSpots += 1;
        spot.occupied = false;
    }

};


class ParkingLotManager {

    std::unordered_map<std::string, Ticket> activeTickets;
    LevelManager l;

public:
    ParkingLotManager() {

    };


    Ticket park(const Vehicle& vehicle)
    {
        location vehicle_loc = l.park_vehicle(vehicle.v_type);
        if(vehicle_loc.level == -1) throw std::runtime_error("Parking lot is filled. try after some time");
        
        Ticket ticket;

        ticket.vehicle_id = vehicle.v_id;
        ticket.t_id = std::to_string(IdGenerator::generate());
        ticket.t_type = vehicle.v_type;
        ticket.t_start = std::chrono::steady_clock::now();
        ticket.level = vehicle_loc.level;
        ticket.spot = vehicle_loc.spot;

        activeTickets[ticket.t_id] = ticket;
        return ticket;
    }

    double calculateCost(const Ticket& ticket, const dt& e_time) 
    {
        //time calculation
        std::chrono::duration <double> v_duration = e_time - ticket.t_start;
        int time_taken = std::chrono::duration_cast<std::chrono::seconds>(v_duration).count();
        int hrs = time_taken / 3600;
        int mins = (time_taken % 3600) / 60;
        if(mins > 30) hrs += 1;

        std::unique_ptr<CostStrategy> cs;
        //cost calculation
        if(ticket.t_type == VehicleType::Bike) {
            cs = std::make_unique<BikeCost>();
        }

        if(ticket.t_type == VehicleType::Car) {
            cs = std::make_unique<CarCost>();
        }

        if(ticket.t_type == VehicleType::Truck) {
            cs = std::make_unique<TruckCost>();
        }
        return cs->getCost(hrs);
    }

    double unpark(const std::string& ticketID)
    {
        auto it = activeTickets.find(ticketID);
        if(it == activeTickets.end()) {
            throw std::runtime_error("Invalid ticket");
        }

        Ticket& ticket = it->second;
        l.unpark_vehicle(ticket);
        dt t_end = std::chrono::steady_clock::now();
        double cost = calculateCost(ticket, t_end);
        activeTickets.erase(it);
        return cost;
    }

};















// class ParkingSpot{
//     using levelMap = std::map<int, std::vector<bool>>;
//     levelMap p_availableParkingSpots;
//     Vehicle* p_vehicle;

// public:
//     ParkingSpot(Vehicle* v) : p_vehicle(v) {
//         for(int i=0; i<levels; i++)
//         {
//             p_availableParkingSpots[i].resize(spots, 0);
//         }
//     };

//     std::optional<std::pair<int, int>> determineSpot()
//     {
//         for(int i=0; i<levels; i++)
//         {
//             for(int j=0; j<spots; j++)
//             {
//                 if(!p_availableParkingSpots[i][j]) return std::pair<int, int>{i, j};
//             }
//         }
//         return std::nullopt;
//     }

//     std::pair<int, int> assignSpot()
//     {
//         auto spot = determineSpot();
//         if(spot.has_value()) {
//             auto [l, s] = *spot;
//             p_availableParkingSpots[l][s] = 1;
//             return {l, s};
//         }
//         return {};
//     }

//     void deAssignSpot(int l, int s)
//     {
//         p_availableParkingSpots[l][s] = 0;
//     }
// };



// class Vehicle {
//     std::chrono::time_point <std::chrono::steady_clock> v_start, v_end;  //out of this class  
//     ParkingSpot* v_parkingspot;
//     VehicleType v_type;
//     int v_id;
    
// public:
//     Vehicle(VehicleType v, ParkingSpot* p) : v_type(v), v_parkingspot(p) {};
    
//     int get_vehiclID() const {
//         return v_id;
//     }

//     VehicleType getVehicleType() const
//     {
//         return v_type;
//     }

//     std::optional<std::pair<int, int>> register_vehicle()
//     {
//         v_start = std::chrono::high_resolution_clock::now();
//         if(!v_parkingspot->determineSpot().has_value()) {
//             return std::nullopt;
//         } 
//         else {
//             return v_parkingspot->assignSpot();
//         }
//     }

//     std::pair<int, int> deregister_vehicle(int l, int s)
//     {
//         // to determine the end time
//         v_end = std::chrono::high_resolution_clock::now();
//         std::chrono::duration <double> v_duration = v_end - v_start;
//         int time_taken = std::chrono::duration_cast<std::chrono::seconds>(v_duration).count();
//         int hrs = time_taken / 3600;
//         int mins = (time_taken % 3600) / 60;   //can be done using stl


//         // to deregister and empty the parking slot
//         v_parkingspot->deAssignSpot(l, s);
//         return {hrs, mins};
//     }

// };


// class Ticket {
//     Vehicle* t_vehicle;
//     int levelAssigned, spotAssigned;

// public:
//      Ticket(Vehicle* v): t_vehicle(v), levelAssigned(-1), spotAssigned(-1) {};
    

//      void setLevelandSpot()
//      {
//         auto parkingData = t_vehicle->register_vehicle();
//         if(parkingData.has_value()) {
//             auto [level, spot] = *parkingData;
//             this->levelAssigned = level;
//             this->spotAssigned = spot;
//         }
//         else {
//             std::cout << "No parking available for this vehicle type" << std::endl;
//         }
//      }

//      int getCost()
//      {
//         VehicleType v = t_vehicle->getVehicleType();
//         int cost = 0;
//         if(v == VehicleType::Bike) cost = static_cast<int>(CostType::Bike);
//         else if(v == VehicleType::Car) cost = static_cast<int>(CostType::Car);
//         else cost = static_cast<int>(CostType::Truck);

//         auto [hrs, min] = t_vehicle->deregister_vehicle(this->levelAssigned, this->spotAssigned);
//         if(min > 30) hrs += 1;

//         return (hrs * cost);
//      }

// };

// class ParkingLot {
    


// };
