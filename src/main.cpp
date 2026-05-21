#include "Simulation.h"

int main(int argc, char* argv[]) {
    SimulationParams params(argc, argv);
    Simulation::Start(params);
    
    return 0;
}